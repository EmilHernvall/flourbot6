#ifndef __STRING_H_
#define __STRING_H_

char* strtolower(const char* in);
int explode(char*** ret, const char* explodestr, char* delim);
char* cut_at_point(char* in, char delim, int count);
char is_numeric(char* str);

#endif
