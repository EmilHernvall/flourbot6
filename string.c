#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include "util.h"
#include "stack.h"

char* strtolower(const char* in)
{
    int len = strlen(in), i;
    char* out = xmalloc(len+1);

    if (!(len > 0)) {
        return NULL;
	}

    for (i = 0; i < len; i++) {
        out[i] = tolower(in[i]);
    }

    out[i] = 0;

    return out;
}

int explode(char*** ret, char* instr, char* delim)
{
	stack stack;
	char** arr = NULL;
	char* str;
	char* result = NULL;
	char* saveptr = NULL;
	int i = 0, len = 0, count = 0, delim_len = 0;

	assert(instr != NULL);
	assert(delim != NULL);

	stack_init(&stack);

	delim_len = strlen(delim);
	assert(delim_len > 0);

	saveptr = instr;
	for (str = instr; ; str++) {
		if (*str == '\0' || strncmp(str, delim, delim_len) == 0) {
			len = str - saveptr;
			result = (char*)xmalloc((len+1)*sizeof(char));
			strncpy(result, saveptr, len);
			result[len] = '\0';
			stack_push(&stack, result);
			saveptr = str + delim_len;
		}

		if (*str == '\0') {
			break;
		}
	}

	count = stack_size(&stack);

	arr = (char**)xmalloc(sizeof(char*) * (count+1));
	for (i = 0; i < count; i++) {
		arr[i] = stack_peek(&stack);
	}

	arr[count] = NULL;
	*ret = arr;

	stack_free(&stack);

	return count;
}

char* cut_at_point(char* in, char delim, int count)
{
	int i, len = strlen(in), match = 0;
	for (i = 0; i < len; i++) {
		if (in[i] == delim) {
			match++;
		}

		if (match == count) {
			return in + i + 1;
		}
	}

	return NULL;
}

char is_numeric(char* str)
{
	char* str2;
	for (str2 = str; str2 != str + strlen(str); str2++) {
		if (!isdigit(*str2)) {
			return 0;
		}
	}

	return 1;
}

#ifdef TEST
int main(void)
{
	char* teststr = "lorem||ipsum dolor||sit amet||";
	char** result;
	int count;

	count = explode(&result, teststr, "||");
	
	int i;
	for (i = 0; i < count; i++) {
		printf("%s\n", result[i]);
		xfree(result[i]);
	}
	xfree(result);
}
#endif
