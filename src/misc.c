#include <string.h>

/* Returns the index of the first occurence of ch in str */
int strind(const char* str, const char ch)
{
    int index = 0;
    while(*str != 0 || str == NULL)
    {
        if(*str == ch)
            break;
        ++index;
        ++str;
    }
    return index;
}
