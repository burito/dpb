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
//	float Ns, Ni;		// specular coefficient, ?
//	vec3 Ka, Kd, Ks, Ke;	// ambient, diffuse, specular, emit
//	float4 colour;		// colour + alpha
//	IMG *map_Ka, *map_Kd, *map_d, *map_bump;	// amb, spec, alpha, bump
	char *name;
	char *map_Ka;
	char *map_Kd;
	char *map_Ks;
	char *map_d;
	char *map_bump;
	int num_triangles;
};

struct WF_TRIANGLE_CORNER
{
	int vertex;
	int texcoord;
	int normal;
	int smoothgroup;
};

struct WF_TRIANGLE
{
	int corner[3];
	vec3 normal;
	int smoothgroup;
	int material;
};


struct packed_verts {
	vec3 p;
	vec3 n;
	vec2 uv;
} __attribute__((packed));

struct smoothgroup_table {
	int id;
	int count;
	struct WF_TRIANGLE **triangles;
};

struct hash_corner { struct WF_TRIANGLE_CORNER key; int value; };

struct WF_OBJ
{
	struct WF_MTL *materials;

	int num_materials;
	int num_groups;



	int current_smoothgroup;
	int *smoothgroups;

	int current_material;

	int num_smoothgroups;
	struct smoothgroup_table *smoothgroup_table;

	struct packed_verts *vertex_buffer_data;
	int3 *index_buffer_data;

	char *filename;

 	struct hash_corner *corners;
	struct WF_TRIANGLE *triangles;
	vec3 *verticies;	// v
	vec2 *texcoords;	// vt
	vec3 *normals;		// vn

	int num_verticies;
	int num_normals;
	int num_texcoords;
	int num_corners;
	int num_triangles;

};



int wf_count_face(struct WF_OBJ *w, char *line);

void mtl_begin(struct WF_MTL *m);
void mtl_end(void);

struct WF_OBJ* wf_load(char *filename);
void wf_free(struct WF_OBJ *w);


struct WF_TRIANGLE_CORNER wf_parse_corner_string(const char *line);
