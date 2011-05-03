#ifndef __CONFIG_H_

#define __CONFIG_H_

#include <stdio.h>

typedef struct _config {
	char* file;
	char* current_section;
	FILE* fh;
} config;

#define CONFIG_EOF -1
#define CONFIG_VALUE 1
#define CONFIG_SECTION 2
#define CONFIG_BLANK 3
#define CONFIG_INVALID 4
#define CONFIG_COMMENT 5

config* config_open(char* file);
void config_close(config* config);
int config_parse(config* config, char** rsection, char** rkey, char** rvalue);

#endif
