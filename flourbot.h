#ifndef __FLOURBOT_H_
#define __FLOURBOT_H_

#define VERSION "Flourbot 6.0 - Pancake"

#define bool short
#define true 1
#define false 0

#include <js/jsapi.h>
#include <mysql/mysql.h>
#include <assert.h>
#include "util.h"
#include "stack.h"
#include "vector.h"
#include "hashtable.h"
#include "socket.h"

typedef struct _script_callback {
	char* command;
	//JSFunction* func;
	jsval func;
} script_callback;

typedef struct _script {
    char* name;
    char* filename;
    JSContext* cx;
	JSScript* script;
	JSObject* gcobj;
	vector* callbacks;
} script;

typedef struct _script_dbconnection {
	char *host;
	char *username;
	char *password;
	char *db;
	MYSQL *mysql;
} script_dbconnection;

typedef struct _script_engine {
    JSRuntime* rt;
	vector *scripts;
	vector *dbconnections;
} script_engine;

typedef struct _user {
	char* nick;
	char* ident;
	char* host;
} user;

typedef struct _channel {
	char* name;

	int in_channel;

	char* topic;
	char* topic_set_by;
	int topic_set_time;

	hashtable users;
} channel;

typedef struct _db_predef {
	char *name;
	char *host;
	char *username;
	char *password;
	int port;
	char *db;
	int conn;
} db_predef;

typedef struct _flourbot {
	char* nick;
	char* user;
	char* desc;
	char* server;
	int port;

	irc_socket* sock;	
	stack* buffer;
	bool is_registered;

	vector *channels;
	vector *predefined_conns;

	script_engine* engine;
	
	int names_parsing_in_progress;
} flourbot;

typedef struct _irc_message {
    const char* raw;
    char* source;
    char* command;
	char** params;
	int paramcount;
} irc_message;

// main.c
void suicide(char* msg);

#endif

