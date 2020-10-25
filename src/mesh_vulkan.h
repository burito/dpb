/*
Copyright (c) 2020 Daniel Burke

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

#include <vulkan/vulkan.h>
#include "vulkan.h"

#include "mesh.h"

struct IMAGE_VULKAN
{
	int2 size;
	int bpp;
	int channels;
	unsigned char *buffer;
	struct VULKAN_TEXTURE vk;
	char *name;
};

struct MATERIAL_VULKAN
{
	char *filename;
	struct IMAGE_VULKAN *image;
};

struct MESH_VULKAN
{
	struct VULKAN_BUFFER vertex_buffer, index_buffer;
	VkIndexType index_type;
	struct MATERIAL_VULKAN *materials;
	int num_materials;
	struct WF_OBJ *wf;
};

struct MESH_VULKAN* mesh_load(char *filename);
void mesh_draw(struct MESH_VULKAN *w);
void mesh_free(struct MESH_VULKAN *w);
