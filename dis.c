#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include "stream.h"

static int_fast8_t mgetc(void* file) {
  int_fast8_t byte;
  return (byte = getc(file)) != EOF? byte : '\0';
}

static Stream file_stream = (Stream) {
  .stream = NULL,
  .getbyte = &mgetc,
};

static char *direction_table[5][8] = { 
    { "[bx + si]"         , "[bx + di]"         , "[bp + si]"         , "[bp + di]"         , "[si]"         , "[di]"         , "0x%.4x"       , "[bx]"          },
    { "[bx + si + 0x%.2x]", "[bx + di + 0x%.2x]", "[bp + si + 0x%.2x]", "[bp + di + 0x%.2x]", "[si + 0x%.2x]", "[di + 0x%.2x]", "[bp + 0x%.2x]", "[bx + 0x%.2x]" },
    { "[bx + si + 0x%.4x]", "[bx + di + 0x%.4x]", "[bp + si + 0x%.4x]", "[bp + di + 0x%.4x]", "[si + 0x%.4x]", "[di + 0x%.4x]", "[bp + 0x%.4x]", "[bx + 0x%.4x]" },

    { "al"                , "cl"                , "dl"                , "bl"                , "ah"           , "ch"           , "dh"           , "bh"           },
    { "ax"                , "cx"                , "dx"                , "bx"                , "sp"           , "bp"           , "si"           , "di"           }
};

static void reg_to_or_from_mem(int_fast8_t fbyte, Stream stream) {
  const int_fast8_t 
    w     = fbyte & 0x1
  , d     = (fbyte & 0x2) >> 1
  , sbyte = stream.getbyte(stream.stream)
  , rm    = sbyte & 0x7
  , reg   = (sbyte & 0x38) >> 3
  , mod   = (sbyte & 0xc0) >> 6;

  const char
    *destination = d? direction_table[0b11 + w][reg] : direction_table[mod + w*(mod == 0b11)][rm]
  , *source      = d? direction_table[mod + w*(mod == 0b11)][rm] : direction_table[0b11 + w][reg];

  int_fast16_t offset = 0;
  if (mod == 0b10 || mod == 0b00 && rm == 0b110) {
    offset  = stream.getbyte(stream.stream);
    offset &= stream.getbyte(stream.stream) << 8; 
  } else if (mod == 0b01) {
    offset = stream.getbyte(stream.stream);
  }
  printf("mov ");
  printf(destination, offset);
  printf(", ");
  printf(source, offset);
  printf("\n");
}

static void (*decode_table[256])(int_fast8_t, Stream) = {
  // ...
  /* mov: 11000__ */ [135 ... 138] = &reg_to_or_from_mem,
  // ...
};

void decode_file(Stream stream) {
  for (int_fast8_t byte = stream.getbyte(stream.stream); byte; byte = stream.getbyte(stream.stream))
    (*(decode_table[(uint8_t) byte]))(byte, stream);
}


int main(int argc, char *argv[]) {
  if (argc != 2) {
    fprintf(stderr, "Usage: %s [file]: decodes [file]\n", argv[0]);
    return 1;
  }

  FILE* input_file = fopen(argv[1], "rb");
  if (!input_file) {
    fprintf(stderr, "(%s) %s\n", argv[1], strerror(errno));
    return 1;
  }
  file_stream.stream = input_file;

  decode_file(file_stream);

  fclose(input_file);
  return 0;
}
