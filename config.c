#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "util.h"
#include "config.h"

static int config_readline(char** buffer, FILE* fh)
{
	int size = 255;
	int read = 0;
	char* outbuffer = (char*)xmalloc(sizeof(char) * size);
	while (!feof(fh)) {
		char c = fgetc(fh);

		if (size == read) {
			size *= 2;
			outbuffer = (char*)xrealloc(outbuffer, sizeof(char) * size);
		}

		outbuffer[read] = c;

		if (c == '\n') {
			outbuffer[read] = 0;
			*buffer = outbuffer;
			return read;
		}

		read++;
	}

	xfree(outbuffer);
	outbuffer = NULL;

	return CONFIG_EOF;
}

config* config_open(char* file)
{
	config* c = (config*)xmalloc(sizeof(config));
	c->file = file;
	c->current_section = NULL;

	c->fh = fopen(file, "r");
	
	if (c->fh != NULL) {
		return c;
	} else {
		return NULL;
	}
}

void config_close(config* config)
{
	if (config->current_section != NULL) {
		xfree(config->current_section);
		config->current_section = NULL;
	}

	fclose(config->fh);

	xfree(config);
	config = NULL;
}

int config_parse(config* config, char** rsection, char** rkey, char** rvalue)
{
	if (feof(config->fh)) {
		return CONFIG_EOF;
	}

	char* line = NULL;
	size_t len = 0, len2 = 0;
	int type;
	if ((len = getline(&line, &len2, config->fh)) == -1) {
		return CONFIG_EOF;
	}

	if (len == 1) {
		return CONFIG_BLANK;
	}

	len--;
	line[len] = 0;

	if (*line == ';') {
		char* value = xstrdup(line);
		*rvalue = value;

		return CONFIG_COMMENT;
	}
	else if (strchr(line, '=')) {
		char* key;
		char* value;

		key = xstrdup(line);
		char* tmp = strchr(key, '=');
		*tmp = 0;
		
		value = xstrdup(tmp + 1);

		*rsection = config->current_section;
		*rkey = key;
		*rvalue = value;

		type = CONFIG_VALUE;
	} 
	else if (*line == '[' && *(line + len - 1) == ']') {
		char* section = xmalloc(len*sizeof(char));
		strncpy(section, line+1, len-2);
		section[len-2] = 0;

		xfree(config->current_section);
		config->current_section = NULL;

		config->current_section = section;

		*rsection = section;

		type = CONFIG_SECTION;
	} 
	else {
		type = CONFIG_INVALID;
	}

	xfree(line);
	line = NULL;

	return type;
}

#ifdef TEST
int main(void)
{
	config* config = config_open("test.conf");
	if (config == NULL) {
		printf("error opening config file!\n");
		exit(1);
	}

	int type;
	char* section;
	char* key;
	char* value;
	while ((type = config_parse(config, &section, &key, &value)) != CONFIG_EOF) {
		if (type == CONFIG_VALUE) {
			printf("%s: %s = %s\n", section, key, value);

			xfree(key);
			key = NULL;

			xfree(value);
			value = NULL;
		}
	}

	config_close(config);

	return 0;
}
#endif
