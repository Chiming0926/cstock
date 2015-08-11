#ifndef C_DATA_H_
#define C_DATA_H_

#include <common.h>

#define MIN_DATA_SIZE 600
#define UPDATE_TIME_FILE "data/update.time"

typedef struct _cdata_
{
	chttp*	http;
	struct cdata_ops*	ops;
} cdata;

struct cdata_ops
{
	void (*update_data)(cdata *d);
	void (*close)(cdata *d);
};

extern cdata *cdata_new(void);

#endif
