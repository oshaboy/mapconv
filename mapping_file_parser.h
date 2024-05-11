#ifndef MAPPING_FILE_PARSER_H
#define MAPPING_FILE_PARSER_H
#include <stdio.h>
#include "string_format.h"
typedef enum {
	RV=1,
	PARTIAL=0x80
} SpecialFlags;
typedef struct  {
	MappingString from;
	MappingString to;
	SpecialFlags special_flags;
} Mapping;



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
convert_result convert_to_utf8(MappingTable table,
	const unsigned char * restrict in,
	size_t inlen,
	unsigned char * restrict out,
	size_t outbuflen,
	size_t * restrict outlen
);
convert_result convert_from_utf8(
	MappingTable table,
	const unsigned char * restrict in,
	size_t inlen,
	unsigned char * restrict out,
	size_t outbuflen,
	size_t * restrict outlen
);
void clear_mapping_table(MappingTable);
#endif