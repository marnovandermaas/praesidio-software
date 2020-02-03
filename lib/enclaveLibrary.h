#ifndef ENCLAVE_LIBRARY_HEADER
#define ENCLAVE_LIBRARY_HEADER

#define ENCLAVE_MANAGEMENT_ID (0xFFFFFFFFFFFFFFFE)
#define ENCLAVE_DEFAULT_ID    (0)
#define ENCLAVE_INVALID_ID    (0xFFFFFFFFFFFFFFFF)

#define MANAGEMENT_ENCLAVE_BASE 0x04000000 //This number should be above 0x020c 0000 which is the end of CLINT and below 0x0400 0000 which is the start of IO
#ifndef DRAM_BASE
#define DRAM_BASE               0x80000000
#endif

typedef uint64_t enclave_id_t;
typedef uint32_t CoreID_t;

#define PAGE_BIT_SHIFT (12)

enum MessageType_t {
  MSG_CREATE_ENCLAVE    = 0x0,
  MSG_DELETE_ENCLAVE    = 0x1,
  MSG_ATTEST            = 0x2,
  MSG_ACQUIRE_PHYS_CAP  = 0x3,
  MSG_DONATE_PAGE       = 0x4,
  MSG_SWITCH_ENCLAVE    = 0x5,
  MSG_INTER_ENCLAVE     = 0x6,
  MSG_SET_ARGUMENT      = 0x7,
  MSG_MASK              = 0xF,
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

#endif //ENCLAVE_LIBRARY_HEADER
