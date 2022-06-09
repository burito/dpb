/*
Copyright (c) 2012 Daniel Burke

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

#ifndef __3DMATHS_H_
#define __3DMATHS_H_ 1

#include <stdint.h>

#ifdef WATCOM
#define sqrtf sqrt
#define sinf sin
#define cosf cos
#define strtof(x,y) atof(x)
#endif


typedef struct float4 {
	float x;
	float y;
	float z;
	float w;
} float4;

typedef union int2 {
	struct { int32_t x, y; };
	int32_t i[2];
} int2;

typedef union {
	struct { int32_t x, y, z; };
	struct { int2 xy; int32_t iz; };
	struct { int32_t ix; int2 yz; };
	int32_t i[3];
} int3;

typedef struct byte4 {
	unsigned char x,y,z,w;
} byte4;


float finvsqrt(float x);

typedef union {
	struct { float x, y; };
	float f[2];
} vec2;

typedef union {
	struct { float x, y, z; };
	struct { vec2 xy; float fz; };
	struct { float fx; vec2 yz; };
	float f[3];
} vec3;

typedef union {
	struct { float x, y, z, w; };
	struct { vec3 xyz; float vw; };
	float f[4];
} vec4;

typedef union {
	float f[9];
	float m[3][3];
} mat3x3;

typedef union {
	float f[16];
	float m[4][4];
} mat4x4;

// taken from Valve's OpenVR headers, don't use
#ifndef __OPENVR_API_FLAT_H__
typedef struct HmdMatrix34_t
{
	float m[3][4]; //float[3][4]
} HmdMatrix34_t;

typedef struct HmdMatrix44_t
{
	float m[4][4]; //float[4][4]
} HmdMatrix44_t;
#endif

void mat4x4_print(mat4x4 m);
mat4x4 mat4x4_invert(mat4x4 m) __attribute__((const));
mat4x4 mat4x4_transpose(mat4x4 m) __attribute__((const));
mat4x4 mat4x4_identity(void) __attribute__((const));
mat4x4 mat4x4_rot_x(float t) __attribute__((const));
mat4x4 mat4x4_rot_y(float t) __attribute__((const));
mat4x4 mat4x4_rot_z(float t) __attribute__((const));
mat4x4 mat4x4_translate_vec3(vec3 v) __attribute__((const));
mat4x4 mat4x4_translate_float(float x, float y, float z) __attribute__((const));
mat4x4 mat4x4_scale_vec3(vec3 v) __attribute__((const));
mat4x4 mat4x4_scale_float(float x, float y, float z) __attribute__((const));
mat4x4 mat4x4_perspective(float near, float far, float width, float height) __attribute__((const));
mat4x4 mat4x4_glfrustum(double left, double right, double bottom, double top, double near, double far) __attribute__((const));
mat4x4 mat4x4_orthographic(float near, float far, float width, float height) __attribute__((const));
mat4x4 mat4x4_glortho(double left, double right, double bottom, double top, double near, double far) __attribute__((const));


vec3 vec3_norm(vec3 v) __attribute__((const));
vec2 vec2_norm(vec2 v) __attribute__((const));
float vec3_dot(vec3 l, vec3 r) __attribute__((const));
vec3 vec3_cross(vec3 l, vec3 r) __attribute__((const));


/*
The following functions are to be called via the
_Generic() macro's mag(), max(), mov(), mul(), add() and sub()
*/

float vec2_mag(vec2 c) __attribute__((const));
float vec3_mag(vec3 v) __attribute__((const));

float vec2_max(vec2 c) __attribute__((const));
float vec3_max(vec3 v) __attribute__((const));

mat4x4 mat4x4_mov_HmdMatrix34(HmdMatrix34_t x) __attribute__((const));
mat4x4 mat4x4_mov_HmdMatrix44(HmdMatrix44_t x) __attribute__((const));

vec3 mat3x3_mul_vec3(mat3x3 l, vec3 r) __attribute__((const));

mat4x4 mat4x4_mul_mat4x4(mat4x4 l, mat4x4 r) __attribute__((const));
vec3 mat4x4_mul_vec3(mat4x4 l, vec3 r) __attribute__((const));
mat4x4 mat4x4_mul_float(mat4x4 l, float r) __attribute__((const));
mat4x4 mat4x4_add_mat4x4(mat4x4 l, mat4x4 r) __attribute__((const));
mat4x4 mat4x4_add_float(mat4x4 l, float r) __attribute__((const));
mat4x4 mat4x4_sub_mat4x4(mat4x4 l, mat4x4 r) __attribute__((const));

vec3 vec3_mul_vec3(vec3 l, vec3 r) __attribute__((const));
vec3 vec3_mul_float(vec3 l, float r) __attribute__((const));
vec2 vec2_mul_vec2(vec2 l, vec2 r) __attribute__((const));
vec2 vec2_mul_float(vec2 l, float r) __attribute__((const));
vec3 vec3_div_vec3(vec3 l, vec3 r) __attribute__((const));
vec3 vec3_div_float(vec3 l, float r) __attribute__((const));
vec2 vec2_div_vec2(vec2 l, vec2 r) __attribute__((const));
vec2 vec2_div_float(vec2 l, float r) __attribute__((const));
vec3 vec3_add_vec3(vec3 l, vec3 r) __attribute__((const));
vec3 vec3_add_float(vec3 l, float r) __attribute__((const));
vec2 vec2_add_vec2(vec2 l, vec2 r) __attribute__((const));
vec2 vec2_add_float(vec2 l, float r) __attribute__((const));
vec2 vec2_sub_vec2(vec2 l, vec2 r) __attribute__((const));
vec3 vec3_sub_vec3(vec3 l, vec3 r) __attribute__((const));
mat3x3 vec3_jacobian_vec3(vec3 l, vec3 r) __attribute__((const));
int32_t vec3_greaterthan_vec3(vec3 l, vec3 r) __attribute__((const));
int32_t vec3_lessthan_vec3(vec3 l, vec3 r) __attribute__((const));




float float_mul(float l, float r) __attribute__((const));
float float_add(float l, float r) __attribute__((const));
float float_sub_float(float l, float r) __attribute__((const));
vec3 float_sub_vec3(float l, vec3 r) __attribute__((const));
float float_div_float(float l, float r) __attribute__((const));
vec2 float_div_vec2(float l, vec2 r) __attribute__((const));

int32_t int_mul(int32_t l, int32_t r) __attribute__((const));
int32_t int_add(int32_t l, int32_t r) __attribute__((const));
int32_t int_sub(int32_t l, int32_t r) __attribute__((const));
int32_t int_div_int(int32_t l, int32_t r) __attribute__((const));

int2 int2_add(int2 l, int2 r) __attribute__((const));
int3 int3_mul_int(int3 l, int32_t r) __attribute__((const));


double clamp_double(double value, double min, double max) __attribute__((const));
float clamp_float(float value, float min, float max) __attribute__((const));
int32_t clamp_int32_t(int32_t value, int32_t min, int32_t max) __attribute__((const));

float mix_float(float x, float y, float a) __attribute__((const));
float smoothstep_float(float edge0, float edge1, float x) __attribute__((const));


#define clamp(X,Y,Z) _Generic(X, \
	double: clamp_double, \
	float: clamp_float, \
	int32_t: clamp_int32_t \
	)(X,Y,Z)

// returns the magnitude of a vec3or
#define mag(X) _Generic(X, \
	vec2: vec2_mag, \
	vec3: vec3_mag \
	)(X)

// returns the largest item in a vector
#define vmax(X) _Generic(X, \
	vec2: vec2_max, \
	vec3: vec3_max \
	)(X)


int32_t max_int(int32_t l, int32_t r) __attribute__((const));
float max_float(float l, float r) __attribute__((const));
double max_double(double l, double r) __attribute__((const));

// returns the higher of the two arguments
#define nmax(X,Y) _Generic(X, \
	int32_t: max_int, \
	float: max_float, \
	double: max_double \
	)(X,Y)


int32_t min_int(int32_t l, int32_t r) __attribute__((const));
float min_float(float l, float r) __attribute__((const));
double min_double(double l, double r) __attribute__((const));

// returns the lower of the two arguments
#define nmin(X,Y) _Generic(X, \
	int32_t: min_int, \
	float: min_float, \
	double: min_double \
	)(X,Y)

#define mov(X) _Generic(X, \
	HmdMatrix34_t: mat4x4_mov_HmdMatrix34, \
	HmdMatrix44_t: mat4x4_mov_HmdMatrix44 \
	)(X)


#define mul(X,Y) _Generic(X, \
	mat3x3: _Generic(Y, \
		default: mat3x3_mul_vec3), \
	mat4x4: _Generic(Y, \
		mat4x4: mat4x4_mul_mat4x4, \
		vec3: mat4x4_mul_vec3, \
		default: mat4x4_mul_float), \
	vec3: _Generic(Y, \
		vec3: vec3_mul_vec3, \
		default: vec3_mul_float), \
	vec2: _Generic(Y, \
		vec2: vec2_mul_vec2, \
		default: vec2_mul_float), \
	int3: _Generic(Y, \
		default: int3_mul_int), \
	float: float_mul, \
	default: int_mul \
	)(X,Y)

#define add(X,Y) _Generic(X, \
	mat4x4: _Generic(Y, \
		mat4x4: mat4x4_add_mat4x4, \
		default: mat4x4_add_float), \
	vec3: _Generic(Y, \
		vec3: vec3_add_vec3, \
		default:vec3_add_float), \
	vec2: _Generic(Y, \
		vec2: vec2_add_vec2, \
		default:vec2_add_float), \
	float: float_add, \
	int2: int2_add, \
	default: int_add \
	)(X,Y)

#define sub(X,Y) _Generic(X, \
	mat4x4: mat4x4_sub_mat4x4, \
	vec3: vec3_sub_vec3, \
	vec2: vec2_sub_vec2, \
	float: _Generic(Y, \
		vec3: float_sub_vec3, \
		default:float_sub_float), \
	default: int_sub \
	)(X,Y)

#define div(X,Y) _Generic(X, \
	vec3: _Generic(Y, \
		vec3: vec3_div_vec3, \
		default: vec3_div_float), \
	vec2: _Generic(Y, \
		vec2: vec2_div_vec2, \
		default: vec2_div_float), \
	float: _Generic(Y, \
		vec2: float_div_vec2, \
		float: float_div_float), \
	default: int_div_int \
	)(X,Y)


#endif /* __3DMATHS_H_ */
