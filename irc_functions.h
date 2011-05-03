#ifndef __IRC_FUNCTIONS_H_
#define __IRC_FUNCTIONS_H_


char* irc_normalizenick(char* nick);
int irc_getnick(const char* userinfo, char* target, size_t size);
int irc_getident(const char* userinfo, char* target, size_t size);
int irc_gethost(const char* userinfo, char* target, size_t size);

#endif
