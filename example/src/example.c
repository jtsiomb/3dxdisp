/* example program showing how to use 3dxdisp */
#include <stdio.h>
#include "3dxdisp.h"
#include "logo.h"


int main(void)
{
	/*
	if(tdx_open() == -1) {
		return 1;
	}
	*/
	if (tdx_open_file(NULL) == -1) {
		return 1;
	}

	tdx_blit(0, 0, 240, 64, logo);
	tdx_update();

	tdx_close();
	return 0;
}
