#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <assert.h>
#include "util.h"
#include "socket.h"

void socket_connect(irc_socket* msocket, const char* host, int port)
{
    int socket_h;

    struct sockaddr_in addr;
	struct hostent *hosts;

	hosts = gethostbyname(host);
	if (hosts == NULL) {
		fprintf(stderr, "Failed to resolve address!");
		exit(-1);
	}

	char ip[30];
	inet_ntop(AF_INET, hosts->h_addr_list[0], ip, sizeof(ip));

	printf("Resolved %s to %s.\n", host, ip);

    if ((socket_h = socket(AF_INET,SOCK_STREAM, 0)) < 0) {
        fprintf(stderr, "Failed to create socket!\n");
        exit(-1);
    } else {
        printf("Succesfully initiated socket.\n");
    }

    memset(&addr, 0, sizeof(addr));

    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);

    if (inet_pton(AF_INET, ip, &addr.sin_addr) <= 0) {
        fprintf(stderr, "Invalid server.\n");
        exit(-1);
    }

    if ( (connect(socket_h, (struct sockaddr*) &addr, sizeof(addr))) < 0) {
        fprintf(stderr, "Unable to connect to %s on port %i.\n", host, port);
    }
    else {
        printf("Connected to server %s on port %i.\n", host, port);
    }

    fcntl(socket_h, F_SETFL, 1);
//    fcntl(socket_h, F_SETFL, O_NONBLOCK);

    msocket->socket = socket_h;
    msocket->host = xstrdup(host);
    msocket->port = port;
}

void socket_close(irc_socket* socket)
{
    xfree(socket->host);
	socket->host = NULL;

    close(socket->socket);
}

void socket_write(irc_socket* socket, const char* msg)
{
//	#ifdef DEBUG
//		printf("%s\n", msg);
//	#endif
    write(socket->socket, msg, strlen(msg));
}

int socket_read(irc_socket* socket, char** ret)
{
    char byte = 0, lastbyte = 0;
    int buf_size = (32 * sizeof(char)), r = 0, totalread = 0;

    char* buf = (char*)xmalloc(buf_size);

    memset(buf,0,buf_size);

    while (1) {

        if (totalread + sizeof(char) >= buf_size) {
            buf_size *= 2;
            buf = (char*)xrealloc(buf, buf_size);
        }

        r = read(socket->socket, &byte, sizeof(char));

        if (r == 0 || r == -1) {
            break;
        }

        if (byte == 10 && lastbyte == 13) {
            buf[totalread-1] = 0;
            break;
        } else {
            buf[totalread] = byte;
        }

        totalread += r;
        lastbyte = byte;

    }

    *ret = buf;
    return totalread;
}
