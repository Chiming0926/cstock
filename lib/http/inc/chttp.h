#ifndef _CHTTP_H_
#define _CHTTP_H_

#define HOST_NAME_LEN	128
#define HTTP_PORT		80
#define PAGE_SIZE 		1024*1024*4
#define STRING_LEN		256

typedef struct _chttp_
{
	int 	sock_fd;
	char	host_name[HOST_NAME_LEN];
	char	session_id[STRING_LEN];
	char	send_data[STRING_LEN*4];
	struct chttp_ops *ops;
} chttp;

struct chttp_ops
{
	bool init;
	bool (*connect)(chttp *c, char *host_name);
	void (*close)(chttp *c);
	bool (*post)(chttp *c, char *request, char *post_data, char *page_buf, int buf_size, int *read_size);
	bool (*get)(chttp *c, char *request, char *page_buf, int buf_size, int *read_size);
	bool (*get2)(chttp *c, char *request, char *page_buf, int buf_size, int *read_size);
};

extern chttp *chttp_new(void);

#endif
