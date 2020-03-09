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

	ok( min(2,4)==2, "min(2,4)" );
	ok( min(2.0f,4.0f)==2.0f, "min(2.0f,4.0f)" );
	ok( min(2.0,4.0)==2.0, "min(2.0,4.0)" );

	ok( max(2,4)==4, "max(2,4)" );
	ok( max(2.0f,4.0f)==4.0f, "max(2.0f,4.0f)" );
	ok( max(2.0,4.0)==4.0, "max(2.0,4.0)" );

}
