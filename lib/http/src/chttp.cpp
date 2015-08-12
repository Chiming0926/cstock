#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>    
#include <arpa/inet.h> 
#include <netdb.h>
#include "../inc/chttp.h"

static bool send_data(int sock, char *data);
static bool send_client_data(chttp *c, int content_len);


static bool send_data(int sock, char *data)
{
    if (sock > 0 && data)
    {
		if (write(sock, data, strlen(data)) < 0)
			goto err;
		return true;
    }
err:
	return false;
}

static bool send_data(int sock, const char *data)
{
    if (sock > 0 && data)
    {
		if (write(sock, data, strlen(data)) < 0)
			goto err;
		return true;
    }
err:
	return false;
}

/*
	Send our basic data to server. Ex: HTTP version, Connection status, Content-Type.. etc
 */
static bool send_client_data(chttp *c, int content_len)
{
	if (c && c->sock_fd > 0)
	{
		char buf[HOST_NAME_LEN];
		if (send_data(c->sock_fd, " HTTP/1.1\r\n") == false) goto err;

		sprintf(buf, "Host: %s\r\n", c->host_name);
		if (send_data(c->sock_fd, buf) == false) goto err;

		if (send_data(c->sock_fd, "Connection n: Keep-alive\r\n") == false) goto err;

		if (content_len > 0)
		{
			sprintf(buf, "Content-Length: %d\r\n", content_len);	
			if (send_data(c->sock_fd, buf) == false) goto err;
		}
		
		if (send_data(c->sock_fd, "Cache-Control: max-age=0\r\n") == false) goto err;

		if (send_data(c->sock_fd, "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8\r\n") == false) goto err;

		if (send_data(c->sock_fd, "Origin: http://www.twse.com.tw\r\n") == false) goto err;

		if (send_data(c->sock_fd, "Upgrade-Insecure-Requests: 1\r\n") == false) goto err;

		if (send_data(c->sock_fd, "User-Agent: Mozilla/5.0 (Windows NT 6.1; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/44.0.2403.130 Safari/537.36\r\n") == false) goto err;

		if (send_data(c->sock_fd, "Content-Type: application/x-www-form-urlencoded\r\n") == false) goto err;

		if (send_data(c->sock_fd, "Referer: http://www.twse.com.tw/ch/trading/fund/TWT38U/TWT38U.php\r\n") == false) goto err;

		if (send_data(c->sock_fd, "Accept-Encoding: gzip, deflate\r\n") == false) goto err;

		if (send_data(c->sock_fd, "Accept-Language: zh-TW,zh;q=0.8,en-US;q=0.6,en;q=0.4,zh-CN;q=0.2\r\n\r\n") == false) goto err;

		return true;
	}
err:
	return false;
}

static bool chttp_connect(chttp *c, char *host_name)
{
    if (host_name == NULL || c == NULL)
        return false;

	int sock = 0;
    struct sockaddr_in server;

	/* Create socket */
    sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock == -1)
    {
        printf("Could not create socket");
        goto exception_chttp_connect;
    }

	/* Get target ip */
	struct hostent *host;
  	if((host = gethostbyname(host_name)) == NULL)
  	{
    	printf("Can't get IP");
    	goto exception_chttp_connect;
  	}	
  	server.sin_addr  		= *((struct in_addr *)host->h_addr_list[0]);
    server.sin_family 		= AF_INET;
    server.sin_port 		= htons(HTTP_PORT); //http

    /* Connect to remote server */
    if (connect(sock , (struct sockaddr *)&server , sizeof(server)) < 0)
    {
        printf("connect failed. Error");
        goto exception_chttp_connect;
    }

	c->sock_fd = sock;
	strcpy(c->host_name, host_name);
    return true;
exception_chttp_connect:
	if (sock > 0)
		close(sock);
	return false;
}

static void chttp_close(chttp *c)
{
	if (c)
	{
		if (c->sock_fd > 0)
			close(c->sock_fd);
		free(c);
		c = NULL;
	}
}

static bool chttp_get(chttp *c, char *request, char *page_buf, int buf_size, int *read_size)
{
	if (c == NULL || page_buf == NULL)
		return false;
	/* check connection status */
	if (c->sock_fd <= 0)
		return false;

	char buf[128];
	fd_set recvfd;
	struct timeval tv;
	int bytes_read = 0;
	int ret;
	
	if (send_data(c->sock_fd, "GET ") == false) goto err;
	if (send_data(c->sock_fd, request) == false) goto err;
    if (send_data(c->sock_fd, " HTTP/1.0\r\n") == false) goto err;
	
	sprintf( buf, "Host: %s\r\n", c->host_name);
	if (send_data(c->sock_fd, buf) == false) goto err;

	if (send_data(c->sock_fd, "User-Agent:  HTMLGET 1.0\r\n\r\n") == false)
		goto err;
   
    FD_ZERO(&recvfd);
    FD_SET(c->sock_fd, &recvfd);
    
    tv.tv_sec  = 120;
    tv.tv_usec = 500000;
    int rv;
    for (;;)
    {
        
        FD_ZERO(&recvfd);
        FD_SET(c->sock_fd, &recvfd);
        rv = select(c->sock_fd+1, &recvfd, NULL, NULL, &tv);
        if ( rv == -1 ) 
        {
            printf("error occurred \n");
            break;
        }
        else if ( rv == 0 )
        {
            printf("timeout \n");
            break;
        }
        else
        {
            if( FD_ISSET(c->sock_fd, &recvfd) )
            {
				for (;;)
				{
                	ret = read(c->sock_fd, page_buf + bytes_read, buf_size);
					if (ret <= 0)
                		goto out;
					bytes_read += ret;	
					if (bytes_read > buf_size)
						goto out;
				}
            }
        }
    }
out:
    *read_size = bytes_read;
	return true;
	
err:
	c->ops->close(c);
	return false;
}

static bool chttp_post(chttp *c, char *request, char *post_data, char *page_buf, int buf_size, int *read_size)
{
	if (c == NULL || page_buf == NULL)
		return false;
	/* check connection status */
	if (c->sock_fd <= 0)
		return false;

	fd_set recvfd;
	struct timeval tv;
	int bytes_read = 0;
	int ret;
	
	if (send_data(c->sock_fd, "POST ") == false) goto err;
	if (send_data(c->sock_fd, request) == false) goto err;

	send_client_data(c, strlen(post_data));

	if (send_data(c->sock_fd, post_data) == false) goto err;
    FD_ZERO(&recvfd);
    FD_SET(c->sock_fd, &recvfd);
    
    tv.tv_sec  = 120;
    tv.tv_usec = 500000;
    int rv;
    for (;;)
    {
        
        FD_ZERO(&recvfd);
        FD_SET(c->sock_fd, &recvfd);
        rv = select(c->sock_fd+1, &recvfd, NULL, NULL, &tv);
        if ( rv == -1 ) 
        {
            printf("error occurred \n");
            break;
        }
        else if ( rv == 0 )
        {
            printf("timeout \n");
            break;
        }
        else
        {
            if( FD_ISSET(c->sock_fd, &recvfd) )
            {
				for (;;)
				{
                	ret = read(c->sock_fd, page_buf + bytes_read, buf_size);
					if (ret <= 0)
                		goto out;
					bytes_read += ret;	
					if (bytes_read > buf_size)
						goto out;
				}
            }
        }
    }
out:
    *read_size = bytes_read;
	return true;
	
err:
	c->ops->close(c);
	return false;
}

static chttp_ops ops;

void init_chttp(void)
{
	printf("@@@@@@ chttp init @@@@@@ \n");
//	chttp_ops ops;
}

chttp *chttp_new(void)
{
	chttp *c = NULL;
	c = (chttp*)malloc(sizeof(chttp));
	if (c == NULL)
		goto err;
	if (ops.init == false)
	{
		ops.connect = chttp_connect;
		ops.close	= chttp_close;
		ops.post	= chttp_post;
		ops.get		= chttp_get;
		ops.init 	= true;
	}
	c->ops = &ops;
	return c;
err:
	if (c)
		free(c);
	return NULL;
}

