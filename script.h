#ifndef __SCRIPT_H_

#define __SCRIPT_H_

#include "flourbot.h"

void script_init(flourbot* fb);
void script_rehash(flourbot* fb);
void script_execute(script_engine* engine, irc_message* msg);
void script_close(script_engine* engine);

#endif

