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

#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
#include <OpenGL/gl.h>
#else
#include <GL/glew.h>
#endif

#include <stdlib.h>
#include <string.h>
#include <stb/stb_image.h>

#include "mesh_gl.h"
#include "log.h"

static void img_glinit(struct IMAGE_OPENGL *image)
{
	int type = 0, intfmt = 0;

	glEnable(GL_TEXTURE_2D);
//	glEnable(GL_COLOR_TABLE);

	glGenTextures(1, &image->id);
	glBindTexture(GL_TEXTURE_2D, image->id);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);


	switch(image->channels) {
	case 4: type = GL_RGBA; intfmt = GL_RGBA8; break;
	case 3: type = GL_RGB; intfmt = GL_RGB8; break;
#ifndef __APPLE__
	case 2: type = GL_LUMINANCE_ALPHA; intfmt = GL_LUMINANCE8_ALPHA8; break;
	case 1: type = GL_LUMINANCE; intfmt = GL_LUMINANCE8; break;
#endif
	default: type = GL_NONE;
	}

//		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
//		glPixelTransferi(GL_INDEX_SHIFT, 8);
//		glColorTable(GL_COLOR_TABLE, GL_RGB8, 256, GL_RGB, GL_UNSIGNED_BYTE, img->palette);

/*	glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	gluBuild2DMipmaps( GL_TEXTURE_2D, t->bpp, t->x, t->y, t->type,
				GL_UNSIGNED_BYTE, t->buf );
*/

	glTexImage2D(GL_TEXTURE_2D, 0, intfmt, image->size.x, image->size.y, 0, type, GL_UNSIGNED_BYTE, image->buffer);
	glBindTexture(GL_TEXTURE_2D, 0);
}


void img_free(struct IMAGE_OPENGL *image)
{
	if(image->id)glDeleteTextures(1, &image->id);
	if(image->name)free(image->name);
	if(image->buffer)stbi_image_free(image->buffer);
	free(image);
}


struct IMAGE_OPENGL* img_load(const char * filename)
{
	log_info("Loading Image(\"%s\")", filename);
	struct IMAGE_OPENGL *image = malloc( sizeof(struct IMAGE_OPENGL) );
	if(image == NULL)
	{
		log_fatal("malloc(image_opengl) = %s", strerror(errno));
		return NULL;
	}
	memset(image, 0, sizeof(struct IMAGE_OPENGL) );
	image->name = strdup(filename);
	image->buffer = stbi_load(filename, &image->size.x, &image->size.y, &image->channels, 0);
	img_glinit(image);
	return image;
}



struct MESH_OPENGL* mesh_load(char *filename)
{
	struct WF_OBJ *wf = wf_load(filename);
	if(!wf)return NULL;

	struct MESH_OPENGL *w;
	w = malloc( sizeof(struct MESH_OPENGL) );
	if( w == NULL )
	{
		log_fatal("malloc(MESH_OPENGL) = %s", strerror(errno));
		return NULL;
	}
	memset(w, 0, sizeof(struct MESH_OPENGL) );
	w->wf = wf;


	glGenVertexArrays( 1, &w->vertex_array );
	glBindVertexArray( w->vertex_array );

	glGenBuffers(1, &w->array_buffer);
	glBindBuffer( GL_ARRAY_BUFFER, w->array_buffer);
	glBufferData( GL_ARRAY_BUFFER, wf->num_verticies*sizeof(struct packed_verts), wf->vertex_buffer_data, GL_STATIC_DRAW );

	glGenBuffers( 1, &w->element_buffer );
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, w->element_buffer );
	glBufferData( GL_ELEMENT_ARRAY_BUFFER, wf->num_triangles*12, wf->index_buffer_data, GL_STATIC_DRAW );

	glEnableVertexAttribArray( 0 );
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(struct packed_verts), (void *)0 );

	glEnableVertexAttribArray( 1 );
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(struct packed_verts), (void *)12 );

	glEnableVertexAttribArray( 2 );
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(struct packed_verts), (void *)24 );

	w->num_materials = wf->num_materials;
	if(w->num_materials)
	{
		w->materials = malloc( sizeof(struct MATERIAL_OPENGL) * w->num_materials );
		if( w->materials == NULL )
		{
			log_fatal("malloc(materials) = %s", strerror(errno));
			return NULL;
		}
		memset(w->materials, 0, sizeof(struct MATERIAL_OPENGL) * w->num_materials );
		for(int i=0; i<w->num_materials; i++)
		{
			if(wf->materials[i].filename)
			{
				w->materials[i].filename = strdup(wf->materials[i].filename);
				w->materials[i].image = img_load(w->materials[i].filename);
			}
		}
	}

	glBindVertexArray( 0 );
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	return w;
}


void mesh_draw(struct MESH_OPENGL *w)
{
	if(!w)return;

	glBindVertexArray( w->vertex_array );
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, w->element_buffer );

	if(w->num_materials)
	{
		int offset = 0;
		for(int i=0; i<w->num_materials; i++)
		{
			if( w->materials[i].filename != NULL )
			{
				glEnable(GL_TEXTURE_2D);
				glBindTexture(GL_TEXTURE_2D, w->materials[i].image->id);
			}

			int length = w->wf->materials[i].num_triangles * 3;
			if( length )
				glDrawElements( GL_TRIANGLES, length, GL_UNSIGNED_INT, (void*)offset );
			offset += length*sizeof(int32_t);
		}
	}
	else
	{
		glDrawElements( GL_TRIANGLES, w->wf->num_triangles*3, GL_UNSIGNED_INT, 0 );
	}

	glBindVertexArray( 0 );

	glDisable(GL_TEXTURE_2D);

	return;
}

void mesh_free(struct MESH_OPENGL *w)
{
	for(int i=0; i<w->num_materials; i++)
	{
		free(w->materials[i].filename);
		img_free(w->materials[i].image);
	}
	wf_free(w->wf);
	free(w);
}
