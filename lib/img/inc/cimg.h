#ifndef _C_IMG_H_
#define _C_IMG_H_

typedef struct _cimg_
{
	struct cimg_ops *ops;
} cimg;

struct cimg_ops
{
	bool (*get_captcha)(char *filepath);
};

extern cimg *cimg_new(void);

#endif