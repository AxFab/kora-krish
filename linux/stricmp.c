#include <ctype.h>

/* Performs case-insensitive string comparison. */
int stricmp(const char *s1, const char *s2)
{
    while (*s1 && (tolower(*s1) == tolower(*s2))) {
        ++s1;
        ++s2;
    }

    return *s1 - *s2;
}

