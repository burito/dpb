/*
Copyright (c) 2013-2020 Daniel Burke

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

#include "3dmaths.h"
//#include "image.h"


struct WF_MTL
{
	float Ns, Ni;		// specular coefficient, ?
	vec3 Ka, Kd, Ks, Ke;	// ambient, diffuse, specular, emit
	float4 colour;		// colour + alpha
//	IMG *map_Ka, *map_Kd, *map_d, *map_bump;	// amb, spec, alpha, bump
	char *name;
	struct WF_MTL *next;
	int nf;
};

struct WF_TRIANGLE
{
	int3 verticies, texcoords, normals;
	vec3 normal;
	int s, g;
	struct WF_MTL *m;
};


struct packed_verts {
	vec3 p;
	vec3 n;
	vec2 uv;
};

struct WF_OBJ
{
	struct WF_MTL *m;
	struct WF_TRIANGLE *triangles;
	int3 *vf;
	int num_materials;
	int num_groups;
	int num_verticies;
	int num_normals;
	int num_texcoords;
	int num_faces;
	int num_triangles;

	vec3 *verticies;	// v
	vec2 *texcoords;	// vt
	vec3 *normals;		// vn

	int current_smoothgroup;
	int *smoothgroups;

	int  ng, ns, nm;
	vec3 *v, *vt, *vn, *fn;
	vec2 *uv;
	struct packed_verts *pv;
	char *filename;
	char **groups;
	int *sgroups;
	uint32_t va, ab, eb;
	void (*draw)(struct WF_OBJ*);
};


struct WF_ARRAY
{
	vec3 v, n;
	vec2 t;
};

int wf_count_face(struct WF_OBJ *w, char *line);

void mtl_begin(struct WF_MTL *m);
void mtl_end(void);

struct WF_OBJ* wf_load(char *filename);
void wf_free(struct WF_OBJ *w);
