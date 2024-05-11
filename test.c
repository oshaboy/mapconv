#include "mapping_file_parser.h"
#include <stdio.h>
int main(void){
	FILE * f=fopen("./mapping_files_ignore/C64IPRI.TXT","rt");
	MappingTable m=parse_mapping_file(f);
	char * test_string="\x78\x7a\x73\x61";
	char outbuf[17];
	size_t outlen;
	convert_to_utf8(
		m,
		test_string,
		4,
		outbuf,
		50,
		&outlen
	);
	printf("%zu %s\n",outlen,outbuf);
	char roundtrip[5];
	size_t rt_outlen;
	convert_from_utf8(
		m,
		outbuf,
		outlen,
		roundtrip,
		5,
		&rt_outlen
	);
	for (int i=0; i<rt_outlen; i++)
		printf("%x %x %x\n",
		roundtrip[i],
		test_string[i],
		roundtrip[i]==test_string[i]
		);


	
	return 0;
}