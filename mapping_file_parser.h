#ifndef MAPPING_FILE_PARSER_H
#define MAPPING_FILE_PARSER_H
#include <stdio.h>
#include "string_format.h"

typedef struct  {
	MappingString from;
	MappingString to;
	uint64_t special_flags;
} Mapping;
#define RV 1
#define PARTIAL 0x80

typedef struct {
	Mapping * table;
	size_t len;
} MappingTable;
MappingTable parse_mapping_file(FILE * mapping_file);
typedef enum {
	CONVERSION_OK,
	INVALID_CHARACTER,
	INCOMPLETE_CHARACTER,
	BUFFER_NOT_BIG_ENOUGH

} convert_result;
convert_result convert(MappingTable table,
	const unsigned char * in,
	size_t inlen,
	unsigned char * out,
	size_t outbuflen,
	size_t * outlen
);

void clear_mapping_table(MappingTable);
#endif