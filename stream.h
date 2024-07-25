#ifndef INCLUDE_home_ford_Projects_performance_oriented_programming_disassemblestreamstream_h_
#define INCLUDE_home_ford_Projects_performance_oriented_programming_disassemblestreamstream_h_

#include <stdlib.h>
#include <stdint.h>

typedef struct Stream {
  void* stream;
  int_fast8_t (*getbyte)(void*);
} Stream;

#endif // INCLUDE_home_ford_Projects_performance_oriented_programming_disassemblestreamstream_h_
