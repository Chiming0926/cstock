#ifndef _CHTTP_H_
#define _CHTTP_H_

#define HOST_NAME_LEN	128
#define HTTP_PORT		80
#define PAGE_SIZE 		1024*1024*4

typedef struct _chttp_
{
	int 	sock_fd;
	char	host_name[HOST_NAME_LEN];
	struct chttp_ops *ops;
} chttp;

struct chttp_ops
{
	bool (*c_connect)(chttp *c, char *host_name);
	void (*c_close)(chttp *c);
	bool (*c_post)(chttp *c, char *request, char *post_data, char *page_buf, int buf_size, int *read_size);
	bool (*c_get)(chttp *c, char *request, char *page_buf, int buf_size, int *read_size);
};

extern void init_chttp(void);
extern chttp *chttp_new(void);

#endif
