#ifndef PRAESIDIO_RING_BENCHMARK_HEADER
#define PRAESIDIO_RING_BENCHMARK_HEADER

#include "communication.h"
#include "praesidiopage.h"

#define PACKET_MAX        ((1 << (PAGE_BIT_SHIFT)) - 2*LENGTH_SIZE)
#define PACKET_INCREMENT  (64)
#define PACKET_START      PACKET_INCREMENT
#define NUMBER_OF_REPS    (256)

#endif
