#ifndef PRAESIDIO_MANAGEMENT_H
#define PRAESIDIO_MANAGEMENT_H

#include "enclaveLibrary.h"

#define PAGE_BIT_SHIFT (12)

//Definition of the base addresses of the three trusted pages:
#define MANAGEMENT_CODE_BASE_ADDRESS ((Address_t) MANAGEMENT_ENCLAVE_BASE)
#define PAGE_DIRECTORY_BASE_ADDRESS ((Address_t) MANAGEMENT_CODE_BASE_ADDRESS + PAGE_SIZE) //Assumes code is less than 1 page in size
#define ENCLAVE_DATA_BASE_ADDRESS ((Address_t) PAGE_DIRECTORY_BASE_ADDRESS + PAGE_SIZE)
#define MANAGEMENT_STACK_BASE_ADDRESS ((Address_t) ENCLAVE_DATA_BASE_ADDRESS + 2*PAGE_SIZE)

#define NUMBER_OF_ENCLAVE_CORES 1

typedef unsigned long Address_t;
typedef uint16_t Permissions_t;
typedef int Register_t;
typedef uint32_t CoreID_t;

struct Context_t {
  Register_t registerFile[32];
  Register_t floatingPointRegisters[32];
  Register_t PC;
  //TODO add control and status registers
};

struct PhysCap_t {
  Address_t base;
  Address_t top;
  Permissions_t perm;
  enclave_id_t owner;
};

enum EnclaveState_t {
  STATE_CREATED         = 0x0,
  STATE_RECEIVINGPAGES  = 0x1,
  STATE_FINALIZED       = 0x2,
};

struct EnclaveData_t {
  enum EnclaveState_t state;
  enclave_id_t eID;
  Address_t codeEntryPoint;
  struct Context_t restorePoint;
};

struct ManagementState_t { //TODO make the runningEnclaves and enclaveCores variables dynamic and based on device tree value.
  enclave_id_t runningEnclaves[NUMBER_OF_ENCLAVE_CORES]; //Initialize as equal to MANAGEMENT_ENCLAVE_ID
  enclave_id_t nextEnclaveID; //Initialize as 1
  struct PhysCap_t rootCapability; //Initialize with special instruction
  Address_t PagePool; //Initialize as Null
  CoreID_t enclaveCores[NUMBER_OF_ENCLAVE_CORES]; //Initialize using which core identifiers belong to all the enclave cores
};

#define SIZE_OF_MESSAGE_TYPE 4 //Assuming message type can fit in this many bits.

enum boolean {
  BOOL_FALSE=0,
  BOOL_TRUE=1,
};

struct Message_t {
  enum MessageType_t type;
  enclave_id_t source;
  enclave_id_t destination;
  unsigned long content; //Assuming that an int can fit an Address_t
};

#endif //PRAESIDIO_MANAGEMENT_H
