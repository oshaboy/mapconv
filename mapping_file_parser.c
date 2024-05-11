
#include "mapping_file_parser.h"
#include "utf8.h"
typedef struct {
	unsigned char len;
	char * str;
} StringLength;

static char * skip_hex_prefix(char * line_buffer_ptr){
	const static struct{
		char * str;
		int len;
	} hex_prefices[]={
		{"0X",2},
		{"X",1},
		{"\\X",2},
		{"/X",2},
		{"U",1},
	};
	for (int i=0;i<sizeof(hex_prefices)/sizeof(hex_prefices[0]);i++){
		if (memcmp(line_buffer_ptr, hex_prefices[i].str, hex_prefices[i].len)==0){
			line_buffer_ptr+=hex_prefices[i].len;
			break;
		}
	}
	return line_buffer_ptr;
}

MappingTable parse_mapping_file(FILE * mapping_file){
	MappingTable result;
	const size_t mapping_size=sizeof(Mapping);	
	result.table=malloc(mapping_size*128);
	bool fail=false;
	size_t table_capacity=128;
	char * line_buffer=malloc(128);
	size_t line_buffer_capacity=128;
	Mapping * table_ptr=result.table;
	while (!feof(mapping_file)){
		char * line_buffer_ptr=line_buffer;
		int codepoint,advance_by;
		unsigned char hex_byte;
		unsigned char from[MAPPING_STRING_MAX_LEN];
		unsigned char * from_ptr=from;
		unsigned char to[MAPPING_STRING_MAX_LEN];
		unsigned char * to_ptr=to;
		while(fgets(line_buffer,line_buffer_capacity, mapping_file)==NULL){
			if (feof(mapping_file)) goto end;
			line_buffer_capacity*=2;
			free(line_buffer);
			line_buffer=malloc(line_buffer_capacity);
		}
		while(isspace(*line_buffer_ptr)) line_buffer_ptr++;
		if (*line_buffer_ptr=='#' || *line_buffer_ptr=='\0')
			continue;
		/*convert to uppercase*/
		for(int i = 0; line_buffer_ptr[i]; i++){
			line_buffer_ptr[i] = toupper(line_buffer_ptr[i]);
		}

		line_buffer_ptr=skip_hex_prefix(line_buffer_ptr);
		table_ptr->special_flags=0;
		do {

			if (sscanf(line_buffer_ptr, "%02hhx%n", &hex_byte,&advance_by)==0){
				*from_ptr++=table_ptr-result.table;
				break;
			} else 
				line_buffer_ptr+=advance_by;
			
			*from_ptr++=hex_byte;

		} while(!isspace(*line_buffer_ptr));
		table_ptr->from=from_cstr_heap(from, from_ptr-from);

		while(isspace(*line_buffer_ptr)) line_buffer_ptr++;
		do {
			line_buffer_ptr=skip_hex_prefix(line_buffer_ptr);
			if (sscanf(line_buffer_ptr,"%x%n",&codepoint,&advance_by)==0)
				table_ptr->special_flags|=PARTIAL;
			else {
				line_buffer_ptr+=advance_by;

				UTF8_Bytes utf8_bytes = put_utf8(codepoint);
				memcpy(to_ptr, utf8_bytes.str,utf8_bytes.len);
				to_ptr+=utf8_bytes.len;
			}

			if (*line_buffer_ptr=='+' || isspace(*line_buffer_ptr)) line_buffer_ptr++;
		} while(
			*line_buffer_ptr &&
			*line_buffer_ptr!='#' && 
			*line_buffer_ptr != '\n'
		);
		table_ptr->to=from_cstr_heap(to, to_ptr-to);
		table_ptr++;
		if (table_ptr-result.table >= table_capacity){
			ptrdiff_t offset=table_ptr-result.table;
			table_capacity*=2;
			result.table=realloc(result.table, mapping_size*table_capacity);
			table_ptr=result.table+offset;
		}
		
	
	}
end:
	result.len=table_ptr-result.table;
	if(fail) {
		clear_mapping_table(result);
		result.table=NULL;
		result.len=0;
	}

	free(line_buffer);
	return result;

}
convert_result convert_to_utf8(
	MappingTable table,
	const unsigned char * restrict in,
	size_t inlen,
	unsigned char * restrict out,
	size_t outbuflen,
	size_t * restrict outlen
){
	size_t outlen_tmp;
	if (outlen==NULL) outlen=&outlen_tmp;
	size_t in_iter=0;
	*outlen=0;

	while (in_iter < inlen) {
		bool found=false;
		bool found_partial=false;
		for (size_t i=0; i<table.len; i++){
			Mapping mapping=table.table[i];
			if (inlen-in_iter < MAPPING_STRING_LENGTH(mapping.from)) {
				if (memcmp(in+in_iter, to_cstr(&mapping.from), inlen-in_iter)==0)
					found_partial=true;
				
			} else if (memcmp(in+in_iter, to_cstr(&mapping.from), MAPPING_STRING_LENGTH(mapping.from))==0){		
				if (mapping.special_flags&PARTIAL)
					found_partial=true;
				else {
					if (*outlen+MAPPING_STRING_LENGTH(mapping.to) > outbuflen)
						return BUFFER_NOT_BIG_ENOUGH;

					memcpy(out+*outlen,to_cstr(&mapping.to), MAPPING_STRING_LENGTH(mapping.to));
					*outlen+=MAPPING_STRING_LENGTH(mapping.to);
					in_iter+=MAPPING_STRING_LENGTH(mapping.from);
					found=true;
					break;
				}
			}
		}
		
		if (!found)
			return found_partial?INCOMPLETE_CHARACTER:INVALID_CHARACTER;
		
	}
	size_t naughts=outbuflen-*outlen;
	naughts=naughts>4?4:naughts;
	memset(out+*outlen, '\0', naughts);
	return CONVERSION_OK;
}
convert_result convert_from_utf8(
	MappingTable table,
	const unsigned char * restrict in,
	size_t inlen,
	unsigned char * restrict  out,
	size_t outbuflen,
	size_t * restrict outlen
){
	size_t outlen_tmp;
	if (outlen==NULL) outlen=&outlen_tmp;
	size_t in_iter=0;
	*outlen=0;

	while (in_iter < inlen) {
		bool found=false;
		bool found_partial=false;
		for (size_t i=0; i<table.len; i++){
			Mapping mapping=table.table[i];
			if (mapping.special_flags & PARTIAL) continue;
			else if (inlen-in_iter < MAPPING_STRING_LENGTH(mapping.to)) {
				if (memcmp(in+in_iter, to_cstr(&mapping.to), inlen-in_iter)==0)
					found_partial=true;
				
			} else if (memcmp(in+in_iter, to_cstr(&mapping.to), MAPPING_STRING_LENGTH(mapping.to))==0){		
				if (*outlen+MAPPING_STRING_LENGTH(mapping.from) > outbuflen)
					return BUFFER_NOT_BIG_ENOUGH;

				memcpy(out+*outlen,to_cstr(&mapping.from), MAPPING_STRING_LENGTH(mapping.from));
				*outlen+=MAPPING_STRING_LENGTH(mapping.from);
				in_iter+=MAPPING_STRING_LENGTH(mapping.to);
				found=true;
				break;
			}

		}
		
		if (!found)
			return found_partial?INCOMPLETE_CHARACTER:INVALID_CHARACTER;
		
	}
	if (outbuflen < *outlen)
		out[*outlen]='\0';
	
	return CONVERSION_OK;
}
void clear_mapping_table(MappingTable table){
	for (size_t i=0; i<table.len; i++){
		Mapping mapping=table.table[i];
		if (!MAPPING_IS_ERROR(mapping.from)&&!MAPPING_STRING_ISPACKED(mapping.from))
			free(to_cstr(&mapping.from));
		if (!MAPPING_IS_ERROR(mapping.to)&&!MAPPING_STRING_ISPACKED(mapping.to))
			free(to_cstr(&mapping.to));
	}
	free(table.table);
}
