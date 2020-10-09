// See LICENSE file for the license details of MIT
// Copyright 2020 Marno van der Maas

#include "management.h"
#include "instructions.h"
#include "praesidiooutput.h"
#include "mailbox.h"

#define PAGE_SIZE (1 << PAGE_BIT_SHIFT) //2^12 = 4096 number of Bytes

struct ManagementState_t state;

enum boolean initialization_done = BOOL_FALSE;

//Defines taken out of encoding.h and clint.cc from Spike
#define CLINT_BASE    (0x02000000)
#define MTIMECMP_BASE (CLINT_BASE + 0x4000)
#define MTIME_BASE    (CLINT_BASE + 0xbff8)
#define IRQ_M_TIMER   (7)
#define MSTATUS_MIE   (0x00000008)
#define CAUSE_SUPERVISOR_ECALL (0x9)

void resetManagementInterruptTimer() {
  volatile unsigned long long currentTime;
  CoreID_t coreID = getCoreID();
  currentTime = *((volatile unsigned long long *) MTIME_BASE);
  currentTime += 100000;
  *((volatile unsigned long long *) (MTIMECMP_BASE + (8*coreID))) = currentTime;
}

void initManagementInterruptTimer() {
  int setMTIE = (1 << IRQ_M_TIMER);
  int setMIE = MSTATUS_MIE;

  resetManagementInterruptTimer();

  asm volatile(
    "csrs mie, %0"
    : //output
    : "r"(setMTIE)//input
    : //clobbered
  );

  asm volatile(
    "csrs mstatus, %0"
    : //output
    : "r"(setMIE)//input
    : //clobbered
  );
}

struct EnclaveData_t * getEnclaveDataPointer(enclave_id_t id) {
    //TODO add support for more enclaves than fit on one page.
    struct EnclaveData_t *enclaveData = (struct EnclaveData_t *) ENCLAVE_DATA_BASE_ADDRESS;
    int i;
    if(id == ENCLAVE_INVALID_ID || id == ENCLAVE_MANAGEMENT_ID || id == ENCLAVE_DEFAULT_ID) {
      return 0;
    }
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
  volatile char x;
  Address_t pageNumber = (baseAddress - DRAM_BASE) / PAGE_SIZE;
  volatile struct page_tag_t *pageTag;
  if(baseAddress < DRAM_BASE) {
    //At the moment we do not support tagging pages outside of DRAM.
    return '0';
  }
  x = *((volatile char *)(PAGE_DIRECTORY_BASE_ADDRESS + (baseAddress % PAGE_SIZE)));
  pageTag = (volatile struct page_tag_t *) (TAGDIRECTORY_BASE + (pageNumber*sizeof(struct page_tag_t)));
  pageTag->owner = id;
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
  enclaveData[i].state = STATE_CREATED;
  enclaveData[i].pagesDonated = 0;
  enclaveData[i].eID = state.nextEnclaveID;
  state.nextEnclaveID += 1;
  enclaveData[i].codeEntryPoint = 0;
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
  thisData->pagesDonated += 1;
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

Address_t waitForEnclave(enclave_id_t enclaveID) {
  Address_t entryPoint = 0;
  struct Message_t message;
  struct EnclaveData_t *enclaveData;
  enclave_id_t tmpID;

#ifdef PRAESIDIO_DEBUG
  output_string("waitForEnclave\n");
#endif
  if(enclaveID == ENCLAVE_INVALID_ID) {
    while (1) {
      receiveMessage(&message);
      if(message.source == ENCLAVE_MANAGEMENT_ID) {
        tmpID = message.source;
        message.source = message.destination;
        message.destination = tmpID;
        sendMessage(&message);
        break;
      }
    }
    tmpID = message.arguments[0];
  } else {
    tmpID = enclaveID;
  }

  enclaveData = getEnclaveDataPointer(tmpID);
  if(enclaveData == 0) {
#ifdef PRAESIDIO_DEBUG
    output_string("waitForEnclave: ERROR enclave entry point not found!\n");
#endif
    return 0;
  }

  SWITCH_ENCLAVE_ID(tmpID);
  entryPoint = enclaveData->codeEntryPoint;
  enclaveData->state = STATE_LIVE;
  return entryPoint;
}

//Returns whether enclave is scheduled on this core.
enclave_id_t managementRoutine(const CoreID_t managementCore) {
  enclave_id_t retVal = ENCLAVE_INVALID_ID;
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
        if(chosenCore == managementCore) {
          retVal = message.arguments[0];
        } else {
          switchEnclave(chosenCore, message.arguments[0]);
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
  enclave_id_t tmpEnclave = ENCLAVE_INVALID_ID;
  SWITCH_ENCLAVE_ID(ENCLAVE_MANAGEMENT_ID - coreID);
  if(oldEnclave != ENCLAVE_INVALID_ID) {
      struct EnclaveData_t *enclaveData = getEnclaveDataPointer(oldEnclave);
      if(enclaveData != 0) {
          enclaveData->state = STATE_EMPTY;
          enclaveData->pagesDonated = 0;
          enclaveData->eID = ENCLAVE_INVALID_ID;
          enclaveData->codeEntryPoint = 0;
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
        enclaveDataPointer[i].state = STATE_EMPTY;
        enclaveDataPointer[i].pagesDonated = 0;
        enclaveDataPointer[i].eID = ENCLAVE_INVALID_ID;
        enclaveDataPointer[i].codeEntryPoint = 0;
      }

      clearWorkingMemory();
      initialization_done = BOOL_TRUE; //This starts the normal world
    }
    initManagementInterruptTimer();
    while(1) {
      tmpEnclave = managementRoutine(coreID);
      if(tmpEnclave != ENCLAVE_INVALID_ID) {
        SWITCH_ENCLAVE_ID(ENCLAVE_MANAGEMENT_ID - coreID);
        return waitForEnclave(tmpEnclave);
      }
      //wait for 1000 milliseconds
    }
  }
  initManagementInterruptTimer();
  return waitForEnclave(ENCLAVE_INVALID_ID);
}

void normalWorld() {
  while(initialization_done == BOOL_FALSE) {
    //Waiting until enclave core 0 has finished initialization
  }
}

//Returns 1 when an enclave is done running and 0 if it was a handled trap and needs to be returned.
int handleTrap() {
  int setMTIP = MSTATUS_MIE;
  int mipVal = 0;
  int mcauseVal = 0;
#ifdef PRAESIDIO_DEBUG
  OUTPUT_CHAR('&');
#endif
  resetManagementInterruptTimer();
  asm volatile(
    "csrc mip, %0"
    : //output
    : "r"(setMTIP)//input
    : //clobbered
  );
  asm volatile(
    "csrr %0, mip"
    : "=r"(mipVal)//output
    : //input
    : //clobbered
  );
  if(mipVal != 0) {
    output_string("management.c: cannot recover from trap.\n");
    while(1){} //TODO put enclave in error mode and return back to initialize.
  }
  asm volatile(
    "csrr %0, mcause"
    : "=r"(mcauseVal)//output
    : //input
    : //clobbered
  );
  if(mcauseVal == CAUSE_SUPERVISOR_ECALL) {
    asm volatile(
      "csrw mcause, zero"
      : //output
      : //input
      : //clobbered
    );
    return 1;
  }
  //TODO check for other causes
  return 0;
}
