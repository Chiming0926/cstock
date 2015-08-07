#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cdata.h"

static void save_file(char *name, char *buf, int size);

static void save_file(char *name, char *buf, int size)
{
	FILE *fp = fopen(name, "wb+");
	if (fp)
	{
	    fwrite(buf, 1, size, fp);
	    fclose(fp);
	}
}

static int find_data_start_position(char *data, int size)
{
	int i;
	for (i=0; i<size; i++)
	{
		if (data[i] == 0x0D && data[i+1] == 0x0A 
			&& data[i+2] == 0x0D && data[i+3] == 0x0A)
			break;
	}
	return i + 10;
}

static bool cdata_get_foreign_investor_sorting_data(cdata *d, int year , int month, int day)
{
	if (d)
	{
		chttp* 	c;
		char* 	data_buf;
		int		read_size, data_position; 
		char	str[STRING_LEN], str1[STRING_LEN];
		data_buf = (char*)malloc(PAGE_SIZE);
		if (data_buf)
		{
			c = chttp_new();
			if (c == NULL)
				goto err;
			strcpy(str, "www.twse.com.tw");
			c->ops->connect(c, str);
			strcpy(str, "/ch/trading/fund/TWT38U/TWT38U.php");
			
			sprintf(str1, "download=csv&qdate=%d%%2F%02d%%2F%02d&sorting=by_issue", year, month, day);
			c->ops->post(c, str, str1, data_buf, PAGE_SIZE, &read_size);
			c->ops->close(c);

			sprintf(str, "%d%02d%02d.csv", year, month, day);
			data_position = find_data_start_position(data_buf, read_size);
			save_file(str, data_buf + data_position, read_size - data_position);
			free(data_buf);
			return true;
		}
	}
err:	
	return false;
}

static void cdata_close(cdata *d)
{
	if (d)
	{
		free(d);
		d = NULL;
	}
}

static cdata_ops ops = 
{
	.get_foreign_investor_sorting_data = cdata_get_foreign_investor_sorting_data,
	.close = cdata_close
};

cdata *cdata_new(void)
{
	cdata *d = NULL;
	d = (cdata*)malloc(sizeof(cdata));
	if (d == NULL)
		goto err;
	
	d->ops = &ops;
	return d;
err:
	if (d)
		free(d);
	return NULL;
}

