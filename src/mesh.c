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

#define _XOPEN_SOURCE 600L	// for strdup on linux
//#define _POSIX_C_SOURCE // for strdup on macos

#ifdef _WIN32
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <libgen.h> // for dirname()

#include <stb/stb_ds.h>

#include "3dmaths.h"
#include "mesh.h"
#include "log.h"

#include <fast_atof.c>

#define MAX_SMOOTHGROUPS 256


void wf_pack(struct WF_OBJ *w)
{
	w->num_triangles = arrlen(w->triangles);
	w->num_verticies = hmlen(w->corners);

	w->vertex_buffer_data = malloc( sizeof(struct packed_verts) * w->num_verticies );
	if( w->vertex_buffer_data == NULL )
	{
		log_fatal("malloc(vertex_buffer) = %s", strerror(errno));
		return;
	}


	for(int i=0; i<hmlen(w->corners); i++)
	{
		int index = w->corners[i].value;
		struct WF_TRIANGLE_CORNER corner = w->corners[i].key;
		w->vertex_buffer_data[index-1].p = w->verticies[corner.vertex-1];
		if(w->texcoords)
			w->vertex_buffer_data[index-1].uv = w->texcoords[corner.texcoord-1];
		if(w->normals)
			w->vertex_buffer_data[index-1].n = w->normals[corner.normal-1];
	}


	w->index_buffer_data = malloc( sizeof(int3) * arrlen(w->triangles));
	if( w->index_buffer_data == NULL )
	{
		log_fatal("malloc(index_buffer) = %s", strerror(errno));
		return;
	}

	if(w->num_materials)
	{
		int offset = 0;
		for(int m=0; m<w->num_materials; m++)
		for(int i=0; i<w->num_triangles; i++)
		{
			if(w->triangles[i].material == m)
			{
				w->index_buffer_data[offset].x = w->triangles[i].corner[0]-1;
				w->index_buffer_data[offset].y = w->triangles[i].corner[1]-1;
				w->index_buffer_data[offset].z = w->triangles[i].corner[2]-1;
				offset++;
			}
		}

	}
	else
	{
		for(int i=0; i<w->num_triangles; i++)
		{
			w->index_buffer_data[i].x = w->triangles[i].corner[0]-1;
			w->index_buffer_data[i].y = w->triangles[i].corner[1]-1;
			w->index_buffer_data[i].z = w->triangles[i].corner[2]-1;
		}
	}
}

void wf_normals(struct WF_OBJ *w)
{
	// calculate per face normals
	for(int i=0; i<arrlen(w->triangles); i++)
	{
		struct WF_TRIANGLE *triangle = &w->triangles[i];
		vec3 a = sub( w->vertex_buffer_data[triangle->corner[0]-1].p, w->vertex_buffer_data[triangle->corner[1]-1].p );
		vec3 b = sub( w->vertex_buffer_data[triangle->corner[0]-1].p, w->vertex_buffer_data[triangle->corner[2]-1].p );
		vec3 t = vec3_cross( a, b );
		triangle->normal = vec3_norm( t );
	}

	// add each triangle's normal to each of its corners normal
	for(int i=0; i<arrlen(w->triangles); i++)
	for(int j=0; j<3; j++)
	{
		struct WF_TRIANGLE *triangle = &w->triangles[i];
		int index = triangle->corner[j]-1;
		w->vertex_buffer_data[index].n = add(w->vertex_buffer_data[index].n, triangle->normal);
	}

	for(int i=0; i<w->num_verticies; i++)
	{
		// if it's for some reason 0, don't do it
		if( vec3_mag( w->vertex_buffer_data[i].n ) > 0.0001)
			w->vertex_buffer_data[i].n = vec3_norm(w->vertex_buffer_data[i].n);
	}
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
	for(int i=0; i<arrlen(w->verticies); i++)
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
	for(int i=0; i<arrlen(w->verticies); i++)
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

/*
 * Read the mtl library file.
 */
void wf_parse_mtllib(struct WF_OBJ *w, char *line_in)
{
	// while it's not whitespace
	while( !(*line_in == ' ' || *line_in == '\t') ) line_in++;
	// while it is whitespace
	while( (*line_in == ' ' || *line_in == '\t') ) line_in++;

	// nested to limit the scope of tmpline
	{
		char *tmpline = line_in;
		while( !(*tmpline == '\n' || *tmpline == '\r' || *tmpline == 0) ) tmpline++;
		*tmpline = 0;
	}

	// dirname() may modify it's argument, so make a copy
	char *tmp = strdup(w->filename);
	if(tmp == NULL)
	{
		log_error("strdup(\"%s\") = %s", w->filename, strerror(errno));
		goto WF_PARSE_STRDUP_FILENAME;
	}
	char *filedir = strdup(dirname(tmp));
	if(filedir == NULL)
	{
		log_error("strdup(dirname(\"%s\")) = %s", tmp, strerror(errno));
		goto WF_PARSE_STRDUP_DIRNAME;
	}
	free(tmp);

	char filepath[1024];
	snprintf(filepath, 1024, "%s/%s", filedir, line_in);

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
	while(fgets(linestr, 1024, fptr))
	{
		char *line = linestr;
		while( *line == ' ' || *line == '\t' ) line++;
		switch(line[0]) {
		case 'n':
			if(strstr(line, "newmtl"))
				w->num_materials++;
			break;
		}
	}

	// allocate space for the materials
	w->materials = malloc( sizeof(struct WF_MTL) * w->num_materials );
	if(w->materials == NULL)
	{
		log_fatal("malloc(materials) = %s", strerror(errno));
		goto WF_PARSE_MTLLIB_MALLOC;
	}
	memset( w->materials, 0, sizeof(struct WF_MTL) * w->num_materials );

	w->num_materials = 0;

	// now that we know how many, and have allocated, parse them
	fseek(fptr, SEEK_SET, 0);
	while(fgets(linestr, 1024, fptr))
	{
		char *line = linestr;
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
				if(w->materials[w->num_materials].name == NULL)
				{
					log_error("strdup(material[i]) = %s", strerror(errno));
				}
				w->current_material = w->num_materials;
				w->num_materials++;
			}
			break;
		case 'm':
			if(strstr(line, "map_"))
			{ // this is a texture map of some kind
				line += 5;
				char **target = NULL;

				switch(line[0]) {
				case 'a':	// map_Ka - ambient
					target = &w->materials[w->current_material].map_Ka;
					break;
				case 's':	// map_Ks - specular
					target = &w->materials[w->current_material].map_Ks;
					break;
				case 'u':	// map_bump - bump map
					target = &w->materials[w->current_material].map_bump;
					break;
				case ' ':	// map_d - stencil map
					target = &w->materials[w->current_material].map_d;
					break;
				default:
					break; // we don't care yet
				case 'd':	// map_Kd - diffuse
					target = &w->materials[w->current_material].map_Kd;
					break;
				}
				if ( target != NULL )
				{
					// while it is not whitespace
					while( !(*line == ' ' || *line == '\t') ) line++;
					// continue until the whitespace ends
					while( *line == ' ' || *line == '\t' ) line++;
					// line now equals the start of the name
					char *tmpline = line;
					while( !(*tmpline == '\n' || *tmpline == '\r' || *tmpline == 0) ) tmpline++;
					*tmpline = 0;
					// the name is now null-terminated, now replace \'s with /'s
					tmpline = line;
					while( !(*tmpline == '\n' || *tmpline == '\r' || *tmpline == 0) )
					{
						if( *tmpline == '\\') *tmpline = '/';
						tmpline++;
					}
					// now copy the string
					snprintf(filepath, 1024, "%s/%s", filedir, line);
					*target = strdup(filepath);
					if(*target == NULL)
					{
						log_error("strdup(target) = %s", strerror(errno));
					}
				}
			}
		}
	}
	return;


	free(w->materials);
	w->num_materials = 0;
WF_PARSE_MTLLIB_MALLOC:
	fclose(fptr);
WF_PARSE_MTLLIB_FOPEN:
	free(filedir);
WF_PARSE_STRDUP_DIRNAME:
	if(tmp != NULL ) free(tmp);
WF_PARSE_STRDUP_FILENAME:
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

	for(int i=0; i<w->num_materials; i++)
	{
		if( strncmp(line, w->materials[i].name, strlen(w->materials[i].name) ) == 0 )
		{
			w->current_material = i;
			return;
		}

	}
	w->current_material = 0;
}

/*
 * Parse a vertex of the form "v 1.0 2.0 3.0"
*/
void wf_parse_vertex(struct WF_OBJ *w, char *line)
{
	vec3 vertex = {{.0f,.0f,.0f}};
	line++;
	for(int i=0; i<3; i++)
	{
		while( (*line == ' ' || *line == '\t') ) line++;
		vertex.f[i] = fast_atof(line);
		while( !(*line == ' ' || *line == '\t' || *line == '\n' || *line == '\r' || *line == 0 ) ) line++;
	}
	arrput(w->verticies, vertex);
	return;
}

/*
 * Parse a normal of the form "vn 1.0 0.0 0.0"
 * It *should* be a unit vector, but we won't assume that.
*/
void wf_parse_normal(struct WF_OBJ *w, char *line)
{
	vec3 normal = {{.0f,.0f,.0f}};
	line+=2;
	for(int i=0; i<3; i++)
	{
		while( (*line == ' ' || *line == '\t') ) line++;
		normal.f[i] = fast_atof(line);
		while( !(*line == ' ' || *line == '\t' || *line == '\n' || *line == '\r' || *line == 0 ) ) line++;
	}
	arrput(w->normals, normal);
	return;
}

/*
 * Parse a Texture Coordinate of the form "vt 0.5 0.5 0.0"
 * We are only interested in 2D texcoords, so ignore the last value
 */
void wf_parse_texcoord(struct WF_OBJ *w, char *line)
{
	vec2 texcoord = {{.0f,.0f}};
	line+=2;
	for(int i=0; i<2; i++)
	{
		while( (*line == ' ' || *line == '\t') ) line++;
		texcoord.f[i] = fast_atof(line);
		if( i == 1 )
		{// flip the Y coordinate of texture maps
			texcoord.f[i] = 1.0 - texcoord.f[i];
		}
		while( !(*line == ' ' || *line == '\t' || *line == '\n' || *line == '\r' || *line == 0 ) ) line++;
	}
	arrput(w->texcoords, texcoord);
	return;
}


/*
 * Parse a corner string of the form "vertex/texcoord/normal".
 * This could be "1/1/1", texcoord and normal are optional, so
 * "1//1", "1/1/", "1//", "1/1", and "1" are all valid.
 * Negative indexes are also possible.
 */
struct WF_TRIANGLE_CORNER wf_parse_corner_string(const char *line)
{
	struct WF_TRIANGLE_CORNER corner = {0,0,0};

	// the first value, not optional, is a vertex index
	corner.vertex = atoi(line);

	// find the next non-number (or hyphen)
	while( (*line >= '0' && *line <= '9') || *line == '-' ) line++;

	// if it's not a '/', then we're done
	if(*line != '/')goto WF_PARSE_FACE_CORNER_DONE;

	// otherwise, we can treat it as another value
	line++;
	corner.texcoord = atoi(line);

	// if we just read a number, move past it
	while( (*line >= '0' && *line <= '9') || *line == '-' ) line++;

	// if we don't find a second slash, we're done
	if( *line != '/' )goto WF_PARSE_FACE_CORNER_DONE;

	// otherwise, read the last value
	line++;
	corner.normal = atoi(line);

WF_PARSE_FACE_CORNER_DONE:
	return corner;
}

/*
 * Convert negative indices into the correct value.
 * Negative indicies indicate "relative to the last defined item".
 */
struct WF_TRIANGLE_CORNER wf_parse_corner_negatives(struct WF_OBJ *w, struct WF_TRIANGLE_CORNER corner)
{
	if(corner.vertex < 0)
	{
		corner.vertex += arrlen(w->verticies) + 1;
	}

	if(corner.texcoord < 0)
	{
		corner.texcoord += arrlen(w->texcoords) + 1;
	}

	if(corner.normal < 0)
	{
		corner.normal += arrlen(w->normals) + 1;
	}
	return corner;
}


int wf_parse_face_corner(struct WF_OBJ *w, char *line)
{
	struct WF_TRIANGLE_CORNER corner;
	// parse the text of the corner
	corner = wf_parse_corner_string(line);
	// replace negative values with the correct index
	corner = wf_parse_corner_negatives(w, corner);
	corner.smoothgroup = w->current_smoothgroup;

	// now build a list of unique corners, abusing a hash to do it
	int index = hmget(w->corners, corner);
	if( index == 0 ) // default value is 0
	{
		w->num_corners++;
		hmput(w->corners, corner, w->num_corners);
		index = w->num_corners;
	}
	return index;
}

/*
 * Parse a face record of the form "f 1/1/1 2/2/2 3/3/3 -1/-1/-1".
 * Groups being "vertex/texcoord/normal".
 * Assume all polygons are convex, so can convert them to triangle fans.
 * Negative indicies indicate "relative to the last defined item",
 * which is why we're keeping a running count.
 * Also possible "1/2", "1//2, 1//"
 */
void wf_parse_face(struct WF_OBJ *w, char *line)
{
	struct WF_TRIANGLE triangle;
	memset(&triangle, 0, sizeof(struct WF_TRIANGLE));

	line++;
	// parse the first triangle
	for(int i=0; i<3; i++)
	{
		// find the next not space
		while( *line == ' ' || *line == '\t' ) line++;

		int index = wf_parse_face_corner(w, line);
		triangle.corner[i] = index;

		// find the next whitespace
		while( !(*line == ' ' || *line == '\n' || *line == '\r' || *line == 0) ) line++;
	}
	// write which smoothgroup this face is in
	triangle.smoothgroup = w->current_smoothgroup;
	triangle.material = w->current_material;
	if(w->num_materials)w->materials[w->current_material].num_triangles++;
	// and add it to the array
	arrput(w->triangles, triangle);
	// now parse the remaining triangles

	// from here, we need to do it differently
	// find the next not space
	while( *line == ' ' || *line == '\t' ) line++;
	// we should be at the start of a corner definition, or an end of line

	// while we're not at the end of the line
	while( !(*line == 0 || *line == '\r' || *line == '\n') )
	{
		// write which smoothgroup this face is in
		triangle.smoothgroup = w->current_smoothgroup;
		triangle.material = w->current_material;
		if(w->num_materials)w->materials[w->current_material].num_triangles++;

		int offset = arrlen(w->triangles) - 1;
		// the first two corners go in every triangle
		triangle.corner[0] = w->triangles[offset].corner[0];
		triangle.corner[1] = w->triangles[offset].corner[2];

		int index = wf_parse_face_corner(w, line);
		triangle.corner[2] = index;
		arrput(w->triangles, triangle);

		// find the next space
		while( !(*line == ' ' || *line == '\t' || *line == '\r' || *line == '\n' || *line == 0) ) line++;

		//       while not whitespace         and     not end of line
		while((*line == ' ' || *line == '\t') && !(*line == 0 || *line == '\r' || *line == '\n') ) line++;
	}
	return;
}

/*
 * I don't know what purpose groups serve, insofar as rendering is concerned
 */
void wf_parse_group(struct WF_OBJ *w, char *line)
{
	// TODO: figure out what these do
	return;
}

/*
 * Set the currently active smoothgroup
 */
void wf_parse_smoothgroup(struct WF_OBJ *w, char *line)
{
	line++;
	// find the next non space character
	while( (*line == ' ' || *line == '\t') ) line++;

	w->current_smoothgroup = atoi(line);
//	log_info("sg found = %d", w->current_smoothgroup);
	return;
}

/*
 * Open the file and parse the Wavefront OBJ data contained within
 */
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

	char line[1024];
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
		arrfree(w->normals);
		w->normals = NULL;
	}
	if(w->texcoords)
	{
		arrfree(w->texcoords);
		w->texcoords = NULL;
	}
	arrfree(w->verticies);
//WF_LOAD_ALLOC:
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
	wf_pack(w);
	wf_normals(w);

	return w;
}


void wf_free(struct WF_OBJ *w)
{
	if(!w)return;

	if(w->index_buffer_data) free(w->index_buffer_data);
	if(w->vertex_buffer_data) free(w->vertex_buffer_data);

	if(w->smoothgroups) free(w->smoothgroups);
	if(w->verticies) arrfree(w->verticies);
	if(w->texcoords) arrfree(w->texcoords);
	if(w->normals) arrfree(w->normals);
	if(w->triangles) arrfree(w->triangles);
	if(w->corners) hmfree(w->corners);
	if(w->filename) free(w->filename);
	free(w);
}
