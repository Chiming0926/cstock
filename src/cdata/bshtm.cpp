#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bshtm.h"

static void get_captcha_url(char *data, int read_size, char *url)
{
	if (data == NULL || url == NULL)
		return;
	int i;
	int counter = read_size - strlen(CAPTCHA_URL_PREFIX);
	if (counter < 0)
		return;
	for (i=0; i<read_size; i++)
	{
		if (strncmp(data+i, CAPTCHA_URL_PREFIX, strlen(CAPTCHA_URL_PREFIX)) == 0)
		{
			memcpy(url, data+i, strlen(CAPTCHA_URL_PREFIX)+36);
			url[strlen(CAPTCHA_URL_PREFIX)+36] = '\0';
			printf("url = %s \n", url);
			break;
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

static bool bshtm_update_data(cdata *d)
{
	if (d)
	{
		chttp* 	c;
		char* 	data_buf;
		int		read_size, data_position; 
		char	str[STRING_LEN], url[STRING_LEN];
		data_buf = (char*)malloc(PAGE_SIZE);
		if (data_buf)
		{
			c = chttp_new();
			if (c == NULL)
				goto err;
			strcpy(str, "bsr.twse.com.tw");
			c->ops->connect(c, str);
			strcpy(str, "/bshtm/bsMenu.aspx");
			c->ops->get(c, str, data_buf, PAGE_SIZE, &read_size);
			get_captcha_url(data_buf, read_size, url);
			//c->ops->get(c, str, data_buf, PAGE_SIZE, &read_size);	
			c->ops->close(c);

			c = chttp_new();
			if (c == NULL)
				goto err;
			strcpy(str, "bsr.twse.com.tw");
			c->ops->connect(c, str);
			sprintf(str, "/bshtm/%s", url);
			c->ops->get(c, str, data_buf, PAGE_SIZE, &read_size);
			c->ops->close(c);
			
			sprintf(str, "captcha.jpg");
			data_position = get_captcha_start_position(data_buf, read_size);
			save_file(str, data_buf + data_position, read_size - data_position);
			free(data_buf);
			return true;
		}
	}
err:	
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


