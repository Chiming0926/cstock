#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "bshtm.h"

#define CTRACE printf("@@@@@@@@@@@@@@@ %s %s %d \n", __FUNCTION__, __FILE__, __LINE__);

typedef struct _bshtm_post_data_
{
	char __VIEWSTATE[STRING_LEN*2];
	char __EVENTVALIDATION[STRING_LEN];
	char url[STRING_LEN];
	char session_id[STRING_LEN];
} bshtm_post_data;

static void parse_http_symbol(char *in, char *out)
{
	if (in == NULL || out == NULL)
		return;
	int i;
	int len = strlen(in);
	int count = 0;

	for (i=0; i<len; i++)
	{
		if (in[i] == 0x2F)
		{
			out[count++] 	= 0x25;
			out[count++] 	= 0x32;
			out[count++] 	= 0x46;
			continue;
		}
		else if (in[i] == 0x3D)
		{
			out[count++] 	= 0x25;
			out[count++] 	= 0x33;
			out[count++] 	= 0x44;
		}
		else if (in[i] == 0x2B)
		{
			out[count++] 	= 0x25;
			out[count++] 	= 0x32;
			out[count++] 	= 0x42;
		}
		else
		{
			out[count++] = in[i];
		}
	}
	out[count] = '\0';
}
	
static void receive_useful_data(char *data, int read_size, bshtm_post_data *bsdata)
{
	if (data == NULL || bsdata == NULL)
		return;
	int i;
	int counter = read_size - strlen(CAPTCHA_URL_PREFIX);
	if (counter < 0)
		return;
	for (i=0; i<read_size; i++)
	{
		if (strncmp(data+i, __VIEWSTATE_PREFIX, strlen(__VIEWSTATE_PREFIX)) == 0)
		{
			char buf[STRING_LEN*2];
			memcpy(buf, data+i+24, __VIEWSTATE_DATA_LEN);
			buf[__VIEWSTATE_DATA_LEN] = '\0';
			parse_http_symbol(buf, bsdata->__VIEWSTATE);
		}
		if (strncmp(data+i, __EVENTVALIDATION_PREFIX, strlen(__EVENTVALIDATION_PREFIX)) == 0)
		{
			char buf[STRING_LEN];
			memcpy(buf, data+i+30, __EVENTVALIDATION_LEN);
			buf[__EVENTVALIDATION_LEN] = '\0';
			parse_http_symbol(buf, bsdata->__EVENTVALIDATION);
		}
		if (strncmp(data+i, CAPTCHA_URL_PREFIX, strlen(CAPTCHA_URL_PREFIX)) == 0)
		{
			memcpy(bsdata->url, data+i, strlen(CAPTCHA_URL_PREFIX)+36);
			bsdata->url[strlen(CAPTCHA_URL_PREFIX)+36] = '\0';
		}	
		if (strncmp(data+i, SEESION_ID_PRIFIX, strlen(SEESION_ID_PRIFIX)) == 0)
		{
			memcpy(bsdata->session_id, data+i+30, SEESION_ID_LEN);
			bsdata->session_id[SEESION_ID_LEN] = '\0';
		}
	}
}

static int get_captcha_start_position(char *data, int size)
{
	int i;
	for (i=0; i<size; i++)
	{
		if (data[i] == 0x0D && data[i+1] == 0x0A 
			&& data[i+2] == 0x0D && data[i+3] == 0x0A)
			break;
	}
	return i + 4;
	
}

static int bshtm_update_data(cdata *d, char *st_num)
{
#if 1	
	if (d)
	{
		int		read_size, captcha_pos; 
		char	host[STRING_LEN], bsMenu[STRING_LEN], str[STRING_LEN];
		bshtm_post_data	bsdata;
		strcpy(host, "bsr.twse.com.tw");
		strcpy(bsMenu, "/bshtm/bsMenu.aspx");
		if (d->bshtm_data_buf)
		{
			/* connect to bsMenu and get some useful data */
			d->ops->http_get(d, host, bsMenu, d->bshtm_data_buf, &read_size, 2, NULL);
			receive_useful_data(d->bshtm_data_buf, read_size, &bsdata);
			/* get captcha */
			sprintf(str, "/bshtm/%s", bsdata.url);
			d->ops->http_get(d, host, str, d->bshtm_data_buf, &read_size, 1, NULL);
			sprintf(str, CAPTCHA_IMAGE);
			
			captcha_pos = get_captcha_start_position(d->bshtm_data_buf, read_size);
			d->ops->save_file(str, d->bshtm_data_buf + captcha_pos, read_size - captcha_pos);
			if (read_size < 2000)
				return CHTTP_FORBIDDEN;
			/* image recognition */
			cimg*	img;
			char	captcha[6];
			img = cimg_new();
			sprintf(str, CAPTCHA_IMAGE);
			if (img->ops->get_captcha(img, str, captcha))
			{
				/* crack the captcha */
				captcha[5] = '\0';
				char post_data[STRING_LEN*8];
				printf("captcha = %s \n", captcha);
				sprintf(post_data, "__EVENTTARGET=&__EVENTARGUMENT=&__LASTFOCUS=&__VIEWSTATE=%s&__EVENTVALIDATION=%s&RadioButton_Normal=RadioButton_Normal&TextBox_Stkno=%s&CaptchaControl1=%s&btnOK=%%E6%%9F%%A5%%E8%%A9%%A2", 
					    bsdata.__VIEWSTATE, bsdata.__EVENTVALIDATION, st_num, captcha);
				d->ops->http_post(d, host, bsMenu, post_data, d->bshtm_data_buf, &read_size, bsdata.session_id);
				/* get bshtm data */
				strcpy(str, "/bshtm/bsContent.aspx");
				d->ops->http_get(d, host, str, d->bshtm_data_buf, &read_size, 2, bsdata.session_id);
				if (read_size < 200)
				{
					return CHTTP_FAIL;
				}
				else
				{
					sprintf(str, "%s.csv", st_num);
					d->ops->save_to_excel(d, d->bshtm_data_buf, read_size, str);	
					return CHTTP_OK;
				}
			}
		}
	}
	return CHTTP_FAIL;
#else
	if (d)
	{
		char* 	data_buf;
		int		read_size, data_position; 
		char	host[STRING_LEN], bsMenu[STRING_LEN], str[STRING_LEN];
		bshtm_post_data	bsdata;
		data_buf = (char*)malloc(PAGE_SIZE);
		strcpy(host, "bsr.twse.com.tw");
		strcpy(bsMenu, "/bshtm/bsMenu.aspx");
		if (data_buf)
		{
			/* connect to bsMenu and get some useful data */
			d->ops->http_get(d, host, bsMenu, data_buf, &read_size, 2);
			receive_useful_data(data_buf, read_size, &bsdata);
			memset(data_buf, 0, read_size);
			
			/* get captcha */
			sprintf(str, "/bshtm/%s", bsdata.url);
			d->ops->http_get(d, host, str, data_buf, &read_size, 1);
			sprintf(str, CAPTCHA_IMAGE);
			data_position = get_captcha_start_position(data_buf, read_size);
			d->ops->save_file(str, data_buf + data_position, read_size - data_position);
			//memset(data_buf, 0, read_size);
			
			/* image recognition */
			cimg*	img;
			char	captcha[6];
			img = cimg_new();
			sprintf(str, CAPTCHA_IMAGE);
	//		gets(captcha);
			if (img->ops->get_captcha(img, str, captcha))
			{
				/* crack the captcha */
				captcha[5] = '\0';
				char post_data[STRING_LEN*8];

				sprintf(post_data, "__EVENTTARGET=&__EVENTARGUMENT=&__LASTFOCUS=&__VIEWSTATE=%s&__EVENTVALIDATION=%s&RadioButton_Normal=RadioButton_Normal&TextBox_Stkno=%s&CaptchaControl1=%s&btnOK=%%E6%%9F%%A5%%E8%%A9%%A2", 
					    bsdata.__VIEWSTATE, bsdata.__EVENTVALIDATION, st_num, captcha);
				d->ops->http_post(d, host, bsMenu, post_data, data_buf, &read_size, bsdata.session_id);
				d->ops->save_file("post.data", data_buf, read_size);
				memset(data_buf, 0, read_size);
				/* get bshtm data */
				strcpy(str, "/bshtm/bsContent.aspx");
				d->ops->http_get(d, host, str, data_buf, &read_size, 2);
				sprintf(str, "%s.csv", st_num);
				d->ops->save_to_excel(d, data_buf, read_size, str);
				memset(data_buf, 0, read_size);
				free(data_buf);	
				if (read_size < 200)
					return false;
				else
					return true;
			}
		}
	}
	return false;
#endif
}

static struct bshtm_ops ops;

void set_bshtm_ops(cdata *d)
{
	if (ops.init == false)
	{
		ops.update_data = bshtm_update_data;
		ops.init = true;
	}
	d->bs_ops = &ops;
}


