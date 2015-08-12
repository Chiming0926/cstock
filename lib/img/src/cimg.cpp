#include <stdio.h>
#include <stdlib.h>
#include "../inc/cimg.h"


cimg *cimg_new(void)
{
	cimg *c = NULL;
	c = (cimg*)malloc(sizeof(cimg));
	if (c == NULL)
		goto err;
	
	//c->ops = &ops;
	return c;
err:
	if (c)
		free(c);
	return NULL;
}

