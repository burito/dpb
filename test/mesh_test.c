#include <ctap/include/ctap.h>

#include "mesh.h"



TESTS {

	struct WF_OBJ *w;
/*
	w = malloc(sizeof(struct WF_OBJ));
	memset(w, 0, sizeof(struct WF_OBJ));

	char oneface[] = "f 1/1/1 2/2/2 3/3/3";
	wf_count_face(w, oneface);
	is_int( w->num_triangles, 1, "count single triangle");
	w->num_triangles = 0;

	char twoface[] = "f 1/1/1 2/2/2 3/3/3 4/4/4";
	wf_count_face(w, twoface);
	is_int( w->num_triangles, 2, "count two triangles");
	w->num_triangles = 0;

	char threeface[] = "f 1/1/1 2/2/2 3/3/3 4/4/4 5/5/5";
	wf_count_face(w, threeface);
	is_int( w->num_triangles, 3, "count three triangles");
	w->num_triangles = 0;

	free(w);
*/
	w = wf_load("../models/lpshead/head.obj");
	ok( w != NULL, "wf_load lpshead" );
	is_int( w->num_verticies, 8844, "lpshead Vertex count" );
	is_int( w->num_faces, 8842, "lpshead Face count" );
	is_int( w->num_triangles, 17684, "lpshead Triangle count" );
	wf_free(w);

	w = wf_load("../models/bunny/bunny.obj");
	ok( w != NULL, "wf_load bunny" );
	is_int( w->num_verticies, 72027, "bunny Vertex count" );
	is_int( w->num_faces, 144046, "bunny Face count" );
	is_int( w->num_triangles, 144046, "bunny Triangle count" );
	wf_free(w);

	w = wf_load("../models/sponza/sponza.obj");
	ok( w != NULL, "wf_load sponza" );
	is_int( w->num_verticies, 153635, "sponza Vertex count" );
	is_int( w->num_faces, 135394, "sponza Face count" );
	is_int( w->num_triangles, 262267, "sponza Triangle count" );
	wf_free(w);

	w = wf_load("../models/buddha/buddha.obj");
	ok( w != NULL, "wf_load buddha" );
	is_int( w->num_verticies, 543524, "buddha Vertex count" );
	is_int( w->num_faces, 1087474, "buddha Face count" );
	is_int( w->num_triangles, 1087474, "buddha Triangle count" );
	wf_free(w);

	w = wf_load("../models/hairball/hairball.obj");
	ok( w != NULL, "wf_load hairball" );
	is_int( w->num_verticies, 1470000, "hairball Vertex count" );
	is_int( w->num_faces, 2880000, "hairball Face count" );
	is_int( w->num_triangles, 2880000, "hairball Triangle count" );
	wf_free(w);

	w = wf_load("../models/San_Miguel/san-miguel-low-poly.obj");
	ok( w != NULL, "wf_load san-miguel-low-poly" );
	is_int( w->num_verticies, 3738829, "san miguel low poly Vertex count" );
	is_int( w->num_faces, 4963199, "san miguel low poly Face count" );
	is_int( w->num_triangles, 5617451, "san miguel low poly Triangle count" );
//	is_int( w->num_groups, 1098, "Group count" );
	wf_free(w);

	w = wf_load("../models/San_Miguel/san-miguel.obj");
	ok( w != NULL, "wf_load san-miguel" );
	is_int( w->num_verticies, 5933233, "san miguel Vertex count" );
	is_int( w->num_faces, 9932385, "san miguel Face count" );
	is_int( w->num_triangles, 9980699, "san miguel Triangle count" );
//	is_int( w->num_groups, 1004, "Group count" );
	wf_free(w);

	w = wf_load("../models/powerplant/powerplant.obj");
	ok( w != NULL, "wf_load powerplant" );
	is_int( w->num_verticies, 5984083, "powerplant Vertex count" );
	is_int( w->num_faces, 12759246, "powerplant Face count" );
	is_int( w->num_triangles, 12759246, "powerplant Triangle count" );
	wf_free(w);

}
