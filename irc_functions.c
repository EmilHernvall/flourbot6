#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

char* irc_normalizenick(char* nick)
{
	if (*nick == '+' || *nick == '@' || *nick == '%') {
		return nick + 1;
	}

	return nick;
}

int irc_getnick(const char* userinfo, char* target, size_t size) 
{   
	char* endpos;
	int len;

	if (userinfo == NULL) {
		return 0;
	}

	assert(target != NULL);
	assert(size > 0);

	endpos = strchr(userinfo,'!');
	if (endpos != NULL) {
		len = endpos - userinfo;
		assert(size > len);

		strncpy(target, userinfo, len);
		target[len] = '\0';

		return len;
	}

	endpos = strchr(userinfo,'@');
	if (endpos != NULL) {
		len = endpos - userinfo;
		assert(size > len);

		strncpy(target, userinfo, len);
		target[len] = '\0';

		return len;
	}

	len = strlen(userinfo);
	if (len > size) {
		len = size;
	}

	strncpy(target, userinfo, size);
	target[len] = '\0';

	return len;
}

int irc_getident(const char* userinfo, char* target, size_t size)
{
	char* startpos;
	char* endpos;
	int len;

	assert(userinfo != NULL);
	assert(target != NULL);
	assert(size > 0);

	startpos = strchr(userinfo, '!');
	if (startpos == NULL) {
		return 0;
	}

	endpos = strchr(userinfo, '@');
	if (endpos == NULL) {
		return 0;
	}
   
	len = endpos - startpos - 1;
	assert(size > len);

	strncpy(target, startpos + 1, len);
	target[len] = '\0';

	return len;
}

int irc_gethost(const char* userinfo, char* target, size_t size) 
{
	char* startpos;
	int len;

	assert(userinfo != NULL);
	assert(target != NULL);
	assert(size > 0);

	startpos = strchr(userinfo, '@');
	if (startpos == NULL) {
		return 0;
	}

	len = strlen(userinfo) - (startpos - userinfo) - 1;
	assert(size > len);

	strncpy(target, startpos + 1, len);
	target[len] = '\0';

	return len;
}

#ifdef TEST
int main(void)
{
	char* userinfo = "aderyn"; //"aderyn!emil@c0la.se";
	char buffer[10];

	if (irc_getnick(userinfo, buffer, sizeof(buffer)) > 0) {
		printf("nick: %s\n", buffer);
	}

	if (irc_getident(userinfo, buffer, sizeof(buffer)) > 0) {
		printf("ident: %s\n", buffer);
	}

	if (irc_gethost(userinfo, buffer, sizeof(buffer)) > 0) {
		printf("host: %s\n", buffer);
	}
}
#endif
