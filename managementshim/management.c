// See LICENSE file for the license details of MIT
// Copyright 2020 Marno van der Maas

#include "management.h"
#include "instructions.h"
#include "praesidiooutput.h"
#include "mailbox.h"

#define PAGE_SIZE (1 << PAGE_BIT_SHIFT) //2^12 = 4096 number of Bytes

struct ManagementState_t state;

enum boolean initialization_done = BOOL_FALSE;

struct EnclaveData_t * getEnclaveDataPointer(enclave_id_t id) {
    //TODO add support for more enclaves than fit on one page.
    struct EnclaveData_t *enclaveData = (struct EnclaveData_t *) ENCLAVE_DATA_BASE_ADDRESS;
    int i;
    for(i = 0; ; i++) {
      if(i >= 2*PAGE_SIZE / sizeof(struct EnclaveData_t)) {
        return 0;
      }
      if(enclaveData[i].eID == id) {
        return &enclaveData[i];
      }
    }
}

void flushRemappingTable() {
  //TODO flush all entries in this core's remapping table
}

void flushL1Cache() {
  //TODO flush all entries in this core's L1 cache
}

void fillPage(Address_t baseAddress, long value) {
  long *basePointer = (long *) baseAddress;
  for(int i = 0; i < PAGE_SIZE/8; i++) {
    basePointer[i] = value;
  }
}

//Returns char because then the load does not get optimized away.
char putPageEntry(Address_t baseAddress, enclave_id_t id) {
  //This load is to simulate the latency that would be caused to get this tag into the cache.
  char *basePointer = (char *) baseAddress;
  //volatile char x = *basePointer; //This is not allowed if management enclave does not own the page.
  volatile char x = *((char *)(PAGE_DIRECTORY_BASE_ADDRESS + (baseAddress % PAGE_SIZE)));
  SET_ARGUMENT_ENCLAVE_IDENTIFIER(id);
  //Set the page to the specified enclave identifier.
  asm volatile (
    "csrrw zero, 0x40F, %0"
    :
    : "r"(basePointer)
    :
  );
  return x;
}

void clearWorkingMemory() {
  //This is not necessary in Spike, but will be necessary when implementing in actual hardware to avoid cold boot attacks.
}

int createEnclave() {
  //TODO add support for more enclaves than fit on one page.
  struct EnclaveData_t *enclaveData = (struct EnclaveData_t *) ENCLAVE_DATA_BASE_ADDRESS;
  int i;
  for(i = 0; ; i++) {
    if(i >= 2*PAGE_SIZE / sizeof(struct EnclaveData_t)) {
      return -1;
    }
    if(enclaveData[i].eID == ENCLAVE_INVALID_ID) {
      break;
    }
  }
#ifdef PRAESIDIO_DEBUG
  output_string("creating enclave in slot: ");
  OUTPUT_CHAR(i + '0');
  output_string(" with ID: ");
  OUTPUT_CHAR(state.nextEnclaveID + '0');
  OUTPUT_CHAR('\n');
#endif
  enclaveData[i].eID = state.nextEnclaveID;
  state.nextEnclaveID += 1;
  enclaveData[i].state = STATE_CREATED;
  return i;
}

enum boolean donatePage(enclave_id_t recipient, Address_t page_base) {
  struct EnclaveData_t * thisData = getEnclaveDataPointer(recipient);
  if(page_base & (PAGE_SIZE-1)) {
#ifdef PRAESIDIO_DEBUG
    output_string("Donate Page: address not at base of page ERROR!\n");
#endif
    return BOOL_FALSE;
  }
  switch(thisData->state) {
    case STATE_CREATED:
      thisData->codeEntryPoint = page_base; //This assumes the first page that is given to an enclave is also the code entry point.
      thisData->state = STATE_BUILDING;
      putPageEntry(page_base, recipient);
      break;
    case STATE_BUILDING:
      putPageEntry(page_base, recipient);
      break;
    default:
#ifdef PRAESIDIO_DEBUG
      output_string("Donate Page: ERROR, enclave cannot receive pages in its current state!\n");
#endif
      return BOOL_FALSE;
  }
  return BOOL_TRUE;
}

enum boolean finalizeEnclave(enclave_id_t eID) {
  struct EnclaveData_t *enclaveData = getEnclaveDataPointer(eID);
  if(enclaveData != 0 && enclaveData->state == STATE_BUILDING) {
    //TODO calculate the enclave measurement
    enclaveData->state = STATE_LIVE;
    return BOOL_TRUE;
  }
#ifdef PRAESIDIO_DEBUG
  else {
    if(enclaveData) {
      output_string("Finalize enclave failed because enclave doesn't exist.\n");
    } else {
      output_string("Finalize enclave failed because enclave is not in building state ");
      output_hexbyte(enclaveData->state);
      OUTPUT_CHAR('\n');
    }
  }
#endif
  return BOOL_FALSE;
}

enum boolean switchEnclave(CoreID_t coreID, enclave_id_t eID) {
  //TODO make this work for actually switching enclaves and not just starting one up.
  struct Message_t command;
  struct EnclaveData_t *enclaveData = getEnclaveDataPointer(eID);
  if(enclaveData != 0 && enclaveData->state == STATE_LIVE) {
      command.source = ENCLAVE_MANAGEMENT_ID;
      command.destination = ENCLAVE_MANAGEMENT_ID - coreID;
      command.type = MSG_RUN;
      command.arguments[0] = eID;
      sendMessage(&command);
      do {
        receiveMessage(&command);
      } while(command.source != ENCLAVE_MANAGEMENT_ID - coreID);
      return BOOL_TRUE;
  }
#ifdef PRAESIDIO_DEBUG
  else {
      output_string("switch enclave: error enclave doesn't exist or it is in wrong state.\n");
  }
#endif
  return BOOL_FALSE;
}

Address_t waitForEnclave() {
  Address_t entryPoint = 0;
  struct Message_t message;
  struct EnclaveData_t *enclaveData;
  enclave_id_t tmpID;
  while(1) {
    receiveMessage(&message);
    if(message.source == ENCLAVE_MANAGEMENT_ID) {
      tmpID = message.source;
      message.source = message.destination;
      message.destination = tmpID;
      sendMessage(&message);
      enclaveData = getEnclaveDataPointer(message.arguments[0]);
      if(enclaveData == 0) {
#ifdef PRAESIDIO_DEBUG
        output_string("waitForEnclave: ERROR enclave entry point not found!\n");
#endif
        return 0;
      }

      SWITCH_ENCLAVE_ID(message.arguments[0]);
      entryPoint = enclaveData->codeEntryPoint;
      enclaveData->state = STATE_LIVE;
      break;
    }
  }
  return entryPoint;
}

//Returns whether enclave is scheduled on this core.
enum boolean managementRoutine(const CoreID_t managementCore) {
  enum boolean retVal = BOOL_FALSE;
  struct Context_t savedContext;
  int index;
  enclave_id_t savedEnclaveID = getCurrentEnclaveID();
  static enclave_id_t internalArgument = ENCLAVE_INVALID_ID;
  CoreID_t chosenCore = managementCore;

  struct Message_t message;
  receiveMessage(&message);

  struct Message_t response;
  response.source = message.destination;
  response.destination = message.source;
  response.type = message.type;
  response.arguments[0] = 0;
  response.arguments[1] = 0;

  if(message.source == ENCLAVE_DEFAULT_ID &&
      message.destination == ENCLAVE_MANAGEMENT_ID) {
    switch(message.type) {
      case MSG_CREATE:
#ifdef PRAESIDIO_DEBUG
        output_string("Received create enclave message.\n");
#endif
        index = createEnclave();
        if(index >= 0) {
          struct EnclaveData_t *enclaveData = (struct EnclaveData_t *) ENCLAVE_DATA_BASE_ADDRESS;
          response.arguments[0] = enclaveData[index].eID;
        } else {
          response.arguments[0] = ENCLAVE_INVALID_ID;
        }
        break;
      case MSG_DONATE_PAGE:
#ifdef PRAESIDIO_DEBUG
        output_string("Received donate page enclave message.\n");
#endif
        donatePage(message.arguments[0], message.arguments[1]);
        break;
      case MSG_FINALIZE:
#ifdef PRAESIDIO_DEBUG
        output_string("Received finalize enclave message.\n");
#endif
        finalizeEnclave(message.arguments[0]);
        break;
      case MSG_RUN:
#ifdef PRAESIDIO_DEBUG
        output_string("Received switch enclave message.\n");
#endif
        //TODO make this work with multiple enclave cores.
        switchEnclave(chosenCore, message.arguments[0]); //TODO actually keep track of which cores are available.
        if(chosenCore == managementCore) {
          retVal = BOOL_TRUE;
        }
        break;
      case MSG_ATTEST:
#ifdef PRAESIDIO_DEBUG
        output_string("Received attest enclave message.\n");
#endif
        //TODO implement attestation using the measurements of the management shim and the enclave.
        break;
      case MSG_DELETE:
#ifdef PRAESIDIO_DEBUG
        output_string("Received delete enclave message.\n");
#endif
        //TODO implement delete enclave when management shim interupts enclaves
        break;
#ifdef PRAESIDIO_DEBUG
      default:
        output_string("Received message with unknown type.\n");
        break;
#endif
    }
    sendMessage(&response);
  }
  return retVal;
}

Address_t initialize() {
  CoreID_t coreID = getCoreID();
  enclave_id_t oldEnclave = getCurrentEnclaveID();
  SWITCH_ENCLAVE_ID(ENCLAVE_MANAGEMENT_ID - coreID);
  if(oldEnclave != ENCLAVE_INVALID_ID) {
      struct EnclaveData_t *enclaveData = getEnclaveDataPointer(oldEnclave);
      if(enclaveData != 0) {
          enclaveData->state = STATE_EMPTY;
          enclaveData->eID = ENCLAVE_INVALID_ID;
      }
  }
  //TODO make a hash of management pages for attestation purposes
  flushRemappingTable();
  flushL1Cache();
  CoreID_t *enclaveCores = (CoreID_t *) 0x2000 /*ROM location of enclave 0's core ID*/;
  if(coreID == enclaveCores[1]) { //TODO fill state.enclaveCores
    SWITCH_ENCLAVE_ID(ENCLAVE_MANAGEMENT_ID);
    if(initialization_done == BOOL_FALSE) {
#ifdef PRAESIDIO_DEBUG
      output_string("management.c: In management enclave.\n");
#endif
      state.nextEnclaveID = 1;
      for(int i = 0; i < NUMBER_OF_ENCLAVE_CORES; i++) {
        state.runningEnclaves[i] = ENCLAVE_MANAGEMENT_ID;
      }
      putPageEntry(MANAGEMENT_CODE_BASE_ADDRESS, ENCLAVE_MANAGEMENT_ID);
      putPageEntry(PAGE_DIRECTORY_BASE_ADDRESS, ENCLAVE_MANAGEMENT_ID);
      putPageEntry(ENCLAVE_DATA_BASE_ADDRESS, ENCLAVE_MANAGEMENT_ID);
      fillPage(PAGE_DIRECTORY_BASE_ADDRESS, 0xFFFFFFFFFFFFFFFF);
      fillPage(ENCLAVE_DATA_BASE_ADDRESS, 0x0000000000000000); //Fill the second page as well.
      struct EnclaveData_t *enclaveDataPointer = (struct EnclaveData_t *) ENCLAVE_DATA_BASE_ADDRESS;
      for(int i = 0; i < 2*PAGE_SIZE / sizeof(struct EnclaveData_t); i++) {
#ifdef PRAESIDIO_DEBUG
        output_string("management.c: clearing enclave data\n");
#endif
        enclaveDataPointer[i].eID = ENCLAVE_INVALID_ID;
      }

      clearWorkingMemory();
      initialization_done = BOOL_TRUE; //This starts the normal world
    }
    while(1) {
      if(managementRoutine(coreID) == BOOL_TRUE) {
        break;
      }
      //wait for 1000 milliseconds
    }
  }
  //setManagementInterruptTimer(1000); //Time in milliseconds
  SWITCH_ENCLAVE_ID(ENCLAVE_MANAGEMENT_ID - coreID);
  return waitForEnclave();
}

void normalWorld() {
  while(initialization_done == BOOL_FALSE) {
    //Waiting until enclave core 0 has finished initialization
  }
}
