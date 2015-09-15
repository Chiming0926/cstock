#ifndef _C_IMG_H_
#define _C_IMG_H_

#include <opencv2/objdetect/objdetect.hpp>
#include <opencv2/ml/ml.hpp>

#define CAPTCHA_IMAGE 		"CaptchaImage.jpg"
#define CAPTCHA_WIDTH 		32
#define CAPTCHA_HEIGHT		32
#define RECOGNITION_DATA_NUM	(36)
#define CIMG_STR_LEN		128
#define MAX_TRAINGIN_PATTERNS 100


typedef struct _cimg_
{
	struct cimg_ops *ops;
} cimg;

struct cimg_ops
{
	bool init;
	CvSVM  svm;
	bool (*get_captcha)(cimg *c, char *file_path,  char *captcha);
	void (*close)(cimg *img);
	void (*train_data)(void);
};

extern cimg *cimg_new(void);

#endif