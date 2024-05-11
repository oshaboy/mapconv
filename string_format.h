#ifndef STRING_FORMAT_H
#define STRING_FORMAT_H
#include <inttypes.h>
#include <limits.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
typedef union {
	struct {
		intptr_t unpacked_str : (sizeof(intptr_t)-1)*CHAR_BIT;
		unsigned char len : CHAR_BIT;
	};
	unsigned char packed_str[sizeof(intptr_t)];
} MappingString;
_Static_assert(sizeof(MappingString)==sizeof(intptr_t));
#define MAPPING_STRING_MAX_LEN ((1<<CHAR_BIT)-1)
#define MAPPING_STRING_PACKED_MAX_LEN (sizeof(intptr_t)-1)
#define MAPPING_STRING_LENGTH(x) ((MAPPING_STRING_PACKED_MAX_LEN-((x).len))&0xff)
#define MAPPING_STRING_ISPACKED(x) ((x).len>=0)
#define MAPPING_IS_ERROR(x) (MAPPING_STRING_LENGTH(x)==MAPPING_STRING_MAX_LEN+1)


static inline MappingString from_cstr_impl(const unsigned char * str, short len){
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

#define from_cstr(x,y) _Generic((x), \
	const unsigned char *: (const MappingString)from_cstr_impl(x,y),\
	unsigned char *: from_cstr_impl(x,y)\
)
static inline MappingString from_cstr_heap(const unsigned char * str, short len){

	unsigned char * new_str;
	/*Avoid Memory Leak with packed strings*/
	if(len<=MAPPING_STRING_PACKED_MAX_LEN)
		new_str=(unsigned char *)str;
	else {
		new_str=(unsigned char *)malloc(len+1);
		memcpy(new_str, str, len);
		new_str[len]='\0';
	}
	
	
	return from_cstr(new_str, len);
}


static inline unsigned char * to_cstr_impl(const MappingString * mystr){
	if (MAPPING_IS_ERROR(*mystr))
		return NULL;
	else 
		return MAPPING_STRING_ISPACKED(*mystr)?
			(unsigned char *)mystr->packed_str:
			(unsigned char *)(intptr_t)mystr->unpacked_str;
}
#define to_cstr(x) _Generic((x), \
	const MappingString *: (const unsigned char *)to_cstr_impl(x),\
	MappingString *: to_cstr_impl(x)\
)
static inline unsigned char * to_cstr_heap(const MappingString * mystr){
	unsigned char * new_str =(unsigned char *) malloc(mystr->len+1);
	memcpy(new_str, to_cstr(mystr), mystr->len);
	new_str[mystr->len]='\0';
	return new_str;
}
#endif