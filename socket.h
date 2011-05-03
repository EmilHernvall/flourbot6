#ifndef __SOCKET_H_
#define __SOCKET_H_

typedef struct {
	int socket;
	char* host;
	int port;
} irc_socket;

void socket_connect(irc_socket* socket, const char* host, int port);
void socket_close(irc_socket* socket);
void socket_write(irc_socket* socket, const char* msg);
int socket_read(irc_socket* socket, char** ret);

#endif
