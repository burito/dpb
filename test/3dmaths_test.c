#include <ctap.h>

#include "3dmaths.h"

int mat4x4_equals(mat4x4 l, mat4x4 r)
{
	for(int i=0; i<16; i++)
	if( l.f[i] != r.f[i] )
		return 0;
	return 1;
}


TESTS {

	mat4x4 a = mat4x4_identity();
	mat4x4 b = mat4x4_transpose(a);

	ok( mat4x4_equals( a, b ), "transpose identity" );
	


}