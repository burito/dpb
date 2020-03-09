/*
Copyright (C) 2019 Daniel Burke

This software is provided 'as-is', without any express or implied
warranty.  In no event will the authors be held liable for any damages
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
3. This notice may not be removed or altered from any source distribution.
*/

#include <math.h>
#include "3dmaths.h"
#include "global.h"
#include "log.h"

static int p_swim = 0;

void fps_movement(vec4 *position, vec4 *angle, float velocity)
{
	// for first person movement
	position->w = 0.5 / (float)vid_width;

	vec3 requested = {{0,0,0}};

	if(keys[KEY_LSHIFT])
		velocity *= 10.0;

	if(keys[KEY_W])
	{
		requested.z -= velocity;
	}
	if(keys[KEY_S])
	{
		requested.z += velocity;
	}
	if(keys[KEY_A])
	{
		requested.x += velocity;
	}
	if(keys[KEY_D])
	{
		requested.x -= velocity;
	}
	if(keys[KEY_LCONTROL])
	{
		requested.y += velocity;
	}
	if(keys[KEY_SPACE])
	{
		requested.y -= velocity;
	}

	if(keys[KEY_LEFT])
	{
		angle->y -= velocity;
	}
	if(keys[KEY_RIGHT])
	{
		angle->y += velocity;
	}
	if(keys[KEY_UP])
	{
		angle->x -= velocity;
	}
	if(keys[KEY_DOWN])
	{
		angle->x += velocity;
	}

	if(mouse[2]) /// right mouse
	{
		angle->x -= mickey_y * 0.003;
		angle->y -= mickey_x * 0.003;
	}
	if(keys[KEY_O])
	{
		p_swim = !p_swim;
		keys[KEY_S] = 0;
		log_info("Swimming is %s.", p_swim ? "engaged" : "off");
	}
	if(keys[KEY_P])
	{
		log_info("Current Position is\n"
			"vec4 position = {{%f, %f, %f, 0.0}};\n"
			"vec4 angle = {{%f, %f, %f, M_PI*0.5}};",
			position->x, position->y, position->z,
			angle->x, angle->y, angle->z);
		keys[KEY_P] = 0;
	}

	if(p_swim)
	{
		float cx = cos(angle->x), sx = sin(angle->x), ty = requested.y;
		requested.y = requested.y * cx - requested.z * sx; // around x
		requested.z = ty * sx + requested.z * cx;
	}


	float cy = cos(angle->y), sy = sin(angle->y), tx = requested.x;
	requested.x = requested.x * cy + requested.z * sy; // around y
	requested.z = tx * sy - requested.z * cy;

	position->xyz = add(position->xyz, requested);
}
