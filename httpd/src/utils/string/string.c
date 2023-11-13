#include <string.h>
#include <stdlib.h>

#include "string.h"

struct string *string_create(const char *str, size_t size)
{
    struct string *string = malloc(sizeof(struct string));
    string->size = size;
    string->data = malloc(size);
    for(size_t i = 0;i<size;i++)
    {
        string->data[i] = str[i];
    }
    return string;
}

int string_compare_n_str(const struct string *str1, const char *str2, size_t n)
{
    for(size_t i = 0; i < n;i++){
        if(str1->data[i] != str2[i])
        {
            if (str1->data[i] > str2[i])
            {
                return 1;
            }
            else
                return -1;
        }
    }
    return 0;
}

void string_concat_str(struct string *str, const char *to_concat, size_t size)
{
    char *concat = malloc(str->size + size);
    for(size_t j = 0; j < str->size;j++)
    {
        concat[j] = str->data[j];
    }
    for(size_t i = str->size - 1;i < str->size + size;i++)
    {
        concat[i] = to_concat[i - str->size + 1];
    }
    str->data = malloc(str->size + size);
    str->data = concat;
    free(concat);
}

void string_destroy(struct string *str)
{
    free(str->data);
    free(str);
}
