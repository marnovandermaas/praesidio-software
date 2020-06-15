#ifndef PRAESIDIO_RING_BENCHMARK_HEADER
#define PRAESIDIO_RING_BENCHMARK_HEADER

#include "communication.h"
#include "praesidiopage.h"

#define PACKET_MAX        ((1 << (PAGE_BIT_SHIFT)) - 16*LENGTH_SIZE)
#define PACKET_START      (1)
#define PACKET_INCREMENT  (64)
#define NUMBER_OF_REPS    (16)

#endif
