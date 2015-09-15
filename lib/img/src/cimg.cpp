#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <stdio.h>
#include <stdlib.h>
#include "../inc/cimg.h"
#include <unistd.h>
#include <sys/stat.h>
#define CTRACE printf("@@@@@@@@@@@@@@@ %s %s %d \n", __FUNCTION__, __FILE__, __LINE__)
using namespace cv;

static Mat recognition_data[RECOGNITION_DATA_NUM];
static const char *recognition_data_string = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";

static void cimg_init()
{
	int i;
	char name[32];
	Mat  tmp;
	for (i=0; i<36; i++)
	{
		sprintf(name, "data/img/%d.png", i);
		tmp = imread(name);
		resize(tmp, recognition_data[i], Size(CAPTCHA_WIDTH, CAPTCHA_HEIGHT));
	}	
}
#if 0
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
	unsigned i, j;

	/*	Get the rect of target character	*/
	for( i = 0; i < contours.size(); i++ )
        rect[i] = boundingRect(contours[i]);

	unsigned data_num = 18;
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
#endif

/*
 * Check the data exists. 
 */ 
static bool is_data_exist(char *file_path)
{
	struct stat sts;
	if ((stat(file_path, &sts)) == -1)
	{
		return false;
	}
	else
	{
		return true;
	}
}

static int total_training_data()
{
	int i, j, total = 0;
	char path[CIMG_STR_LEN];
	for (i=0; i<RECOGNITION_DATA_NUM; i++)
	{
		/* max training */
		for (j=0; j<MAX_TRAINGIN_PATTERNS; j++) 
		{
			sprintf(path, "data/img/%c/%d.png", recognition_data_string[i], j);
			if (is_data_exist(path))
				total++;
			else
				break;
		}
	}
	return total;
}

static void cimg_test_data_training()
{
	int i;
	vector<string> img_path;
	printf("training data \n");



	CvMat *data_mat, *res_mat; 
	Mat input_img, resize_img;
	int  n = 0;
	input_img = imread("data/img/2/0.png");
	resize(input_img, resize_img, Size(64, 64));
	//imshow("resize_img", resize_img);
	//waitKey(0);

	data_mat = cvCreateMat( 2, 1764, CV_32FC1 );  
    cvSetZero( data_mat );   
    res_mat = cvCreateMat( 2, 1, CV_32FC1 );  
    cvSetZero( res_mat );  

	{
		HOGDescriptor *hog = new 
			HOGDescriptor(cvSize(64,64),cvSize(16,16),cvSize(8,8),cvSize(8,8),9); 
		vector<float>descriptors;
	    hog->compute(resize_img, descriptors,Size(1,1), Size(0,0));     
		printf("HOG dims: %d \n",descriptors.size());  
		n = 0;
		i = 0;
		for(vector<float>::iterator iter=descriptors.begin();iter!=descriptors.end();iter++)  
	    {  
			if (n < 10)
				printf("%f\n", *iter);
	        cvmSet(data_mat, i, n, *iter);  
	        n++;  
	    }  
		cvmSet(res_mat, i, 0, 0);  
	}
	printf("\n");
	{
		i = 1; n = 0;
		input_img = imread("data/img/3/0.png");
		resize(input_img, resize_img, Size(64, 64));
		HOGDescriptor *hog2 = new 
			HOGDescriptor(cvSize(64,64),cvSize(16,16),cvSize(8,8),cvSize(8,8),9); 
		vector<float>descriptors2;
	    hog2->compute(resize_img, descriptors2,Size(1,1), Size(0,0));     
		printf("HOG dims: %d \n",descriptors2.size());  
		n = 0;
		i = 1;
		for(vector<float>::iterator iter=descriptors2.begin();iter!=descriptors2.end();iter++)  
	    {  
			if (n < 10)
				printf("%f\n", *iter);
	        cvmSet(data_mat, i, n, *iter);  
	        n++;  
	    }  
		cvmSet(res_mat, i, 0, 1);  
	}


	CvSVM svm ;//= CvSVM();    
    CvSVMParams param;    
    CvTermCriteria criteria;    
    criteria = cvTermCriteria( CV_TERMCRIT_EPS, 1000, FLT_EPSILON );    
    param = CvSVMParams( CvSVM::C_SVC, CvSVM::LINEAR, 10.0, 0.09, 1.0, 10.0, 0.5, 1.0, NULL, criteria );    
	
	svm.train( data_mat, res_mat, Mat(), Mat(), param );
    svm.save( "SVM_DATA.xml" ); 

	{
		i = 1; n = 0;
		input_img = imread("data/img/2/0.png");
		resize(input_img, resize_img, Size(64, 64));
		HOGDescriptor *hog2 = new 
			HOGDescriptor(cvSize(64,64),cvSize(16,16),cvSize(8,8),cvSize(8,8),9); 
		vector<float>descriptors;
	    hog2->compute(resize_img, descriptors,Size(1,1), Size(0,0));     
		printf("HOG dims: %d \n",descriptors.size());  
		CvMat* SVMtrainMat=cvCreateMat(1,descriptors.size(),CV_32FC1);  
        n=0;  
        for(vector<float>::iterator iter=descriptors.begin();iter!=descriptors.end();iter++)  
        {  
			if (n < 10)
				printf("SVMtrainMat %f\n", *iter);
            cvmSet(SVMtrainMat,0,n,*iter);  
            n++;  
        }  
		printf("predict = %f \n", svm.predict(SVMtrainMat));
	}
	cvReleaseMat( &data_mat );  
	cvReleaseMat( &res_mat );  
/*
	for (i=0; i<10; i++)
	{
		for (j=0; j<10; j++)
		{
			sprintf(str, "data/img/%d/%d.png", i, j);
			if (is_data_exist(str))
				
		}
	}
*/	
}
#if 1
static void cimg_train_data()
{
	int i, j;
	vector<string> img_path;
	printf("training data \n");

	CvMat *data_mat, *res_mat; 
	Mat input_img, resize_img;
	int  cnt, n = 0;
	int total_data = total_training_data();
	printf("total_data = %d \n", total_data);
	data_mat = cvCreateMat( total_data, 1764, CV_32FC1 );  
    cvSetZero( data_mat );   
    res_mat = cvCreateMat( total_data, 1, CV_32FC1 );  
    cvSetZero( res_mat );  
	char path[CIMG_STR_LEN];
	cnt = 0;
	for (i=0; i<RECOGNITION_DATA_NUM; i++)
	{
		/* max training size */
		for (j=0; j<100; j++) 
		{
			sprintf(path, "data/img/%c/%d.png", recognition_data_string[i], j);
			if (!is_data_exist(path)) break;
			input_img = imread(path);
			HOGDescriptor *hog = new 
				HOGDescriptor(cvSize(CAPTCHA_WIDTH, CAPTCHA_HEIGHT), cvSize(8, 8), cvSize(4, 4), cvSize(4, 4), 9); 
			vector<float>descriptors;
		    hog->compute(input_img, descriptors, Size(1,1), Size(0,0));     
			n = 0;
			for(vector<float>::iterator iter=descriptors.begin();iter!=descriptors.end();iter++)  
		    {  
		        cvmSet(data_mat, cnt, n, *iter);  
		        n++;  
		    }  
		//	printf("@@@@@@@@@@@@@ cnt = %d, i = %d \n", cnt, i);
			cvmSet(res_mat, cnt, 0, i);
			cnt++;
		}
	}
	/* training data by SVM */
	SVM  svm; 
	SVMParams  param;    
    CvTermCriteria criteria;    
	criteria = cvTermCriteria( CV_TERMCRIT_EPS, 1000, FLT_EPSILON );    
    param = CvSVMParams( CvSVM::C_SVC, CvSVM::LINEAR, 10.0, 0.09, 1.0, 10.0, 0.5, 1.0, NULL, criteria );    
	svm.train( data_mat, res_mat, NULL, NULL, param );     
	svm.save("SVM_DATA.xml"); 
/*
	{
		input_img = imread("3.png");
		HOGDescriptor *hog = new 
				HOGDescriptor(cvSize(CAPTCHA_WIDTH, CAPTCHA_HEIGHT), cvSize(8, 8), cvSize(4, 4), cvSize(4, 4), 9); 
			vector<float>descriptors;
	    hog->compute(input_img, descriptors,Size(1,1), Size(0,0));     
		printf("HOG dims: %d \n",descriptors.size());  
		CvMat* SVMtrainMat=cvCreateMat(1,descriptors.size(),CV_32FC1);  
        n=0;  
        for(vector<float>::iterator iter=descriptors.begin();iter!=descriptors.end();iter++)  
        {  
            cvmSet(SVMtrainMat,0,n,*iter);  
            n++;  
        }  
		printf("predict = %f \n", svm.predict(SVMtrainMat));
	}

	for (i=0; i<10; i++)
	{
		for (j=0; j<10; j++)
		{
			sprintf(str, "data/img/%d/%d.png", i, j);
			if (is_data_exist(str))
				
		}
	}
*/	
}
#endif

/*
static double get_mse(Mat img1, Mat img2)
{
	Mat s1, gimg2;
	cvtColor(img2, gimg2, CV_RGBA2GRAY);
	absdiff(img1, gimg2, s1);
	s1 = s1.mul(s1);
    Scalar s = sum(s1);         // sum elements per channel
    return s.val[0] + s.val[1] + s.val[2]; // sum channels
}
*/

static char get_recognition_result(cimg *c, Mat img)
{
	char result = 'A';	
#if 0
	
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
	
#else
	int n;
	HOGDescriptor *hog = new 
				HOGDescriptor(cvSize(CAPTCHA_WIDTH, CAPTCHA_HEIGHT), cvSize(8, 8), cvSize(4, 4), cvSize(4, 4), 9); 
	vector<float>descriptors;
	hog->compute(img, descriptors, Size(1, 1), Size(0, 0));     
	CvMat* SVMtrainMat = cvCreateMat(1, descriptors.size(), CV_32FC1);  
    n=0;  
    for(vector<float>::iterator iter = descriptors.begin(); iter!=descriptors.end(); iter++)  
    {  
        cvmSet(SVMtrainMat, 0, n, *iter);  
        n++;  
    }  
	result = recognition_data_string[(int)c->ops->svm.predict(SVMtrainMat)];
#endif	
	return result;
}

static bool cimg_get_captcha(cimg *c, char *file_path, char *captcha)
{		
	if (file_path != NULL && captcha != NULL)
	{
	//	printf("CV Version %s \n", CV_VERSION);

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
		unsigned i, j;
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
			captcha[i] = get_recognition_result(c, result);
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
		ops.train_data	= cimg_train_data;
		ops.init 		= true;
		ops.svm.load("SVM_DATA.xml");
	}
	c->ops = &ops;
	return c;
err:
	if (c)
		free(c);
	return NULL;
}

