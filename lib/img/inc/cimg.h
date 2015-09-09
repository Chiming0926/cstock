#ifndef _C_IMG_H_
#define _C_IMG_H_

#define CAPTCHA_IMAGE 		"CaptchaImage.jpg"
#define CAPTCHA_WIDTH 		30
#define CAPTCHA_HEIGHT		30
#define RECOGNITION_DATA_NUM	(36 * 2)

typedef struct _cimg_
{
	struct cimg_ops *ops;
} cimg;

struct cimg_ops
{
	bool init;
	bool (*get_captcha)(char *file_path,  char *captcha);
	void (*close)(cimg *img);
};

extern cimg *cimg_new(void);

#endif