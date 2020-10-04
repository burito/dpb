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

#define _XOPEN_SOURCE 500	// for strtok_r and strdup on linux

#ifdef _WIN32
#define strtok_r strtok_s
#endif

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

void mtl_free(WF_MTL *m)
{
	if(!m)return;
	if(m->name)free(m->name);
	if(m->map_Ka)img_free(m->map_Ka);
	if(m->map_Kd)img_free(m->map_Kd);
	if(m->map_d)img_free(m->map_d);
	if(m->map_bump)img_free(m->map_bump);
	free(m);

}

static WF_MTL* mtl_newmtl(char *hostpath, FILE *fptr, char *name)
{
	WF_MTL *m = malloc(sizeof(WF_MTL));
	if(!m)return 0;
	memset(m, 0, sizeof(WF_MTL));
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
}


static void mtl_load(WF_OBJ *w, char *filename)
{
	if(!w)return;
	if(!filename)return;
	if(!w->filename)return;
//	char *filepath = repath(w->filename, filename);
	log_info("Loading Wavefront MTL(\"%s\");", filename);
	return;
#ifdef NO
	FILE *fptr = fopen(filepath, "r");
	if(!fptr)
	{
		log_error("fopen(\"%s\") %s", filepath, strerror(errno));
		free(filepath);
		return;
	}

	char buf[BUF_LEN];
	while(fgets(buf, BUF_LEN, fptr))
	if(strstr(buf, "newmtl"))
	{
		tailchomp(buf);
		WF_MTL *m = mtl_newmtl(filepath, fptr, buf+7);
		if(m)
		{
			w->nm++;
			m->next = w->m;
			w->m = m;
		}
	}
	free(filepath);
	fclose(fptr);
#endif
}



static WF_MTL* find_material(WF_MTL *m, char *name)
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





void wf_interleave(WF_OBJ *w)
{
	if(w == NULL)return;
	log_debug("At interleave we have %d/%d/%d", w->nv, w->nn, w->nt);
	if(w->nv == 0)return;
	struct packed_verts *pv = malloc(w->nv * sizeof(struct packed_verts));
	if(pv == NULL)
	{
		log_error("malloc(w->nv*sizeof(packed_verts))");
		return;
	}
	memset(pv, 0, w->nv * sizeof(struct packed_verts));
	// interleave the verts, normals and texture vec2s for the VBO
	for(int i=0; i<w->nv; i++)
	{
		pv[i].p = w->v[i];
		if(w->nv == w->nn)
		{
			pv[i].n = w->vn[i];
		}
		if(w->uv)
		{
			pv[i].uv = w->uv[i];
		}
	}
	log_debug("Interleaved ok!");
	w->pv = pv;

	// now store the faces, per material for fast rendering
	if(w->nf <= 0)
	{
		log_warning("no faces in model");
		return;
	}
	int3 *f = malloc(w->nf*sizeof(int3));
	if(f == NULL)
	{
		log_error("malloc(w->nf*sizeof(int3))");
		return;
	}
	WF_MTL *m = w->m;
	int o=0;
	for(;m;m = m->next)
	{
		m->nf = 0;
		for(int i=0; i<w->nf; i++)
		if(w->f[i].m == m)
		{
			f[o++] = w->f[i].f;
			m->nf++;
		}
	}

	for(;o < w->nf; o++)
	for(int i=0; i<w->nf; i++)
	if(w->f[i].m == 0)
		f[o++] = w->f[i].f;
	w->vf = f;
	log_debug("Interleaved faces ok!");
//	free(w->v); w->v = 0;
//	free(w->vn); w->vn = 0;
}

static void wf_face_normals(WF_OBJ *w)
{
	if(!w)return;
	if(!w->nf)return;

	vec3 a, b, t;

	// Generate per face normals
	for(int i=0; i<w->nf; i++)
	{
		a = sub( w->v[w->f[i].f.x], w->v[w->f[i].f.y] );
		b = sub( w->v[w->f[i].f.x], w->v[w->f[i].f.z] );
		t = vec3_cross( a, b );
		w->f[i].normal = vec3_norm( t );
	}
}


static void wf_vertex_normals(WF_OBJ *w)
{
	if(!w)return;
	if(!w->nf)return;
	if(!w->vn)return;

	typedef struct LLIST
	{
		int face;
		struct LLIST* next;
	} LLIST;

	LLIST *vert = malloc(sizeof(LLIST)*w->nv);
	memset(vert, 0, sizeof(LLIST)*w->nv);

	LLIST *tmp;
	int index;

	// tell each vert about it's faces
	for(int i=0; i<w->nf; i++)
//	if(w->f[i].s == smoothgroup)
	{
		index = w->f[i].f.x;
		tmp = malloc(sizeof(LLIST));
		tmp->face = i;
		tmp->next = vert[index].next;
		vert[index].next = tmp;

		index = w->f[i].f.y;
		tmp = malloc(sizeof(LLIST));
		tmp->face = i;
		tmp->next = vert[index].next;
		vert[index].next = tmp;

		index = w->f[i].f.z;
		tmp = malloc(sizeof(LLIST));
		tmp->face = i;
		tmp->next = vert[index].next;
		vert[index].next = tmp;
	}

	// now average the normals and store in the output
	for(int i=0; i<w->nv; i++)
	{
		tmp = vert[i].next;
		if(!tmp)continue;
		vec3 t = {.f={0,0,0}};
		while(tmp)
		{
			t = add(t, w->f[tmp->face].normal);
			LLIST *last = tmp;
			tmp = tmp->next;
			free(last);
		}

		if((t.x*t.x + t.y*t.y + t.z*t.z )>0.1)
		{
			w->vn[i] = vec3_norm(t);
		}

	}
	free(vert);
}



static void wf_normals(WF_OBJ *w)
{
	if(!w)return;
	if(w->nv == w->nn)return;

	if(w->vn)free(w->vn);
	w->vn = malloc(sizeof(vec3)*w->nv);
	memset(w->vn, 0, sizeof(vec3)*w->nv);
	w->nn = w->nv;

	wf_face_normals(w);
	wf_vertex_normals(w);


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

#ifdef NO
static void wf_gpu_load(WF_OBJ *w)
{
	if(!w)return;
	w->va = w->ab = w->eb  = 0;

	glGenVertexArrays( 1, &w->va );
	glBindVertexArray( w->va );

	glGenBuffers(1, &w->ab);
	glBindBuffer( GL_ARRAY_BUFFER, w->ab);
	glBufferData( GL_ARRAY_BUFFER, w->nv*sizeof(struct packed_verts), w->pv, GL_STATIC_DRAW );

	glGenBuffers( 1, &w->eb );
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, w->eb );
	glBufferData( GL_ELEMENT_ARRAY_BUFFER, w->nf*12, w->vf, GL_STATIC_DRAW );

	glEnableVertexAttribArray( 0 );
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(struct packed_verts), (void *)0 );

	glEnableVertexAttribArray( 1 );
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(struct packed_verts), (void *)12 );

	glEnableVertexAttribArray( 2 );
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(struct packed_verts), (void *)24 );

	glBindVertexArray( 0 );
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}


void wf_draw(WF_OBJ *w)
{
	if(!w)return;
	glBindVertexArray( w->va );
	glDrawElements( GL_TRIANGLES, w->nf*3, GL_UNSIGNED_INT, 0 );
	glBindVertexArray( 0 );
	return;
}
#endif

#endif

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

	int num_triangles = -3;

	char delim[] = " \n\r";
	char *saveptr = NULL;
	char *token = NULL;
	token = strtok_r(line, delim, &saveptr);
	do {
		token = strtok_r(NULL, delim, &saveptr);
		num_triangles++;
	} while( token != NULL);

	w->num_triangles += num_triangles;
	w->smoothgroups[w->current_smoothgroup] += num_triangles;
	return num_triangles;
}

void wf_count_group(struct WF_OBJ *w, char *line)
{
	w->num_groups++;
	return;
}


void wf_count_smoothgroup(struct WF_OBJ *w, char *line)
{
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
	return;
}

void wf_count_material(struct WF_OBJ *w, char *line)
{
	w->num_materials++;
	return;
}

/* all of the "parse" functions call their respective "count" function
 * to eliminate counting errors, or at least ensure they're consistent
 */

/*
 * Parse a vertex of the form "v 1.0 2.0 3.0"
*/
void wf_parse_vertex(struct WF_OBJ *w, char *line)
{
	char delim[] = " \n\r";
	char *saveptr = NULL;
	char *token = NULL;
	token = strtok_r(line, delim, &saveptr);
	for(int i=0; i<3; i++)
	{
		token = strtok_r(NULL, delim, &saveptr);
		w->verticies[w->num_verticies].f[i] = fast_atof(token);
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
	char delim[] = " \n\r";
	char *saveptr = NULL;
	char *token = NULL;
	token = strtok_r(line, delim, &saveptr);
	for(int i=0; i<3; i++)
	{
		token = strtok_r(NULL, delim, &saveptr);
		w->normals[w->num_normals].f[i] = fast_atof(token);
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
	char delim[] = " \n\r";
	char *saveptr = NULL;
	char *token = NULL;
	token = strtok_r(line, delim, &saveptr);
	for(int i=0; i<2; i++)
	{
		token = strtok_r(NULL, delim, &saveptr);
		w->texcoords[w->num_texcoords].f[i] = fast_atof(token);
	}
	wf_count_texcoord(w, line);
	return;
}

/*
 * Parse a face record of the form "f 1/1/1 2/2/2 3/3/3 -1/-1/-1".
 * Groups being "vertex/texcoord/normal".
 * Assume all polygons are convex, so can convert them to triangle fans.
 * Negative indicies indicate "relative to the last defined item",
 * which is why we're keeping a running count.
 */
void wf_parse_face(struct WF_OBJ *w, char *line)
{
	int num_triangles =  wf_count_face(w, line);
	int offset = w->num_triangles - num_triangles;

	char corner_delim[] = " \n\r";
	char *corner_saveptr = NULL;
	char *token = NULL;
	token = strtok_r(line, corner_delim, &corner_saveptr);

	// parse the first triangle
	for(int i=0; i<3; i++)
	{
		token = strtok_r(NULL, corner_delim, &corner_saveptr);
		int value;
		char item_delim[] = "/";
		char *item_saveptr = NULL;
		char *item = strtok_r(token, item_delim, &item_saveptr);
		value = atoi(item);
		if(value < 0) value = value + w->num_verticies;
		w->triangles[offset].verticies.i[i] = value;

		item = strtok_r(NULL, item_delim, &item_saveptr);
		value = atoi(item);
		if(value < 0) value = value + w->num_texcoords;
		w->triangles[offset].texcoords.i[i] = value;

		item = strtok_r(NULL, item_delim, &item_saveptr);
		value = atoi(item);
		if(value < 0) value = value + w->num_normals;
		w->triangles[offset].normals.i[i] = value;
	}

	// now parse the remaining triangles
	int first = offset;
	offset++;
	for(; offset < w->num_triangles; offset++)
	{
		// the first two corners go in every triangle
		w->triangles[offset].verticies.xy = w->triangles[first].verticies.xy;
		w->triangles[offset].texcoords.xy = w->triangles[first].texcoords.xy;
		w->triangles[offset].normals.xy = w->triangles[first].normals.xy;

		// now parse the third corner just as before
		token = strtok_r(NULL, corner_delim, &corner_saveptr);
		int value;
		char item_delim[] = "/";
		char *item_saveptr = NULL;
		char *item = strtok_r(token, item_delim, &item_saveptr);
		value = atoi(item);
		if(value < 0) value = value + w->num_verticies;
		w->triangles[offset].verticies.z = value;

		item = strtok_r(NULL, item_delim, &item_saveptr);
		value = atoi(item);
		if(value < 0) value = value + w->num_texcoords;
		w->triangles[offset].texcoords.z = value;

		item = strtok_r(NULL, item_delim, &item_saveptr);
		value = atoi(item);
		if(value < 0) value = value + w->num_normals;
		w->triangles[offset].normals.z = value;
	}

	return;
}

void wf_parse_group(struct WF_OBJ *w, char *line)
{
	wf_count_group(w, line);
	return;
}


void wf_parse_smoothgroup(struct WF_OBJ *w, char *line)
{
	wf_count_smoothgroup(w, line);
	return;
}

/*
 * Parse a material of the form "usemtl mat_name".
 */
void wf_parse_material(struct WF_OBJ *w, char *line)
{
	wf_count_material(w, line);
	return;
}


int wf_alloc_first_buffers(struct WF_OBJ *w)
{
	// alloc the memory
	if(w->num_verticies)
	{
		w->verticies = malloc(sizeof(vec3)*w->num_verticies);
		if(w->verticies == NULL)
		{
			log_fatal("malloc(verticies) = %d", strerror(errno));
			goto WF_ALLOC_VERTICIES;
		}
	}
	if(w->num_texcoords)
	{
		w->texcoords = malloc(sizeof(vec2)*w->num_texcoords);
		if(w->texcoords == NULL)
		{
			log_fatal("malloc(texcoords) = %d", strerror(errno));
			goto WF_ALLOC_TEXCOORDS;
		}
	}
	if(w->num_normals)
	{
		w->normals = malloc(sizeof(vec3)*w->num_normals);
		if(w->normals == NULL)
		{
			log_fatal("malloc(normals) = %d", strerror(errno));
			goto WF_ALLOC_NORMALS;
		}
	}
	if(w->num_triangles)
	{
		w->triangles = malloc( sizeof(struct WF_TRIANGLE) * w->num_triangles);
		if(w->triangles == NULL)
		{
			log_fatal("malloc(triangles) = %d", strerror(errno));
			goto WF_ALLOC_TRIANGLES;
		}
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

WF_ALLOC_TRIANGLES:
	if(w->num_triangles) free(w->triangles);
WF_ALLOC_NORMALS:
	if(w->num_normals) free(w->normals);
WF_ALLOC_TEXCOORDS:
	if(w->num_texcoords) free(w->verticies);
WF_ALLOC_VERTICIES:
	free(w->verticies);
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
		log_warning("malloc() = %s", strerror(errno));
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
		if(strstr(line, "mtllib"))
			wf_count_material(w, line);
			log_info("mtl");
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

	for(int i=0; i<MAX_SMOOTHGROUPS; i++)
	{
		if(w->smoothgroups[i] > 0)
		{
			log_info("Smoothgroup[%d] = %d", i, w->smoothgroups[i]);
		}
	}


	log_info("Verticies=%d, Normals=%d, Texcoords=%d, Faces=%d, Triangles=%d",
		w->num_verticies, w->num_normals, w->num_texcoords, w->num_faces,
		w->num_triangles);

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

	fseek(fptr, 0, SEEK_SET);
	while(fgets(line, 1024, fptr))
	switch(line[0]) {
	case 'm':
		if(strstr(line, "mtllib"))
			wf_parse_material(w, line);
			log_info("mtl");
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
	if(w->normals) free(w->normals);
	if(w->texcoords) free(w->texcoords);
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
/*
	if(w->nv != w->nn)wf_normals(w);
	wf_texvec2s(w);

	wf_interleave(w);
//	wf_gpu_load(w);
//	w->draw = wf_draw;

	log_debug("All done");
*/
	return w;
}


void wf_free(struct WF_OBJ *w)
{
	if(!w)return;
/*
	if(w->v)free(w->v);
	if(w->vt)free(w->vt);
	if(w->vn)free(w->vn);
	if(w->pv)free(w->pv);
	if(w->f)free(w->f);
	if(w->fn)free(w->fn);
#ifdef NO
	if(w->ab)glDeleteBuffers(1, &w->ab);
	if(w->eb)glDeleteBuffers(1, &w->eb);
#endif
	WF_MTL *mt, *m = w->m;
	while(m)
	{
		mt = m->next;
		mtl_free(m);
		m = mt;
	}
*/
	if(w->smoothgroups) free(w->smoothgroups);
	if(w->verticies) free(w->verticies);
	if(w->texcoords) free(w->texcoords);
	if(w->normals) free(w->normals);
	if(w->triangles) free(w->triangles);
	if(w->filename) free(w->filename);
	free(w);
}


int main(int argc, char *argv[])
{
	log_init();
	struct WF_OBJ *w;
	w = wf_load("../models/sponza/sponza.obj");
	wf_free(w);

	return 0;
}