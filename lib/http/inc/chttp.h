#ifndef _CHTTP_H_
#define _CHTTP_H_

#define HOST_NAME_LEN	128
#define HTTP_PORT		80

typedef struct _chttp_
{
	int 	sock_fd;
	struct chttp_ops *ops;
} chttp;

struct chttp_ops
{
	bool (*http_connect)(chttp *c, char *host_name);
};

extern void init_chttp(void);

#endif
