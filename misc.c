/*  
 * misc.c - misc functions for VladBot
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * Misc. functions that don't really fit anywhere
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>
#include <sys/types.h>
// #include <sys/time.h>
#include <pwd.h>
#include <unistd.h>
#include <time.h>

#include "config.h"
#include "misc.h"

static	char	usernick[MAXLEN];
static	char	timebuf[MAXLEN];
static 	char	idlestr[MAXLEN];
static	char	path_buf[MAXLEN];

static	char	*months[] =
{
	"Jan", "Feb", "Mar", "Apr", "May", "Jun", 
	"Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};

int 	ischannel(char *channel)
{
    	return(*channel == '#' || *channel == '&');
}

#if defined(__APPLE__) && defined(__MACH__)

/*      $OpenBSD: memrchr.c,v 1.2 2007/11/27 16:22:12 martynas Exp $    */

/*
 * Copyright (c) 2007 Todd C. Miller <Todd.Miller@courtesan.com>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * $FreeBSD$
 */

/*
 * Reverse memchr()
 * Find the last occurrence of 'c' in the buffer 's' of size 'n'.
 */
void *
memrchr(const void *s, int c, size_t n)
{
        const unsigned char *cp;

        if (n != 0) {
                cp = (unsigned char *)s + n;
                do {
                        if (*(--cp) == (unsigned char)c)
                                return((void *)cp);
                } while (--n != 0);
        }
        return(NULL);
}

#endif


int	isnick(char *nick)
{
	int	i;

	for(i=0; nick[i]; i++)
		if((i==0 && !isalpha(nick[i])) || !isalnum(nick[i]))
			if(!strrchr("|_-[]\\{}^", nick[i]))
				return FALSE;
	return TRUE;
}

char	*getnick(char *nick_userhost)
{
	strcpy(usernick, nick_userhost);
	return(strtok( usernick, "!" ));
}

char	*time2str(long time)
{
	struct tm	*btime;

	btime = localtime(&time);
	if(time && (sprintf(timebuf, "%-2.2d:%-2.2d:%-2.2d %s %-2.2d %d", 
		      btime->tm_hour, btime->tm_min, btime->tm_sec, 
		      months[btime->tm_mon], btime->tm_mday, 
		      btime->tm_year + 1900)))
		return timebuf;
	return(NULL);
} 

char	*time2small(long time)
{
	struct	tm	*btime;

	btime = localtime( &time );
	if(time && (sprintf(timebuf, "%s %-2.2d", 
		      months[btime->tm_mon], btime->tm_mday)))
		return timebuf;
	return(NULL);
}

char	*idle2str(long time)
{
	int	days,
		hours,
		mins,
		secs;
	
	days  = (time/86400);
	hours = ((time - (days*86400))/3600);
	mins  = ((time - (days*86400) - (hours*3600))/60);
	secs  = (time - (days*86400) - (hours*3600) - (mins*60));

	sprintf(idlestr, "%d day%s, %d hour%s, %d minute%s and %d second%s",
		days, EXTRA_CHAR(days), hours, EXTRA_CHAR(hours),
		mins, EXTRA_CHAR(mins), secs, EXTRA_CHAR(secs)); 
	return idlestr;
}

char	*mstrcpy(char **dest, char *src)
{
	if(src)
	{
		*dest = (char *) malloc(strlen(src)+1);
		strcpy(*dest, src);
	}
	else
		*dest = NULL;
	return *dest;
}

char	*mstrncpy(char **dest, char *src, int len)
{
	if(src)
	{
		*dest = (char *) malloc(len+1);
		strncpy(*dest, src, len);
	}
	else
		*dest = NULL;
	return *dest;
}

char	*get_token(char **src, char *token_sep)
/*
 * Just a little more convenient than strtok()
 * This function returns a pointer to the first token
 * in src, and makes src point to the "new string".
 *
 */
{
	char	*tok;

	if(!(src && *src && **src))
		return NULL;

	/* first, removes leading token_sep's */
	while(**src && strchr(token_sep, **src))
		(*src)++;
	
	/* first non token_sep */
	if(**src)
		tok = *src;
	else
		return NULL;

	/* Make *src point after token */
	*src = strpbrk(*src, token_sep);
	if(*src)
	{
		**src = '\0';
		(*src)++;
		while(**src && strchr(token_sep, **src))
			(*src)++;
	}
	else
		*src = "";
	return tok;
}
	
char	*get_string(char **src)
/* 
 * Put's a \0 after the first non-escaped " in the string,
 * returns NULL on error (no closing ") and translates escapes.
 */
{
	char	*s;
	int	p=0;
	
	if(!(src && *src && **src && **src == '"'))
		return NULL;

	s = ++(*src);
	while(**src)
	{
		if(**src == '"')
		{
			s[p] = '\0';
			(*src)++;
			return s;
		}
		if(**src == '\\')
		{
			(*src)++;
			if(!**src)
				return NULL;
			switch(**src)
			{
			case 'b': 	
				s[p++] = '\002';
				break;
			case 'u':	
				s[p++] = ''; /* '\031'; */
				break;
			case 'i':	
				s[p++] = ''; /* '\022'; */
				break;
			case '"':	
				s[p++] = '"';
				break;
			case '\\':
				s[p++] = '\\';
				break;
			default:
				s[p++] = **src;
				break;
			}
		}
		else
			s[p++] = **src;
		(*src)++;
	}
	return NULL;				
}

char	*expand_twiddle(char *s)
/*
 * Expands:
 * ~username	into username's homedir
 * ~/		into the owners homedir.
 * Trailing path will be added
 */
{
	struct 	passwd	*entry;

	if(!s)
		return NULL;
	
	if(*s == '~')
	{
		s++;
		if(*s == '/')
		{
			/* my homedir */
			s++;
			entry = getpwuid(getuid());
		}
		else
		{
			char	*user;

			if((user = get_token(&s, "/")))
				entry = getpwnam(user);
			else
				entry = NULL;
		}
		if(!entry)
			return NULL;
		sprintf(path_buf, "%s/%s", entry->pw_dir, s);		
		return path_buf;
	}
	else
		return s;
}

/* 
char 	*strcasestr(const char *s1, const char *s2)
{
    char n1[256], n2[256];
    int i;

    for ( i = 0; s1[i] != '\0'; i++ )
        n1[i] = toupper( s1[i] );
    n1[i] = '\0';

    for ( i = 0; s2[i] != '\0'; i++ )
        n2[i] = toupper( s2[i] );
    n2[i] = '\0';
    return( strstr( n1,n2 ) );
}
*/
