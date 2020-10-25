/*
Copyright (c) 2013 Daniel Burke

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
#include <string.h>
#include <stdlib.h>

#include <vulkan/vulkan.h>

#include "vulkan.h"
#include "vulkan_helper.h"


#include <stb/stb_image.h>
#include "mesh_vulkan.h"

#define LOG_NO_TRACE
#include "log.h"


extern VkFormat pixel_format;
extern VkColorSpaceKHR color_space;

static void img_vulkan_init(struct IMAGE_VULKAN *img)
{

	log_trace("img_vulkan_init()");
	VkResult result;
	struct VULKAN_BUFFER staging;

	VkDeviceSize buffer_size = img->size.x * img->size.y * ( img->channels );
	VkFormat format;

	switch(img->channels){
	case 1: format = VK_FORMAT_R8_UNORM; break;
	case 2: format = VK_FORMAT_R8G8_UNORM; break;
	case 4: format = VK_FORMAT_R8G8B8A8_SRGB; break;
	default:
		log_error("Unexpected channel count");
		return;
	}

	result = vk_buffer( VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
		VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &staging, buffer_size, img->buffer);
	if(result != VK_SUCCESS)
	{
		log_warning("vk_buffer = %s", vulkan_result(result));
	}

	VkImageCreateInfo image_crinf = {
		VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,	// VkStructureType          sType;
		NULL,					// const void*              pNext;
		0,					// VkImageCreateFlags       flags;
		VK_IMAGE_TYPE_2D,			// VkImageType              imageType;
		format,					// VkFormat                 format;
		{img->size.x, img->size.y, 1},		// VkExtent3D               extent;
		1,					// uint32_t                 mipLevels;
		1,					// uint32_t                 arrayLayers;
		VK_SAMPLE_COUNT_1_BIT,			// VkSampleCountFlagBits    samples;
		VK_IMAGE_TILING_OPTIMAL,		// VkImageTiling            tiling;
		VK_IMAGE_USAGE_TRANSFER_DST_BIT |	// VkImageUsageFlags        usage;
		VK_IMAGE_USAGE_SAMPLED_BIT,
		VK_SHARING_MODE_EXCLUSIVE,		// VkSharingMode            sharingMode;
		1,					// uint32_t                 queueFamilyIndexCount;
		&vk.desired_queuefamily,		// const uint32_t*          pQueueFamilyIndices;
		VK_IMAGE_LAYOUT_UNDEFINED		// VkImageLayout            initialLayout;
	};

	result = vkCreateImage(vk.device, &image_crinf, NULL, &img->vk.image);
	if(result != VK_SUCCESS)
	{
		log_warning("vkCreateImage = %s", vulkan_result(result));
	}

	VkMemoryRequirements memory_requirements;
	vkGetImageMemoryRequirements(vk.device, img->vk.image, &memory_requirements);
	uint32_t memory_type = find_memory_type(memory_requirements, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	buffer_size = memory_requirements.size;

	VkMemoryAllocateInfo alloc_info = {
		VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,	// VkStructureType    sType;
		NULL,					// const void*        pNext;
		memory_requirements.size,		// VkDeviceSize       allocationSize;
		memory_type				// uint32_t           memoryTypeIndex;
	};

	result = vkAllocateMemory(vk.device, &alloc_info, NULL, &img->vk.memory);
	if(result != VK_SUCCESS)
	{
		log_warning("vkAllocateMemory = %s", vulkan_result(result));
	}
	result = vkBindImageMemory(vk.device, img->vk.image, img->vk.memory, 0);
	if(result != VK_SUCCESS)
	{
		log_warning("vkBindImageMemory = %s", vulkan_result(result));
	}

	VkCommandBuffer cmdbuf;
	VkCommandBufferAllocateInfo cmdbufallocinf = {
		VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,	// VkStructureType         sType;
		NULL,						// const void*             pNext;
		vk.commandpool,					// VkCommandPool           commandPool;
		VK_COMMAND_BUFFER_LEVEL_PRIMARY,		// VkCommandBufferLevel    level;
		1						// uint32_t                commandBufferCount;
	};
	result = vkAllocateCommandBuffers(vk.device, &cmdbufallocinf, &cmdbuf);
	if( result != VK_SUCCESS )
	{
		log_fatal("vkAllocateCommandBuffers = %s", vulkan_result(result));
//		goto TODO_ERROR_HANDLING;
	}

	VkCommandBufferBeginInfo vk_cmdbegin = {
		VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,	// VkStructureType                          sType;
		NULL,						// const void*                              pNext;
		VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT,	// VkCommandBufferUsageFlags                flags;
		NULL						// const VkCommandBufferInheritanceInfo*    pInheritanceInfo;
	};
	result = vkBeginCommandBuffer(cmdbuf, &vk_cmdbegin);
	if( result != VK_SUCCESS )
	{
		log_fatal("vkBeginCommandBuffer = %s", vulkan_result(result));
//		goto TODO_ERROR_HANDLING;
	}

	VkImageMemoryBarrier buffer_copy_barrier = {
		VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,		// VkStructureType            sType;
		NULL,						// const void*                pNext;
		0,						// VkAccessFlags              srcAccessMask;
		VK_ACCESS_TRANSFER_WRITE_BIT,			// VkAccessFlags              dstAccessMask;
		VK_IMAGE_LAYOUT_UNDEFINED,			// VkImageLayout              oldLayout;
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,		// VkImageLayout              newLayout;
		0,						// uint32_t                   srcQueueFamilyIndex;
		0,						// uint32_t                   dstQueueFamilyIndex;
		img->vk.image,					// VkImage                    image;
		{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 }	// VkImageSubresourceRange    subresourceRange;
	};

	vkCmdPipelineBarrier(cmdbuf,
		VK_PIPELINE_STAGE_HOST_BIT,
		VK_PIPELINE_STAGE_TRANSFER_BIT,
		0, 0, NULL, 0, NULL, 1, &buffer_copy_barrier);

	VkBufferImageCopy buffer_image_copy = {
		0,			// VkDeviceSize                bufferOffset;
		img->size.x,		// uint32_t                    bufferRowLength;
		img->size.y,		// uint32_t                    bufferImageHeight;
		{			// VkImageSubresourceLayers    imageSubresource;
			VK_IMAGE_ASPECT_COLOR_BIT,	// VkImageAspectFlags    aspectMask;
			0,				// uint32_t              mipLevel;
			0,				// uint32_t              baseArrayLayer;
			1				// uint32_t              layerCount;
		},{ 0, 0, 0},			// VkOffset3D                  imageOffset;
		{ img->size.x, img->size.y, 1}	// VkExtent3D                  imageExtent;
	};

	vkCmdCopyBufferToImage(cmdbuf, staging.buffer, img->vk.image,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &buffer_image_copy );

	VkImageMemoryBarrier image_barrier = {
		VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,		// VkStructureType            sType;
		NULL,						// const void*                pNext;
		VK_ACCESS_TRANSFER_WRITE_BIT,			// VkAccessFlags              srcAccessMask;
		VK_ACCESS_SHADER_READ_BIT,			// VkAccessFlags              dstAccessMask;
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,		// VkImageLayout              oldLayout;
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,	// VkImageLayout              newLayout;
		0,						// uint32_t                   srcQueueFamilyIndex;
		0,						// uint32_t                   dstQueueFamilyIndex;
		img->vk.image,					// VkImage                    image;
		{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 }	// VkImageSubresourceRange    subresourceRange;
	};

	vkCmdPipelineBarrier(cmdbuf,
		VK_PIPELINE_STAGE_TRANSFER_BIT,
		VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
		0, 0, NULL, 0, NULL, 1, &image_barrier);

	img->vk.image_layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	result = vkEndCommandBuffer(cmdbuf);
	if( result != VK_SUCCESS )
	{
		log_fatal("vkEndCommandBuffer = %s", vulkan_result(result));
//		goto TODO_ERROR_HANDLING;
	}

	VkPipelineStageFlags vkflags = VK_PIPELINE_STAGE_TRANSFER_BIT;
	VkSubmitInfo submit_info = {
		VK_STRUCTURE_TYPE_SUBMIT_INFO,		// VkStructureType                sType;
		NULL,					// const void*                    pNext;
		0,					// uint32_t                       waitSemaphoreCount;
		NULL,					// const VkSemaphore*             pWaitSemaphores;
		&vkflags,				// const VkPipelineStageFlags*    pWaitDstStageMask;
		1,					// uint32_t                       commandBufferCount;
		&cmdbuf,				// const VkCommandBuffer*         pCommandBuffers;
		0,					// uint32_t                       signalSemaphoreCount;
		NULL					// const VkSemaphore*             pSignalSemaphores;
	};
	result = vkQueueSubmit(vk.queue, 1, &submit_info, VK_NULL_HANDLE);
	if( result != VK_SUCCESS )
	{
		log_fatal("vkQueueSubmit = %s", vulkan_result(result));
//		goto TODO_ERROR_HANDLING;
	}
	result = vkQueueWaitIdle(vk.queue);
	if( result != VK_SUCCESS )
	{
		log_fatal("vkQueueWaitIdle = %s", vulkan_result(result));
//		goto TODO_ERROR_HANDLING;
	}

	vkFreeCommandBuffers(vk.device, vk.commandpool, 1, &cmdbuf);
	vkFreeMemory(vk.device, staging.memory, NULL);
	vkDestroyBuffer(vk.device, staging.buffer, NULL);

	VkImageViewCreateInfo image_view_crinf = {
		VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,	// VkStructureType            sType;
		NULL,						// const void*                pNext;
		0,						// VkImageViewCreateFlags     flags;
		img->vk.image,					// VkImage                    image;
		VK_IMAGE_VIEW_TYPE_2D,				// VkImageViewType            viewType;
		format,						// VkFormat                   format;
		{	VK_COMPONENT_SWIZZLE_IDENTITY,		// VkComponentMapping         components;
			VK_COMPONENT_SWIZZLE_IDENTITY,
			VK_COMPONENT_SWIZZLE_IDENTITY,
			VK_COMPONENT_SWIZZLE_IDENTITY
		},
		{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 }	// VkImageSubresourceRange    subresourceRange;
	};

	result = vkCreateImageView(vk.device, &image_view_crinf, NULL, &img->vk.imageview);
	if( result != VK_SUCCESS )
	{
		log_warning("vkCreateImageView = %s", vulkan_result(result));
	}
}


void img_vulkan_end(struct IMAGE_VULKAN *img)
{
	vkDestroyImageView(vk.device, img->vk.imageview, NULL);
	vkDestroyImage(vk.device, img->vk.image, NULL);
	vkFreeMemory(vk.device, img->vk.memory, NULL);
}

void img_free(struct IMAGE_VULKAN *img)
{
	img_vulkan_end(img);
	if(img->name)free(img->name);
	if(img->buffer)stbi_image_free(img->buffer);
	free(img);
}


struct IMAGE_VULKAN* img_load(const char * filename)
{
	log_info("Loading Image(\"%s\")", filename);
	const int size = sizeof(struct IMAGE_VULKAN);
	struct IMAGE_VULKAN *i = malloc(size);
	if(!i)return 0;
	memset(i, 0, size);
	i->name = strdup(filename);
	stbi_info(filename,  &i->size.x, &i->size.y, &i->channels);
	int wanted = i->channels;
	if(wanted == 3)wanted = 4;
	i->buffer = stbi_load(filename, &i->size.x, &i->size.y, &i->channels, wanted);
	i->channels = wanted;
	img_vulkan_init(i);
	return i;
}


/*
// TODO: at some point, should query available formats, and find best matches for ones I'm interested in
	for(int i = 1; i< 183; i++)
	{
		format = i;
		VkImageFormatProperties img_format_properties;
		result = vkGetPhysicalDeviceImageFormatProperties(vk.physical_device, format,
			VK_IMAGE_TYPE_2D,
			VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
			0,
			&img_format_properties
			);
		if(result != VK_ERROR_FORMAT_NOT_SUPPORTED)
		{
			log_warning("vkGetPhysicalDeviceImageFormatProperties = %s - %s", vulkan_result(result), vulkan_format(format));
		}

	}
	log_info("done!");
*/



struct MESH_VULKAN* mesh_load(char *filename)
{
	struct WF_OBJ *wf = wf_load(filename);
	if(!wf)return NULL;

	struct MESH_VULKAN *w;
	w = malloc( sizeof(struct MESH_VULKAN) );
	if( w == NULL )
	{
		log_fatal("malloc(MESH_VULKAN) = %s", strerror(errno));
		return NULL;
	}
	memset(w, 0, sizeof(struct MESH_VULKAN) );
	w->wf = wf;

	VkResult result;
	struct VULKAN_BUFFER vertex_staging;
	struct VULKAN_BUFFER index_staging;
	VkDeviceSize vertex_buffer_size = w->wf->num_corners * 32;
	VkDeviceSize index_buffer_size = w->wf->num_triangles*12;

	vk_buffer(VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
		VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		&vertex_staging, vertex_buffer_size, w->wf->vertex_buffer_data );

	vk_buffer(VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
		VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		&index_staging, index_buffer_size, w->wf->index_buffer_data );

	vk_buffer(VK_BUFFER_USAGE_TRANSFER_DST_BIT |
		VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		&w->vertex_buffer, vertex_staging.size, NULL);

	vk_buffer(VK_BUFFER_USAGE_TRANSFER_DST_BIT |
		VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		&w->index_buffer, index_staging.size, NULL);

	VkCommandBuffer cmdbuf;
	VkCommandBufferAllocateInfo cmdbufallocinf = {
		VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,	// VkStructureType         sType;
		NULL,						// const void*             pNext;
		vk.commandpool,					// VkCommandPool           commandPool;
		VK_COMMAND_BUFFER_LEVEL_PRIMARY,		// VkCommandBufferLevel    level;
		1						// uint32_t                commandBufferCount;
	};
	result = vkAllocateCommandBuffers(vk.device, &cmdbufallocinf, &cmdbuf);
	if( result != VK_SUCCESS )
	{
		log_fatal("vkAllocateCommandBuffers = %s", vulkan_result(result));
//		goto TODO_ERROR_HANDLING;
	}

	VkCommandBufferBeginInfo vk_cmdbegin = {
		VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,	// VkStructureType                          sType;
		NULL,						// const void*                              pNext;
		VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT,	// VkCommandBufferUsageFlags                flags;
		NULL						// const VkCommandBufferInheritanceInfo*    pInheritanceInfo;
	};
	result = vkBeginCommandBuffer(cmdbuf, &vk_cmdbegin);
	if( result != VK_SUCCESS )
	{
		log_fatal("vkBeginCommandBuffer = %s", vulkan_result(result));
//		goto TODO_ERROR_HANDLING;
	}

	VkBufferCopy region = {
		0,	// VkDeviceSize    srcOffset;
		0,	// VkDeviceSize    dstOffset;
		0	// VkDeviceSize    size;
	};
	region.size = vertex_staging.size;
	vkCmdCopyBuffer(cmdbuf, vertex_staging.buffer, w->vertex_buffer.buffer, 1, &region);
	region.size = index_staging.size;
	vkCmdCopyBuffer(cmdbuf, index_staging.buffer, w->index_buffer.buffer, 1, &region);
	result = vkEndCommandBuffer(cmdbuf);
	if( result != VK_SUCCESS )
	{
		log_fatal("vkEndCommandBuffer = %s", vulkan_result(result));
//		goto TODO_ERROR_HANDLING;
	}

	VkPipelineStageFlags vkflags = VK_PIPELINE_STAGE_TRANSFER_BIT;
	VkSubmitInfo submit_info = {
		VK_STRUCTURE_TYPE_SUBMIT_INFO,		// VkStructureType                sType;
		NULL,					// const void*                    pNext;
		0,					// uint32_t                       waitSemaphoreCount;
		NULL,					// const VkSemaphore*             pWaitSemaphores;
		&vkflags,				// const VkPipelineStageFlags*    pWaitDstStageMask;
		1,					// uint32_t                       commandBufferCount;
		&cmdbuf,				// const VkCommandBuffer*         pCommandBuffers;
		0,					// uint32_t                       signalSemaphoreCount;
		NULL					// const VkSemaphore*             pSignalSemaphores;
	};
	result = vkQueueSubmit(vk.queue, 1, &submit_info, VK_NULL_HANDLE);
	if( result != VK_SUCCESS )
	{
		log_fatal("vkQueueSubmit = %s", vulkan_result(result));
//		goto TODO_ERROR_HANDLING;
	}
	result = vkQueueWaitIdle(vk.queue);
	if( result != VK_SUCCESS )
	{
		log_fatal("vkQueueWaitIdle = %s", vulkan_result(result));
//		goto TODO_ERROR_HANDLING;
	}

	vkFreeCommandBuffers(vk.device, vk.commandpool, 1, &cmdbuf);
	vkFreeMemory(vk.device, index_staging.memory, NULL);
	vkDestroyBuffer(vk.device, index_staging.buffer, NULL);
	vkFreeMemory(vk.device, vertex_staging.memory, NULL);
	vkDestroyBuffer(vk.device, vertex_staging.buffer, NULL);

	w->index_type = VK_INDEX_TYPE_UINT32;

	w->num_materials = wf->num_materials;
	if(w->num_materials)
	{
		w->materials = malloc( sizeof(struct MATERIAL_VULKAN) * w->num_materials );
		if( w->materials == NULL )
		{
			log_fatal("malloc(materials) = %s", strerror(errno));
			return NULL;
		}
		memset(w->materials, 0, sizeof(struct MATERIAL_VULKAN) * w->num_materials );
		for(int i=0; i<w->num_materials; i++)
		{
			if(wf->materials[i].filename)
			{
				w->materials[i].filename = strdup(wf->materials[i].filename);
				w->materials[i].image = img_load(w->materials[i].filename);
			}
		}
	}

	return w;
}

void wf_gpu_unload(struct MESH_VULKAN *w)
{
	vkFreeMemory(vk.device, w->index_buffer.memory, NULL);
	vkDestroyBuffer(vk.device, w->index_buffer.buffer, NULL);
	vkFreeMemory(vk.device, w->vertex_buffer.memory, NULL);
	vkDestroyBuffer(vk.device, w->vertex_buffer.buffer, NULL);
}
