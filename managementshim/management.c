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
#define CLINT_BASE      (0x02000000)
#define MTIMECMP_BASE   (CLINT_BASE + 0x4000)
#define MTIME_BASE      (CLINT_BASE + 0xbff8)
#define IRQ_M_TIMER     (7)
#define MSTATUS_MIE     (0x00000008)
#define MSTATUS_SUM     (0x00040000)
#define MSTATUS_MPP_MSB (0x00001000)
#define MSTATUS_MPP_LSB (0x00000800)

#define CAUSE_SUPERVISOR_ECALL (0x9)
#define CAUSE_FETCH_PAGE_FAULT (0xc)
#define CAUSE_LOAD_PAGE_FAULT  (0xd)
#define CAUSE_STORE_PAGE_FAULT (0xf)

void resetManagementInterruptTimer() {
  volatile unsigned long long currentTime;
  CoreID_t coreID = getCoreID();
  currentTime = *((volatile unsigned long long *) MTIME_BASE);
  currentTime += 100000;
  *((volatile unsigned long long *) (Address_t) (MTIMECMP_BASE + (8*coreID))) = currentTime;
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
  enclaveData[i].receivePages = 0;
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

enclave_id_t waitForEnclave(enclave_id_t enclaveID) {
  Address_t entryPoint = 0;
  struct Message_t message;
  struct EnclaveData_t *enclaveData;
  enclave_id_t tmpID;

#ifdef PRAESIDIO_DEBUG
  output_string("management.c: waitForEnclave\n");
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
  if(enclaveData == 0 || enclaveData->codeEntryPoint == 0 || enclaveData->eID != tmpID) {
#ifdef PRAESIDIO_DEBUG
    output_string("management.c: waitForEnclave ERROR enclave entry point not found!\n");
#endif
    return ENCLAVE_INVALID_ID;
  }

  SWITCH_ENCLAVE_ID(tmpID);
  enclaveData->state = STATE_LIVE;
  return tmpID;
}

//Returns whether enclave is scheduled on this core.
enclave_id_t managementRoutine(const CoreID_t managementCore) {
  enclave_id_t retVal = ENCLAVE_INVALID_ID;
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
        output_string("management.c: Received create enclave message.\n");
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
        output_string("management.c: Received donate page enclave message.\n");
#endif
        donatePage(message.arguments[0], message.arguments[1]);
        break;
      case MSG_FINALIZE:
#ifdef PRAESIDIO_DEBUG
        output_string("management.c: Received finalize enclave message.\n");
#endif
        finalizeEnclave(message.arguments[0]);
        break;
      case MSG_RUN:
#ifdef PRAESIDIO_DEBUG
        output_string("management.c: Received switch enclave message.\n");
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
        output_string("management.c: Received attest enclave message.\n");
#endif
        //TODO implement attestation using the measurements of the management shim and the enclave.
        break;
      case MSG_DELETE:
#ifdef PRAESIDIO_DEBUG
        output_string("management.c: Received delete enclave message.\n");
#endif
        //TODO implement delete enclave when management shim interupts enclaves
        break;
#ifdef PRAESIDIO_DEBUG
      default:
        output_string("management.c: Received message with unknown type.\n");
        break;
#endif
    }
    sendMessage(&response);
  }
  return retVal;
}

//TODO import this from the encoding.h in Spike
// page table entry (PTE) fields
#define PTE_V           (0x001) // Valid
#define PTE_R           (0x002) // Read
#define PTE_W           (0x004) // Write
#define PTE_X           (0x008) // Execute
#define PTE_U           (0x010) // User
#define PTE_G           (0x020) // Global
#define PTE_A           (0x040) // Accessed
#define PTE_D           (0x080) // Dirty
#define PTE_PPN_SHIFT   (10)
#define SATP_MODE_SV39  (8L)

enum boolean installPageTable(Address_t pageTableBase, enclave_id_t id) {
  uint64_t tmpEntry = 0;
  uint64_t *pageTablePtr = (uint64_t *) pageTableBase;
  struct EnclaveData_t *data = getEnclaveDataPointer(id);

  if(data == 0 || data->state != STATE_LIVE || data->pagesDonated == 0 || data->codeEntryPoint == 0 || data->receivePages != 0) {
    output_string("management.c: error in installing page table\n");
    return BOOL_FALSE;
  }

  if(data->pagesDonated > (1 << 9)) { //This means the second level of page table is needed
    output_string("management.c: error currently not supporting enclaves larger than 2 MiB\n");
    data->state = STATE_ERROR;
    return BOOL_FALSE;
  }

  //Fill three pages with zeroes, which will be used to set up an SV39 page table
  for(int i = 0; i < 3*PAGE_SIZE/sizeof(uint64_t); i++) {
    pageTablePtr[i] = 0;
  }

  //Set up first-level page table
  tmpEntry = PTE_V;
  tmpEntry |= ((pageTableBase + PAGE_SIZE) >> PAGE_BIT_SHIFT) << PTE_PPN_SHIFT;
  pageTablePtr[ENCLAVE_VIRTUAL_ADDRESS_BASE >> (PAGE_BIT_SHIFT + 9 + 9)] = tmpEntry; //Only mapping addresses starting from ENCLAVE_VIRTUAL_ADDRESS_BASE

  //Set up second-level page table
  tmpEntry = PTE_V;
  tmpEntry |= ((pageTableBase + 2*PAGE_SIZE) >> PAGE_BIT_SHIFT) << PTE_PPN_SHIFT;
  pageTablePtr[PAGE_SIZE/sizeof(uint64_t)] = tmpEntry; //Assuming 2^9 pages (2 MiB) should be enough memory per enclave for now

  //Set up third-level page table
  tmpEntry = PTE_W | PTE_R | PTE_V;
  tmpEntry |= (MAILBOX_BASE >> PAGE_BIT_SHIFT) << PTE_PPN_SHIFT;
  pageTablePtr[2*PAGE_SIZE/sizeof(uint64_t)] =tmpEntry; //First mapped page are the mailboxes
  if(MAILBOX_SIZE > PAGE_SIZE) {
    output_string("management.c: error not all mailboxes will be accessible to the enclave.\n");
    data->state = STATE_ERROR;
    return BOOL_FALSE;
  }
  for(uint16_t i = 0; i < data->pagesDonated; i++) { //Followed by the donated pages from the normal world
    tmpEntry = PTE_X | PTE_W | PTE_R | PTE_V;
    tmpEntry |= ((data->codeEntryPoint + i*PAGE_SIZE) >> PAGE_BIT_SHIFT) << PTE_PPN_SHIFT;
    pageTablePtr[2*PAGE_SIZE/sizeof(uint64_t) + i + 1] = tmpEntry;
  }

  //Enable paging in S-mode
  tmpEntry = ((SATP_MODE_SV39 << 60) | (pageTableBase >> PAGE_BIT_SHIFT));
  asm volatile(
    "csrw satp, %0"
    : //output
    : "r"(tmpEntry)//input
    : //clobbered
  );

  //Check that satp was written successfully
  asm volatile(
    "csrr %0, satp"
    : "=r"(tmpEntry)//output
    : //input
    : //clobbered
  );
  if(tmpEntry == 0) {
    data->state = STATE_ERROR;
    output_string("management.c: error failure enabling paging\n");
    return BOOL_FALSE;
  }

  //Set mstatus to return to s-mode and enable supervisor user memory access
  tmpEntry = MSTATUS_MPP_MSB;
  asm volatile(
    "csrc mstatus, %0"
    : //output
    : "r"(tmpEntry) //input
    : //clobbered
  );
  tmpEntry = MSTATUS_MPP_LSB | MSTATUS_SUM;
  asm volatile(
    "csrs mstatus, %0"
    : //output
    : "r"(tmpEntry) //input
    : //clobbered
  );

  //Set mepc to the start of enclave code in preparation for mret
  tmpEntry = ENCLAVE_VIRTUAL_ADDRESS_BASE + MAILBOX_SIZE;
  asm volatile(
    "csrw mepc, %0"
    : //output
    : "r"(tmpEntry) //input
    : //clobbered
  );

  return BOOL_TRUE;
}

void initialize() {
  CoreID_t coreID = getCoreID();
  enclave_id_t oldEnclave = getCurrentEnclaveID();
  enclave_id_t tmpEnclave = ENCLAVE_INVALID_ID;

  SWITCH_ENCLAVE_ID(ENCLAVE_MANAGEMENT_ID - coreID);
  if(oldEnclave != ENCLAVE_INVALID_ID) {
      struct EnclaveData_t *enclaveData = getEnclaveDataPointer(oldEnclave);
      if(enclaveData != 0) {
          enclaveData->state = STATE_EMPTY;
          enclaveData->pagesDonated = 0;
          enclaveData->receivePages = 0;
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
#ifdef PRAESIDIO_DEBUG
      output_string("management.c: clearing enclave data");
#endif
      for(int i = 0; i < 2*PAGE_SIZE / sizeof(struct EnclaveData_t); i++) {
#ifdef PRAESIDIO_DEBUG
        OUTPUT_CHAR('.');
#endif
        enclaveDataPointer[i].state = STATE_EMPTY;
        enclaveDataPointer[i].pagesDonated = 0;
        enclaveDataPointer[i].receivePages = 0;
        enclaveDataPointer[i].eID = ENCLAVE_INVALID_ID;
        enclaveDataPointer[i].codeEntryPoint = 0;
      }
#ifdef PRAESIDIO_DEBUG
      OUTPUT_CHAR('\n');
#endif

      clearWorkingMemory();
      initialization_done = BOOL_TRUE; //This starts the normal world
    }
    initManagementInterruptTimer();
    while(1) {
      tmpEnclave = managementRoutine(coreID);
      if(tmpEnclave != ENCLAVE_INVALID_ID) {
        SWITCH_ENCLAVE_ID(ENCLAVE_MANAGEMENT_ID - coreID);
        waitForEnclave(tmpEnclave);
        //TODO calculate page table base based on hart ID
        if(installPageTable(ENCLAVE_PAGE_TABLES_BASE_ADDRESS, tmpEnclave) == BOOL_TRUE) {
          return;
        } else {
          output_string("management.c: failure installing pages entering infinite loop.\n");
          while(1) {}
        }
      }
      //wait for 1000 milliseconds
    }
  }
  initManagementInterruptTimer();
  tmpEnclave = waitForEnclave(ENCLAVE_INVALID_ID);
  //TODO Calculate the base address of the page table when there is more than one enclave core
  //if(installPageTable(<<baseAddress>>, tmpEnclave) == BOOL_TRUE) return ENCLAVE_VIRTUAL_ADDRESS_BASE;
  output_string("management.c: error currently only supporting single core entering infinite loop.\n");
  while(1) {}
}

void normalWorld() {
  while(initialization_done == BOOL_FALSE) {
    //Waiting until enclave core 0 has finished initialization
  }
}

struct trapReturn {
    int status; //assuming this is a0
    Address_t retAddr; //assuming this is a1
};

void __advance_mepc() {
  uint64_t nextpc;
  asm volatile(
    "csrr %0, mepc"
    : "=r"(nextpc)//output
    : //input
    : //clobbered
  );
  nextpc += 4; //Jump to instruction after ecall, which is 4 bytes
  asm volatile(
    "csrw mepc, %0"
    : //output
    : "r"(nextpc)//input
    : //clobbered
  );
}

//Returns:
// status = 2 if it was an ecall and retAddr is populated
// status = 1 when an enclave is done running
// status = 0 if it was a handled trap and needs to be returned
struct trapReturn handleTrap(enum ManagementCall_t callType, Address_t argAddress, enclave_id_t argId) {
  int setMTIP = MSTATUS_MIE;
  int mipVal = 0;
  int mcauseVal = 0;
  struct EnclaveData_t *data = 0;
  uint64_t tmpEntry;
  enclave_id_t oldEnclave = getCurrentEnclaveID();
  struct trapReturn retVal;
  retVal.status = 0;
  retVal.retAddr = 0;

  SWITCH_ENCLAVE_ID(ENCLAVE_MANAGEMENT_ID);
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
  switch(mcauseVal) {
    case CAUSE_SUPERVISOR_ECALL:
      switch(callType) {
        case MANAGE_EXIT:
          asm volatile(
            "csrw mcause, zero"
            : //output
            : //input
            : //clobbered
          );
          retVal.status = 1;
          break;
        case MANAGE_SETREADER:
          //TODO return physical address
          data = getEnclaveDataPointer(oldEnclave);
          retVal.status = 2;
          retVal.retAddr = argAddress - ENCLAVE_VIRTUAL_ADDRESS_BASE - MAILBOX_SIZE + data->codeEntryPoint;
          retVal.retAddr = (retVal.retAddr - DRAM_BASE) >> PAGE_BIT_SHIFT;

          volatile struct page_tag_t *page_tag = (volatile struct page_tag_t *) (TAGDIRECTORY_BASE + (retVal.retAddr*sizeof(struct page_tag_t)));
          SWITCH_ENCLAVE_ID(oldEnclave); //This is to check that the enclave is the owner of the page.
          page_tag->reader = argId;
          SWITCH_ENCLAVE_ID(ENCLAVE_MANAGEMENT_ID);
          __advance_mepc();
          break;
        case MANAGE_MAPMAIL:
          //TODO map page into virt mem
          data = getEnclaveDataPointer(oldEnclave);
          tmpEntry = PTE_R | PTE_V;
          tmpEntry |= ((argAddress) >> PAGE_BIT_SHIFT) << PTE_PPN_SHIFT;
          ((uint64_t *) ENCLAVE_PAGE_TABLES_BASE_ADDRESS)[2*PAGE_SIZE/sizeof(uint64_t) + data->pagesDonated + 1] = tmpEntry;
          retVal.status = 2;
          retVal.retAddr = ENCLAVE_VIRTUAL_ADDRESS_BASE + MAILBOX_SIZE + data->pagesDonated*PAGE_SIZE;
          data->pagesDonated += 1;
          data->receivePages += 1;
          __advance_mepc();
          break;
        default:
#ifdef PRAESIDIO_DEBUG
          output_string("management.c: Error invalid syscall.\n");
#endif
          retVal.status = 1;
          break;
      }
      break;
    case CAUSE_FETCH_PAGE_FAULT:
    case CAUSE_LOAD_PAGE_FAULT:
    case CAUSE_STORE_PAGE_FAULT:
#ifdef PRAESIDIO_DEBUG
      output_string("management.c: Page fault in enclave.\n");
#endif
      //TODO put enclave into error mode
      retVal.status = 1;
      break;
    default:
      break;
    //TODO check for other causes
  }

  SWITCH_ENCLAVE_ID(oldEnclave);
  return retVal;
}
