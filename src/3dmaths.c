/*
Copyright (c) 2012,2021 Daniel Burke

This software is provided 'as-is', without any express or implied
warranty. In no event will the authors be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

   1. The origin of this software must not be misrepresented; you must not
   claim that you wrote the original software. If you use this software
   in a product, an acknowledgment in the product documentation would be
   appreciated but is not required.

   2. Altered source versions must be plainly marked as such, and must not be
   misrepresented as being the original software.

   3. This notice may not be removed or altered from any source
   distribution.
*/

// NOTE: can change from C to SSE or LLVM versions for unmeasured perf change
#include "../deps/small-matrix-inverse/invert4x4_c.h"

#include <math.h>
#include <stdio.h>

#include "3dmaths.h"

/*
 * Fast Inverse Square Root.
 * http://www.beyond3d.com/content/articles/8/
 */
float finvsqrt(float x)
{
	union finv { int i; float f; } u;
	u.f = x;

	float xhalf = 0.5f*x;
	u.i = 0x5f3759df - (u.i>>1);
	x = u.f;
	x = x*(1.5f - xhalf*x*x); /* repeat this line to improve accuracy */
	return x;
}

/*
 * Prints a given 4x4 matrix.
 * Useful for debugging.
 */
void mat4x4_print(mat4x4 m)
{
	printf("\t%f\t%f\t%f\t%f\n\t%f\t%f\t%f\t%f\n\t%f\t%f\t%f\t%f\n\t%f\t%f\t%f\t%f\n",
		m.m[0][0], m.m[0][1], m.m[0][2], m.m[0][3],
		m.m[1][0], m.m[1][1], m.m[1][2], m.m[1][3],
		m.m[2][0], m.m[2][1], m.m[2][2], m.m[2][3],
		m.m[3][0], m.m[3][1], m.m[3][2], m.m[3][3]
	);
}

/*
 * Returns the Inverse of a matrix.
 */
mat4x4 mat4x4_invert(mat4x4 m)
{
	mat4x4 x;
	invert4x4(m.f, x.f);
	return x;
}

/*
 * Returns the Transpose of a given matrix.
 */
mat4x4 mat4x4_transpose(mat4x4 m)
{
	mat4x4 x = { .f={
	m.m[0][0], m.m[1][0], m.m[2][0], m.m[3][0],
	m.m[0][1], m.m[1][1], m.m[2][1], m.m[3][1],
	m.m[0][2], m.m[1][2], m.m[2][2], m.m[3][2],
	m.m[0][3], m.m[1][3], m.m[2][3], m.m[3][3]
	}};
	return x;
}

/*
 * Returns the Identity matrix.
 */
mat4x4 mat4x4_identity(void)
{
	mat4x4 a = { .f={
		1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1
	}};
	return a;
}

/*
 * Returns a Rotation matrix around the X axis.
 */
mat4x4 mat4x4_rot_x(float t)
{
	mat4x4 a = { .f={
		1,       0,      0, 0,
		0,  cos(t), sin(t), 0,
		0, -sin(t), cos(t), 0,
		0,       0,      0, 1
	}};
	return a;
}

/*
 * Returns a Rotation matrix around the Y axis.
 */
mat4x4 mat4x4_rot_y(float t)
{
	mat4x4 a = { .f={
		cos(t), 0, -sin(t), 0,
		     0, 1,       0, 0,
		sin(t), 0,  cos(t), 0,
		     0, 0,       0, 1
	}};
	return a;
}

/*
 * Returns a Rotation matrix around the Z axis.
 */
mat4x4 mat4x4_rot_z(float t)
{
	mat4x4 a = { .f={
		 cos(t), sin(t), 0, 0,
		-sin(t), cos(t), 0, 0,
		      0,      0, 1, 0,
		      0,      0, 0, 1
	}};
	return a;
}

/*
 * Returns a translation matrix for the given vector.
 */
mat4x4 mat4x4_translate_vec3(vec3 v)
{
	mat4x4 a = { .f={
		1, 0, 0, v.x,
		0, 1, 0, v.y,
		0, 0, 1, v.z,
		0, 0, 0, 1
	}};
	return a;
}

/*
 * Returns a translation matrix for the given vector.
 */
mat4x4 mat4x4_translate_float(float x, float y, float z)
{
	vec3 a = { .f={ x, y, z}};
	return mat4x4_translate_vec3(a);
}

/*
 * Returns a scaling matrix for the given vector.
 */
mat4x4 mat4x4_scale_vec3(vec3 v)
{
	mat4x4 a = { .f={
		v.x, 0, 0, 0,
		0, v.y, 0, 0,
		0, 0, v.z, 0,
		0, 0, 0, 1
	}};
	return a;
}

/*
 * Returns a scaling matrix for the given vector.
 */
mat4x4 mat4x4_scale_float(float x, float y, float z)
{
	vec3 a = { .f={ x, y, z}};
	return mat4x4_scale_vec3(a);
}

/*
 * This is equivalent to glFrustum(), except looking into -Z.
 * (+Z goes out of the screen towards the user)
 */
mat4x4 mat4x4_perspective(float near, float far, float width, float height)
{ // http://www.songho.ca/opengl/gl_projectionmatrix.html#perspective
	mat4x4 ret = { .f={
		near/(0.5*width), 0, 0, 0,
		0, near/(0.5*height), 0, 0,
		0, 0, (-(far+near))/(far-near), (-2*far*near)/(far-near),
		0, 0, -1, 0
	}};
	return ret;
}

/*
 * This is an exact implementation of glFrustum().
 * See: https://www.khronos.org/registry/OpenGL-Refpages/gl2.1/xhtml/glFrustum.xml
 */
mat4x4 mat4x4_glfrustum(double left, double right, double bottom, double top, double near, double far)
{ // https://lmb.informatik.uni-freiburg.de/people/reisert/opengl/doc/glFrustum.html
	double A = (right + left) / (right - left);
	double B = (top + bottom) / (top - bottom);
	double C = (far + near) / (far - near);
	double D = (2.0*far*near) / (far - near);

	mat4x4 ret = { .f={
		(2*near)/(right - left), 0, A, 0,
		0, (2*near)/(top-bottom), B, 0,
		0, 0, C, D,
		0, 0, -1, 0
	}};
	return ret;
}

/*
 * This is a simplified version of glOrtho().
 * http://www.songho.ca/opengl/gl_projectionmatrix.html#ortho
 */
mat4x4 mat4x4_orthographic(float near, float far, float width, float height)
{
	mat4x4 ret = { .f={
		1/(0.5*width), 0, 0, 0,
		0, 1/(0.5*height), 0, 0,
		0, 0, (-2)/(far-near), -((far*near)/(far-near)),
		0, 0, 0, 1
	}};
	return ret;
}

/*
 * This is an exact implementation of glOrtho().
 * https://www.khronos.org/registry/OpenGL-Refpages/gl2.1/xhtml/glOrtho.xml
 */
mat4x4 mat4x4_glortho(double left, double right, double bottom, double top, double near, double far)
{ // http://ps-2.kev009.com/tl/techlib/manuals/adoclib/libs/openglrf/glortho.htm
	double tx = -((right+left) / (right-left));
	double ty = -((top+bottom) / (top-bottom));
	double tz = -((far+near) / (far-near));

	mat4x4 ret = { .f={
		2.0/(right-left), 0, 0, tx,
		0, 2.0/(top-bottom), 0, ty,
		0, 0, -2.0/(far-near), tz,
		0, 0, 0, 1
	}};
	return ret;
}

/*
 * Returns the Magnitude of a Vector.
 * ie. the Length, without the sqrt() applied.
 */
float vec2_mag(vec2 c)
{
	return c.x*c.x + c.y*c.y;
}

/*
 * Returns the Magnitude of a Vector.
 * ie. the Length, without the sqrt() applied.
 */
float vec3_mag(vec3 v)
{
	return v.x*v.x + v.y*v.y + v.z*v.z;
}

/*
 * Returns the Greater of the two supplied numbers.
 */
float vec2_max(vec2 c)
{
	return c.x >= c.y ? c.x : c.y;
}

/*
 * Returns the Greater of the three supplied numbers.
 */
float vec3_max(vec3 v)
{
	return (v.x>=v.y && v.x>=v.z) ? v.x : (v.y>=v.z?v.y:v.z);
}

/*
 * Returns the normalised vector.
 * i.e. makes the length = 1.
 */
vec3 vec3_norm(vec3 v)
{
	float len = sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
	float scale = 1.0f / len;
	return mul(v,scale);
}

/*
 * Returns the normalised vector.
 * i.e. makes the length = 1.
 */
vec2 vec2_norm(vec2 v)
{
	float len = sqrt(v.x * v.x + v.y * v.y);
	float scale = 1.0f / len;
	return mul(v,scale);
}


/*
 * Returns the inner product (dot product) of two vectors.
 */
float vec3_dot(vec3 l, vec3 r)
{
	return l.x * r.x + l.y * r.y + l.z * r.z;
}

/*
 * Returns the outer product (cross product) of two vectors.
 */
vec3 vec3_cross(vec3 l, vec3 r)
{
	vec3 x;
	x.x = l.y * r.z - l.z * r.y;
	x.y = l.z * r.x - l.x * r.z;
	x.z = l.x * r.y - l.y * r.x;
	return x;
}

/*
 * Returns the Jacobian of two vectors.
 */
mat3x3 vec3_jacobian_vec3(vec3 l, vec3 r)
{
	mat3x3 x = {{
		l.x/r.x, l.x/r.y, l.x/r.z,
		l.y/r.x, l.y/r.y, l.y/r.z,
		l.z/r.x, l.z/r.y, l.z/r.z,
	}};
	return x;
}

/*
 * Checks if every element of the left vector is
 * greater than every element of the right vector.
 *
 * Useful for testing if a vector is inside a volume.
 */
int vec3_greaterthan_vec3(vec3 l, vec3 r)
{
	if(l.x > r.x)
	if(l.y > r.y)
	if(l.z > r.z)
		return 1;
	return 0;
}

/*
 * Checks if every element of the left vector is
 * less than every element of the right vector.
 *
 * Useful for testing if a vector is inside a volume.
 */
int vec3_lessthan_vec3(vec3 l, vec3 r)
{
	if(l.x < r.x)
	if(l.y < r.y)
	if(l.z < r.z)
		return 1;
	return 0;
}

/*
The following functions are to be called via the
_Generic() macro's max(), mul(), add() and sub()
*/

/*
 * Returns the greater of the two int arguments.
 */
int max_int(int l, int r)
{
	return l > r ? l : r;
}

/*
 * Returns the greater of the two float arguments.
 */
float max_float(float l, float r)
{
	return l > r ? l : r;
}

/*
 * Returns the greater of the two double arguments.
 */
double max_double(double l, double r)
{
	return l > r ? l : r;
}

/*
 * Returns the lower of the two int arguments.
 */
int min_int(int l, int r)
{
	return l < r ? l : r;
}

/*
 * Returns the lower of the two float arguments.
 */
float min_float(float l, float r)
{
	return l < r ? l : r;
}

/*
 * Returns the lower of the two double arguments.
 */
double min_double(double l, double r)
{
	return l < r ? l : r;
}

/*
 * Returns the product of a matrix and a vector.
 */
vec3 mat3x3_mul_vec3(mat3x3 l, vec3 r)
{
	vec3 x;
	for (int i = 0; i < 3; i++)
	{
		x.f[i] = l.m[i][0]*r.x + l.m[i][1]*r.y + l.m[i][2]*r.z;
	}
	return x;
}

/*
 * Converts a HmdMatrix34 to a mat4x4.
 * Useful for working with OpenVR.
 */
mat4x4 mat4x4_mov_HmdMatrix34(HmdMatrix34_t x)
{
	mat4x4 r = { .f={
		x.m[0][0], x.m[1][0], x.m[2][0], 0.0f,
		x.m[0][1], x.m[1][1], x.m[2][1], 0.0f,
		x.m[0][2], x.m[1][2], x.m[2][2], 0.0f,
		x.m[0][3], x.m[1][3], x.m[2][3], 1.0f,
	}};
	return r;
}

/*
 * Converts a HmdMatrix44 to a mat4x4.
 * Useful for working with OpenVR.
 */
mat4x4 mat4x4_mov_HmdMatrix44(HmdMatrix44_t x)
{	// they're the same, trust me!
	return mat4x4_transpose( *((mat4x4*)((void*)&x)) );
}

/*
 * Returns the product of two matricies.
 */
mat4x4 mat4x4_mul_mat4x4(mat4x4 l, mat4x4 r)
{
	mat4x4 ret;
	for(int y=0; y<4; y++)
	for(int x=0; x<4; x++)
		ret.m[y][x] =
			l.m[y][0] * r.m[0][x] +
			l.m[y][1] * r.m[1][x] +
			l.m[y][2] * r.m[2][x] +
			l.m[y][3] * r.m[3][x];
	return ret;
}

/*
 * Returns the product of a matrix and a vector.
 */
vec3 mat4x4_mul_vec3(mat4x4 l, vec3 r)
{
	vec3 x;
	for (int i = 0; i < 3; i++)
	{
		x.f[i] = l.m[i][0]*r.x + l.m[i][1]*r.y + l.m[i][2]*r.z + l.m[i][3];
	}
	return x;
}

/*
 * Returns the sum of two matrices.
 */
mat4x4 mat4x4_add_mat4x4(mat4x4 l, mat4x4 r)
{
	mat4x4 x;
	for(int i=0; i<16; i++)
		x.f[i] = l.f[i] + r.f[i];
	return x;
}

/*
 * Adds a float to every element of a matrix.
 */
mat4x4 mat4x4_add_float(mat4x4 l, float r)
{
	mat4x4 x;
	for(int i=0; i<16; i++)
		x.f[i] = l.f[i] + r;
	return x;
}

/*
 * Multiplies every element of a matrix by a float.
 */
mat4x4 mat4x4_mul_float(mat4x4 l, float r)
{
	mat4x4 x;
	for(int i=0; i<16; i++)
		x.f[i] = l.f[i] * r;
	return x;
}

/*
 * Subracts a float from every element of a matrix.
 */
mat4x4 mat4x4_sub_mat4x4(mat4x4 l, mat4x4 r)
{
	mat4x4 x;
	for(int i=0; i<16; i++)
		x.f[i] = l.f[i] - r.f[i];
	return x;
}

/*
 * Returns the product of two vectors.
 */
vec3 vec3_mul_vec3(vec3 l, vec3 r)
{
	vec3 x;
	x.x = l.x * r.x;
	x.y = l.y * r.y;
	x.z = l.z * r.z;
	return x;
}

/*
 * Returns the product of two vectors.
 */
vec2 vec2_mul_vec2(vec2 l, vec2 r)
{
	vec2 x;
	x.x = l.x * r.x;
	x.y = l.y * r.y;
	return x;
}

/*
 * Returns the quotient of two vectors.
 */
vec3 vec3_div_vec3(vec3 l, vec3 r)
{
	vec3 x;
	x.x = l.x / r.x;
	x.y = l.y / r.y;
	x.z = l.z / r.z;
	return x;
}

/*
 * Returns the sum of two vectors.
 */
vec3 vec3_add_vec3(vec3 l, vec3 r)
{
	vec3 x = { .f={ l.x + r.x, l.y + r.y, l.z + r.z }};
	return x;
}

/*
 * Returns the sum of two vectors.
 */
vec2 vec2_add_vec2(vec2 l, vec2 r)
{
	vec2 x = { .f={ l.x + r.x, l.y + r.y }};
	return x;
}

/*
 * Adds a float to every element of a vector.
 */
vec3 vec3_add_float(vec3 l, float r)
{
	vec3 x = { .f={ l.x + r, l.y + r, l.z + r }};
	return x;
}

/*
 * Adds a float to every element of a vector.
 */
vec2 vec2_add_float(vec2 l, float r)
{
	vec2 x = { .f={ l.x + r, l.y + r }};
	return x;
}


/*
 * Subtracts a float from every element of a vector.
 */
vec2 vec2_sub_vec2(vec2 l, vec2 r)
{
	vec2 x = { .f={ l.x - r.x, l.y - r.y }};
	return x;
}
/*
 * Subtracts a float from every element of a vector.
 */
vec3 vec3_sub_vec3(vec3 l, vec3 r)
{
	vec3 x = { .f={ l.x - r.x, l.y - r.y, l.z - r.z }};
	return x;
}

/*
 * Multiplies every element of a vector by a float.
 */
vec3 vec3_mul_float(vec3 l, float r)
{
	vec3 a = { .f={ l.x*r, l.y*r, l.z*r}};
	return a;
}

/*
 * Multiplies every element of a vector by a float.
 */
vec2 vec2_mul_float(vec2 l, float r)
{
	vec2 a = { .f={ l.x*r, l.y*r}};
	return a;
}


/*
 * Divides every element of a vector by a float.
 */
vec3 vec3_div_float(vec3 l, float r)
{
	vec3 a = { .f={ l.x/r, l.y/r, l.z/r}};
	return a;
}

/*
 * Multiplies two integers.
 */
int int_mul(int l, int r)
{
	return l * r;
}

/*
 * Adds two integers.
 */
int int_add(int l, int r)
{
	return l + r;
}

/*
 * Subracts an integer from another.
 */
int int_sub(int l, int r)
{
	return l - r;
}

/*
 * Divides an integer by another.
 */
int int_div(int l, int r)
{
	return l / r;
}

/*
 * Adds two vectors element-wise.
 */
int2 int2_add(int2 l, int2 r)
{
	int2 x = {l.x+r.x, l.y+r.y};
	return x;
}

/*
 * Multiplies a vector by an integer.
 */
int3 int3_mul_int(int3 l, int r)
{
	int3 x = {{l.x*r, l.y*r, l.z*r}};
	return x;
}

/*
 * Multiplies two floats.
 */
float float_mul(float l, float r)
{
	return l * r;
}

/*
 * Adds two floats.
 */
float float_add(float l, float r)
{
	return l + r;
}

/*
 * Subtracts one float from another.
 */
float float_sub_float(float l, float r)
{
	return l - r;
}

/*
 * Divides one float by another.
 */
float float_div_float(float l, float r)
{
	return l / r;
}

/*
 * Divides a float, by each element of a vec2, returning a vec2
 */
vec2 float_div_vec2(float l, vec2 r)
{
	vec2 x = { .x = l / r.x, .y = l / r.y };
	return x;
}

/*
 * Subtracts a vector from a 3-vector created from
 * the given float, elementwise.
 */
vec3 float_sub_vec3(float l, vec3 r)
{
	vec3 x = { .x = l - r.x, .y = l - r.y, .z = l - r.z};
	return x;
}
