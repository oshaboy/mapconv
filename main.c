#include "mapping_file_parser.h"
#include <getopt.h>
#include <stdio.h>
int main(int argc, char * argv[]){
	int opt;
	FILE *from_mapfile=NULL,*to_mapfile=NULL, *instream=stdin;
	bool is_stdin=true;
	while((opt=getopt(argc,argv,"f:t:"))!=0){
		switch (opt){
			case 'f':
			from_mapfile=fopen(argv[optind],"rt");
			break;
			case 't':
			to_mapfile=fopen(argv[optind],"rt");
			break;
			default:
			break;

		}
	}
	if (argc>=optind+1){
		instream=fopen(argv[optind],"rt");
		is_stdin=false;
	}

	
	MappingTable from_maptable=parse_mapping_file(from_mapfile);
	MappingTable to_maptable=parse_mapping_file(to_mapfile);
	
	
	clear_mapping_table(from_maptable);
	clear_mapping_table(to_maptable);
	return 0;
}