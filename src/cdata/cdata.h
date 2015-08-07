#ifndef C_DATA_H_
#define C_DATA_H_

#include <common.h>

typedef struct _cdata_
{
	chttp*	http;
	struct cdata_ops*	ops;
} cdata;

struct cdata_ops
{
	bool (*get_foreign_investor_sorting_data)(cdata *d, int year , int month, int day);
	void (*close)(cdata *d);
};

extern cdata *cdata_new(void);

#endif
