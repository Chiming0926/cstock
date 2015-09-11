#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>    
#include <arpa/inet.h> 
#include <netdb.h>
#include "../inc/chttp.h"

static bool send_data(int sock, char *data);
static bool parse_client_data(chttp *c, int content_len);
#define CTRACE printf("@@@@@@@@@@@@@@@ %s %s %d \n", __FUNCTION__, __FILE__, __LINE__)

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
 *	Send our basic data to server. Ex: HTTP version, Connection status, Content-Type.. etc
 */
static bool send_client_data(chttp *c, int content_len)
{
	if (c && c->sock_fd > 0)
	{
		char buf[HOST_NAME_LEN];	
		sprintf(buf, " HTTP/1.1\r\n");
		//strcat(c->send_data, buf);
		send_data(c->sock_fd, buf);
		
		sprintf(buf, "Host: %s\r\n", c->host_name);
		//strcat(c->send_data, buf);
		send_data(c->sock_fd, buf);
		
		sprintf(buf, "User-Agent: Mozilla/5.0 (Windows NT 6.1; WOW64; rv:40.0) Gecko/20100101 Firefox/40.0\r\n");
		//strcat(c->send_data, buf);
		send_data(c->sock_fd, buf);
		
		sprintf(buf, "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\r\n");
		//strcat(c->send_data, buf);
		send_data(c->sock_fd, buf);
		
		sprintf(buf, "Accept-Language: zh-TW,zh;q=0.8,en-US;q=0.5,en;q=0.3\r\n");
		//strcat(c->send_data, buf);
		send_data(c->sock_fd, buf);
		
		sprintf(buf, "Accept-Encoding: gzip, deflate\r\n");
		//strcat(c->send_data, buf);
		send_data(c->sock_fd, buf);
		
		sprintf(buf, "Referer: http://bsr.twse.com.tw/bshtm/bsMenu.aspx\r\n");
		//strcat(c->send_data, buf);
		send_data(c->sock_fd, buf);
		
		if (c->session_id[0] != 0)
		{
			sprintf(buf, "Cookie: ASP.NET_SessionId=%s\r\n", c->session_id);
			send_data(c->sock_fd, buf);
			//strcat(c->send_data, buf);
		}

		sprintf(buf, "Connection: keep-alive\r\n");
		//strcat(c->send_data, buf);
		send_data(c->sock_fd, buf);

		sprintf(buf, "Content-Type: application/x-www-form-urlencoded\r\n");
		//strcat(c->send_data, buf);
		send_data(c->sock_fd, buf);
		if (content_len > 0)
		{
			sprintf(buf, "Content-Length: %d\r\n", content_len);
			//strcat(c->send_data, buf);
			send_data(c->sock_fd, buf);
		}
		//strcat(c->send_data, "\r\n");
		sprintf(buf, "\r\n");
		send_data(c->sock_fd, buf);
		return true;
	}
	return false;
}

static void printf_host_ip_address(char *host_name)
{
	struct hostent *host;
	struct in_addr **addr_list;
  	if((host = gethostbyname(host_name)) == NULL)
  	{
    	printf("Can't get IP");
    	return;
  	}	
	int i;
	addr_list = (struct in_addr **)host->h_addr_list;
    for (i = 0; addr_list[i] != NULL; i++) 
	{
        printf("%s ", inet_ntoa(*addr_list[i]));
    }
	printf("\n");
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
	//printf_host_ip_address(host_name);
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

static bool check_end_message(char *data , int size)
{		
	int i=0;
	for (i=0; i<size; i++)
	{
		if (data[i] == 0x0d && data[i+1] == 0x0a && data[i+2] == 0x0d 
			&& data[i+3] == 0x0a)
			return true;
	}
	return false;
}

static bool check_html_end_message(char *data, int size)
{
	int i=0;
	for (i=0; i<size; i++)
	{
		if (strncmp(data+i, "</html>", strlen("</html>")) == 0)
			return true;
	}
	return false;
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
					if (check_html_end_message(page_buf + bytes_read - ret, ret))
						goto out;
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

static bool chttp_get2(chttp *c, char *request, char *page_buf, int buf_size, int *read_size)
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
	char buf[STRING_LEN];
	
	sprintf(buf, "GET %s", request);
	send_data(c->sock_fd, buf);
	send_client_data(c, 0);
	FD_ZERO(&recvfd);
    FD_SET(c->sock_fd, &recvfd);
    tv.tv_sec  = 20;
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
					bytes_read += ret;
					if (strncmp(request, "/bshtm/bsContent.aspx", strlen("/bshtm/bsContent.aspx")) == 0)
					{
						if (ret < 200)
							goto out;
						printf("get read size = %d\n", ret);
						if (check_end_message(page_buf + bytes_read - ret, ret))
						goto out;
					}
					if (ret <= 0)
                		goto out;	
					if (check_html_end_message(page_buf + bytes_read - ret, ret))
						goto out;
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

	char buf[STRING_LEN];

	sprintf(buf, "POST %s", request);
	send_data(c->sock_fd, buf);
	send_client_data(c, strlen(post_data));
	send_data(c->sock_fd, post_data);
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
					if (check_html_end_message(page_buf + bytes_read - ret, ret))
						goto out;
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
		ops.get2	= chttp_get2;
		ops.init 	= true;
	}
	c->ops = &ops;
	c->session_id[0] = 0;
	return c;
err:
	if (c)
		free(c);
	return NULL;
}

