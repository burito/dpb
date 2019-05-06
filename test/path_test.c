#include <stdio.h>
#define DPB_PATH_IMPLEMENTATION
#include "dpb_path.h"

int main(int argc, char* argv[])
{
	printf( "Saved Games: %s\n", dpb_path_savedgames() );
	return 0;
}