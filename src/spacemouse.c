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

#include <stdint.h>
#include "3dmaths.h"
#include "log.h"
#include "hidapi.h"

vec3 spacemouse_translation = {{0,0,0}};
vec3 spacemouse_rotation = {{0,0,0}};

static hid_device *spacemouse = NULL;

// Returns 1 if it's a known Spacemouse, 0 otherwise
static int spacemouse_supported_device(struct hid_device_info *device)
{
	// https://www.3dconnexion.eu/nc/service/faqs/faq/how-can-i-check-if-my-usb-3d-mouse-is-recognized-by-windows.html
	switch(device->vendor_id) {
	case 0x256F:
		switch(device->product_id) {
		case 0xC633:	// SpaceMouse Enterprise
		case 0xC652:	// Universal Receiver
		case 0xC635:	// SpaceMouse Compact
		case 0xC632:	// SpaceMouse Pro Wireless Receiver
		case 0xC631:	// SpaceMouse Pro Wireless (cabled
		case 0xC62F:	// SpaceMouse Wireless Receiver
		case 0xC62E:	// SpaceMouse Wireless (cabled
			return 1;
		}
		break;
	case 0x046D:
		switch(device->product_id) {
		case 0xC62B:	// SpaceMouse Pro
		case 0xC629:	// SpacePilot Pro
		case 0xC627:	// SpaceExplorer
		case 0xC626:	// SpaceNavigator
		case 0xC628:	// SpaceNavigator for Notebooks
		case 0xC623:	// SpaceTraveler
		case 0xC625:	// SpacePilot
		case 0xC621:	// SpaceBall 5000 USB
		case 0xC603:	// SpaceMouse Plus USB
//		case 0xC603:	// SpaceMouse Plus XT USB
		case 0xC606:	// SpaceMouse Classic USB
		case 0xC605:	// CadMan
			return 1;
		}
		break;
	}
	return 0;
}


// Open comms with a spacemouse, if it's plugged in
void spacemouse_init(void)
{
	int result;
	result = hid_init();
	if( result != 0 )
	{
		log_warning("hid_init() = %d", result);
		return;
	}

	struct hid_device_info *hid_device_info;
	hid_device_info = hid_enumerate(0,0);
	if(hid_device_info == NULL)
	{
		log_error("hid_enumerate() failed");
		return;
	}

	uint16_t vendor_id = 0;
	uint16_t product_id = 0;

	struct hid_device_info *devices = hid_device_info;
	while(devices)
	{
		if(spacemouse_supported_device(devices))
		{
			vendor_id = devices->vendor_id;
			product_id = devices->product_id;
			log_info("Found \"%ls\"", devices->product_string);
		}
		devices = devices->next;
	}
	hid_free_enumeration(hid_device_info);

	spacemouse = hid_open(vendor_id, product_id, NULL);
	if(spacemouse == NULL)
	{
		#if defined(_WIN32) || defined(__APPLE__)
		log_warning("hid_open() failed");
		#else
		log_warning("hid_open() failed\n"
			"*** Run the following command to fix\n"
			"sudo bash -c \"cat >> /etc/udev/rules.d/51-spacemouse.rules\" << EOF\n"
			"ATTRS{idVendor}==\"%04hx\", ATTRS{idProduct}==\"%04hx\", "
			"TAG+=\"uaccess\"\n"
			"EOF\n"
			"*** Then unplug and replug the device.\n",
			vendor_id, product_id);
		// https://askubuntu.com/questions/978552/how-do-i-make-libusb-work-as-non-root
		#endif
		return;
	}

	result = hid_set_nonblocking(spacemouse, 1);
	if( result != 0 )
	{
		log_warning("hid_set_nonblocking() = %d", result);
		return;
	}

	spacemouse_translation = (vec3){{0,0,0}};
	spacemouse_rotation = (vec3){{0,0,0}};
}


// get the current status of the spacemouse, if it's plugged in
void spacemouse_tick(void)
{
	// if we haven't found a spacemouse
	if( spacemouse == NULL )
	{
		return;
	}

	// the message the spacemouse sends
	struct spacemouse_message {
		uint8_t type;
		int16_t x;
		int16_t y;
		int16_t z;
	} __attribute__((packed));

	// the maximum magnitude of each axis
	#define SPACEMOUSE_MAX 350

	struct spacemouse_message message;
	int result;
	// while there is a message to read
	while( 0 < (result = hid_read(spacemouse,
				(unsigned char*)&message,
				sizeof(struct spacemouse_message))
		))
	{
		// is the message the correct size?
		if(result != sizeof(struct spacemouse_message))
		{
			log_warning("unexpected message size = %d", result);
			continue;
		}
		switch(message.type) {
		case 1:	// translation
			spacemouse_translation = (vec3){{
				(float)message.x / SPACEMOUSE_MAX,
				(float)message.y / SPACEMOUSE_MAX,
				(float)message.z / SPACEMOUSE_MAX
				}};
			break;
		case 2:	// rotation
			spacemouse_rotation = (vec3){{
				(float)message.x / SPACEMOUSE_MAX,
				(float)message.y / SPACEMOUSE_MAX,
				(float)message.z / SPACEMOUSE_MAX
				}};
			break;
		case 3:	// button press
			// we're not using the buttons. The official drivers
			// spawn their own custom menu when you press the
			// primary button, making it useless if they are
			// installed
			break;
		default:
			log_warning("unexpected type=%d, x=%d, y=%d, z=%d",
				message.type, message.x, message.y, message.z);
		}
	}
}

// shutdown the interface
void spacemouse_shutdown(void)
{
	hid_exit();
}
