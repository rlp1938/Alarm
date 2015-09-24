
/*      alarm.c
 *
 *	Copyright 2015 Bob Parker <rlp1938@gmail.com>
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with this program; if not, write to the Free Software
 *	Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 *	MA 02110-1301, USA.
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <string.h>
#include <libgen.h>
#include <ctype.h>
#include <time.h>
#include "fileops.h"
#include "firstrun.h"

char *getactdata(char *start, char *end, char *prm);
void failure (const char *emsg);
int countchars(char *str, int chr);
void runit(char *prog, char *param);
unsigned int addclocktime(time_t start, int hours, int minutes,
													int seconds);

char *helpmsg = "NAME\n\talarm - a program to notify when a user "
" specified time has been reached\n"
"\nSYNOPSIS\n\talarm [option] hh:mm:ss [A|a|P|p]\n"
"\talarm [option] mm:ss [A|a|P|p]\n"
"\talarm [option] hh:mm: [A|a|P|p]\n"
"\talarm [option] mm [A|a|P|p]\n"
"\talarm [option] :ss [A|a|P|p]\n"
"\nDESCRIPTION\n\tThe program notifies the user when the input time is"
" reached. \n"
"\tBy default the input time is added to the start time, in other words\n\t"
"cooking timer mode, but optionally an actual clock"
 " time may be input,\n\tie alarm clock mode.\n"
"See OPTIONS below.\n"
"\nOPTIONS\n\t-h - prints helpfile.\n"
"\t-a - Interprets the input time as clock time, not an increment\n\tfrom program start. "
"You should use the hh:mm:ss or hh:mm: time\n\tspecifiers when using this option.\n"
"NOTES"
"\n\tTimes may be specified in 24 hour or AMPM format, case insensitive.\n\tIf PM is input"
" then 12 hours will be added to the hours field if it's\n\tless than 12 but not otherwise."
" Apart from that no range checking will\n\tbe done. Consequently a 90 second interval"
" may be input as :90 or 1:30.\n"
"\tIn alarm mode the time specified will be the next occurrence of that\n\ttime so if you"
" input 6:00 at 9 am then the alarm will be scheduled for\n\t6 am tomorrow."
"\n\tThe actual alarm notification is done by running a movie player to\n\tdisplay "
"a short musical clip. Such clip together with the player is\n\tnamed in the file:\n"
"\t.config/alarm/alarm.txt or if that does not exist,\n\t/usr./local/etc/alarm/alarm.txt\n"
;
void dohelp(int forced);


char *memabslimit;	// set by readfile, it's memmargin more than the data


int main(int argc, char **argv)
{
	int opt, numcolon;
	int totsec, seconds, minutes, hours;
	fdata sfd;
	time_t now;
	char *user, *start, *end, *prog2run, *mov2show, *timespec;
	char userbuf[FILENAME_MAX];
	char *fmt = "/home/%s/.config/alarm/alarm.txt";
	int cookingmode;
	int ampm = 0;
	// defaults
	cookingmode = 1;
	while((opt = getopt(argc, argv, ":ha")) != -1) {
		switch(opt) {
		case 'h':
			dohelp(0);
		break;
		case 'a':
			// absolute time of day ie alarm clock mode;
			cookingmode = 0;
		break;
		case ':':
			fprintf(stderr, "Option %c requires an argument\n",optopt);
			dohelp(1);
		break;
		case '?':
			fprintf(stderr, "Unknown option: %c\n",optopt);
			dohelp(1);
		break;
		} //switch()
	}//while()
	// now process the non-option arguments

	now = time(NULL);

	// 1.Check that argv[???] exists.
	if (!(argv[optind])) {
		fprintf(stderr, "No time specifier provided.\n");
		dohelp(1);
	}
	timespec = argv[optind];
	optind++;
	// go look for am or pm
	if (argv[optind]) {
		char *cp = argv[optind];
		if (*cp == 'P' || *cp == 'p') ampm = 1;
		// and it just doesn't matter if there is a following M|m or not
	}
	// Read my config file
	user = getenv("USER");
	sprintf(userbuf,  fmt, user);
	sfd = readfile(userbuf, 0, 0);
	if(sfd.from) {
		start = sfd.from;
		end = sfd.to;
	} else {
		firstrun("alarm", "alarm.txt");
		user = getenv("HOME");
		char advice[NAME_MAX];
		sprintf(advice, "Installed alarm.txt at %s.config/alarm/\n",
				user);
		fputs(advice, stderr);
		fputs("Please edit that file to suit your needs.\n", stderr);
		exit(EXIT_SUCCESS);
	}

	prog2run = getactdata(start, end, "program");
	mov2show = getactdata(start, end, "data");

	// now deal with the users timespec
	numcolon = countchars(timespec, ':');
	hours = minutes = seconds = 0;
	char *wstr = strdup(timespec);
	switch(numcolon) {
		char *cp;
		case 0:
		// the number is minutes;
		if (strlen(wstr)) minutes = strtol(wstr, NULL, 10);
		break;
		case 1:
		// have mm:ss or :ss or mm:
		cp = strchr(wstr, ':');
		*cp = '\0';
		if (strlen(wstr)) minutes = strtol(wstr, NULL, 10);
		cp++; // look at the second half
		wstr = cp;
		if (strlen(wstr)) seconds = strtol(wstr, NULL, 10);
		break;
		case 2:
		// have hh:mm:ss or hh:mm:
		cp = strchr(wstr, ':');
		*cp = '\0';
		if (strlen(wstr)) hours = strtol(wstr, NULL, 10);
		cp++; // next part
		wstr = cp;
		cp = strchr(wstr, ':');
		*cp = '\0';
		if (strlen(wstr)) minutes = strtol(wstr, NULL, 10);
		cp++;
		wstr = cp;
		if (strlen(wstr)) seconds = strtol(wstr, NULL, 10);
		break;
		default:
		fprintf(stderr,
				"Badly formed time specification: %s\n", timespec);
		exit(EXIT_FAILURE);
		break;
	}
	free(wstr);
	// total seconds to elapse before alarm
	if (ampm) {
		if (!(cookingmode)) {
			if (hours < 12) hours += 12;
		}
	}
	if(!(cookingmode)) { // used as an alarm clock, not cooking timer
		totsec = addclocktime(now, hours, minutes, seconds);
	} else {
		totsec = seconds + 60 * minutes + 3600 * hours;
	}
	printf("Total seconds: %d\n", totsec);
	sleep(totsec);
	runit(prog2run, mov2show);
	return 0;
}//main()

void dohelp(int forced)
{
  fputs(helpmsg, stderr);
  exit(forced);
}

void failure(const char *emsg) {
	/*
	 * The only exit point in the event of something invalid in command
	*/

	fprintf(stderr, "%s\n", emsg);

	exit(EXIT_FAILURE);
} // failure()

char *getactdata(char *start, char *end, char *prm)
{
	//  look for prm between start and end if found return the string
	// after '='
	// else return NULL
	static char *result;
	char *cp;
	result = memmem(start, end - start, prm, strlen(prm));
	if(!(result)) return NULL;
	cp = memchr(result, '=', end - result);
	if (!(cp)) {
		fprintf(stderr, "Badly formed parameter line for %s\n", prm);
		exit(EXIT_FAILURE);
	}
	result = cp + 1;	// point past '='
	cp = memchr(result, '\n', end - result);
	if (!(cp)) {
		fprintf(stderr, "No line end for parameter: %s\n", prm);
		exit(EXIT_FAILURE);
	}
	*cp = '\0';
	return result;
}

int countchars(char *str, int chr)
{
	// counts the occurences of chr in str
	char *cp;
	int res = 0;
	cp = str;
	while ((cp = strchr(cp, chr))) {
		res++;
		cp++;
	}
	return res;
}

void runit(char *prog, char *param)
{
	char buf[FILENAME_MAX];
	strcpy(buf, prog);
	strcat(buf, " ");
	strcat(buf, param);
	dosystem("./alarm.sh");
}

unsigned int addclocktime(time_t start, int hours, int minutes,
													int seconds)
{
	// add on the the hours etc to start
	time_t alarmtime;
	struct tm *st, *end;
	st = localtime(&start);
	end = localtime(&start);	// initialise it the lazy way
	if (hours < st->tm_hour) {	// it's tomorrow
		end->tm_hour = hours + 24;	// I think I can get away with this
	} else if (hours == st->tm_hour && minutes < st->tm_min) {
		end->tm_hour = hours + 24;
	}	else { // will ignore the pathological case of hours and minutes == but seconds less
		end->tm_hour = hours;
	}
	end->tm_sec = seconds;
	end->tm_min = minutes;
	alarmtime = mktime(end);
	return (alarmtime - start);
} // addclocktime()
