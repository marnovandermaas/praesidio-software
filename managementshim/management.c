typedef unsigned long uint64_t; //TODO make sure it is 64-bit
#include "management.h"
#include "instructions.h"
#include <stdio.h>
#include "../riscv/debug.h"

#define PAGE_SIZE (1 << PAGE_BIT_SHIFT) //2^12 = 4096 number of Bytes

struct ManagementState_t state;

enum boolean initialization_done = BOOL_FALSE;

//Writes a character to display.
void output_char(char c) {
  asm volatile ( //This instruction writes a value to a CSR register. This is a custom register and I have modified Spike to print whatever character is written to this CSR. This is my way of printing without requiring the support of a kernel or the RISC-V front end server.
    "csrrw zero, 0x404, %0" //CSRRW rd, csr, rs1
    : //output operands
    : "r"(c) //input operands
    : //clobbered registers
  );
}

//Calls output_char in a loop.
void output_string(char *s) {
  int i = 0;
  while(s[i] != '\0') {
    output_char(s[i]);
    i++;
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
  setArgumentEnclaveIdentifier(id);
  //Set the page to the specified enclave identifier.
  asm volatile (
    "csrrw zero, 0x40F, %0"
    :
    : "r"(basePointer)
    :
  );
  return x;
}

void resetNormalWorld() {
  //TODO somehow reset the remapping table of the normal world.
}

void clearWorkingMemory() {
  //TODO go through all pages in working memory (except for the management code)
  //This is not necessary in Spike, but will be necessary when implementing in actual hardware to avoid cold boot attacks.
}

//TODO this function should be inline
void saveCurrentContext(struct Context_t *context) {
  //TODO save current context to context argument.
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
  output_char(i + '0');
  output_string(" with ID: ");
  output_char(state.nextEnclaveID + '0');
  output_char('\n');
#endif
  enclaveData[i].eID = state.nextEnclaveID;
  state.nextEnclaveID += 1;
  enclaveData[i].state = STATE_CREATED;
  return i;
}

enum boolean donatePage(enclave_id_t recipient, Address_t page_base) {
  //TODO look for enclave data in enclave data structure.
  struct EnclaveData_t *enclaveData = (struct EnclaveData_t *) ENCLAVE_DATA_BASE_ADDRESS;
  int i;
  for(i = 0; ; i++) {
    if(i >= 2*PAGE_SIZE / sizeof(struct EnclaveData_t)) {
#ifdef PRAESIDIO_DEBUG
      output_string("Donate Page: enclaveID ERROR!\n");
#endif
      return BOOL_FALSE;
    }
    if(enclaveData[i].eID == recipient) {
      break;
    }
  }
  if(page_base & (PAGE_SIZE-1)) {
#ifdef PRAESIDIO_DEBUG
    output_string("Donate Page: address not at base of page ERROR!\n");
#endif
    return BOOL_FALSE;
  }
  switch(enclaveData[i].state) {
    case STATE_CREATED:
      enclaveData[i].codeEntryPoint = page_base; //This assumes the first page that is given to an enclave is also the code entry point.
      enclaveData[i].state = STATE_RECEIVINGPAGES;
      putPageEntry(page_base, recipient);
      break;
    case STATE_RECEIVINGPAGES:
      putPageEntry(page_base, recipient);
      break;
    case STATE_FINALIZED:
#ifdef PRAESIDIO_DEBUG
      output_string("Donate Page: enclave already running ERROR!\n");
#endif
      return BOOL_FALSE;
  }
}

void switchEnclave(CoreID_t coreID, enclave_id_t eID) {
  //TODO make this work for actually switching enclaves and not just starting one up.
  struct Message_t command;
  command.source = ENCLAVE_MANAGEMENT_ID;
  command.destination = ENCLAVE_MANAGEMENT_ID - coreID;
  command.type = MSG_SWITCH_ENCLAVE;
  command.content = eID;
  sendMessage(&command);
  do {
    receiveMessage(&command);
  } while(command.source != ENCLAVE_MANAGEMENT_ID - coreID);
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
      enclaveData = (struct EnclaveData_t *) ENCLAVE_DATA_BASE_ADDRESS; //TODO make this dependent on the received message.
      int i;
      for(i = 0; ; i++) {
        if(i >= 2*PAGE_SIZE / sizeof(struct EnclaveData_t)) {
#ifdef PRAESIDIO_DEBUG
          output_string("waitForEnclave: ERROR enclave entry point not found!\n");
#endif
          return 0;
        }
        if(enclaveData[i].eID == message.content) {
          break;
        }
      }

      switchEnclaveID(message.content);
      entryPoint = enclaveData[i].codeEntryPoint;
      break;
    }
  }
  return entryPoint;
}

enclave_id_t internalArgument = ENCLAVE_INVALID_ID;
CoreID_t nextIdleCore = 2;

void managementRoutine() {
  struct Context_t savedContext;
  int index;
  saveCurrentContext(&savedContext);
  enclave_id_t savedEnclaveID = getCurrentEnclaveID();
  //char prntString[5] = "t  \n";

  struct Message_t message;
  receiveMessage(&message);

  struct Message_t response;
  response.source = message.destination;
  response.destination = message.source;
  response.type = message.type;
  response.content = 0;

  if(message.source == ENCLAVE_DEFAULT_ID &&
      message.destination == ENCLAVE_MANAGEMENT_ID) {
    //prntString[2] = ((char) message.type) + '0';
    //output_string(prntString);
    switch(message.type) {
      case MSG_CREATE_ENCLAVE:
#ifdef PRAESIDIO_DEBUG
        output_string("Received create enclave message.\n");
#endif
        index = createEnclave();
        if(index >= 0) {
          struct EnclaveData_t *enclaveData = (struct EnclaveData_t *) ENCLAVE_DATA_BASE_ADDRESS;
          response.content = enclaveData[index].eID;
        } else {
          response.content = ENCLAVE_INVALID_ID;
        }
        break;
      case MSG_DELETE_ENCLAVE:
#ifdef PRAESIDIO_DEBUG
        output_string("Received delete enclave message.\n");
#endif
        break;
      case MSG_ATTEST: //Not needed yet.
        break;
      case MSG_ACQUIRE_PHYS_CAP: //Not needed yet.
        break;
      case MSG_DONATE_PAGE:
#ifdef PRAESIDIO_DEBUG
        output_string("Received donate page enclave message.\n");
#endif
        donatePage(internalArgument, message.content);
        break;
      case MSG_SWITCH_ENCLAVE:
#ifdef PRAESIDIO_DEBUG
        output_string("Received switch enclave message.\n");
#endif
        switchEnclave(nextIdleCore++, message.content);
        break;
      case MSG_SET_ARGUMENT: //TODO include this in the donate page message.
#ifdef PRAESIDIO_DEBUG
        output_string("Received set argument message.\n");
        output_string("internalArgument set to: ");
        output_char(message.content + '0');
        output_char('\n');
#endif
        internalArgument = message.content;
        break;
    }
    sendMessage(&response);
  }
  //TODO inter enclave messages
}

Address_t initialize() {
  CoreID_t coreID = getCoreID();
  switchEnclaveID(ENCLAVE_MANAGEMENT_ID - coreID);
  flushRemappingTable();
  flushL1Cache();
  CoreID_t *enclaveCores = (CoreID_t *) 0x2000 /*ROM location of enclave 0's core ID*/;
  if(coreID == enclaveCores[1]) { //TODO fill state.enclaveCores
    switchEnclaveID(ENCLAVE_MANAGEMENT_ID);
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

    resetNormalWorld();
    clearWorkingMemory();
    initialization_done = BOOL_TRUE; //This starts the normal world
    while(1) {
      managementRoutine();
      //wait for 1000 milliseconds
    }
  }
  //setManagementInterruptTimer(1000); //Time in milliseconds
  return waitForEnclave();
}

void normalWorld() {
  while(initialization_done == BOOL_FALSE) {
    //Waiting until enclave core 0 has finished initialization
  }
}
