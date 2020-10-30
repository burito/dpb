/*
Copyright (c) 2020 Daniel Burke

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
#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
#include <OpenGL/gl.h>
#else
#include <GL/glew.h>
#endif

#include "mesh.h"

struct IMAGE_OPENGL
{
	int2 size;
	int bpp;
	int channels;
	unsigned char *buffer;
	GLuint id;
	char *name;
};



struct MATERIAL_OPENGL
{
	// diffuse
	char *file_Kd;
	struct IMAGE_OPENGL *map_Kd;

	// specular
	char *file_Ks;
	struct IMAGE_OPENGL *map_Ks;

	char *file_d;		// mask
	char *file_bump;	// bump
};

struct MESH_OPENGL
{
	GLuint vertex_array;
	GLuint array_buffer;
	GLuint element_buffer;
	struct MATERIAL_OPENGL *materials;
	int num_materials;
	struct WF_OBJ *wf;
};

struct MESH_OPENGL* mesh_load(char *filename);
void mesh_draw(struct MESH_OPENGL *w);
void mesh_free(struct MESH_OPENGL *w);
