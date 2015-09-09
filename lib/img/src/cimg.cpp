#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <stdio.h>
#include <stdlib.h>
#include "../inc/cimg.h"


using namespace cv;
using namespace std;

static Mat recognition_data[RECOGNITION_DATA_NUM];
static char *recognition_data_string = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";

static void cimg_init()
{
	int i;
	char name[32];
	for (i=0; i<36; i++)
	{
		sprintf(name, "data/img/%d.png", i);
		recognition_data[i] = imread(name);
	}	
}

static void generate_pattern(char *file, bool last)
{
#if 0	
	printf("CV Version %s \n", CV_VERSION);

	Mat image;
		image = imread(CAPTCHA_IMAGE);
	if (!image.data) // Check for invalid input
	{
		printf("Failed to read the image \n");
    	return ;
	}
	
	/*	do some image processes.  */
	Mat eroded, blur, edge, dilation; 
	Mat element(4, 4, CV_8U,Scalar(1));  
    erode(image, eroded, element);  
	GaussianBlur(eroded, blur, Size(5, 5), 0, 0); 
	Canny(blur, edge, 30, 150);
	dilate(edge, dilation, element, Point(-1,-1));

//	imshow("dilation", dilation);
//	waitKey(0);
	//  
	vector<vector<Point> > contours; 
	vector<Vec4i> hierarchy;
	Mat src_copy = dilation.clone();
	printf("@@@@@@@@@@@@@@@@@@@@@@@ %s %s %d \n", __FUNCTION__, __FILE__, __LINE__);
	findContours( src_copy, contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE);
	printf("@@@@@@@@@@@@@@@@@@@@@@@ %s %s %d \n", __FUNCTION__, __FILE__, __LINE__);
	vector<Rect> rect(contours.size());
#else	
	#if 1
	Mat image;
 	image = imread(file);
	if (!image.data) // Check for invalid input
    {
		printf("Failed to read the image \n");
        return ;
    }
	
	/*	do some image processes.  */
	Mat eroded, blur, edge, dilation, dilation2; 
	Mat element(4, 4, CV_8UC1,Scalar(1));  
    //erode(image, eroded, element);  
	//GaussianBlur(eroded, blur, Size(5, 5), 0, 0); 
	Canny(image, edge, 0, 255);
	dilate(edge, dilation, element, Point(-1,-1));
	dilate(image, dilation2, element, Point(-1,-1));

	vector<vector<Point> > contours; 
	vector<Vec4i> hierarchy;
	Mat src_copy = dilation.clone();
	findContours( src_copy, contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE);
	vector<Rect> rect(contours.size());
	int i, j;

	/*	Get the rect of target character	*/
	for( i = 0; i < contours.size(); i++ )
        rect[i] = boundingRect(contours[i]);

	int data_num = 18;
	Rect parse_rect[data_num];
	j=0;
	for (i=0; i<contours.size(); i++)
	{
		if (rect[i].height > 20)
		{
			parse_rect[j++] = rect[i];
		}
	}
	
	// sort parse_rect
	for (i=0; i<data_num; i++)
	{
		for (j=i+1; j<data_num; j++)
		{
			if (parse_rect[i].x > parse_rect[j].x)
			{
				Rect tmp;
				tmp = parse_rect[j];
				parse_rect[j] = parse_rect[i];
				parse_rect[i] = tmp;
			}
		}
	}
	Mat tmp, final;
	char win_name[32];
	for (i=0; i<data_num; i++)
	{
		tmp = dilation2(parse_rect[i]);
		sprintf(win_name, "data/img/%d.png", last ? i+18 : i);
		resize(tmp, final, Size(CAPTCHA_WIDTH, CAPTCHA_HEIGHT));
		//imshow(win_name, result[i]); 
		imwrite(win_name, final);
	}
	#endif
	#endif

    waitKey(0);    
}


static double get_mse(Mat img1, Mat img2)
{
	Mat s1, gimg2;
	cvtColor(img2, gimg2, CV_RGBA2GRAY);
    absdiff(img1, gimg2, s1);
	s1 = s1.mul(s1);           
    Scalar s = sum(s1);         // sum elements per channel
    return s.val[0] + s.val[1] + s.val[2]; // sum channels
}

static char get_recognition_result(Mat img)
{
	char result = 'A';
	double tmp, mse = 0xFFFFFFFF;
	int i;
	
	for (i=0 ;i<RECOGNITION_DATA_NUM; i++)
	{
		if (recognition_data[i].data)
		{
			tmp = get_mse(img, recognition_data[i]);
			if (mse > tmp)
			{
				mse = tmp;
				result = recognition_data_string[i%36];
			}
		}
	}
	return result;
}

static bool cimg_get_captcha(char *file_path, char *captcha)
{	
	if (file_path != NULL && captcha != NULL)
	{
		printf("CV Version %s \n", CV_VERSION);

		Mat image;
 		image = imread(CAPTCHA_IMAGE);
		if (!image.data) // Check for invalid input
    	{
			printf("Failed to read the image \n");
        	return false;
    	}
		
		/*	do some image processes.  */
		Mat eroded, blur, edge, dilation; 
		Mat element(4, 4, CV_8U,Scalar(1));  
	    erode(image, eroded, element);  
		GaussianBlur(eroded, blur, Size(5, 5), 0, 0); 
		Canny(blur, edge, 30, 150);
		dilate(edge, dilation, element, Point(-1,-1));

		vector<vector<Point> > contours; 
		vector<Vec4i> hierarchy;
		Mat src_copy = dilation.clone();
		findContours( src_copy, contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE);
		vector<Rect> rect(contours.size());

		int i, j;

		/*	Get the rect of target character	*/
		for( i = 0; i < contours.size(); i++ )
	    {
			rect[i] = boundingRect(contours[i]);
		}
		Rect parse_rect[5];
		j=0;
		for (i=0; i<contours.size(); i++)
		{
			if (rect[i].width > 20 && rect[i].height > 20)
			{
				if (rect[i].width > 40)
					return false;
				parse_rect[j++] = rect[i];
			}
		}

		if (j != 5)
		{
			return false;
		}
		
		// sort parse_rect
		for (i=0; i<5; i++)
		{
			for (j=i+1; j<5; j++)
			{
				if (parse_rect[i].x > parse_rect[j].x)
				{
					Rect tmp;
					tmp = parse_rect[j];
					parse_rect[j] = parse_rect[i];
					parse_rect[i] = tmp;
				}
			}
		}
		
		Mat tmp, result;
		i = 0;
		char name[32];
		for (i=0; i<5; i++)
		{
			tmp = dilation(parse_rect[i]);
			resize(tmp, result, Size(CAPTCHA_WIDTH, CAPTCHA_HEIGHT));
			sprintf(name, "%d.png", i);
			imwrite(name, result);
			captcha[i] = get_recognition_result(result);
		}
		return true;
	}
	return false;
}

static void cimg_close(cimg *img)
{
	if (img)
		free(img);
	img = NULL;
}

static cimg_ops ops;

cimg *cimg_new(void)
{
	cimg *c = NULL;
	c = (cimg*)malloc(sizeof(cimg));
	if (c == NULL)
		goto err;
	
	if (ops.init == false)
	{
		cimg_init();
		ops.get_captcha	= cimg_get_captcha;
		ops.close		= cimg_close;
		ops.init 		= true;
	}
	c->ops = &ops;
	return c;
err:
	if (c)
		free(c);
	return NULL;
}

