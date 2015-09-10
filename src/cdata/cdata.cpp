#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cdata.h"
#include "bshtm.h"
#include <time.h>
#include <unistd.h>
#include <sys/stat.h>
#define CTRACE printf("@@@@@@@@@@@@@@@ %s %s %d \n", __FUNCTION__, __FILE__, __LINE__)

static bool cdata_get_investor_sorting_data(cdata *d, int year , int month, int day, int type);

static void cdata_save_file(char *name, char *buf, int size)
{
	//printf("name = %s \n", name);
	FILE *fp = fopen(name, "wb");
	if (fp)
	{
	    fwrite(buf, 1, size, fp);
		fflush (fp);
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

/*
 * Get the sorting data of three primary investor.  
 */
static bool cdata_get_three_primary_investor_sorting_data(cdata *d, int year , int month, int day)
{
	int counter = 0;
	if (cdata_get_investor_sorting_data(d, year, month, day, 0)) counter++;
	if (cdata_get_investor_sorting_data(d, year, month, day, 1)) counter++;
	if (cdata_get_investor_sorting_data(d, year, month, day, 2)) counter++;
	/* modify update-time */
	if (counter == 3)
	{
		char buf[10];
		FILE *fp = NULL;
		fp = fopen(UPDATE_TIME_FILE, "wb");
		if (fp)
		{
			sprintf(buf,"%04d%02d%02d", year+1911, month, day);
			fwrite(buf, 1, strlen(buf), fp);
			fflush(fp);
			fclose(fp);
		}
	}
	return true;
}

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

static void cdata_save_to_excel(cdata *d, char *data, int size, char *file_path)
{
	if (d == NULL || data == NULL)
		return;
	int data_position;
	data_position = find_data_start_position(data, size);
	d->ops->save_file(file_path, data + data_position, size - data_position);
}

/*
 * Get sorting data by different inverstor. Ex: foreign investor, trust investor and dealer.
 * type 0 is foreign invertor
 * type 1 is dealer
 * type 2 is trust invertor
 */
static bool cdata_get_investor_sorting_data(cdata *d, int year , int month, int day, int type)
{
	if (d)
	{
		chttp* 	c;
		char* 	data_buf;
		int		read_size, data_position; 
		char	str[STRING_LEN], str1[STRING_LEN], file_path[STRING_LEN];
		
		if (type == 0) 
			sprintf(file_path, "data/fis_data/%d%02d%02d.csv", year, month, day);
		else if(type == 1)
			sprintf(file_path, "data/dealer_data/%d%02d%02d.csv", year, month, day);
		else if(type == 2)
			sprintf(file_path, "data/trust_data/%d%02d%02d.csv", year, month, day);
		
		/* if data is exist, return true */
		if (is_data_exist(file_path)) return true;	
		data_buf = (char*)malloc(PAGE_SIZE);
		if (data_buf)
		{
			c = chttp_new();
			if (c == NULL)
				goto err;
			strcpy(str, "www.twse.com.tw");
			c->ops->connect(c, str);

			if (type == 0)
				strcpy(str, "/ch/trading/fund/TWT38U/TWT38U.php");
			else if (type == 1)
				strcpy(str, "/ch/trading/fund/TWT43U/TWT43U.php");
			else if (type == 2)
				strcpy(str, "/ch/trading/fund/TWT44U/TWT44U.php");
			
			sprintf(str1, "download=csv&qdate=%d%%2F%02d%%2F%02d&sorting=by_issue", year, month, day);
			c->ops->post(c, str, str1, data_buf, PAGE_SIZE, &read_size);
			c->ops->close(c);
		
			if (read_size < MIN_DATA_SIZE)
			{
				free(data_buf);
				goto err;
			}

			data_position = find_data_start_position(data_buf, read_size);
			d->ops->save_file(file_path, data_buf + data_position, read_size - data_position);
			free(data_buf);
			return true;
		}
	}
err:	
	return false;
}

static bool cdata_get_bshtm_data(cdata *d, int stock_number, int year , int month, int day)
{
	if (d)
	{
		int 	i, j, read_size;
		char* 	data_buf;
		char	host[STRING_LEN], st_list[STRING_LEN];
		char 	st_num[5];
		int 	cnt = 0;
		data_buf = (char*)malloc(PAGE_SIZE);
		strcpy(host, "isin.twse.com.tw");
		strcpy(st_list, "/isin/C_public.jsp?strMode=2");
		if (data_buf)
		{
			/* connect to bsMenu and get some useful data */
			d->ops->http_get(d, host, st_list, data_buf, &read_size, 2);
			for (i=0xD0; i<read_size; i++)
			{
				if (strncmp(data_buf+i, "<tr><td bgcolor=#FAFAD2>", strlen("<tr><td bgcolor=#FAFAD2>")) == 0)
				{
					cnt++;
					memcpy(st_num, data_buf+i+24, 4);
					st_num[4] = '\0';
					printf("st_num = %s \n", st_num);
					for (j=0; j<100; j++)
					{
						if (d->bs_ops->update_data(d, st_num))
						{
							break;
						}
						usleep(3000);
					}
					if (strncmp(st_num, "1435", 4) == 0)
					{
						break;
					}
				}
			}
			free(data_buf);
		}
		/*
		for (i=0; i<100; i++)
		{
			if (d->bs_ops->update_data(d, "2417"))
			{
				printf("@@@@@@@@@@@@@@@@@@@ i = %d\n", i);
				break;
			}
		}*/
		
		return true;
	}
	return false;
}

static int cdata_http_get(cdata *d, char *host_name, char *request, char *data_buf, int *read_size , int type)
{
	if (data_buf == NULL)
		return -1;
	int ret = -1;
	CTRACE;
	chttp* 	c;
	c = chttp_new();
	if (c == NULL)
		return ret;
	CTRACE;
	printf("request = %s \n", request);
	c->ops->connect(c, host_name);
	if (type == 1)
		c->ops->get(c, request, data_buf, PAGE_SIZE, read_size);
	else
	{
		CTRACE;
		c->ops->get2(c, request, data_buf, PAGE_SIZE, read_size);
	}
	c->ops->close(c);
	ret = 0;
	return ret;
}

static int cdata_http_post(cdata *d, char *host_name, char *request, char *post_data, 
	char *data_buf, int *read_size, char *session_id)
{
	int ret = -1;
	chttp* 	c;

	c = chttp_new();
	if (c == NULL)
		goto err;
	strcpy(c->session_id, session_id);
	c->ops->connect(c, host_name);
	c->ops->post(c, request, post_data, data_buf, PAGE_SIZE, read_size);		
	c->ops->close(c);
	ret = 0;
err:
	return ret;
}


static struct tm get_last_update_date()
{
	struct tm date;
	char buf[5];
	memset(&date, 0, sizeof(struct tm));
	FILE *fp = NULL;
	fp = fopen(UPDATE_TIME_FILE, "rb");
	if (fp)
	{
		int ret;
		ret = fread(buf, 1, 4, fp);
		buf[4] = '\0';
		if (ret != 4)
		{
			date.tm_year = 0;
			return date;
		}
		date.tm_year = atoi(buf);
		ret = fread(buf, 1, 2, fp);
		buf[2] = '\0';
		date.tm_mon = atoi(buf);
		ret = fread(buf, 1, 2, fp);
		buf[2] = '\0';
		date.tm_mday = atoi(buf);
		fclose(fp);
	}
	return date;
}

static void update_data(cdata *d)
{
	struct tm last_update_date;
	last_update_date = get_last_update_date();

	/* there is no data in our system, update all data */
	if (last_update_date.tm_year == 0)
	{
		int i, j, k;
		time_t t = time(NULL);
		struct tm *tm = localtime(&t);

	 	/* update current month */
		for (i=1; i<tm->tm_mday+1; i++)
			cdata_get_three_primary_investor_sorting_data(d, tm->tm_year-11, tm->tm_mon+1, i);
 
		/* update the remaining months of the year */
		for (i=0; i<tm->tm_mon-1; i++)
		{
			for (j=0; j<31; j++)
				cdata_get_three_primary_investor_sorting_data(d, tm->tm_year-11, i+1, j+1);	
		}

		/* update the remaining years ( 2005 ~ current_year - 1) */
		for (i=94; i<tm->tm_year-11; i++)
		{
			for (j=0; j<12; j++)
			{
				for (k=0; k<31; k++)
					cdata_get_three_primary_investor_sorting_data(d, i, j+1, k+1);	
			}
		}	

		/* update the data about 2004 */
		for (i=17; i<32; i++)	
			cdata_get_three_primary_investor_sorting_data(d, 93, 12, i);
		return;
	}

	/* Only need to update new data */
	time_t t = time(NULL);
	struct tm *tm = localtime(&t);
	tm->tm_year += 1900;
	tm->tm_mon  += 1;
	printf("%04d-%02d-%02d \n", tm->tm_year, tm->tm_mon, tm->tm_mday);
	if ((tm->tm_mday != last_update_date.tm_mday) ||
		(tm->tm_mon  != last_update_date.tm_mon)  || 
		(tm->tm_year != last_update_date.tm_year))
	{
		int i,j,k;
		/* Long time no update */
		if (tm->tm_year != last_update_date.tm_year)
		{
			/* update the data of last update-year */
			for (i=last_update_date.tm_mon; i<13; i++)
			{
				for (j=0; j<31; j++)
					cdata_get_three_primary_investor_sorting_data
						(d, last_update_date.tm_year-1911, i, j+1);
			}
			/* update data of the whole year if data of specific years is not exist */
			for (i=last_update_date.tm_year+1; i<tm->tm_year; i++)
			{
				for (j=0; j<12; j++)
				{
					for (k=0; k<31; k++)
						cdata_get_three_primary_investor_sorting_data(d, i-1911, j+1, k+1);	
				}
			}
			/* update remaining data */
			for (i=1; i<tm->tm_mon; i++)
			{
				for (k=0; k<31; k++)
					cdata_get_three_primary_investor_sorting_data(d, tm->tm_year-1911, i, k+1);
			}	
			for (i=1; i<tm->tm_mday+1; i++)
				cdata_get_three_primary_investor_sorting_data(d, tm->tm_year-1911, tm->tm_mon, i);
			return;
		}
		
		if (tm->tm_mon != last_update_date.tm_mon)
		{
			for (i=last_update_date.tm_mon; i<tm->tm_mon; i++)
			{
				for (k=0; k<31; k++)
					cdata_get_three_primary_investor_sorting_data(d, tm->tm_year-1911, i, k+1);
			}	
			for (i=1; i<tm->tm_mday+1; i++)
				cdata_get_three_primary_investor_sorting_data(d, tm->tm_year-1911, tm->tm_mon, i);
			return;
		}

		if (tm->tm_mday != last_update_date.tm_mday)
		{
			for (i=last_update_date.tm_mday; i<tm->tm_mday+1; i++)
				cdata_get_three_primary_investor_sorting_data(d, tm->tm_year-1911, tm->tm_mon, i);
			return;
		}
	}
}

static void cdata_close(cdata *d)
{
	if (d)
	{
		if (d->bshtm_data_buf)
			free(d->bshtm_data_buf);
		free(d);
		d = NULL;
	}
}

static cdata_ops ops;

cdata *cdata_new(void)
{
	cdata *d = NULL;
	d = (cdata*)malloc(sizeof(cdata));
	if (d == NULL)
		goto err;
	if (ops.init == false)
	{
		ops.update_data 	= update_data,
		ops.close 			= cdata_close,
		ops.get_bshtm_data	= cdata_get_bshtm_data,
		ops.save_to_excel	= cdata_save_to_excel;
		ops.save_file		= cdata_save_file;
		ops.http_get		= cdata_http_get;
		ops.http_post		= cdata_http_post;
		ops.init = true;
	}
	
	d->ops = &ops;
	d->bshtm_data_buf = (char*)malloc(PAGE_SIZE);
	if (d->bshtm_data_buf == NULL)
		goto err;
	set_bshtm_ops(d);
	return d;
err:
	if (d)
		free(d);
	return NULL;
}

