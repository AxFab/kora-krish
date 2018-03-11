#include <ctype.h>
#include <string.h>

#define TCHAR char
#define _SFX(n)  str ## n
#define _CTYPE(n) is ## n

/* Read the first character of a string */
#define _tcsrd(x)  ((x)[0])
/* Get a pointer to the next character on a string */
#define _tcsnext(x)  (&(x)[1])


/* Appends src to dest. */
#define _tcscat _SFX(cat)
/* Finds c in str. */
#define _tcschr _SFX(chr)
/* Compares one string to another. */
#define _tcscmp _SFX(cmp)
/* Copies string src to dest. */
#define _tcscpy _SFX(cpy)
/* Scans a string. */
#define _tcscspn _SFX(cspn)
/* Performs case-insensitive string comparison. */
#define _tcsicmp _SFX(icmp)
/* Calculates length of a string. */
#define _tcslen _SFX(len)
/* Calculates length of a string. */
#define _tcsnlen _SFX(nlen)
/* Appends at most maxlen characters of src to dest. */
#define _tcsncat _SFX(ncat)
/* Compares at most maxlen characters of one string to another. */
#define _tcsncmp _SFX(ncmp)
/* Copies at most maxlen characters of src to dest. */
#define _tcsncpy _SFX(ncpy)
/* Scans one string for the first occurrence of any character that's in a second string. */
#define _tcspbrk _SFX(pbrk)
/* Finds the last occurrence of c in str. */
#define _tcsrchr _SFX(rchr)
/* Scans a string for a segment that is a subset of a set of characters. */
#define _tcsspn _SFX(spn)
/* Finds the first occurrence of a substring in another string. */
#define _tcsstr _SFX(str)
/* Scans s1 for the first token not contained in s2. */
#define _tcstok _SFX(tok)
/* duplicate a string  */
#define _tcsdup _SFX(dup)
/* duplicate a string at most maxlen characters */
#define _tcsndup _SFX(ndup)
/* Convert a string to lowercase. */
#define _tcslwr _SFX(lwr)
/* Convert a string to uppercase. */
#define _tcsupr _SFX(upr)
/* Compare characters of two strings without regard to case. */
#define _tcsnicmp _SFX(nicmp)
/* Reverse characters of a string. */
#define _tcsrev _SFX(rev)
/* Set characters of a string to a character. */
#define _tcsset _SFX(set)
/* Initialize characters of a string to a given format. */
#define _tcsnset _SFX(nset)

/* Scans s1 for the first token not contained in s2. */
#define _tcstok_r _SFX(tok_r)

/* Compare strings using locale-specific information. */
#define _tcscoll _SFX(coll)
#define _tcsicoll _SFX(icoll)
#define _tcsncoll _SFX(ncoll)
#define _tcsnicoll _SFX(nicoll)

/* String transformation */
#define _tcsxfrm _SFX(xfrm)



#define _istalnum _CTYPE(alnum)
#define _istalpha _CTYPE(alpha)
#define _istcntrl _CTYPE(cntrl)
#define _istdigit _CTYPE(digit)
#define _istgraph _CTYPE(graph)
#define _istlower _CTYPE(lower)
#define _istprint _CTYPE(print)
#define _istpunct _CTYPE(punct)
#define _istspace _CTYPE(space)
#define _istupper _CTYPE(upper)
#define _istxdigit _CTYPE(xdigit)

#define _istblank _CTYPE(blank)


#define _totlower tolower
#define _totupper toupper

