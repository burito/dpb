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

#include "mesh_gl.h"
#include "log.h"


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
	glBufferData( GL_ELEMENT_ARRAY_BUFFER, wf->num_faces*12, wf->index_buffer_data, GL_STATIC_DRAW );

	glEnableVertexAttribArray( 0 );
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(struct packed_verts), (void *)0 );

	glEnableVertexAttribArray( 1 );
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(struct packed_verts), (void *)12 );

	glEnableVertexAttribArray( 2 );
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(struct packed_verts), (void *)24 );

	glBindVertexArray( 0 );
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	return w;
}


void mesh_draw(struct MESH_OPENGL *w)
{
	if(!w)return;
	glBindVertexArray( w->vertex_array );
	glDrawElements( GL_TRIANGLES, w->wf->num_faces*3, GL_UNSIGNED_INT, 0 );
	glBindVertexArray( 0 );
	return;
}

void mesh_free(struct MESH_OPENGL *w)
{
	wf_free(w->wf);
	free(w);
}
