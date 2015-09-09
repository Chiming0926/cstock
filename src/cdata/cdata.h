#ifndef C_DATA_H_
#define C_DATA_H_

#include <common.h>


#define MIN_DATA_SIZE 600
#define UPDATE_TIME_FILE "data/update.time"

typedef struct _cdata_
{
	chttp*	http;
	struct cdata_ops*	ops;
	struct bshtm_ops*	bs_ops;
} cdata;

struct cdata_ops
{
	bool init;
	void (*save_to_excel)(cdata *d, char *data, int size, char *file_path);
	void (*update_data)(cdata *d);
	void (*close)(cdata *d);
	bool (*get_bshtm_data)(cdata *d, int stock_number, int year , int month, int day);
	int  (*http_post)(cdata *d, char *host_name, char *request, char *post_data, 
						char *data_buf, int *read_size, char *session_id);
	int  (*http_get)(cdata *d, char *host_name, char *request, char *data_buf, int *read_size , int type);
};

struct bshtm_ops
{
	bool init;
	bool (*update_data)(cdata *d, char *st_num);
};

extern cdata *cdata_new(void);
extern void save_file(char *name, char *buf, int size);
#endif
