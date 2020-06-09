#ifndef PRAESIDIO_RING_BENCHMARK_HEADER
#define PRAESIDIO_RING_BENCHMARK_HEADER

#include "communication.h"
#include "praesidiopage.h"

#define START_PACKET_SIZE ((1 << (PAGE_BIT_SHIFT - 1)) - 16*LENGTH_SIZE)
#define NUMBER_OF_REPS (2048)

#endif
