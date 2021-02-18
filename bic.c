 /* See LICENSE file for license details. */
#include <sys/select.h>

#include <ctype.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <stdbool.h>

#include "arg.h"
#include "config.h"

char *argv0;
static char *host = DEFAULT_HOST;
static char *port = DEFAULT_PORT;
static char *password;
static char nick[32];
static char bufin[4096];
static char bufout[4096];
static char channel[256];
static time_t trespond;
static FILE *srv;

#undef strlcpy
#include "strlcpy.c"
#include "util.c"

#define DIGIT case '0':case '1':case '2':case '3':case '4':case '5':case '6':case '7':case '8':case '9':
#define TWO(x, y) (COLORS[(((x - '0') * 10) + (y - '0'))])

static void
pout(char *channel, char *fmt, ...) {
	static char timestr[80];
	time_t t;
	va_list ap;

	va_start(ap, fmt);
	vsnprintf(bufout, sizeof bufout, fmt, ap);
	va_end(ap);
	t = time(NULL);
	strftime(timestr, sizeof timestr, TIMESTAMP_FORMAT, localtime(&t));
	fprintf(stdout, "%s%-12s: %s%s \x1b[0m", CHANCOLOR, channel, TIMECOLOR, timestr);

    bool bold = false, italic = false, underline = false, strikethrough = false;
    unsigned char fg = 0, bg = 0;
    unsigned int i = 0;

    while (bufout[i]) {
        if (fg) {
            fprintf(stdout, "\x1b[38;5;%um", (unsigned char) fg);
        }
        if (bg) {
            fprintf(stdout, "\x1b[48;5;%um", (unsigned char) bg);
        }
        char c = bufout[i++];
        switch (c) {
        case 0x02: 
            {
                if (bold = !bold) {
                    fprintf(stdout, "\x1b[1m");
                } else {
                    fprintf(stdout, "\x1b[0m");
                }
                break;
            }
        case 0x1d: 
            {
                if (italic = !italic) {
                    fprintf(stdout, "\x1b[3m");
                } else {
                    fprintf(stdout, "\x1b[0m");
                }
                break;
            }
        case 0x1f: 
            {
                if (underline = !underline) {
                    fprintf(stdout, "\x1b[4m");
                } else {
                    fprintf(stdout, "\x1b[0m");
                }
                break;
            }
        case 0x1e: 
            {
                if (strikethrough = !strikethrough) {
                    fprintf(stdout, "\x1b[9m");
                } else {
                    fprintf(stdout, "\x1b[0m");
                }
                break;
            }
        case 0x03:
            {
                char a = bufout[i + 0], b = bufout[i + 1], c = bufout[i + 2], d = bufout[i + 3], e = bufout[i + 4];
                switch (a) {
                DIGIT
                    {
                        switch (b) {
                            DIGIT
                                {
                                    fg = TWO(a, b);
                                    i += 2;
                                    switch (c) {
                                    case ',':
                                        {
                                            switch (e) {
                                            DIGIT
                                                {
                                                    bg = TWO(d, e);
                                                    i += 3;
                                                    break;
                                                }
                                            default:
                                                {
                                                    bg = COLORS[d - '0'];
                                                    i += 2;
                                                    break;
                                                }
                                            }
                                            break;
                                        }
                                    default: break;
                                    }
                                    break;
                                }
                            case ',':
                                {
                                    fg = COLORS[a - '0'];
                                    i += 2;
                                    switch (d) {
                                    DIGIT
                                        {
                                            bg = TWO(c, d);
                                            i += 2;
                                            break;
                                        }
                                    default:
                                        {
                                            bg = COLORS[c - '0'];
                                            ++i;
                                        }
                                    }
                                    break;
                                }
                            default:
                                {
                                    fg = COLORS[a - '0'];
                                    ++i;
                                }
                        }
                        break;
                    }
                case ',':
                    {
                        fg = DEFAULT_FG;
                        bg = DEFAULT_BG;
                        putchar(',');
                        ++i;
                        break;
                    } 
                default:
                    {
                        fg = DEFAULT_FG;
                        bg = DEFAULT_BG;
                    }
                }
                break;
            }
        default:
            putchar(c);
        }
    }
    puts("\x1b[0m");
}

static void
sout(char *fmt, ...) {
	va_list ap;

	va_start(ap, fmt);
	vsnprintf(bufout, sizeof bufout, fmt, ap);
	va_end(ap);
	fprintf(srv, "%s\r\n", bufout);
}

static void
privmsg(char *channel, char *msg) {
	if(channel[0] == '\0') {
		pout("", "No channel to send to");
		return;
	}
    fprintf(stdout, "\x1b[1A");
	pout(channel, "<%s> %s", nick, msg);
	sout("PRIVMSG %s :%s", channel, msg);
}

static void
parsein(char *s) {
	char c, *p;
    unsigned int i;

	if(s[0] == '\0')
		return;
	skip(s, '\n');
	if(s[0] != COMMAND_PREFIX_CHARACTER) {
        for (i = 0, c = s[i]; c; c = s[++i]) {
            switch (s[i]) {
            case COLOR_ALT_CHAR:
                {
                    s[i] = 3;
                    break;
                }
            case BOLD_ALT_CHAR:
                {
                    s[i] = 2;
                    break;
                }
            case ITALICS_ALT_CHAR:
                {
                    s[i] = 0x1d;
                    break;
                }
            case UNDERLINE_ALT_CHAR:
                {
                    s[i] = 0x1f;
                    break;
                }
            case STRIKETHROUGH_ALT_CHAR:
                {
                    s[i] = 0x1e;
                    break;
                }
            case ESCAPE_CHAR:
                {
                    s[i] = ' ';
                    ++i;
                }
            default: break;
            }
        }
		privmsg(channel, s);
		return;
	}
	c = *++s;
	if(c != '\0' && isspace(s[1])) {
		p = s + 2;
		switch(c) {
        case 'q':
            sout("QUIT");
            exit(1);
		case 'j':
			sout("JOIN %s", p);
			if(channel[0] == '\0')
				strlcpy(channel, p, sizeof channel);
			return;
		case 'l':
			s = eat(p, isspace, 1);
			p = eat(s, isspace, 0);
			if(!*s)
				s = channel;
			if(*p)
				*p++ = '\0';
			if(!*p)
				p = DEFAULT_PARTING_MESSAGE;
			sout("PART %s :%s", s, p);
			return;
		case 'm':
			s = eat(p, isspace, 1);
			p = eat(s, isspace, 0);
			if(*p)
				*p++ = '\0';
			privmsg(s, p);
			return;
		case 's':
			strlcpy(channel, p, sizeof channel);
			return;
		}
	}
	sout("%s", s);
}

static void
parsesrv(char *cmd) {
	char *usr, *par, *txt;

	usr = host;
	if(!cmd || !*cmd)
		return;
	if(cmd[0] == ':') {
		usr = cmd + 1;
		cmd = skip(usr, ' ');
		if(cmd[0] == '\0')
			return;
		skip(usr, '!');
	}
	skip(cmd, '\r');
	par = skip(cmd, ' ');
	txt = skip(par, ':');
	trim(par);
	if(!strcmp("PONG", cmd))
		return;
	if(!strcmp("PRIVMSG", cmd))
		pout(par, "<%s> %s", usr, txt);
	else if(!strcmp("PING", cmd))
		sout("PONG %s", txt);
	else {
		pout(usr, ">< %s (%s): %s", cmd, par, txt);
		if(!strcmp("NICK", cmd) && !strcmp(usr, nick))
			strlcpy(nick, txt, sizeof nick);
	}
}


static void
usage(void) {
	eprint("usage: bic [-h host] [-p port] [-n nick] [-k keyword] [-v]\n", argv0);
}

int
main(int argc, char *argv[]) {
	struct timeval tv;
	const char *user = getenv("USER");
	int n;
	fd_set rd;

	strlcpy(nick, user ? user : "unknown", sizeof nick);
	ARGBEGIN {
	case 'h':
		host = EARGF(usage());
		break;
	case 'p':
		port = EARGF(usage());
		break;
	case 'n':
		strlcpy(nick, EARGF(usage()), sizeof nick);
		break;
	case 'k':
		password = EARGF(usage());
		break;
	case 'v':
		eprint("sib-"VERSION", © 2021 sugarfi\n");
		break;
	default:
		usage();
	} ARGEND;

	/* init */
	srv = fdopen(dial(host, port), "r+");
	if (!srv)
		eprint("fdopen:");
	/* login */
	if(password)
		sout("PASS %s", password);
	sout("NICK %s", nick);
	sout("USER %s localhost %s :%s", nick, host, nick);
	fflush(srv);
	setbuf(stdout, NULL);
	setbuf(srv, NULL);
	setbuf(stdin, NULL);
#ifdef __OpenBSD__
	if (pledge("stdio", NULL) == -1)
		eprint("error: pledge:");
#endif
	for(;;) { /* main loop */
		FD_ZERO(&rd);
		FD_SET(0, &rd);
		FD_SET(fileno(srv), &rd);
		tv.tv_sec = 120;
		tv.tv_usec = 0;
		n = select(fileno(srv) + 1, &rd, 0, 0, &tv);
		if(n < 0) {
			if(errno == EINTR)
				continue;
			eprint("bic: error on select():");
		}
		else if(n == 0) {
			if(time(NULL) - trespond >= 300)
				eprint("bic shutting down: parse timeout\n");
			sout("PING %s", host);
			continue;
		}
		if(FD_ISSET(fileno(srv), &rd)) {
			if(fgets(bufin, sizeof bufin, srv) == NULL)
				eprint("bic: remote host closed connection\n");
			parsesrv(bufin);
			trespond = time(NULL);
		}
		if(FD_ISSET(0, &rd)) {
			if(fgets(bufin, sizeof bufin, stdin) == NULL)
				eprint("bic: broken pipe\n");
			parsein(bufin);
		}
	}
	return 0;
}
