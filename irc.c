#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "flourbot.h"
#include "socket.h"
#include "string.h"
#include "irc_functions.h"
#include "script.h"

#define IRC_CALLBACK(name) void name (flourbot* bot, irc_message* msg)

struct irc_command_callback {
	const char* command;
	void (*callback)(flourbot*, irc_message*);
};

static channel* get_channel(flourbot* bot, char* name)
{
	int i;
	for (i = 0; i < vector_count(bot->channels); i++) {
		channel* c = vector_get(bot->channels, i);
		if (strcmp(c->name, name) == 0) {
			return c;
		}
	}
	
	return NULL;
}

static void cleanup_nicks(user* u)
{
	xfree(u->nick);
	xfree(u->ident);
	xfree(u->host);
	xfree(u);
}

static IRC_CALLBACK(irc_handle_msg)
{
	if (msg->paramcount < 2) {
		return;
	}

	char nick[100];
	if (!irc_getnick(msg->source, nick, sizeof(nick))) {
		return;
	}

	// CTCP
	if (*msg->params[1] == 1) {

		if (strncmp(msg->params[1]+1, "VERSION", 7) == 0) {
			char buffer[250];
			sprintf(buffer, "notice %s :%cVERSION %s%c\r\n", nick, 1, VERSION, 1);
			socket_write(bot->sock, buffer);
			printf("%s requested CTCP Version.\n", nick);
		} else if (strncmp(msg->params[1]+1, "ACTION", 6) == 0) {
	    	printf(" * %s %s\n", nick, msg->params[1]+8);
		} else {
			printf("<%s in %s> %s\n", nick, msg->params[0], msg->params[1]);
		}

	} else {

		if (strncmp(nick, "Aderyn", 6) == 0) {
			if (strncmp(msg->params[1], "suicide", 7) == 0) {
				quit("Suicide!");
			}
		}

		printf("<%s in %s> %s\n", nick, msg->params[0], msg->params[1]);
	}
}

static IRC_CALLBACK(irc_handle_notice)
{
	if (msg->paramcount >= 2) {
		char nick[100];
		if (nick != NULL && irc_getnick(msg->source, nick, sizeof(nick)) > 0) {
			printf(" -%s- %s\n", nick, msg->params[1]);
		} else {
			printf(" - %s\n", msg->params[1]);
		}
	}
}

static IRC_CALLBACK(irc_handle_join)
{
	if (msg->paramcount >= 1) {
		char nick[100];
		if (!irc_getnick(msg->source, nick, sizeof(nick))) {
			return;
		}
		
		printf(" * %s has joined %s\n", nick, msg->params[0]);
		
		// If it's this bot that has joined a channel we need to modified
		// our stored channel data to match.
		if (strcmp(nick, bot->nick) == 0) {
			
			channel* chan = get_channel(bot, msg->params[0]);
			if (chan == NULL) {
				chan = (channel*)xmalloc(sizeof(channel));
				chan->name = xstrdup(msg->params[0]);
				chan->topic = NULL;
				chan->topic_set_by = NULL;
				chan->topic_set_time = 0;
				hashtable_init(&chan->users, 50, 0.75);
				
				vector_add(bot->channels, chan);
				
				printf(" * Channel %s is unconfigured.\n", msg->params[0]);
			}
			
			chan->in_channel = true;
		} 
		// Another user has joined the channel. Locate the appropriate channel
		// and add the user to the list.
		else {
			channel* chan = get_channel(bot, msg->params[0]);
			if (chan != NULL) {
				user* n = (user*)xmalloc(sizeof(user));
				n->nick = xstrdup(nick);

				char ident[100];
				if (irc_getident(msg->source, ident, sizeof(ident))) {
					n->ident = xstrdup(ident);
				} else {
					n->ident = NULL;
				}

				char host[100];
				if (irc_gethost(msg->source, host, sizeof(host))) {
					n->host = xstrdup(host);
				} else {
					n->ident = NULL;
				}

				char* key = strtolower(irc_normalizenick(nick));
				hashtable_insert(&chan->users, key, n);
				xfree(key);
			}
		}
	}
}

static IRC_CALLBACK(irc_handle_part)
{
	if (msg->paramcount == 0) {
		return;
	}

	char nick[100];
	if (!irc_getnick(msg->source, nick, sizeof(nick))) {
		return;
	}

	if (msg->paramcount >= 2) {
		printf(" * %s has left %s (%s)\n", nick, msg->params[0], msg->params[1]);
	} else {
		printf(" * %s has left %s\n", nick, msg->params[0]);
	}
	
	channel* chan = get_channel(bot, msg->params[0]);
	if (chan != NULL) {
		if (strcmp(nick, bot->nick) == 0) {
			chan->in_channel = false;
		}
		else {
			char* key = strtolower(irc_normalizenick(nick));
			user *u = hashtable_delete(&chan->users, key);
			xfree(key);

			if (u != NULL) {
				xfree(u->nick);
				xfree(u->ident);
				xfree(u->host);
				xfree(u);
			}
		}
	}
}

static IRC_CALLBACK(irc_handle_quit)
{
	char nick[100];
	if (!irc_getnick(msg->source, nick, sizeof(nick))) {
		return;
	}

	if (msg->paramcount >= 1) {
		printf(" * %s has quit IRC (%s)\n", nick, msg->params[0]);
	} else {
		printf(" * %s has quit IRC\n", nick);
	}

	int j;
	for (j = 0; j < vector_count(bot->channels); j++) {
		channel *chan = vector_get(bot->channels, j);

		if (chan == NULL) {
			continue;
		}

		char* key = strtolower(irc_normalizenick(nick));
		user *u = hashtable_delete(&chan->users, key);
		xfree(key);

		if (u != NULL) {
			xfree(u->nick);
			xfree(u->ident);
			xfree(u->host);
			xfree(u);
		}
	}
}

static IRC_CALLBACK(irc_handle_mode)
{
	char nick[100];
	if (!irc_getnick(msg->source, nick, sizeof(nick))) {
		return;
	}

	if (msg->paramcount >= 3) {
		printf(" * %s changed mode for %s to: %s %s\n", nick, msg->params[0], msg->params[1], msg->params[2]);
	}	
	else if (msg->paramcount >= 2) {
		printf(" * %s changed mode for %s to: %s\n", nick, msg->params[0], msg->params[1]);
	}
}

static IRC_CALLBACK(irc_handle_kick)
{
	char nick[100];
	if (!irc_getnick(msg->source, nick, sizeof(nick))) {
		return;
	}

	if (msg->paramcount >= 3) {
		printf(" * %s was kicked from %s by %s: %s\n", msg->params[1], msg->params[0], nick, msg->params[2]);
	} else if (msg->paramcount >= 2) {
		printf(" * %s was kicked from %s by %s.\n", msg->params[1], msg->params[0], nick);
	}

	channel *chan = get_channel(bot, msg->params[0]);
	char *kicked = msg->params[1];

	char* key = strtolower(irc_normalizenick(kicked));
	user *u = hashtable_delete(&chan->users, key);
	xfree(key);

	if (u != NULL) {
		xfree(u->nick);
		xfree(u->ident);
		xfree(u->host);
		xfree(u);
	}
}

static IRC_CALLBACK(irc_handle_nick)
{
	if (msg->paramcount >= 1) {
		char nick[100];
		if (!irc_getnick(msg->source, nick, sizeof(nick))) {
			return;
		}

		printf(" * %s is now known as %s\n", nick, msg->params[0]);

		int j;
		for (j = 0; j < vector_count(bot->channels); j++) {
			channel *chan = vector_get(bot->channels, j);

			if (chan == NULL) {
				continue;
			}

			char* key;
			key = strtolower(irc_normalizenick(nick));
			user* u = hashtable_delete(&chan->users, key);
			free(key);

			if (u != NULL) {
				xfree(u->nick);
				u->nick = xstrdup(msg->params[0]);

				key = strtolower(irc_normalizenick(msg->params[0]));
				hashtable_insert(&chan->users, key, u);
				xfree(key);
			}
		}
	}
}

/*
:besrv.c0la.se 353 Foobot = #c0la :Foobot Aderyn uppe @Hjorten mejjad Erandir Mmxsc[aw] doden_ zappe wikro Soode plux @Tappe wildie Christoffer_V @Pezzen Rojola
:besrv.c0la.se 366 Foobot #c0la :End of /NAMES list.
*/
static IRC_CALLBACK(irc_handle_names_response)
{
	if (msg->paramcount < 4) {
		return;
	}
	
	channel* chan = get_channel(bot, msg->params[2]);
	if (chan == NULL) {
		return;
	}
	
	// If this is the first names-response we need to
	// de allocate the previous nicks array.
	if (!bot->names_parsing_in_progress) {
		hashtable_free(&chan->users, cleanup_nicks);
		hashtable_init(&chan->users, 100, 0.75);
		bot->names_parsing_in_progress = true;
	}
	
	char** nicks;
	int nickcount = explode(&nicks, msg->params[3], " ");

	int i;
	for (i = 0; i < nickcount; i++) {
		if (strlen(nicks[i]) == 0) {
			continue;
		}

		user* u = (user*)malloc(sizeof(user));
		u->nick = nicks[i];
		u->ident = NULL;
		u->host = NULL;

		char* key = strtolower(irc_normalizenick(nicks[i]));
		hashtable_insert(&chan->users, key, u);
		xfree(key);
	}
	
	xfree(nicks);
	nicks = NULL;
	
	printf("%s: %s\n", msg->params[2], msg->params[3]);
}

static IRC_CALLBACK(irc_handle_numeric)
{
	char* buffer;
	int command = atoi(msg->command);
	switch (command) {
		case 1: // welcome
			bot->is_registered = true;
		case 2: // host, version
		case 3: // date
			if (msg->paramcount >= 2) {
				printf("%s\n", msg->params[1]);
			}
			break;
		/*case 4: // host version modes
		case 5: // supported
			break;*/

		// usage stats
		case 251:
		case 255:
		case 265:
		case 266:
			if (msg->paramcount >= 2) {
				printf("%s\n", msg->params[1]);
			}
			break;
		case 252:
			if (msg->paramcount >= 2) {
				printf("%s operators online\n", msg->params[1]);
			}
			break;
		case 254:
			if (msg->paramcount >= 2) {
				printf("%s channels formed\n", msg->params[1]);
			}
			break;
			
		
		case 332: // topic
			if (msg->paramcount >= 3) {
				printf("Topic for %s is '%s'\n", msg->params[1], msg->params[2]);
			}
			break;
		case 333: // topic data
			if (msg->paramcount >= 4) {
				time_t changed = (time_t)atoi(msg->params[3]);
				char timebuf[20];
				strftime(timebuf,20,"%Y-%m-%d %H:%M:%S",gmtime(&changed));
				printf("Set by %s on %s.\n", msg->params[2], timebuf);
				
				//bot->
			}
			break;
			
		case 353: // names
			irc_handle_names_response(bot, msg);
			break;
		case 366: //names end
			bot->names_parsing_in_progress = false;
			break;

		case 372: // motd
		case 375: // motd start
		case 376: // motd end
		case 422: // motd missing
			if (msg->paramcount >= 2) {
				printf("%s\n", msg->params[1]);
			}
			break;	
		
		default:
			printf("%s\n", msg->raw);
			break;
	}
}

static IRC_CALLBACK(irc_handle_ping)
{
	#ifdef DEBUG
		//printf("Responded to ping.\n");
	#endif

	char pongbuffer[100];
	sprintf(pongbuffer, "PONG :%s\r\n", msg->params[0]);
	socket_write(bot->sock, pongbuffer);
}

static IRC_CALLBACK(irc_handle_default)
{
	if (is_numeric(msg->command)) {
		irc_handle_numeric(bot, msg);
	} else {
	    printf("%s\n", msg->raw);
	}
}

/*
The Augmented BNF representation for this is:

message    =  [ ":" prefix SPACE ] command [ params ] crlf
prefix     =  servername / ( nickname [ [ "!" user ] "@" host ] )
command    =  1*letter / 3digit
params     =  *14( SPACE middle ) [ SPACE ":" trailing ]
		   =/ 14( SPACE middle ) [ SPACE [ ":" ] trailing ]

nospcrlfcl =  %x01-09 / %x0B-0C / %x0E-1F / %x21-39 / %x3B-FF
				; any octet except NUL, CR, LF, " " and ":"
middle     =  nospcrlfcl *( ":" / nospcrlfcl )
trailing   =  *( ":" / " " / nospcrlfcl )

SPACE      =  %x20        ; space character
crlf       =  %x0D %x0A   ; "carriage return" "linefeed"
*/
void handle_message(flourbot* bot, const char* line)
{
    char** result;
    int count;

    if (!(strlen(line) > 0))
      return;

    count = explode(&result, line, " ");

    irc_message msg;
    msg.raw = line;

	// when the sender is specified, the first argument begins with :
	// if that isn't the case, the first argument is the command
	int commandPos;
	if (line[0] == ':') {
		msg.source = result[0] + 1;
		msg.command = strtolower(result[1]);
		commandPos = 1;
	} else {
		msg.source = NULL;
		msg.command = strtolower(result[0]);
		commandPos = 0;
	}

	// all the values that follows the command are called parameters.
	// they are all counted and stored one by one until we reach a 
	// parameter that begins with :
	
	// count the parameter count in order to allocate an array of the
	// suitable size
	int i, paramCount = 0;
	for (i = commandPos + 1; i < count; i++) {
		paramCount++;
		if (result[i][0] == ':') {
			break;
		}
	}
	
	// transfer the parameters to the target array
	msg.paramcount = paramCount;
	msg.params = (char**)xmalloc(sizeof(char*) * paramCount);
	int j = 0;
	for (i = commandPos + 1; i < count; i++) {
		if (result[i][0] == ':') {
			msg.params[j] = cut_at_point((char*)line, ' ', i) + 1;
			break;
		} else {
			msg.params[j] = result[i];
		}
		j++;
	}
	
	struct irc_command_callback callbacks[] = { 
		{ "privmsg", &irc_handle_msg },
		{ "notice", &irc_handle_notice },
		{ "nick", &irc_handle_nick },
		{ "join", &irc_handle_join },
		{ "part", &irc_handle_part },
		{ "quit", &irc_handle_quit },
		{ "mode", &irc_handle_mode },
		{ "kick", &irc_handle_kick },
		{ "ping", &irc_handle_ping },
		{ NULL, &irc_handle_default } };

	int c = 0;
	while (true) {
		if (callbacks[c].command == NULL || 
			 strncmp(callbacks[c].command, msg.command, strlen(callbacks[c].command)) == 0) {

			(callbacks[c].callback)(bot, &msg);
			break;

		} else {
			c++;
		}
	}

	script_execute(bot->engine, &msg);

	xfree(msg.command);
	msg.command = NULL;

	xfree(msg.params);
	msg.params = NULL;

	for (i = 0; i < count; i++) {
		xfree(result[i]);
		result[i] = NULL;
	}

	xfree(result);
	result = NULL;
}
