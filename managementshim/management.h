// See LICENSE file for the license details of MIT
// Copyright 2020 Marno van der Maas

#ifndef PRAESIDIO_MANAGEMENT_H
#define PRAESIDIO_MANAGEMENT_H

#include "unsignedinteger.h"
#include "enclaveLibrary.h"
#include "praesidiooutput.h"

//Definition of the base addresses:
#define MANAGEMENT_CODE_BASE_ADDRESS ((Address_t) MANAGEMENT_SHIM_BASE)
#define PAGE_DIRECTORY_BASE_ADDRESS ((Address_t) MANAGEMENT_CODE_BASE_ADDRESS + 2*PAGE_SIZE) //Assumes code is less than 2 pages in size
#define ENCLAVE_DATA_BASE_ADDRESS ((Address_t) PAGE_DIRECTORY_BASE_ADDRESS + PAGE_SIZE)
#define MANAGEMENT_STACK_BASE_ADDRESS ((Address_t) ENCLAVE_DATA_BASE_ADDRESS + 2*PAGE_SIZE)

#define NUMBER_OF_ENCLAVE_CORES 1

typedef unsigned long Address_t;
typedef uint16_t Permissions_t;
typedef int Register_t;

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
  STATE_EMPTY           = 0x0,
  STATE_CREATED         = 0x1,
  STATE_BUILDING        = 0x2,
  STATE_LIVE            = 0x3,
  STATE_ERROR           = 0x4,
};

struct EnclaveData_t {
  enum EnclaveState_t state;
  uint16_t pagesDonated;
  enclave_id_t eID;
  Address_t codeEntryPoint;
  struct Context_t restorePoint;
};

struct ManagementState_t { //TODO make the runningEnclaves and enclaveCores variables dynamic and based on device tree value.
  enclave_id_t runningEnclaves[NUMBER_OF_ENCLAVE_CORES]; //Initialize as equal to ENCLAVE_MANAGEMENT_ID
  enclave_id_t nextEnclaveID; //Initialize as 1
  struct PhysCap_t rootCapability; //Initialize with special instruction
  Address_t PagePool; //Initialize as Null
  CoreID_t enclaveCores[NUMBER_OF_ENCLAVE_CORES]; //Initialize using which core identifiers belong to all the enclave cores
};

#endif //PRAESIDIO_MANAGEMENT_H
