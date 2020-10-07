/*
Copyright (c) 2012-2020 Daniel Burke

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

#define _XOPEN_SOURCE 500	// for strdup on linux

#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
#include <OpenGL/gl3.h>
#else
#include <GL/glew.h>
#endif

#ifdef _WIN32
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <libgen.h> // for dirname()

#include "3dmaths.h"
#include "mesh.h"
#include "glerror.h"
#include "log.h"

#include <fast_atof.c>

#define BUF_LEN 1024
#define MAX_SMOOTHGROUPS 256

#ifdef NO
void mtl_begin(WF_MTL *m)
{
//	GLfloat spec[] = {m->Ks.x, m->Ks.y, m->Ks.z, 1.0f};
//	GLfloat amb[] = {m->Ka.x, m->Ka.y, m->Ka.z, 1.0f};
//	GLfloat diff[] = {m->Kd.x, m->Kd.y, m->Kd.z, 1.0f};
//	GLfloat emit[] = {m->Ke.x, m->Ke.y, m->Ke.z, 1.0f};
//	glMateriali(GL_FRONT, GL_SHININESS, m->Ns);
//	glMaterialfv(GL_FRONT, GL_SPECULAR, spec);
//	glMaterialfv(GL_FRONT, GL_AMBIENT, amb);
//	glMaterialfv(GL_FRONT, GL_DIFFUSE, diff);
//	glMaterialfv(GL_FRONT, GL_EMISSION, emit);
//	glColor4fv(diff);
//	glColor4f(m->colour.x, m->colour.y, m->colour.z, m->colour.w);

	glActiveTexture(GL_TEXTURE0);
	if(m->map_Kd)
	{	glEnable(GL_TEXTURE_2D); glBindTexture(GL_TEXTURE_2D, m->map_Kd->id); }
	else
	{	glDisable(GL_TEXTURE_2D); glBindTexture(GL_TEXTURE_2D, 0); }

	if(m->map_d)
	{
//		glActiveTexture(GL_TEXTURE2);
//		glEnable(GL_TEXTURE_2D);
//		glBindTexture(GL_TEXTURE_2D, m->map_d->id);
	}
	if(m->map_bump)
	{
//		glActiveTexture(GL_TEXTURE3);
//		glEnable(GL_TEXTURE_2D);
//		glBindTexture(GL_TEXTURE_2D, m->map_bump->id);

	}
	glActiveTexture(GL_TEXTURE0);

}
void mtl_end(void)
{
	GLfloat amb[] = {0.2f, 0.2f, 0.2f, 1.0f};
	GLfloat diff[] = {0.8f, 0.8f, 0.8f, 1.0f};
	GLfloat zero[] = {0.0f, 0.0f, 0.0f, 1.0f};
#ifndef __APPLE__
	glMateriali(GL_FRONT, GL_SHININESS, 0);
	glMaterialfv(GL_FRONT, GL_AMBIENT, amb);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, diff);
	glMaterialfv(GL_FRONT, GL_SPECULAR, zero);
	glMaterialfv(GL_FRONT, GL_EMISSION, zero);

	glColor4f(1,1,1,1);
	glMateriali(GL_FRONT, GL_SHININESS, 0);
	glActiveTexture(GL_TEXTURE0);
	glDisable(GL_TEXTURE_2D);glBindTexture(GL_TEXTURE_2D, 0);
	glActiveTexture(GL_TEXTURE1);
	glDisable(GL_TEXTURE_2D);glBindTexture(GL_TEXTURE_2D, 0);
	glActiveTexture(GL_TEXTURE2);
	glDisable(GL_TEXTURE_2D);glBindTexture(GL_TEXTURE_2D, 0);
	glActiveTexture(GL_TEXTURE3);
	glDisable(GL_TEXTURE_2D);glBindTexture(GL_TEXTURE_2D, 0);
	glActiveTexture(GL_TEXTURE0);
#endif
}


static void wf_texvec2s(WF_OBJ *w)
{
	if(!w)return;
	if(!w->nt)return;
	const int size = w->nv * sizeof(vec2);
	w->uv = malloc(size);
	memset(w->uv, 0, size);

	int uvcopy = 0;

	for(int i=0; i<w->nf; i++)
	{
		if(mag(w->uv[w->f[i].f.x]) > 0.1)
		{
			uvcopy++;
		}
		w->uv[w->f[i].f.x] = w->vt[w->f[i].t.x].xy;
		w->uv[w->f[i].f.y] = w->vt[w->f[i].t.y].xy;
		w->uv[w->f[i].f.z] = w->vt[w->f[i].t.z].xy;
	}
	log_debug("UV's copied, wanted %d verts", uvcopy);
}

#endif
/*
void mtl_free(struct WF_MTL *m)
{
	if(!m)return;
	if(m->name)free(m->name);
	if(m->map_Ka)img_free(m->map_Ka);
	if(m->map_Kd)img_free(m->map_Kd);
	if(m->map_d)img_free(m->map_d);
	if(m->map_bump)img_free(m->map_bump);
	free(m);

}


struct WF_MTL* mtl_newmtl(char *hostpath, FILE *fptr, char *name)
{
	struct WF_MTL *m = malloc(sizeof(WF_MTL));
	if(m == NULL)
	{
		log_fatal("malloc(WF_MTL) = %s", strerror(errno));
		goto MTL_NEWMTL_MALLOC;
	}
	memset(m, 0, sizeof(struct WF_MTL));

	m->colour.x = m->colour.y = m->colour.z = m->colour.w = 1.0f;
	m->Ns = 50.0f;							// specular coefficient
	m->Ka.x = m->Ka.y = m->Ka.z = 0.2f;		// ambient
	m->Kd.x = m->Kd.y = m->Kd.z = 0.8f;		// diffuse
	m->Ks.x = m->Ks.y = m->Ks.z = 0.2f;		// specular
	m->name = hcopy(name);
	char buf[BUF_LEN];
	float *targetf;
	vec3 *targetf3;
	IMG **targeti;
	char *path;

	int i=0;

	long last = ftell(fptr);

	while(fgets(buf, BUF_LEN, fptr))
	{
		tailchomp(buf);
		if(strlen(buf) < 3)break;
		for(i = 0; whitespace(buf[i]); i++);

		switch(buf[i]) {
		case 'N':
			switch(buf[i+1]) {
			default:
			case 's': targetf = &m->Ns; break;		// specular coefficient
			case 'i': targetf = &m->Ni; break;		// ?
			}
			*targetf = fast_atof(buf+i+3);
			break;
		case 'K':
			targetf3 = 0;
			switch(buf[i+1]) {
			case 'a': targetf3 = &m->Ka; break;		// ambient
			case 'd': targetf3 = &m->Kd; break;		// diffuse
			case 's': targetf3 = &m->Ks; break;		// specular
			case 'e': targetf3 = &m->Ke; break;		// emissive
			}
			if(!targetf3)break;
			i+=2;
			sscanf(buf+i, "%f %f %f", &targetf3->x, &targetf3->y, &targetf3->z);
			break;
		case 'm':
			switch(buf[i+5]) {
			case 'a': targeti = NULL; break; //targeti = &m->map_Ka; break;		// ambient
			case 'd': targeti = &m->map_Kd; break;		// diffuse
//			case 'u': targeti = &m->map_bump; break;	// bump
			default:  targeti = NULL; break;
			}
			if(!targeti)break;
			while(!whitespace(buf[i]))i++;
			i++;
			path = repath(hostpath, buf+i);
			*targeti = img_load(path);
			free(path);
			break;
		case 'd':
			m->colour.w = fast_atof(buf+i+2);		// alpha
			break;
		case 'T':									// colour?
		sscanf(buf+i, "Tf %f %f %f", &m->colour.x, &m->colour.y, &m->colour.z);
			break;
		case 'n':
			fseek(fptr, last, SEEK_SET);
			return m;
		}
		last = ftell(fptr);
	}
	return m;

	free(m);
MTL_NEWMTL_MALLOC:
	return NULL;
}
*/



/*

struct WF_MTL* find_material(struct WF_MTL *m, char *name)
{
	if(!m)return 0;
	if(!name)return 0;
	if(!m->name)return 0;
	while(strcmp(m->name, name))
	{
		if(!m->next)return 0;
		m = m->next;
		if(!m->name)return 0;
	}
	return m;
}

*/

/*
 * Interleave the Verticies, Normals and Texcoords into a buffer appropriate
 * for rendering on a GPU.
 */
void wf_interleave(struct WF_OBJ *w)
{
	if(w == NULL)return;
	log_debug("At interleave we have %d/%d/%d", w->num_verticies, w->num_normals, w->num_texcoords);
	if(w->num_verticies <= 0)return;
	if(w->num_triangles <= 0) return;

	w->vertex_buffer_data = malloc( w->num_verticies * sizeof(struct packed_verts) );
	if(w->vertex_buffer_data == NULL)
	{
		log_fatal("malloc(vertex_buffer_data) = %s", strerror(errno));
		goto WF_INTERLEAVE_MALLOC_VERTEXBUFFER;
	}
	memset(w->vertex_buffer_data, 0, w->num_verticies * sizeof(struct packed_verts) );

	// interleave the verts, normals and texture vec2s for the VBO
	for(int i=0; i<w->num_verticies; i++)
	{
		w->vertex_buffer_data[i].p = w->verticies[i];
		if(w->num_verticies == w->num_normals)
		{
			w->vertex_buffer_data[i].n = w->normals[i];
		}
		if(w->texcoords)
		{
//			w->vertex_buffer_data[i].uv = w->texcoords[i];
		}
	}
	log_debug("Interleaved ok!");

	w->index_buffer_data = malloc( w->num_triangles*sizeof(int3) );
	if(w->index_buffer_data == NULL)
	{
		log_fatal("malloc(index_buffer_data) = %s", strerror(errno));
		goto WF_INTERLEAVE_MALLOC_INDEXBUFFER;
	}

	// copy the faces to the index buffer
	// TODO: batch these by material
	for(int i=0; i<w->num_triangles; i++)
	for(int j=0; j<3; j++)
	{
		w->index_buffer_data[i].i[j] = w->triangles[i].corner[j].vertex;
	}

	log_debug("Interleaved faces ok!");

	return;

	free(w->index_buffer_data);
WF_INTERLEAVE_MALLOC_INDEXBUFFER:
	free(w->vertex_buffer_data);
	w->vertex_buffer_data = NULL;
WF_INTERLEAVE_MALLOC_VERTEXBUFFER:
	return;


}



/*
 * Generate per-vertex normals, from existing face normals.
 */
void wf_vertex_normals(struct WF_OBJ *w)
{
	if(!w)return;

	// we can throw away the existing normals
	if(w->normals)
	{
		free(w->normals);
		w->normals = NULL;
	}
	w->num_normals = w->num_verticies;

	w->normals = malloc( sizeof(vec3)*w->num_normals );
	if(w->normals == NULL)
	{
		log_fatal("malloc(normals) = %d", strerror(errno));
		return;
	}
	memset( w->normals, 0, sizeof(vec3)*w->num_normals );

	// add the normal of each face to the vertex normal
	for(int i=0; i<w->num_triangles; i++)
	for(int j=0; j<3; j++)
	{
		struct WF_TRIANGLE_CORNER *corner = &w->triangles[i].corner[j];

		w->normals[corner->vertex] = add(w->normals[corner->vertex], w->triangles[i].normal);
	}

	// normalise the result
	for(int i=0; i<w->num_verticies; i++)
	{
		// if it's for some reason 0, don't do it
		if( vec3_mag( w->normals[i] ) > 0.0001)
			w->normals[i] = vec3_norm(w->normals[i]);
	}
}

/*
 * Generate per face normals.
 */
static void wf_face_normals(struct WF_OBJ *w)
{
	if(!w)return;
	if(!w->num_triangles)return;

	vec3 a, b, t;

	for(int i=0; i<w->num_triangles; i++)
	{
		struct WF_TRIANGLE *triangle = &w->triangles[i];
		a = sub( w->verticies[triangle->corner[0].vertex], w->verticies[triangle->corner[1].vertex] );
		b = sub( w->verticies[triangle->corner[0].vertex], w->verticies[triangle->corner[2].vertex] );
		t = vec3_cross( a, b );
		triangle->normal = vec3_norm( t );
	}
}


/*
 * Calculate the normals of the object.
 */
void wf_normals(struct WF_OBJ *w)
{
	if(!w)return;

	w->num_smoothgroups = 0; // there's always 1, the 0th group, aka "off"
	w->smoothgroup_table = malloc( sizeof(struct smoothgroup_table) * MAX_SMOOTHGROUPS);
	if(w->smoothgroup_table == NULL)
	{
		log_fatal("malloc(smoothgroup_table) = %d", strerror(errno));
		return; // TODO: return error
	}

	memset(w->smoothgroup_table, 0, sizeof(struct smoothgroup_table) * MAX_SMOOTHGROUPS);
	for(int i=0; i< w->num_triangles; i++)
	{
		struct WF_TRIANGLE *triangle = &w->triangles[i];
		// find an existing entry that matches
		for(int j=0; j<w->num_smoothgroups; j++)
		{
			if( triangle->smoothgroup == w->smoothgroup_table[j].id )
			{// it matches an existing entry
				w->smoothgroup_table[j].count++;
				// update the triangle
				triangle->smoothgroup = j;
				goto WF_NORMALS_SMOOTHGROUP_CONTINUE;
			}
		}
		// we've search everywhere, this smoothgroup is the first of its kind
		if(w->num_smoothgroups == MAX_SMOOTHGROUPS-1)
		{// woops, out of empty table entries, jam it in the last slot
			log_warning("Assumption broken, increase MAX_SMOOTHGROUPS.");
			w->smoothgroup_table[w->num_smoothgroups].count++;
			triangle->smoothgroup = MAX_SMOOTHGROUPS-1;
		}
		else
		{// still space left, record it properly
			w->smoothgroup_table[w->num_smoothgroups].id = triangle->smoothgroup;
			w->smoothgroup_table[w->num_smoothgroups].count++;
			triangle->smoothgroup = w->num_smoothgroups;
			w->num_smoothgroups++;
		}
		// and onto the next
WF_NORMALS_SMOOTHGROUP_CONTINUE:
		continue;
	}

	// we're doing face normals regardless
	wf_face_normals(w);

	// if there are no smoothgroups, or only one used smoothgroup, then it's easy!
	if( w->num_smoothgroups == 1 && w->smoothgroup_table[0].id != 0 )
	{
		wf_vertex_normals(w);
		wf_interleave(w);
		return;
	}

	// we know it's the default smoothgroup (off)
	if( w->num_smoothgroups == 1 )
	{
		w->vertex_buffer_data = malloc(sizeof(struct packed_verts) * w->num_triangles * 3);
		if(w->smoothgroup_table == NULL)
		{
			log_fatal("malloc(vertex_buffer_data) = %d", strerror(errno));
			return; // TODO: return error
		}
		memset(w->vertex_buffer_data, 0, sizeof(struct packed_verts) * w->num_triangles * 3);

		w->index_buffer_data = malloc( sizeof(int3) * w->num_triangles );
		if(w->smoothgroup_table == NULL)
		{
			log_fatal("malloc(index_buffer_data) = %d", strerror(errno));
			return; // TODO: return error
		}
		memset(w->index_buffer_data, 0, sizeof(int3) * w->num_triangles);

		for(int i=0; i<w->num_triangles; i++)
		for(int j=0; j<3; j++)
		{
			w->index_buffer_data[i].i[j] = i*3 + j;

			w->vertex_buffer_data[i*3+j].p = w->verticies[w->triangles[i].corner[j].vertex];
			if( w->texcoords > 0 )
				w->vertex_buffer_data[i*3+j].uv = w->texcoords[w->triangles[i].corner[j].texcoord];
			w->vertex_buffer_data[i*3+j].n = w->triangles[i].normal;
		}

		w->num_verticies = w->num_triangles * 3;
		return;
	}

	// Print out the smoothgroups for debugging
	for(int i=0; i<w->num_smoothgroups; i++)
	{
		struct smoothgroup_table *entry = &w->smoothgroup_table[i];
		log_info("Smoothgroup[%d] = {%d, %d}", i, entry->id, entry->count);
	}


	// allocate memory for the smoothgroup triangle lists
	for(int i=0; i<w->num_smoothgroups; i++)
	{
		struct smoothgroup_table *entry = &w->smoothgroup_table[i];
		entry->triangles = malloc( entry->count * sizeof(struct WF_TRIANGLE*) );
		memset( entry->triangles, 0, entry->count * sizeof(struct WF_TRIANGLE*) );
		entry->count = 0;
	}

	// sort the triangles into the smoothgroup lists
	for(int i=0; i< w->num_triangles; i++)
	{ // we relabelled all the triangles smoothgroups, so we don't need checking here
		struct WF_TRIANGLE *triangle = &w->triangles[i];
		struct smoothgroup_table *table = &w->smoothgroup_table[triangle->smoothgroup];
		table->triangles[ table->count ] = triangle;
		table->count++;
	}

	//int *vert_sg = malloc( w->num_verticies * w->num_smoothgroups

	// every triangle that is not in a smoothgroup, gets its own verticies


	// find out how many triangles each vertex is in
	int *vertex_face_count = malloc(sizeof(int) * w->num_verticies);
	memset(vertex_face_count, 0, sizeof(int) * w->num_verticies);
	for(int i=0; i<w->num_triangles; i++)
	{
		struct WF_TRIANGLE *triangle = &w->triangles[i];
		for(int j=0; j<3; j++)
		{
			vertex_face_count[triangle->corner[j].vertex]++;
		}
	}

	int min_faces = vertex_face_count[0];
	int max_faces = 0;
	for(int i=0; i<w->num_verticies; i++)
	{
		if( max_faces < vertex_face_count[i] )
			max_faces = vertex_face_count[i];
		if( min_faces > vertex_face_count[i] )
			min_faces = vertex_face_count[i];
	}

	log_debug("min = %d, max = %d", min_faces, max_faces);



	// now we know how many smoothgroups there are, we can start sorting verticies
	// if a vertex is in more than one smoothgroup, it must be duplicated
	// because we are rendering with per-vertex normals
	// create a table, listing which smoothgroups each vertex is in
	for(int i=0; i<w->num_triangles; i++)
	{

	}

	// now normalise the smoothgroup numbers to something sane
	//  walk through the faces, identifying unique smoothgroups



	// now allocate a 2d array of the form array[num_verticies][num_smoothgroups]



/*
	if(w->vn)free(w->vn);
	w->vn = malloc(sizeof(vec3)*w->nv);
	memset(w->vn, 0, sizeof(vec3)*w->nv);
	w->nn = w->nv;

	wf_face_normals(w);
	wf_vertex_normals(w);
*/

}


/*
 * Finds the total volume needed to contain the object, and scales it to 1.
 */
static void wf_bound(struct WF_OBJ *w)
{
	if(!w)return;
	vec3 *v = w->verticies;
	if(!v)return;
	vec3 min, max, size, mid;
	max = min = v[0];
	for(int i=0; i<w->num_verticies; i++)
	{
		if(v[i].x < min.x)min.x = v[i].x;
		if(v[i].x > max.x)max.x = v[i].x;

		if(v[i].y < min.y)min.y = v[i].y;
		if(v[i].y > max.y)max.y = v[i].y;

		if(v[i].z < min.z)min.z = v[i].z;
		if(v[i].z > max.z)max.z = v[i].z;
	}

	size = sub(max, min);
	float longest = vmax(size);
	mid = sub(longest, size);
	mid = mul(mid, 0.5);
//	longest = longest * (12.0 / 14.0);
	for(int i=0; i<w->num_verticies; i++)
	{
		v[i] = sub(v[i], min);
		v[i] = add(v[i], mid);
		v[i] = mul(v[i], 1.0 / longest);
		v[i] = mul(v[i], (12.0 / 14.0));
		v[i] = add(v[i], (1.0 / 14.0));
	}

	log_debug("Volume = (%f, %f, %f)", size.x, size.y, size.z);
}

/*******************************************************************************
 * Here begins the code for parsing the Wavefront file itself.
 ******************************************************************************/


void wf_count_vertex(struct WF_OBJ *w, char *line)
{
	w->num_verticies++;
	return;
}

void wf_count_normal(struct WF_OBJ *w, char *line)
{
	w->num_normals++;
	return;
}

void wf_count_texcoord(struct WF_OBJ *w, char *line)
{
	w->num_texcoords++;
	return;
}

int wf_count_face(struct WF_OBJ *w, char *line)
{
	w->num_faces++;
	line++;

	// get to the first block
	while( *line == ' ' || *line == '\t' ) line++;

	// get to the second block
	while( !(*line == ' ' || *line == '\t') ) line++;
	while( *line == ' ' || *line == '\t' ) line++;

	// get to the third block
	while( !(*line == ' ' || *line == '\t') ) line++;
	while( *line == ' ' || *line == '\t' ) line++;

	int num_triangles = 0;

	while( !(*line == 0 || *line == '\n') )
	{
		num_triangles++;
		while( !(*line == ' ' || *line == '\t' || *line == 0 || *line == '\n') ) line++;
		while( *line == ' ' || *line == '\t' ) line++;

	}

	w->num_triangles += num_triangles;
//	w->smoothgroups[w->current_smoothgroup] += num_triangles;
	return num_triangles;
}

void wf_count_group(struct WF_OBJ *w, char *line)
{
	w->num_groups++;
	return;
}


void wf_count_smoothgroup(struct WF_OBJ *w, char *line)
{
/*
	int smoothgroup = 0;
	int found = sscanf(line, "s %d", &smoothgroup);
	if(!found)
	{
		smoothgroup = 0;
//		log_debug("smoothgroup disabled");
	}
	else
	{
		if(smoothgroup > 255) smoothgroup = 255;
//		log_debug("smoothgroup set to %d", smoothgroup);
	}
	w->current_smoothgroup = smoothgroup;
*/
	return;
}

void wf_count_material(struct WF_OBJ *w, char *line)
{
//	w->num_materials++;
	return;
}

/* all of the "parse" functions call their respective "count" function
 * to eliminate counting errors, or at least ensure they're consistent
 */

/*
 * Read the mtl library file.
 */
void wf_parse_mtllib(struct WF_OBJ *w, char *line)
{
	// while it's not whitespace
	while( !(*line == ' ' || *line == '\t') ) line++;
	// while it is whitespace
	while( (*line == ' ' || *line == '\t') ) line++;

	{
		char *tmpline = line;
		while( !(*tmpline == '\n' || *tmpline == '\r' || *tmpline == 0) ) tmpline++;
		*tmpline = 0;
	}
	// dirname() may modify it's argument, so make a copy
	char *tmp = strdup(w->filename);
	char *filedir = dirname(tmp);
	char filepath[1024];
	snprintf(filepath, 1024, "%s/%s", filedir, line);

	log_info("Loading Wavefront MTL(\"%s\");", filepath);

	FILE *fptr = fopen(filepath, "r");
	if(!fptr)
	{
		log_error("fopen(\"%s\") = %s", filepath, strerror(errno));
		goto WF_PARSE_MTLLIB_FOPEN;
	}

	// count the materials
	w->num_materials = 0;
	char linestr[1024];
	while(fgets(line, 1024, fptr))
	{
		while( *line == ' ' || *line == '\t' ) line++;
		switch(line[0]) {
		case 'n':
			if(strstr(line, "newmtl"))
				w->num_materials++;
			break;
		}
	}

	log_info("Found %d materials.", w->num_materials);
	w->materials = malloc( sizeof(struct WF_MTL) * w->num_materials );
	if(w->materials == NULL)
	{
		log_fatal("malloc(materials) = %s", strerror(errno));
		goto WF_PARSE_MTLLIB_MALLOC;
	}
	w->num_materials = 0;

	// now that we know how many, and have allocated, parse them
	fseek(fptr, SEEK_SET, 0);
	while(fgets(line, 1024, fptr))
	{
		while( *line == ' ' || *line == '\t' ) line++;
		switch(line[0]) {
		case 'n':
			if(strstr(line, "newmtl"))
			{
				// while it is not whitespace
				while( !(*line == ' ' || *line == '\t') ) line++;
				// continue until the whitespace ends
				while( *line == ' ' || *line == '\t' ) line++;
				// line now equals the start of the name
				char *tmpline = line;
				while( !(*tmpline == '\n' || *tmpline == '\r' || *tmpline == 0) ) tmpline++;
				*tmpline = 0;
				// the name is now null-terminated, copy it
				w->materials[w->num_materials].name = strdup(line);
				w->current_material = w->num_materials;
				w->num_materials++;
			}
			break;
		case 'm':
			if(strstr(line, "map_"))
			{ // this is a texture map of some kind
				line += 5;
				switch(line[0]) {
				case 'a':	// map_Ka - ambient
				case 's':	// map_Ks - specular
				case 'u':	// map_bump - bump map
				case ' ':	// map_d - stencil map
				default:
					break; // we don't care yet
				case 'd':	// map_Kd - diffuse
					// while it is not whitespace
					while( !(*line == ' ' || *line == '\t') ) line++;
					// continue until the whitespace ends
					while( *line == ' ' || *line == '\t' ) line++;
					// line now equals the start of the name
					char *tmpline = line;
					while( !(*tmpline == '\n' || *tmpline == '\r' || *tmpline == 0) ) tmpline++;
					*tmpline = 0;
					// the name is now null-terminated, copy it
					snprintf(filepath, 1024, "%s/%s", filedir, line);
					w->materials[w->current_material].filename = strdup(filepath);
					break;
				}
			}
		}
	}

	for(int i=0; i<w->num_materials; i++)
	{
		log_debug("Material[%d] = (%s, %s)", i, w->materials[i].name, w->materials[i].filename);
	}

	goto WF_PARSE_MTLLIB_SUCCESS;

WF_PARSE_MTLLIB_MALLOC:
	w->num_materials = 0;
WF_PARSE_MTLLIB_SUCCESS:
	fclose(fptr);
WF_PARSE_MTLLIB_FOPEN:
	free(tmp);
	return;
}

/*
 * Set the currently active material.
 */
void wf_parse_usemtl(struct WF_OBJ *w, char *line)
{
	// while it's not whitespace
	while( !(*line == ' ' || *line == '\t') ) line++;
	// while it is whitespace
	while( (*line == ' ' || *line == '\t') ) line++;

}


/*
 * Parse a vertex of the form "v 1.0 2.0 3.0"
*/
void wf_parse_vertex(struct WF_OBJ *w, char *line)
{
	line++;
	for(int i=0; i<3; i++)
	{
		while( (*line == ' ' || *line == '\t') ) line++;
		w->verticies[w->num_verticies].f[i] = fast_atof(line);
		while( !(*line == ' ' || *line == '\t' || *line == '\n' || *line == '\r' || *line == 0 ) ) line++;
	}
	wf_count_vertex(w, line);
	return;
}

/*
 * Parse a normal of the form "vn 1.0 0.0 0.0"
 * It *should* be a unit vector, but we won't assume that.
*/
void wf_parse_normal(struct WF_OBJ *w, char *line)
{
	line+=2;
	for(int i=0; i<3; i++)
	{
		while( (*line == ' ' || *line == '\t') ) line++;
		w->normals[w->num_normals].f[i] = fast_atof(line);
		while( !(*line == ' ' || *line == '\t' || *line == '\n' || *line == '\r' || *line == 0 ) ) line++;
	}
	wf_count_normal(w, line);
	return;
}

/*
 * Parse a Texture Coordinate of the form "vt 0.5 0.5 0.0"
 * We are only interested in 2D texcoords, so ignore the last value
 */
void wf_parse_texcoord(struct WF_OBJ *w, char *line)
{
	line+=2;
	for(int i=0; i<2; i++)
	{
		while( (*line == ' ' || *line == '\t') ) line++;
		w->texcoords[w->num_texcoords].f[i] = fast_atof(line);
		while( !(*line == ' ' || *line == '\t' || *line == '\n' || *line == '\r' || *line == 0 ) ) line++;
	}
	wf_count_texcoord(w, line);
	return;
}

/*
 * helper function for wf_parse_face() to read a corner
 */
void wf_parse_face_corner(struct WF_OBJ *w, struct WF_TRIANGLE_CORNER *corner, char *line)
{
	int value = atoi(line);
	if(value < 0) value = value + w->num_verticies;
	else value--;
	corner->vertex = value;

	// find the next non-number (or hyphen)
	while( (*line >= '0' && *line <= '9') || *line == '-' ) line++;

	// if it's not a '/', then we're done
	if(*line != '/')return;

	// otherwise, we can treat it as another value
	line++;
	value = atoi(line);
	if(value < 0) value = value + w->num_texcoords;
	else value--;
	corner->texcoord = value;

	// if we just read a number, move past it
	while( (*line >= '0' && *line <= '9') || *line == '-' ) line++;

	// if we don't find a second slash, we're done
	if( *line != '/' )return;

	// otherwise, read the last value
	line++;
	value = atoi(line);
	if(value < 0) value = value + w->num_normals;
	else value--;
	corner->normal = value;
}

/*
 * Parse a face record of the form "f 1/1/1 2/2/2 3/3/3 -1/-1/-1".
 * Groups being "vertex/texcoord/normal".
 * Assume all polygons are convex, so can convert them to triangle fans.
 * Negative indicies indicate "relative to the last defined item",
 * which is why we're keeping a running count.
 * Also possible "1/2", "1//2"
 */
void wf_parse_face(struct WF_OBJ *w, char *line)
{
	int num_triangles =  wf_count_face(w, line);
	int offset = w->num_triangles - num_triangles;

	line++;
	// parse the first triangle
	for(int i=0; i<3; i++)
	{
		// find the next not space
		while( *line == ' ' || *line == '\t' || *line == 0 ) line++;

		wf_parse_face_corner(w, &w->triangles[offset].corner[i], line);

		// find the next space
		while( !(*line == ' ' || *line == '\n' || *line == '\r' || *line == 0) ) line++;
	}
	// write which smoothgroup this face is in
	w->triangles[offset].smoothgroup = w->current_smoothgroup;

	// now parse the remaining triangles
	int first = offset;
	offset++;
	for(; offset < w->num_triangles; offset++)
	{
		// write which smoothgroup this face is in
		w->triangles[offset].smoothgroup = w->current_smoothgroup;

		// the first two corners go in every triangle
		w->triangles[offset].corner[0] = w->triangles[first].corner[0];
		w->triangles[offset].corner[1] = w->triangles[first].corner[2];

		// find the next not space
		while( *line == ' ' || *line == '\t' || *line == 0 ) line++;

		wf_parse_face_corner(w, &w->triangles[offset].corner[2], line);

		// find the next space
		while( !(*line == ' ' || *line == '\n' || *line == '\r' || *line == 0) ) line++;
	}

	return;
}

/*
 * I don't know what purpose groups serve, insofar as rendering is concerned
 */
void wf_parse_group(struct WF_OBJ *w, char *line)
{
	wf_count_group(w, line);
	return;
}


void wf_parse_smoothgroup(struct WF_OBJ *w, char *line)
{
	wf_count_smoothgroup(w, line);

	line++;
	// find the next non space character
	while( (*line == ' ' || *line == '\t') ) line++;

	w->current_smoothgroup = atoi(line);
//	log_info("sg found = %d", w->current_smoothgroup);
	return;
}

/*
 * Parse a material of the form "usemtl mat_name".
 */
void wf_parse_material(struct WF_OBJ *w, char *line)
{
//	wf_count_material(w, line);
	return;
}


int wf_alloc_first_buffers(struct WF_OBJ *w)
{
	// alloc the memory
	if(w->num_verticies)
	{
		w->verticies = malloc( sizeof(vec3)*w->num_verticies );
		if(w->verticies == NULL)
		{
			log_fatal("malloc(verticies) = %d", strerror(errno));
			goto WF_ALLOC_VERTICIES;
		}
		memset( w->verticies, 0, sizeof(vec3)*w->num_verticies );
	}
	if(w->num_texcoords)
	{
		w->texcoords = malloc( sizeof(vec2)*w->num_texcoords );
		if(w->texcoords == NULL)
		{
			log_fatal("malloc(texcoords) = %d", strerror(errno));
			goto WF_ALLOC_TEXCOORDS;
		}
		memset( w->texcoords, 0, sizeof(vec2)*w->num_texcoords );
	}
	if(w->num_normals)
	{
		w->normals = malloc( sizeof(vec3)*w->num_normals );
		if(w->normals == NULL)
		{
			log_fatal("malloc(normals) = %d", strerror(errno));
			goto WF_ALLOC_NORMALS;
		}
		memset( w->normals, 0, sizeof(vec3)*w->num_normals );
	}
	if(w->num_triangles)
	{
		w->triangles = malloc( sizeof(struct WF_TRIANGLE) * w->num_triangles );
		if(w->triangles == NULL)
		{
			log_fatal("malloc(triangles) = %d", strerror(errno));
			goto WF_ALLOC_TRIANGLES;
		}
		memset( w->triangles, 0, sizeof(struct WF_TRIANGLE) * w->num_triangles );
	}

	/* Replaces this horrible code
	// alloc the memory
	if(w->nv)w->v = malloc(sizeof(vec3)*w->nv);
	if(w->nt)w->vt = malloc(sizeof(vec3)*w->nt);
	if(w->nn)w->vn = malloc(sizeof(vec3)*w->nn);
	if(w->nf)
	{
		w->f = malloc(sizeof(WF_FACE)*w->nf);
		memset(w->f, 0, sizeof(WF_FACE)*w->nf);
	}
	if(w->ng)w->groups = malloc(sizeof(char*)*w->ng);
	if(w->ns)w->sgroups = malloc(sizeof(int)*w->ns);
	*/

	return 0;


	if(w->triangles)
	{
		free(w->triangles);
		w->triangles = NULL;
	}
WF_ALLOC_TRIANGLES:
	if(w->normals)
	{
		free(w->normals);
		w->normals = NULL;
	}
WF_ALLOC_NORMALS:
	if(w->texcoords)
	{
		free(w->texcoords);
		w->texcoords = NULL;
	}
WF_ALLOC_TEXCOORDS:
	free(w->verticies);
	w->verticies = NULL;
WF_ALLOC_VERTICIES:
	return 1;
}

struct WF_OBJ* wf_parse(char *filename)
{
	// open the file
	log_info("Loading Wavefront OBJ(\"%s\");", filename);
	FILE *fptr = fopen(filename, "r");
	if(fptr == NULL)
	{
		log_error("fopen(\"%s\") %s", filename, strerror(errno));
		goto WF_LOAD_FOPEN;
	}
	// allocate the structure
	struct WF_OBJ *w = malloc(sizeof(struct WF_OBJ));
	if(w == NULL)
	{
		log_fatal("malloc() = %s", strerror(errno));
		goto WF_LOAD_MALLOC_STRUCT;
	}
	memset(w, 0, sizeof(struct WF_OBJ));
	// copy the filename into the structure
	w->filename = strdup(filename);
	if(w->filename == NULL)
	{
		log_fatal("strdup() = %s", strerror(errno));
		goto WF_LOAD_STRDUP_FILENAME;
	}

	// allocate smoothgroup array
	w->smoothgroups = malloc(sizeof(int) * MAX_SMOOTHGROUPS);
	if(w->smoothgroups == NULL)
	{
		log_fatal("malloc(smoothgroups) = %s", strerror(errno));
		goto WF_LOAD_ALLOC_SMOOTHGROUPS;
	}
	memset(w->smoothgroups, 0, sizeof(int)*MAX_SMOOTHGROUPS);

	// count the verts, faces, objects, materials
	char line[1024];
	while(fgets(line, 1024, fptr))
	switch(line[0]) {
	case 'm':
//		if(strstr(line, "mtllib"))
//			wf_count_material(w, line);
//			log_info("mtl");
		break;
	case 'v':	// vertex data
		switch(line[1]) {
		case ' ': wf_count_vertex(w, line); break;
		case 'n': wf_count_normal(w, line); break;
		case 't': wf_count_texcoord(w, line); break;
		}
		break;
	case 'f':	// face
		wf_count_face(w, line);
		break;
	case 'o':
	case 'g': wf_count_group(w, line); break;
	case 's': wf_count_smoothgroup(w, line); break;
	}

	log_info("Verticies=%d, Normals=%d, Texcoords=%d, Faces=%d, Triangles=%d, Groups=%d, Materials=%d",
		w->num_verticies, w->num_normals, w->num_texcoords, w->num_faces,
		w->num_triangles, w->num_groups, w->num_materials);

	if( wf_alloc_first_buffers(w) )
	{
		log_fatal("Allocating wavefront item buffers failed");
		goto WF_LOAD_ALLOC;
	}


	// reset to 0 now that it's been allocated
	w->num_materials = 0;
	w->num_verticies = 0;
	w->num_normals = 0;
	w->num_texcoords = 0;
	w->num_faces = 0;
	w->num_triangles = 0;
	w->num_groups = 0;
	w->current_smoothgroup = 0;
	w->current_material = 0;

	fseek(fptr, 0, SEEK_SET);
	while(fgets(line, 1024, fptr))
	switch(line[0]) {
	case 'm':
		if(strstr(line, "mtllib"))
			wf_parse_mtllib(w, line);
//			log_info("mtl");
		break;
	case 'u':
		if(strstr(line, "usemtl"))
			wf_parse_usemtl(w, line);
//			log_info("mtl");
		break;
	case 'v':	// vertex data
		switch(line[1]) {
		case ' ': wf_parse_vertex(w, line); break;
		case 'n': wf_parse_normal(w, line); break;
		case 't': wf_parse_texcoord(w, line); break;
		}
		break;
	case 'f':	// face
		wf_parse_face(w, line);
		break;
	case 'o':
	case 'g': wf_parse_group(w, line); break;
	case 's': wf_parse_smoothgroup(w, line); break;
	}

	return w;

	free(w->smoothgroups);
WF_LOAD_ALLOC_SMOOTHGROUPS:
	if(w->normals)
	{
		free(w->normals);
		w->normals = NULL;
	}
	if(w->texcoords)
	{
		free(w->texcoords);
		w->texcoords = NULL;
	}
	free(w->verticies);
WF_LOAD_ALLOC:
	free(w->filename);
WF_LOAD_STRDUP_FILENAME:
	free(w);
WF_LOAD_MALLOC_STRUCT:
	fclose(fptr);
WF_LOAD_FOPEN:
	return NULL;
}

/*******************************************************************************
 * Here begins the user facing functions.
 ******************************************************************************/

/*
 * Load a WaveFront OBJ file
 */
struct WF_OBJ* wf_load(char * filename)
{
	struct WF_OBJ *w = wf_parse(filename);
	if(w == NULL)
	{
		return NULL;
	}

	wf_bound(w);
	wf_normals(w);
/*
	if(w->nv != w->nn)wf_normals(w);
	wf_texvec2s(w);


//	wf_gpu_load(w);
//	w->draw = wf_draw;

	log_debug("All done");
*/
	return w;
}


void wf_free(struct WF_OBJ *w)
{
	if(!w)return;

	if(w->index_buffer_data) free(w->index_buffer_data);
	if(w->vertex_buffer_data) free(w->vertex_buffer_data);

	if(w->smoothgroups) free(w->smoothgroups);
	if(w->verticies) free(w->verticies);
	if(w->texcoords) free(w->texcoords);
	if(w->normals) free(w->normals);
	if(w->triangles) free(w->triangles);
	if(w->filename) free(w->filename);
	free(w);
}

/*
int main(int argc, char *argv[])
{
	log_init();
	struct WF_OBJ *w;
	w = wf_load("../models/lpshead/head.OBJ");
	wf_free(w);
	w = wf_load("../models/bunny/bunny.obj");
	wf_free(w);
	w = wf_load("../models/sponza/sponza.obj");
	wf_free(w);
	w = wf_load("../models/buddha/buddha.obj");
	wf_free(w);
	w = wf_load("../models/hairball/hairball.obj");
	wf_free(w);
	w = wf_load("../models/San_Miguel/san-miguel-low-poly.obj");
	wf_free(w);
	w = wf_load("../models/San_Miguel/san-miguel.obj");
	wf_free(w);
	w = wf_load("../models/powerplant/powerplant.obj");
	wf_free(w);
	return 0;
}
*/
