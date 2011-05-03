CC = gcc
OBJS = main.o irc.o irc_functions.o socket.o string.o stack.o config.o script.o vector.o util.o hashtable.o
DEBUG_FLAGS = -g -DDEBUG -Wall
CFLAGS = -DXP_UNIX $(DEBUG_FLAGS)
LIBS = -lmysqlclient -lm -lpthread -lz -ljs

flourbot : $(OBJS)
	$(CC) $(DEBUG_FLAG) -o flourbot $(OBJS) $(LIBS)

main.o : main.c
irc.o : irc.c
irc_functions.o : irc_functions.c
socket.o : socket.c
string.o : string.c
stack.o : stack.c
vector.o : vector.c
config.o : config.c
script.o : script.c
util.o : util.c
hashtable.o : hashtable.c

clean :
	rm $(OBJS) flourbot
