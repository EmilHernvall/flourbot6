#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <pthread.h>

#include "flourbot.h"
#include "socket.h"
#include "script.h"
#include "irc.h"
#include "config.h"

static bool run;
static bool killed = false;
static bool reload = false;
static char* quit_msg = VERSION;

/*
 * Tells the bot to quit by switching of the main loop.
 */
void quit(char* msg)
{
	quit_msg = xstrdup(msg);
	run = false;
}

/*
 * Handle exit signals.
 */
void handle_exit(int code)
{
	killed = true;
}

void handle_reload(int code)
{
	reload = true;
}

/*
 * Parse configuration file.
 */
void parse_config(flourbot* c, char* cfgfile)
{
	config* config = config_open(cfgfile);
	if (config == NULL) {
		fprintf(stderr, "error opening config file!\n");
		exit(1);
	}

	c->nick = NULL;
	c->user = NULL;
	c->desc = NULL;
	c->server = NULL;
	c->port = 0;

	c->channels = (vector*)xmalloc(sizeof(vector));
	vector_init(c->channels);

	c->engine->scripts = (vector*)xmalloc(sizeof(vector));
	vector_init(c->engine->scripts);

	c->engine->dbconnections = (vector*)xmalloc(sizeof(vector));
	vector_init(c->engine->dbconnections);

	c->predefined_conns = (vector*)xmalloc(sizeof(vector));
	vector_init(c->predefined_conns);

	int type;
	char* section;
	char* key;
	char* value;
	script* thisscript = NULL;
	db_predef* db = NULL;
	while ((type = config_parse(config, &section, &key, &value)) != CONFIG_EOF) {
//		printf("%d %d %d %d\n", type, section, key, value);

		if (type == CONFIG_SECTION) {
			
			if (strncmp(section, "script", 5) == 0) {
				thisscript = (script*)xmalloc(sizeof(script));
				vector_add(c->engine->scripts, thisscript);
			}
			else if (strncmp(section, "db", 2) == 0) {
				db = (db_predef*)xmalloc(sizeof(db_predef));
				db->name = NULL;
				db->host = NULL;
				db->username = NULL;
				db->password = NULL;
				db->port = 0;
				db->db = NULL;
				db->conn = -1;
				vector_add(c->predefined_conns, db);
			}

		}
		else if (type == CONFIG_VALUE) {

			// flourbot
			if (strncmp(section, "flourbot", 8) == 0) {
				
				if (strncmp(key, "nick", 4) == 0) {
					c->nick = xstrdup(value);
				}
				else if (strncmp(key, "user", 4) == 0) {
					c->user = xstrdup(value);
				}
				else if (strncmp(key, "desc", 4) == 0) {
					c->desc = xstrdup(value);
				}
				else if (strncmp(key, "server", 6) == 0) {
					c->server = xstrdup(value);
				}
				else if (strncmp(key, "port", 4) == 0) {
					c->port = atoi(value);
				}

			}
			else if (strncmp(section, "channel", 7) == 0) {

				if (strncmp(key, "name", 4) == 0) {
					channel* thischannel = (channel*)xmalloc(sizeof(channel));
					thischannel->name = xstrdup(value);
					thischannel->topic = NULL;
					thischannel->topic_set_by = NULL;
					thischannel->topic_set_time = 0;
					hashtable_init(&thischannel->users, 50, 0.75);
					thischannel->in_channel = 0;
					vector_add(c->channels, thischannel);
				}

			}
			else if (strncmp(section, "script", 6) == 0) {
				
				if (strncmp(key, "name", 4) == 0) {
					thisscript->name = xstrdup(value);
				}
				else if (strncmp(key, "filename", 4) == 0) {
					thisscript->filename = xstrdup(value);
				}

			}
			else if (strncmp(section, "db", 2) == 0) {

				if (strncmp(key, "name", 4) == 0) {
					db->name = xstrdup(value);
				} else if (strncmp(key, "host", 4) == 0) {
					db->host = xstrdup(value);
				} else if (strncmp(key, "username", 8) == 0) {
					db->username = xstrdup(value);
				} else if (strncmp(key, "password", 8) == 0) {
					db->password = xstrdup(value);
				} else if (strncmp(key, "port", 4) == 0) {
					db->port = atoi(value);
				} else if (strncmp(key, "db", 2) == 0) {
					db->db = xstrdup(value);
				}
			}

			xfree(key);
			key = NULL;

			xfree(value);
			value = NULL;
		}
	}

	c->names_parsing_in_progress = false;

	config_close(config);
}

static void* network_thread(flourbot* bot)
{
	char* data = NULL;
	int len = 0;
	while (run) {
        if ((len = socket_read(bot->sock, &data)) != 0) {
			assert(len > 0);
			assert(data != NULL);

			stack_push(bot->buffer, data);
        } else {
            usleep(100);
        }
	}
}

int main(int argc, char** argv)
{
	if (argc != 2) {
		printf("usage: flourbot conffile\n");
		return 1;
	}

	flourbot bot;

    irc_socket msocket;
	bot.sock = &msocket;
	bot.buffer = (stack*)xmalloc(sizeof(stack));
	stack_init(bot.buffer);

	script_engine engine;
	bot.engine = &engine;

	parse_config(&bot, argv[1]);

	/*printf("predefined conns:\n");
	int k;
	for (k = 0; k < vector_count(bot.predefined_conns); k++) {
		db_predef *db = vector_get(bot.predefined_conns, k);
		printf("name: %s\n", db->name);	
		printf("host: %s\n", db->host);	
		printf("username: %s\n", db->username);	
		printf("password: %s\n", db->password);	
		printf("db: %s\n", db->db);	
	}*/

	if (bot.nick == NULL) {
		fprintf(stderr, "Error in config: nick is missing!\n");
		return 1;
	}
	if (bot.user == NULL) {
		fprintf(stderr, "Error in config: user is missing!\n");
		return 1;
	}
	if (bot.desc == NULL) {
		fprintf(stderr, "Error in config: desc is missing!\n");
		return 1;
	}
	if (bot.server == NULL) {
		fprintf(stderr, "Error in config: server is missing!\n");
		return 1;
	}
	if (bot.port == 0) {
		fprintf(stderr, "Error in config: port is missing!\n");
		return 1;
	}

	// Initialize script engine
	script_init(&bot);

	// connect to server
    socket_connect(&msocket, bot.server, bot.port);

	// Load scripts	

	// Introduce ourselves to the server
    char buffer[256 * sizeof(char)];

    memset(buffer, 0, 256*sizeof(char));
    sprintf(buffer, "nick %s\r\n", bot.nick);
    socket_write(&msocket, buffer);
	
    memset(buffer, 0, 256*sizeof(char));
    sprintf(buffer, "user %s %s %s :%s\r\n", bot.user, bot.user, bot.user, bot.desc);
    socket_write(&msocket, buffer);

	bot.is_registered = false;
	bool has_joined = false;

	// Add signal handlers to ease clean shutdown
	signal(SIGTERM, handle_exit);
	signal(SIGINT, handle_exit);
	signal(SIGHUP, handle_reload);

	run = true;

	// Spawn a thread for reading from the network
	pthread_t thread;
	pthread_create(&thread, NULL, network_thread, &bot);

    while (run == true)
    {
		if (killed) {
			quit_msg = "Received TERM/INT signal.\n";
			printf("%s", quit_msg);
			break;
		}

		if (reload) {
			script_rehash(&bot);
			reload = false;
		}

		if (bot.is_registered && has_joined == false) {
			printf("Joining channels:\n");

			int i = 0;
			for (i = 0; i < vector_count(bot.channels); i++) {
				channel *chan = vector_get(bot.channels, i);
			    memset(buffer, 0, 256*sizeof(char));
			    sprintf(buffer, "join %s\r\n", chan->name);
			    socket_write(&msocket, buffer);

				printf("Joining %s.\n", chan->name);
			}

			has_joined = true;
		}

		if (stack_size(bot.buffer) > 0) {
			char* msgData = stack_peek(bot.buffer);

		    handle_message(&bot, msgData);

	        xfree(msgData);
			msgData = NULL;
		}

		usleep(10);
    }

	printf("Shutting down.\n");

    memset(buffer, 0, 256*sizeof(char));
    sprintf(buffer, "quit :%s\r\n", quit_msg);
    socket_write(&msocket, buffer);
//	usleep(1000000);

//	socket_close(&msocket);

	xfree(bot.nick);
	bot.nick = NULL;

	xfree(bot.user);
	bot.user = NULL;

	xfree(bot.desc);
	bot.desc = NULL;

	xfree(bot.server);
	bot.server = NULL;

    int i = 0;
    for (i = 0; i < vector_count(bot.channels); i++) {
		channel *chan = vector_get(bot.channels, i);

		xfree(chan->name);
		chan->name = NULL;

		xfree(chan);
		chan = NULL;
    }

	vector_free(bot.channels);

	script_close(&engine);

	int j = 0;
	for (j = 0; j < vector_count(engine.scripts); j++) {
		script *s = vector_get(engine.scripts, j);

		xfree(s->name);
		s->name = NULL;

		xfree(s->filename);
		s->filename = NULL;

		xfree(s);
		s = NULL;
	}

	vector_free(engine.scripts);

    return 0;
}

