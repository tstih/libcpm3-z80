/*
 * stdlib.c
 *
 * Standard C library stdlib implementation.
 *
 * NOTES:
 *  rand() function uses long, can we do it with int? 
 *  Is there a way to preserve exit code? CPM 3 System control block 
 *  might have this?
 *  qsort is missing.
 *
 * MIT License (see: LICENSE)
 * copyright (c) 2021 tomaz stih
 *
 * 28.04.2021   tstih
 *
 */
#include <ctype.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include <util/mem.h>
#include <util/leanmean.h>

#include <sys/bdos.h>

/* Newline type. */
char nltype=NL_LF;

void exit(int status)
{
    status;
    /* Unfortunately, the status is lost in CP/M. */
    bdos(P_TERMCPM, 0);
}

int abs(int i)
{
    return i < 0 ? -i : i;
}

int atoi(const char *str)
{
    int res = 0;
    for (int i = 0; str[i] != '\0'; ++i)
        res = res * 10 + str[i] - '0';
    return res;
}

char* itoa(int num, char* str, int base)
{
	int i = 0;
	bool isneg = false;
	/* Handle 0 explicitely, otherwise empty string is printed for 0 */
	if (num == 0)
	{
		str[i++] = '0';
		str[i] = '\0';
		return str;
	}
	// In standard itoa(), negative numbers are handled only with
	// base 10. Otherwise numbers are considered unsigned.
	if (num < 0 && base == 10)
	{
		isneg = true;
		num = -num;
	}
	// Process individual digits
	while (num != 0)
	{
		int rem = num % base;
		str[i++] = (rem > 9)? (rem-10) + 'a' : rem + '0';
		num = num/base;
	}
	// If number is negative, append '-'
	if (isneg)
		str[i++] = '-';
	str[i] = '\0'; // Append string terminator
	// Reverse the string
	strrev(str, i);
	return str;
}

#ifndef LEAN_AND_MEAN

static unsigned long _next = 1;

int rand(void) /* RAND_MAX assumed to be 32767 */
{
    _next = _next * 1103515245 + 12345;
    return (unsigned int)(_next / 65536) % 32768;
}

#else 

static unsigned int _next = 1;

int rand(void) /* RAND_MAX assumed to be 32767 */
{
    _next = _next * 753 + 22444;
    return (unsigned int)(_next % 32768);
}

#endif /* LEAN_AND_MEAN */

void srand(unsigned int seed)
{
    _next = seed;
}



const static char _cvt_in[] = {
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9,           /* '0' - '9' */
    100, 100, 100, 100, 100, 100, 100,      /* punctuation */
    10, 11, 12, 13, 14, 15, 16, 17, 18, 19, /* 'A' - 'Z' */
    20, 21, 22, 23, 24, 25, 26, 27, 28, 29,
    30, 31, 32, 33, 34, 35,
    100, 100, 100, 100, 100, 100,           /* punctuation */
    10, 11, 12, 13, 14, 15, 16, 17, 18, 19, /* 'a' - 'z' */
    20, 21, 22, 23, 24, 25, 26, 27, 28, 29,
    30, 31, 32, 33, 34, 35};


#ifndef LEAN_AND_MEAN

unsigned long strtoul(char *string, char **end_ptr, int base)
{
    register char *p;
    register unsigned long int result = 0;
    register unsigned digit;
    int any_digits = 0;

    /* Skip any leading blanks. */
    p = string;
    while (isspace(*p))
    {
        p += 1;
    }
    if (base == 0)
    {
        if (*p == '0')
        {
            p += 1;
            if (*p == 'x')
            {
                p += 1;
                base = 16;
            }
            else
            {
                any_digits = 1;
                base = 8;
            }
        }
        else
            base = 10;
    }
    else if (base == 16)
    {
        if ((p[0] == '0') && (p[1] == 'x'))
        {
            p += 2;
        }
    }

    if (base == 8)
    {
        for (;; p += 1)
        {
            digit = *p - '0';
            if (digit > 7)
            {
                break;
            }
            result = (result << 3) + digit;
            any_digits = 1;
        }
    }
    else if (base == 10)
    {
        for (;; p += 1)
        {
            digit = *p - '0';
            if (digit > 9)
            {
                break;
            }
            result = (10 * result) + digit;
            any_digits = 1;
        }
    }
    else if (base == 16)
    {
        for (;; p += 1)
        {
            digit = *p - '0';
            if (digit > ('z' - '0'))
            {
                break;
            }
            digit = _cvt_in[digit];
            if (digit > 15)
            {
                break;
            }
            result = (result << 4) + digit;
            any_digits = 1;
        }
    }
    else
    {
        for (;; p += 1)
        {
            digit = *p - '0';
            if (digit > ('z' - '0'))
            {
                break;
            }
            digit = _cvt_in[digit];
            if (digit >= base)
            {
                break;
            }
            result = result * base + digit;
            any_digits = 1;
        }
    }

    if (!any_digits)
        p = string;
    if (end_ptr != 0)
        *end_ptr = p;
    return result;
}

long strtol(char *nptr, char **endptr, int base)
{
    long result;
    char *p = nptr;
    while (isspace(*p))
    {
        p++;
    }
    if (*p == '-')
    {
        p++;
        result = -strtoul(p, endptr, base);
    }
    else
    {
        if (*p == '+')
            p++;
        result = strtoul(p, endptr, base);
    }
    if (endptr != 0 && *endptr == p)
    {
        *endptr = nptr;
    }
    return result;
}

#endif /* LEAN_AND_MEAN */

void *malloc(size_t size)
{
    return _alloc((uint16_t)&_heap,size);
}

void free(void *p)
{
    _dealloc((uint16_t)&_heap,p);
}

void *calloc(size_t num, size_t size)
{
    size_t bytes = num * size;
    void *result = malloc(bytes);
    if (result!=NULL)
        memset(result, 0, bytes);
    return result;
}

int splitpath(
    const char *path,
    char *drive,
    int *user,
    char *fname,
    char *ext)
{
    /* set all defaults */
    *drive = 'A' + bdos(DRV_GET, 0); /* current drive */
    *user = bdos(F_USERNUM, 0xff);   /* current user */
    strcpy(fname, "        ");       /* 8 spaces */
    strcpy(ext, "   ");              /* 3 ext */

    /* is filename empty? */
    if (strlen(path) == 0)
        return -1;

    /* does it start with a digit?*/
    unsigned char i = 0;
    char c = toupper(path[i]) & 0x7f; /* 7 bit ascii */
    if (c >= 'A' && c <= 'P' && path[i + 1] == ':')
    { /* drive */
        *drive = c;
        i += 2;
        c = toupper(path[i]) & 0x7f; /* next char */
    }

    /* c contains first leter of filename, i is index */
    unsigned char len = 0;
    /* forbidden chars in cp/m 2 */
    while (c && len < MAX_FNAME && !ispunct(c))
    {
        fname[len] = c;
        len++;
        i++;
        c = toupper(path[i]) & 0x7f; /* next char */
    }
    fname[len] = 0;

    /* if the end then file has no extension*/
    if (c == 0)
        return 0;

    /* must be dot! */
    if (c != '.')
        return -1;
    else
    {
        i++;
        c = toupper(path[i]) & 0x7f; /* next char */
    }

    /* get the extension */
    len = 0;
    while (c && len < MAX_EXT && !ispunct(c))
    {
        ext[len] = c;
        len++;
        i++;
        c = toupper(path[i]) & 0x7f; /* next char */
    }
    ext[len] = 0;

    /* if the end then we have drive, file and extension */
    if (c == 0)
        return 0;

    /* if not, then check for user area */
    if (c != '[')
        return -1;
    else
    {
        i++;
        c = toupper(path[i]) & 0x7f; /* next char */
    }

    /* optional G */
    if (c == 'G')
    {
        i++;
        c = toupper(path[i]) & 0x7f; /* next char */
    }

    /* prepare user */
    len = 0;
    char u[3];
    /* must be digit */
    if (!isdigit(c))
        return -1;
    else
    {
        u[len] = c;
        len++;
        i++;
        c = toupper(path[i]) & 0x7f; /* next char */
    }

    /* can be the second digit */
    if (isdigit(c))
    {
        u[len] = c;
        len++;
        i++;
        c = toupper(path[i]) & 0x7f; /* next char */
    }

    u[len] = 0;
    *user = atoi(u);
    /* user area must be in range */
    if (*user < 0 || *user > 15)
        return -1;

    /* must conclude with ] */
    if (c != ']')
        return -1;

    /* and that's it. ignore the rest! */
    return 0;
}

#define MAX_EL_LEN 8
void _swap(void *v1, void *v2, size_t size)
{
    char temp[MAX_EL_LEN];
    memcpy(temp, v1, size);
    memcpy(v1, v2, size);
    memcpy(v2, temp, size);
}

void qsort(void *base, size_t nitems, size_t size, int (*compar)(const void *, const void *))
{
    static unsigned gap, byte_gap, i;
    static char *p;
    for (gap = nitems >> 1; gap > 0; gap >>= 1)
    {
        byte_gap = gap * size;
        for (i = gap; i < nitems; ++i)
            for (p = base + i * size - byte_gap; p >= base; p -= byte_gap)
            {
                if ((*compar)(p, p + byte_gap) <= 0)
                    break;
                _swap(p, p + byte_gap, size);
            }
    }
}