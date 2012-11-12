/* vi: set sw=4 ts=4:
 *
 * wc.c - Word count
 *
 * Copyright 2011 Rob Landley <rob@landley.net>
 *
 * See http://opengroup.org/onlinepubs/9699919799/utilities/wc.html

USE_WC(NEWTOY(wc, "mcwl", TOYFLAG_USR|TOYFLAG_BIN))

config WC
	bool "wc"
	default y
	help
	  usage: wc -lwcm [FILE...]

	  Count lines, words, and characters in input.

	  -l	show lines
	  -w	show words
	  -c	show bytes
	  -m	show characters

	  By default outputs lines, words, bytes, and filename for each
	  argument (or from stdin if none). Displays only either bytes
	  or characters.
*/

#define FOR_wc
#include "toys.h"

GLOBALS(
	unsigned long totals[3];
)

static void show_lengths(unsigned long *lengths, char *name)
{
	int i, nospace = 1;
	for (i=0; i<3; i++) {
		if (!toys.optflags || (toys.optflags&(1<<i))) {
			xprintf(" %ld"+nospace, lengths[i]);
			nospace = 0;
		}
		TT.totals[i] += lengths[i];
	}
	if (*toys.optargs) xprintf(" %s", name);
	xputc('\n');
}

static void do_wc(int fd, char *name)
{
	int i, len, clen=1, space;
	wchar_t wchar;
	unsigned long word=0, lengths[]={0,0,0};

	for (;;) {
		len = read(fd, toybuf, sizeof(toybuf));
		if (len<0) {
			perror_msg("%s",name);
			toys.exitval = EXIT_FAILURE;
		}
		if (len<1) break;
		for (i=0; i<len; i+=clen) {
			if(toys.optflags&8) {
				clen = mbrtowc(&wchar, toybuf+i, len-i, 0);
				if(clen==(size_t)(-1)) {
					if(i!=len-1) {
						clen = 1;
						continue;
					}
					else break;
				}
				if(clen==(size_t)(-2)) break;
				if(clen==0) clen=1;
				space = iswspace(wchar);
			}
			else space = isspace(toybuf[i]);

			if (toybuf[i]==10) lengths[0]++;
			if (space) word=0;
			else {
				if (!word) lengths[1]++;
				word=1;
			}
			lengths[2]++;
		}
	}

	show_lengths(lengths, name);
}

void wc_main(void)
{
	setlocale(LC_ALL, "");
	toys.optflags |= (toys.optflags&8)>>1;
	loopfiles(toys.optargs, do_wc);
	if (toys.optc>1) show_lengths(TT.totals, "total");
}