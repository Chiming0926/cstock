#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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

static bool bshtm_update_data(cdata *d, char *st_num)
{
	if (d)
	{
		printf("d->bshtm_data_buf = %08x %d\n", d->bshtm_data_buf, __LINE__);
		int		read_size, captcha_pos; 
		char	host[STRING_LEN], bsMenu[STRING_LEN], str[STRING_LEN];
		bshtm_post_data	bsdata;
		strcpy(host, "bsr.twse.com.tw");
		strcpy(bsMenu, "/bshtm/bsMenu.aspx");
		if (d->bshtm_data_buf)
		{
			/* connect to bsMenu and get some useful data */
			CTRACE;
			printf("d->bshtm_data_buf = %08x\n", d->bshtm_data_buf);
			d->ops->http_get(d, host, bsMenu, d->bshtm_data_buf, &read_size, 2);
			CTRACE;
			receive_useful_data(d->bshtm_data_buf, read_size, &bsdata);
			/* get captcha */
			sprintf(str, "/bshtm/%s", bsdata.url);
			printf("url = %s \n", str);
			d->ops->http_get(d, host, str, d->bshtm_data_buf, &read_size, 1);
			CTRACE;
			sprintf(str, CAPTCHA_IMAGE);
			
			captcha_pos = get_captcha_start_position(d->bshtm_data_buf, read_size);
			d->ops->save_file(str, d->bshtm_data_buf + captcha_pos, read_size - captcha_pos);
			/* image recognition */
			cimg*	img;
			char	captcha[6];
			img = cimg_new();
			sprintf(str, CAPTCHA_IMAGE);
			if (img->ops->get_captcha(str, captcha))
			{
				/* crack the captcha */
				captcha[5] = '\0';
				char post_data[STRING_LEN*8];

				sprintf(post_data, "__EVENTTARGET=&__EVENTARGUMENT=&__LASTFOCUS=&__VIEWSTATE=%s&__EVENTVALIDATION=%s&RadioButton_Normal=RadioButton_Normal&TextBox_Stkno=%s&CaptchaControl1=%s&btnOK=%%E6%%9F%%A5%%E8%%A9%%A2", 
					    bsdata.__VIEWSTATE, bsdata.__EVENTVALIDATION, st_num, captcha);
				CTRACE;
				d->ops->http_post(d, host, bsMenu, post_data, d->bshtm_data_buf, &read_size, bsdata.session_id);
				CTRACE;
				/* get bshtm data */
				strcpy(str, "/bshtm/bsContent.aspx");
				CTRACE;
				d->ops->http_get(d, host, str, d->bshtm_data_buf, &read_size, 2);
				CTRACE;
				d->ops->save_file("test.asp", d->bshtm_data_buf, read_size);
				CTRACE;
				sprintf(str, "%s.csv", st_num);
				d->ops->save_to_excel(d, d->bshtm_data_buf, read_size, str);	
				if (read_size < 200)
					return false;
				else
					return true;
			}
		}
	}
	return false;
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


