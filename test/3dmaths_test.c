#include <ctap/include/ctap.h>

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

	ok( nmin(2,4)==2, "nmin(2,4)" );
	ok( nmin(2.0f,4.0f)==2.0f, "nmin(2.0f,4.0f)" );
	ok( nmin(2.0,4.0)==2.0, "nmin(2.0,4.0)" );

	ok( nmax(2,4)==4, "nmax(2,4)" );
	ok( nmax(2.0f,4.0f)==4.0f, "nmax(2.0f,4.0f)" );
	ok( nmax(2.0,4.0)==4.0, "nmax(2.0,4.0)" );

}
