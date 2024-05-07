#ifndef STRING_FORMAT_H
#define STRING_FORMAT_H
#include <inttypes.h>
#include <limits.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
typedef union {
	struct {
		unsigned char packed_str[0];
		intptr_t unpacked_str : (sizeof(intptr_t)-1)*CHAR_BIT;
		unsigned char len : CHAR_BIT;
	};
	unsigned char raw[sizeof(intptr_t)];
} MappingString;

#define MAPPING_STRING_MAX_LEN ((1<<CHAR_BIT)-1)
#define MAPPING_STRING_PACKED_MAX_LEN (sizeof(intptr_t)-1)
#define MAPPING_STRING_LENGTH(x) ((MAPPING_STRING_PACKED_MAX_LEN-((x).len))&0xff)
#define MAPPING_STRING_ISPACKED(x) ((x).len>=0)
#define MAPPING_IS_ERROR(x) (MAPPING_STRING_LENGTH(x)==MAPPING_STRING_MAX_LEN+1)

static MappingString from_cstr(const unsigned char * str, short len){
	MappingString result;
	result.len=MAPPING_STRING_PACKED_MAX_LEN-len;
	if (len < 0) len=strnlen((char *)str,MAPPING_STRING_MAX_LEN+1);
	if (len > MAPPING_STRING_MAX_LEN )
		result.len=(MAPPING_STRING_PACKED_MAX_LEN-(MAPPING_STRING_MAX_LEN+1))&0xff;
	else if (len <= MAPPING_STRING_PACKED_MAX_LEN){
		memcpy(result.packed_str, str,len);
		result.packed_str[len]='\0';
	} else 
		result.unpacked_str=(intptr_t)str;
	return result;
}
static MappingString from_cstr_heap(const unsigned char * str, short len){

	unsigned char * new_str;
	const unsigned char * new_str_const;
	if(len<=MAPPING_STRING_PACKED_MAX_LEN)
		new_str_const=str;
	else {
		new_str=(unsigned char *)malloc(len+1);
		memcpy(new_str, str, len);
		new_str[len]='\0';
		new_str_const=new_str;
	}

	return from_cstr(new_str_const, len);
}

static unsigned char * to_cstr(const MappingString * mystr){
	if (MAPPING_IS_ERROR(*mystr))
		return "\xff";
	else 
		return MAPPING_STRING_ISPACKED(*mystr)?
			mystr->packed_str:(unsigned char *)mystr->unpacked_str;
}


static unsigned char * to_cstr_heap(MappingString * mystr){
	unsigned char * new_str =(unsigned char *) malloc(mystr->len+1);
	memcpy(new_str, to_cstr(mystr), mystr->len);
	new_str[mystr->len]='\0';
	return new_str;
}
#endif