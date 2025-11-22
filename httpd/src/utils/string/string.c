#include "string.h"

#include <stdlib.h>
#include <string.h>

struct string *string_create(const char *str, size_t size)
{
    if (!str)
        return NULL;

    struct string *res = malloc(sizeof(struct string));
    if (!res)
        return NULL;

    res->data = malloc(size);
    if (!res->data)
    {
        free(res);
        return NULL;
    }

    memcpy(res->data, str, size);
    res->size = size;

    return res;
}

int string_compare_n_str(const struct string *str1, const char *str2, size_t n)
{
    if (!str1 || !str2)
        return -1;

    size_t minlen = n < str1->size ? n : str1->size;
    return memcmp(str1->data, str2, minlen);
}

void string_concat_str(struct string *str, const char *to_concat, size_t size)
{
    if (!str || !to_concat || size == 0)
        return;

    size_t fullsize = str->size + size;
    char *tmp = realloc(str->data, fullsize);
    if (!tmp)
        return;

    memcpy(tmp + str->size, to_concat, size);
    str->data = tmp;
    str->size = fullsize;
}

void string_destroy(struct string *str)
{
    if (!str)
        return;

    free(str->data);
    free(str);
}
