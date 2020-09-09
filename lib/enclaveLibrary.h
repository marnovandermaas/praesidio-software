// See LICENSE file for the license details of MIT
// Copyright 2020 Marno van der Maas

#ifndef ENCLAVE_LIBRARY_HEADER
#define ENCLAVE_LIBRARY_HEADER

#include "praesidiopage.h"

#define ENCLAVE_MANAGEMENT_ID (0xFFFFFFFE)
#define ENCLAVE_DEFAULT_ID    (0x00000000)
#define ENCLAVE_INVALID_ID    (0xFFFFFFFF)

 //These base number should be above 0x020c 0000 which is the end of CLINT and below 0x8000 0000 which is the start of DRAM
#define MANAGEMENT_SHIM_BASE (0x04000000)
#define MAILBOX_BASE            (0x03000000)
#define MAILBOX_SIZE            (1 << PAGE_BIT_SHIFT)

#ifndef DRAM_BASE
#define DRAM_BASE               (0x80000000)
#endif

typedef uint32_t enclave_id_t;
typedef uint32_t CoreID_t;

enum MessageType_t {
  MSG_CREATE      = 0x0,
  MSG_DONATE_PAGE = 0x1,
  MSG_FINALIZE    = 0x2,
  MSG_ATTEST      = 0x3,
  MSG_RUN         = 0x4,
  MSG_SHARE_PAGE  = 0x5,
  MSG_DELETE      = 0x6,
  MSG_INVALID     = 0xF, //This must be higher value than the other message types
};

enum boolean {
  BOOL_FALSE=0,
  BOOL_TRUE=1,
};

struct Message_t {
  enum MessageType_t type;
  enclave_id_t source;
  enclave_id_t destination;
  uint64_t arguments[2];
};

#endif //ENCLAVE_LIBRARY_HEADER
