#include <stdio.h>
#include <stdlib.h>
#include <js/jsapi.h>
#include <errno.h>
#include <mysql/mysql.h>
#include "string.h"
#include "socket.h"
#include "stack.h"
#include "script.h"
#include "irc_functions.h"

#define STACK_CHUNK_SIZE    8192

static const char* argnames[] = { "source", "command", "params" };

static JSClass global_class = {
	"global",0,
	JS_PropertyStub,JS_PropertyStub,JS_PropertyStub,JS_PropertyStub,
	JS_EnumerateStub,JS_ResolveStub,JS_ConvertStub,JS_FinalizeStub
};

void reportError(JSContext *cx, const char *message, JSErrorReport *report)
{
    fprintf(stderr, "JSERROR: %s:%u:%s\n",
	    report->filename, (unsigned int) report->lineno, message);
}

static JSBool
Console_println(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
	flourbot* bot = (flourbot*)JS_GetContextPrivate(cx);
	
	JSString* string = JS_ValueToString(cx, argv[0]);
	printf("%s\n", JS_GetStringBytes(string));

	*rval = JSVAL_VOID;
	return JS_TRUE;
}

static JSBool
Bot_getNick(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
	flourbot* bot = (flourbot*)JS_GetContextPrivate(cx);
	
	JSString* string = JS_NewString(cx, bot->nick, strlen(bot->nick));

	*rval = STRING_TO_JSVAL(string);
	return JS_TRUE;
}

static JSBool
Bot_addCallback(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
	flourbot* bot = (flourbot*)JS_GetContextPrivate(cx);

	if (argc != 2) {
		*rval = JSVAL_VOID;
		return JS_TRUE;
	}

	char* command = JS_GetStringBytes(JS_ValueToString(cx, argv[0]));

	int i;
	for (i = 0; i < vector_count(bot->engine->scripts); i++) {
		script* s = vector_get(bot->engine->scripts, i);
		if (s->cx == cx) {
			script_callback* callback = (script_callback*)xmalloc(sizeof(script_callback));
			callback->command = xstrdup(command);
			callback->func = argv[1];

			JS_AddNamedRoot(cx, &callback->func, "callback");

			vector_add(s->callbacks, callback);
			break;
		}
	}

	*rval = JSVAL_VOID;
	return JS_TRUE;
}

static JSBool
IRC_raw(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
	flourbot* bot = (flourbot*)JS_GetContextPrivate(cx);
	
	JSString* string = JS_ValueToString(cx, argv[0]);
	char* cstr = JS_GetStringBytes(string);

	char buffer[strlen(cstr) + 10];
	sprintf(buffer, "%s\r\n", cstr);
	socket_write(bot->sock, buffer);

	*rval = JSVAL_VOID;
	return JS_TRUE;
}

static JSBool
IRC_msg(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
	flourbot* bot = (flourbot*)JS_GetContextPrivate(cx);

	JSString* str_target = JS_ValueToString(cx, argv[0]);
	char* cstr_target = JS_GetStringBytes(str_target);

	JSString* str_msg = JS_ValueToString(cx, argv[1]);
	char* cstr_msg = JS_GetStringBytes(str_msg);

	char buffer[strlen(cstr_target) + strlen(cstr_msg) + 20];
	sprintf(buffer, "privmsg %s :%s\r\n", cstr_target, cstr_msg);
	socket_write(bot->sock, buffer);

	*rval = JSVAL_VOID;
	return JS_TRUE;
}

static JSBool
IRC_notice(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
	flourbot* bot = (flourbot*)JS_GetContextPrivate(cx);

	JSString* str_target = JS_ValueToString(cx, argv[0]);
	char* cstr_target = JS_GetStringBytes(str_target);

	JSString* str_msg = JS_ValueToString(cx, argv[1]);
	char* cstr_msg = JS_GetStringBytes(str_msg);

	char buffer[strlen(cstr_target) + strlen(cstr_msg) + 20];
	sprintf(buffer, "notice %s :%s\r\n", cstr_target, cstr_msg);
	socket_write(bot->sock, buffer);

	*rval = JSVAL_VOID;
	return JS_TRUE;
}

static JSBool
IRC_mode(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
	flourbot* bot = (flourbot*)JS_GetContextPrivate(cx);

	JSString* str_target = JS_ValueToString(cx, argv[0]);
	char* cstr_target = JS_GetStringBytes(str_target);

	JSString* str_mode = JS_ValueToString(cx, argv[1]);
	char* cstr_mode = JS_GetStringBytes(str_mode);

	JSString* str_params = JS_ValueToString(cx, argv[2]);
	char* cstr_params = JS_GetStringBytes(str_params);

	char buffer[strlen(cstr_target) + strlen(cstr_mode) + strlen(cstr_params) + 20];
	sprintf(buffer, "mode %s %s :%s\r\n", cstr_target, cstr_mode, cstr_params);
	socket_write(bot->sock, buffer);

	*rval = JSVAL_VOID;
	return JS_TRUE;
}

static JSBool
IRC_kick(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
	flourbot* bot = (flourbot*)JS_GetContextPrivate(cx);

	JSString* str_channel = JS_ValueToString(cx, argv[0]);
	char* cstr_channel = JS_GetStringBytes(str_channel);

	JSString* str_subject = JS_ValueToString(cx, argv[1]);
	char* cstr_subject = JS_GetStringBytes(str_subject);

	JSString* str_desc = JS_ValueToString(cx, argv[1]);
	char* cstr_desc = JS_GetStringBytes(str_desc);

	char buffer[strlen(cstr_channel) + strlen(cstr_subject) + strlen(cstr_desc) + 20];
	sprintf(buffer, "kick %s %s :%s\r\n", cstr_channel, cstr_subject, cstr_desc);
	socket_write(bot->sock, buffer);

	*rval = JSVAL_VOID;
	return JS_TRUE;
}

static JSBool
IRC_getChannels(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
	flourbot* bot = (flourbot*)JS_GetContextPrivate(cx);
	
	if (vector_count(bot->channels) == 0) {
		*rval = JSVAL_VOID;
		return JS_TRUE;
	}

	JSObject* channelsObj = JS_NewArrayObject(cx, 0, NULL);
	
	int i;
	JSString* str;
	jsval val;
	for (i = 0; i < vector_count(bot->channels); i++) {
		channel *chan = vector_get(bot->channels, i);
		str = JS_NewStringCopyZ(cx, chan->name);
		val = STRING_TO_JSVAL(str);
		JS_SetElement(cx, channelsObj, i, &val);
	}
	
	*rval = OBJECT_TO_JSVAL(channelsObj);
	return JS_TRUE;
}

static JSBool
IRC_getNicks(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
	flourbot* bot = (flourbot*)JS_GetContextPrivate(cx);

	JSString* str_channel = JS_ValueToString(cx, argv[0]);
	char* cstr_channel = JS_GetStringBytes(str_channel);
	
	if (vector_count(bot->channels) == 0) {
		*rval = JSVAL_VOID;
		return JS_TRUE;
	}

	channel* chan = NULL;
	int i;
	for (i = 0; i < vector_count(bot->channels); i++) {
		channel* chan2 = vector_get(bot->channels, i);
		if (strcmp(chan2->name, cstr_channel) == 0) {
			chan = chan2;
			break;
		}
	}

	if (chan == NULL) {
		*rval = JSVAL_VOID;
		return JS_TRUE;
	}

	JSObject* nicksObj = JS_NewArrayObject(cx, 0, NULL);	

	JSString* str;
	jsval val;
	int j, k = 0;
	for (i = 0; i < chan->users.size; i++) {
		hashtable_bucket* bucket = chan->users.data[i];
		if (bucket == NULL) {
			continue;
		}

		for (j = 0; j < bucket->size; j++) {
			user *u = bucket->data[j];
			char *n = bucket->key[j];
			if (u == NULL) {
				continue;
			}

			str = JS_NewStringCopyZ(cx, irc_normalizenick(u->nick));
			val = STRING_TO_JSVAL(str);
			JS_SetElement(cx, nicksObj, k, &val);
			k++;
		}
	}
	
	*rval = OBJECT_TO_JSVAL(nicksObj);
	return JS_TRUE;
}

static JSBool
IRC_isUserOnline(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
	flourbot* bot = (flourbot*)JS_GetContextPrivate(cx);

	char* channel_name = JS_GetStringBytes(JS_ValueToString(cx, argv[0]));
	char* nick = strtolower(JS_GetStringBytes(JS_ValueToString(cx, argv[1])));

	if (vector_count(bot->channels) == 0) {
		*rval = JSVAL_FALSE;
		return JS_TRUE;
	}

	channel* chan = NULL;
	int i;
	for (i = 0; i < vector_count(bot->channels); i++) {
		channel* chan2 = vector_get(bot->channels, i);
		if (strcmp(chan2->name, channel_name) == 0) {
			chan = chan2;
			break;
		}
	}

	if (chan == NULL) {
		*rval = JSVAL_FALSE;
		return JS_TRUE;
	}

	user* u = hashtable_get(&chan->users, nick);
	if (u != NULL) {
		*rval = JSVAL_TRUE;
		return JS_TRUE;
	}

	xfree(nick);

	*rval = JSVAL_FALSE;
	return JS_TRUE;
}

static JSBool
IRC_getNickCount(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
	flourbot* bot = (flourbot*)JS_GetContextPrivate(cx);

	char* channel_name = JS_GetStringBytes(JS_ValueToString(cx, argv[0]));

	if (vector_count(bot->channels) == 0) {
		*rval = JSVAL_VOID;
		return JS_TRUE;
	}

	channel* chan = NULL;
	int i;
	for (i = 0; i < vector_count(bot->channels); i++) {
		channel* chan2 = vector_get(bot->channels, i);
		if (strcmp(chan2->name, channel_name) == 0) {
			chan = chan2;
			break;
		}
	}

	if (chan == NULL) {
		*rval = JSVAL_VOID;
		return JS_TRUE;
	}

	*rval = INT_TO_JSVAL(hashtable_count(&chan->users));
	return JS_TRUE;
}

static JSBool
DB_connect(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
	flourbot* bot = (flourbot*)JS_GetContextPrivate(cx);

	char* host = JS_GetStringBytes(JS_ValueToString(cx, argv[0]));
	char* username = JS_GetStringBytes(JS_ValueToString(cx, argv[1]));
	char* password = JS_GetStringBytes(JS_ValueToString(cx, argv[2]));
	char* db = JS_GetStringBytes(JS_ValueToString(cx, argv[3]));

	// check for an existing connection with the specified parameters
	int i;
	for (i = 0; i < vector_count(bot->engine->dbconnections); i++) {
		script_dbconnection *dbconn = vector_get(bot->engine->dbconnections, i);
		if (strcmp(dbconn->host, host) == 0 &&
			strcmp(dbconn->username, username) == 0 &&
			strcmp(dbconn->password, password) == 0 &&
			strcmp(dbconn->db, db) == 0) {

			*rval = INT_TO_JSVAL(i);
			return JS_TRUE;
		}
	}

	script_dbconnection *conn = (script_dbconnection*)xmalloc(sizeof(script_dbconnection));
	conn->host = host;
	conn->username = username;
	conn->password = password;
	conn->db = db;
	conn->mysql = (MYSQL*)xmalloc(sizeof(MYSQL));
	mysql_init(conn->mysql);

	if (mysql_real_connect(conn->mysql, host, username, password, db, 0, NULL, 0) == NULL) {
		printf("DB Error: %s\n", mysql_error(conn->mysql));
		*rval = INT_TO_JSVAL(-1);
		return true;
	}

	vector_add(bot->engine->dbconnections, conn);

	*rval = INT_TO_JSVAL(vector_count(bot->engine->dbconnections) - 1);
	return JS_TRUE;
}

static JSBool
DB_connectByPredef(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
	flourbot* bot = (flourbot*)JS_GetContextPrivate(cx);

	char* name = JS_GetStringBytes(JS_ValueToString(cx, argv[0]));

	// check for an existing connection with the specified parameters
	int i, match = -1;
	db_predef *db;
	for (i = 0; i < vector_count(bot->predefined_conns); i++) {
		db = vector_get(bot->predefined_conns, i);
		if (strcmp(db->name, name) == 0) {
			if (db->conn != -1) {
				*rval = INT_TO_JSVAL(db->conn);
				return JS_TRUE;
			}

			match = i;
		}
	}

	if (match == -1) {
		*rval = JSVAL_VOID;
		return JS_TRUE;
	}

	script_dbconnection *conn = (script_dbconnection*)xmalloc(sizeof(script_dbconnection));
	conn->host = db->host;
	conn->username = db->username;
	conn->password = db->password;
	conn->db = db->db;
	conn->mysql = (MYSQL*)xmalloc(sizeof(MYSQL));
	mysql_init(conn->mysql);

	if (mysql_real_connect(conn->mysql, db->host, db->username, db->password, db->db, 0, NULL, 0) == NULL) {
		printf("DB Error: %s\n", mysql_error(conn->mysql));
		*rval = INT_TO_JSVAL(-1);
		return JS_TRUE;
	}

	vector_add(bot->engine->dbconnections, conn);

	int idx = vector_count(bot->engine->dbconnections) - 1;
	db->conn = idx;

	*rval = INT_TO_JSVAL(idx);
	return JS_TRUE;
}

static int expand(char *target, int size, char *format, char **params, int count)
{
	int i, placeholders = 0;
	for (i = 0; i < strlen(format); i++) {
		if (format[i] == '?') {
			placeholders++;
		}
	}

	if (placeholders != count) {
		printf("[DEBUG] Invalid placeholder-count: %d %d\n", placeholders, count);
		return 1;
	}

	int j = 0, k = 0;
	for (i = 0; i < strlen(format); i++) {
		if (format[i] == '?') {
			int len = strlen(params[k]);
			if (j + len > size) {
				printf("[DEBUG] Buffer not large enough.\n");
				break;
			}

			target[j] = 0;
			strcat(target, params[k]);
			j += strlen(params[k]);
			k++;
		} else {
			target[j] = format[i];
			j++;
		}
	}

	target[j] = 0;

	return 0;
}

static JSBool
DB_query(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
	flourbot* bot = (flourbot*)JS_GetContextPrivate(cx);

	if (argc != 3) {
		printf("Invalid parameter count for DB.query()");

		*rval = JSVAL_VOID;
		return JS_TRUE;
	}

	int connId;
	JS_ValueToInt32(cx, argv[0], &connId);

	char* query = JS_GetStringBytes(JS_ValueToString(cx, argv[1]));

	JSObject *params;
	JS_ValueToObject(cx, argv[2], &params);

	script_dbconnection *conn = vector_get(bot->engine->dbconnections, connId);
	if (conn == NULL) {
		return false;
	}

	int paramCount = 0;
	if (JS_IsArrayObject(cx, params) == JS_TRUE) {
		jsuint paramLen;
		JS_GetArrayLength(cx, params, &paramLen);
		paramCount = paramLen;
	}

	char* newQuery = NULL;
	if (paramCount > 0) {
		char **cstrParams = (char**)xmalloc(sizeof(char*) * paramCount);
		int k;
		jsval cur;
		int totalLen = 0;
		for (k = 0; k < paramCount; k++) {
			JS_GetElement(cx, params, k, &cur);

			char *cur_str = JS_GetStringBytes(JS_ValueToString(cx, cur));

			int len = strlen(cur_str);
			totalLen += len;

			char *tmp = (char*)xmalloc(len * sizeof(char) * 2);
			mysql_real_escape_string(conn->mysql, tmp, cur_str, len);

			cstrParams[k] = tmp;
		}

		newQuery = (char*)xmalloc(2 * (strlen(query) + totalLen) * sizeof(char));

		if (!expand(newQuery, 2 * (strlen(query) + totalLen), query, cstrParams, paramCount)) {
			query = newQuery;
		}

		for (k = 0; k < paramCount; k++) {
			xfree(cstrParams[k]);
			cstrParams[k] = NULL;
		}
		xfree(cstrParams);
		cstrParams = NULL;
	}

	if (mysql_real_query(conn->mysql, query, strlen(query))) {
		printf("DB Error: %s\n", mysql_error(conn->mysql));
		return false;
	}

	MYSQL_RES *res = mysql_store_result(conn->mysql);
	if (res == NULL) {
		*rval = JSVAL_VOID;
		return JS_TRUE;
	}

	int rowCount = mysql_num_rows(res);
	if (rowCount == 0) {
		*rval = JSVAL_VOID;
		return JS_TRUE;
	}

	MYSQL_FIELD *fields = mysql_fetch_fields(res);

	// allocate output array
	JSObject* dbresObj = JS_NewArrayObject(cx, 0, NULL);

	int fieldCount = mysql_num_fields(res);
	MYSQL_ROW row = NULL;
	int i = 0, j;
	jsval val;
	while ((row = mysql_fetch_row(res)) != NULL) {
		JSObject *rowObj = JS_NewObject(cx, NULL, NULL, NULL);

		for (j = 0; j < fieldCount; j++) {
			jsval data = STRING_TO_JSVAL(JS_NewString(cx, xstrdup(row[j]), strlen(row[j])));
			JS_SetProperty(cx, rowObj, fields[j].name, &data);
		}

		val = OBJECT_TO_JSVAL(rowObj);
		JS_SetElement(cx, dbresObj, i, &val);

		i++;
	}

	mysql_free_result(res);

	xfree(newQuery);
	newQuery = NULL;

	*rval = OBJECT_TO_JSVAL(dbresObj);
	return JS_TRUE;
}

static char* script_loadbuffer(char* file)
{
	FILE* fh = fopen(file, "r");
	if (fh == NULL) {
		printf("failed to open %s: %s\n", file, strerror(errno));
		exit(0);
	}

	int totalread = 0, read = 0, buffersize = 512 * sizeof(char), chunksize = 256;
	char* buffer = (char*)xmalloc(buffersize);
	while (!feof(fh)) {
		if (totalread + chunksize * sizeof(char) > buffersize) {
			buffersize = (buffersize + chunksize * sizeof(char)) * 2;
			buffer = (char*)xrealloc(buffer, buffersize);
		}

		int read = fread(buffer + totalread, sizeof(char), chunksize, fh);
		totalread += read;
	}

	buffer[totalread] = 0;

	fclose(fh);

	return buffer;
}

static void script_load(flourbot* fb, script* s)
{
	//char* buffer = script_loadbuffer(s->filename);

	printf("Loaded script %s.\n", s->filename);

	JSObject *global;

	s->cx = JS_NewContext(fb->engine->rt, 0x1000);

	//JS_SetGCZeal(s->cx, 2);
	JS_SetContextPrivate(s->cx, (void*)fb);
	JS_SetOptions(s->cx, JSOPTION_VAROBJFIX);
	JS_SetErrorReporter(s->cx, reportError);
	
	global = JS_NewObject(s->cx, &global_class, NULL, NULL);
	JS_InitStandardClasses(s->cx, global);

	// Console object
	JSObject* console = JS_DefineObject(s->cx, global, "Console", &global_class, NULL, 0);
	JSFunction* println = JS_DefineFunction(s->cx, console, "println", &Console_println, 1, 0);

	// Bot Object
	JSObject* bot = JS_DefineObject(s->cx, global, "Bot", &global_class, NULL, 0);
	JSFunction* addCallback = JS_DefineFunction(s->cx, bot, "addCallback", &Bot_addCallback, 2, 0);
	JSFunction* getNick = JS_DefineFunction(s->cx, bot, "getNick", &Bot_getNick, 1, 0);

	// IRC Object
	JSObject* irc = JS_DefineObject(s->cx, global, "IRC", &global_class, NULL, 0);
	JSFunction* raw = JS_DefineFunction(s->cx, irc, "raw", &IRC_raw, 1, 0);
	JSFunction* msg = JS_DefineFunction(s->cx, irc, "msg", &IRC_msg, 2, 0);
	JSFunction* notice = JS_DefineFunction(s->cx, irc, "notice", &IRC_notice, 2, 0);
	JSFunction* mode = JS_DefineFunction(s->cx, irc, "mode", &IRC_mode, 3, 0);
	JSFunction* kick = JS_DefineFunction(s->cx, irc, "kick", &IRC_kick, 3, 0);
	JSFunction* getChannels = JS_DefineFunction(s->cx, irc, "getChannels", &IRC_getChannels, 0, 0);
	JSFunction* getNicks = JS_DefineFunction(s->cx, irc, "getNicks", &IRC_getNicks, 1, 0);
	JSFunction* isUserOnline = JS_DefineFunction(s->cx, irc, "isUserOnline", &IRC_isUserOnline, 2, 0);
	JSFunction* getNickCount = JS_DefineFunction(s->cx, irc, "getNickCount", &IRC_getNickCount, 1, 0);

	// DB Object
	JSObject* db = JS_DefineObject(s->cx, global, "DB", &global_class, NULL, 0);
	JSFunction* connect = JS_DefineFunction(s->cx, db, "connect", &DB_connect, 4, 0);
	JSFunction* connectByPredef = JS_DefineFunction(s->cx, db, "connectByPredef", &DB_connectByPredef, 1, 0);
	JSFunction* query = JS_DefineFunction(s->cx, db, "query", &DB_query, 2, 0);

	int lineno;
	s->script = JS_CompileFile(s->cx, JS_GetGlobalObject(s->cx), s->filename);
	s->gcobj = JS_NewScriptObject(s->cx, s->script);

	JS_AddNamedRoot(s->cx, &s->gcobj, "gcobj");

	s->callbacks = (vector*)xmalloc(sizeof(vector));
	vector_init(s->callbacks);

	if (s->script == NULL) {
		printf("Failed to compile %s!\n", s->filename);
	} else {
		jsval result;
		JS_ExecuteScript(s->cx, JS_GetGlobalObject(s->cx), s->script, &result);
		if (JS_IsExceptionPending(s->cx)) {
            JS_ReportPendingException(s->cx);
		}
	}
}

static void script_reload(flourbot* fb, script* s)
{
	printf("Reloading script %s.\n", s->filename);
	
	// Clean out old script
	JS_RemoveRoot(s->cx, &s->gcobj);
	
	int i;
	for (i = 0; i < vector_count(s->callbacks); i++) {
		script_callback* callback = vector_get(s->callbacks, i);
		JS_RemoveRoot(s->cx, &callback->func);
		xfree(callback->command);
		callback->command = NULL;
	}

	vector_free(s->callbacks);

	// Load new
	int lineno;
	s->script = JS_CompileFile(s->cx, JS_GetGlobalObject(s->cx), s->filename);
	s->gcobj = JS_NewScriptObject(s->cx, s->script);

	JS_AddRoot(s->cx, &s->gcobj);

	if (s->script == NULL) {
		printf("Failed to compile %s!\n", s->filename);
	} else {
		jsval result;
		JS_ExecuteScript(s->cx, JS_GetGlobalObject(s->cx), s->script, &result);
		if (JS_IsExceptionPending(s->cx)) {
            JS_ReportPendingException(s->cx);
		}
	}
}

void script_execute(script_engine* engine, irc_message* msg)
{
    int j = 0;
    for (j = 0; j < vector_count(engine->scripts); j++) {

		script* s = vector_get(engine->scripts, j);
		if (vector_count(s->callbacks) == 0) {
			continue;
		}

		JSObject* rootObject = JS_NewObject(s->cx, NULL, NULL, NULL);
		jsval jsRootObject = OBJECT_TO_JSVAL(rootObject);
		JS_AddRoot(s->cx, &jsRootObject);

		jsval source = JSVAL_NULL;
		if (msg->source != NULL) {
			source = STRING_TO_JSVAL(JS_NewStringCopyZ(s->cx, msg->source));
			JS_SetProperty(s->cx, rootObject, "source", &source);
		}

		jsval command = JSVAL_NULL;
		if (msg->command != NULL) {
			command = STRING_TO_JSVAL(JS_NewStringCopyZ(s->cx, msg->command));
			JS_SetProperty(s->cx, rootObject, "command", &command);
		}
		
		jsval params = JSVAL_NULL;
		if (msg->params != NULL) {
			JSObject* paramsObj = JS_NewArrayObject(s->cx, 0, NULL);
			
			int i;
			JSString* str;
			jsval val;
			for (i = 0; i < msg->paramcount; i++) {
				str = JS_NewStringCopyZ(s->cx, msg->params[i]);
				val = STRING_TO_JSVAL(str);
				JS_SetElement(s->cx, paramsObj, i, &val);
			}
			
			params = OBJECT_TO_JSVAL(paramsObj);
			JS_SetProperty(s->cx, rootObject, "params", &params);
		}

		jsval argv[3] = { source, command, params };

		JSObject* global = JS_GetGlobalObject(s->cx);

		int i;
		for (i = 0; i < vector_count(s->callbacks); i++) {
			script_callback* callback = vector_get(s->callbacks, i);

			if (strcasecmp(callback->command, msg->command) != 0) {
				continue;
			}

			jsval rval;
			if (JSVAL_IS_OBJECT(callback->func)) {
				JSObject *func;
				JS_ValueToObject(s->cx, callback->func, &func);
				if (JS_ObjectIsFunction(s->cx, func)) {
					JS_CallFunction(s->cx, global, (JSFunction*)func, 3, argv, &rval);
					if (JS_IsExceptionPending(s->cx)) {
    		    	    JS_ReportPendingException(s->cx);
					}
				}
			}
		}

		JS_RemoveRoot(s->cx, &jsRootObject);

		JS_GC(s->cx);
	}
}

void script_init(flourbot* fb)
{
	printf("Initializing script support, loading scripts:\n");

	fb->engine->rt = JS_NewRuntime(0x100000);

    int j = 0;
    for (j = 0; j < vector_count(fb->engine->scripts); j++) {
		script_load(fb, vector_get(fb->engine->scripts, j));
    }
}

void script_rehash(flourbot* fb)
{
	printf("Rehashing scripts:\n");

    int j = 0;
    for (j = 0; j < vector_count(fb->engine->scripts); j++) {
		script_reload(fb, vector_get(fb->engine->scripts, j));
    }
}

void script_close(script_engine* e)
{
    int j = 0;
    for (j = 0; j < vector_count(e->scripts); j++) {
		script* s = vector_get(e->scripts, j);

		// Clean out old script
		JS_RemoveRoot(s->cx, &s->gcobj);
	
		int i;
		for (i = 0; i < vector_count(s->callbacks); i++) {
			script_callback* callback = vector_get(s->callbacks, i);

			JS_RemoveRoot(s->cx, &callback->func);

			xfree(callback->command);
			callback->command = NULL;
		}

		vector_free(s->callbacks);

		JS_DestroyContext(s->cx);
	}

	JS_DestroyRuntime(e->rt);
}

