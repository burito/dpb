#include "global.h"
#include "log.h"

int main_init(int argc, char *argv[])
{

	return 0;
}

void main_loop(void)
{
	if(keys[KEY_ESCAPE])
	{
		log_info("Shutdown on : Button press (ESC)");
		killme=1;
	}
}

void main_end(void)
{

}

void gfx_resize(void)
{

}


#ifdef __APPLE__

void osx_view_init(void)
{

}


#endif
