#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>    
#include <arpa/inet.h> 
#include <netdb.h>
#include "../inc/chttp.h"

static void chttp_send_data(int sock, char *data);


static void chttp_send_data(int sock, char *data)
{
    if (sock > 0 && data)
    {
 	    write(sock, data, strlen(data));
    }
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
	
    return true;
exception_chttp_connect:
	if (sock > 0)
		close(sock);
	return false;
}

void init_chttp(void)
{
	printf("@@@@@@ chttp init @@@@@@ \n");
	chttp_ops ops;
	//ops.http_connect = chttp_connect;
}

