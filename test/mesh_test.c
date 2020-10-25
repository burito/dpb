#define _XOPEN_SOURCE 500	// for strdup on linux

#include <ctap/include/ctap.h>

#include "mesh.h"
#include <stb/stb_ds.h>


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

	char output[80];

	////////////////////////////////////////////////////////////////////////
	// verify that the corner parsing is correct
	////////////////////////////////////////////////////////////////////////
	struct WF_TRIANGLE_CORNER corner;

	corner = wf_parse_corner_string("1/1/1");
	snprintf(output, 80, "%d/%d/%d", corner.vertex, corner.texcoord, corner.normal);
	is(output, "1/1/1", "Corner 1");

	corner = wf_parse_corner_string("2/2/");
	snprintf(output, 80, "%d/%d/%d", corner.vertex, corner.texcoord, corner.normal);
	is(output, "2/2/0", "Corner 2");

	corner = wf_parse_corner_string("3//");
	snprintf(output, 80, "%d/%d/%d", corner.vertex, corner.texcoord, corner.normal);
	is(output, "3/0/0", "Corner 3");

	corner = wf_parse_corner_string("4//4");
	snprintf(output, 80, "%d/%d/%d", corner.vertex, corner.texcoord, corner.normal);
	is(output, "4/0/4", "Corner 4");

	corner = wf_parse_corner_string("5/");
	snprintf(output, 80, "%d/%d/%d", corner.vertex, corner.texcoord, corner.normal);
	is(output, "5/0/0", "Corner 5");

	corner = wf_parse_corner_string("6");
	snprintf(output, 80, "%d/%d/%d", corner.vertex, corner.texcoord, corner.normal);
	is(output, "6/0/0", "Corner 6");

	corner = wf_parse_corner_string("7/7");
	snprintf(output, 80, "%d/%d/%d", corner.vertex, corner.texcoord, corner.normal);
	is(output, "7/7/0", "Corner 7");

	////////////////////////////////////////////////////////////////////////
	// verify that it can load all of our test models
	////////////////////////////////////////////////////////////////////////
	w = wf_load("../models/lpshead/head.OBJ");
	ok( w != NULL, "wf_load lpshead" );
	is_int( arrlen(w->verticies), 8844, "lpshead Vertex count" );
	is_int( arrlen(w->triangles), 17684, "lpshead Triangle count" );
	wf_free(w);

	w = wf_load("../models/bunny/bunny.obj");
	ok( w != NULL, "wf_load bunny" );
	is_int( arrlen(w->verticies), 72027, "bunny Vertex count" );
	is_int( arrlen(w->triangles), 144046, "bunny Triangle count" );
	wf_free(w);

	w = wf_load("../models/sponza/sponza.obj");
	ok( w != NULL, "wf_load sponza" );
	is_int( arrlen(w->verticies), 153635, "sponza Vertex count" );
	is_int( arrlen(w->triangles), 262267, "sponza Triangle count" );
	wf_free(w);

	w = wf_load("../models/buddha/buddha.obj");
	ok( w != NULL, "wf_load buddha" );
	is_int( arrlen(w->verticies), 543524, "buddha Vertex count" );
	is_int( arrlen(w->triangles), 1087474, "buddha Triangle count" );
	wf_free(w);

	w = wf_load("../models/hairball/hairball.obj");
	ok( w != NULL, "wf_load hairball" );
	is_int( arrlen(w->verticies), 1470000, "hairball Vertex count" );
	is_int( arrlen(w->triangles), 2880000, "hairball Triangle count" );
	wf_free(w);

	w = wf_load("../models/San_Miguel/san-miguel-low-poly.obj");
	ok( w != NULL, "wf_load san-miguel-low-poly" );
	is_int( arrlen(w->verticies), 3738829, "san miguel low poly Vertex count" );
	is_int( arrlen(w->triangles), 5617451, "san miguel low poly Triangle count" );
//	is_int( w->num_groups, 1098, "Group count" );
	wf_free(w);

	w = wf_load("../models/San_Miguel/san-miguel.obj");
	ok( w != NULL, "wf_load san-miguel" );
	is_int( arrlen(w->verticies), 5933233, "san miguel Vertex count" );
	is_int( arrlen(w->triangles), 9980699, "san miguel Triangle count" );
//	is_int( w->num_groups, 1004, "Group count" );
	wf_free(w);

	w = wf_load("../models/powerplant/powerplant.obj");
	ok( w != NULL, "wf_load powerplant" );
	is_int( arrlen(w->verticies), 5984083, "powerplant Vertex count" );
	is_int( arrlen(w->triangles), 12759246, "powerplant Triangle count" );
	wf_free(w);


}
