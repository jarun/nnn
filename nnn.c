/* See LICENSE file for copyright and license details. */
#include <sys/stat.h>
#if defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__) \
 || defined(__APPLE__)
# include <sys/types.h>
#else
# include <sys/sysmacros.h>
#endif
#include <sys/wait.h>
#include <sys/statvfs.h>
#include <sys/resource.h>

#include <ctype.h>
#include <curses.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <grp.h>
#include <libgen.h>
#include <limits.h>
#ifdef __gnu_hurd__
#define PATH_MAX 4096
#endif
#include <locale.h>
#include <pwd.h>
#include <regex.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <wchar.h>
#include <readline/readline.h>

#ifndef __USE_XOPEN_EXTENDED
#define __USE_XOPEN_EXTENDED 1
#endif
#include <ftw.h>

#ifdef DEBUG
static int
xprintf(int fd, const char *fmt, ...)
{
	char buf[BUFSIZ];
	int r;
	va_list ap;

	va_start(ap, fmt);
	r = vsnprintf(buf, sizeof(buf), fmt, ap);
	if (r > 0)
		r = write(fd, buf, r);
	va_end(ap);
	return r;
}

#define DEBUG_FD 8
#define DPRINTF_D(x) xprintf(DEBUG_FD, #x "=%d\n", x)
#define DPRINTF_U(x) xprintf(DEBUG_FD, #x "=%u\n", x)
#define DPRINTF_S(x) xprintf(DEBUG_FD, #x "=%s\n", x)
#define DPRINTF_P(x) xprintf(DEBUG_FD, #x "=0x%p\n", x)
#else
#define DPRINTF_D(x)
#define DPRINTF_U(x)
#define DPRINTF_S(x)
#define DPRINTF_P(x)
#endif /* DEBUG */

#define VERSION "v1.1"
#define LEN(x) (sizeof(x) / sizeof(*(x)))
#undef MIN
#define MIN(x, y) ((x) < (y) ? (x) : (y))
#define ISODD(x) ((x) & 1)
#define CONTROL(c) ((c) ^ 0x40)
#define TOUPPER(ch) \
	(((ch) >= 'a' && (ch) <= 'z') ? ((ch) - 'a' + 'A') : (ch))
#define MAX_CMD_LEN 5120
#define CURSYM(flag) (flag ? CURSR : EMPTY)
#define FILTER '/'

struct assoc {
	char *regex; /* Regex to match on filename */
	char *mime; /* File type */
};

/* Supported actions */
enum action {
	SEL_QUIT = 1,
	SEL_CDQUIT,
	SEL_BACK,
	SEL_GOIN,
	SEL_FLTR,
	SEL_MFLTR,
	SEL_SEARCH,
	SEL_NEXT,
	SEL_PREV,
	SEL_PGDN,
	SEL_PGUP,
	SEL_HOME,
	SEL_END,
	SEL_CD,
	SEL_CDHOME,
	SEL_CDBEGIN,
	SEL_CDLAST,
	SEL_TOGGLEDOT,
	SEL_DETAIL,
	SEL_STATS,
	SEL_MEDIA,
	SEL_FMEDIA,
	SEL_DFB,
	SEL_FSIZE,
	SEL_BSIZE,
	SEL_MTIME,
	SEL_REDRAW,
	SEL_COPY,
	SEL_HELP,
	SEL_RUN,
	SEL_RUNARG,
};

struct key {
	int sym;         /* Key pressed */
	enum action act; /* Action */
	char *run;       /* Program to run */
	char *env;       /* Environment variable to run */
};

#include "config.h"

typedef struct entry {
	char name[NAME_MAX];
	mode_t mode;
	time_t t;
	off_t size;
	off_t bsize;
} *pEntry;

typedef unsigned long ulong;

/* Externs */
#ifdef __APPLE__
extern int add_history(const char *);
#else
extern void add_history(const char *string);
#endif

extern int wget_wch(WINDOW *, wint_t *);

/* Global context */
static struct entry *dents;
static int ndents, cur, total_dents;
static int idle;
static char *opener;
static char *fb_opener;
static char *nlay="nlay";
static char *player;
static char *copier;
static char *desktop_manager;
static off_t blk_size;
static size_t fs_free;
static int open_max;
static const double div_2_pow_10 = 1.0 / 1024.0;

/* For use in functions which are isolated and don't return the buffer */
static char g_buf[MAX_CMD_LEN];

/*
 * Layout:
 * .---------
 * | cwd: /mnt/path
 * |
 * |    file0
 * |    file1
 * |  > file2
 * |    file3
 * |    file4
 *      ...
 * |    filen
 * |
 * | Permission denied
 * '------
 */

static void printmsg(char *);
static void printwarn(void);
static void printerr(int, char *);
static void redraw(char *path);

static rlim_t
max_openfds()
{
	struct rlimit rl;
        rlim_t limit;

	limit = getrlimit(RLIMIT_NOFILE, &rl);
	if (limit != 0)
		return 32;

	limit = rl.rlim_cur;
	rl.rlim_cur = rl.rlim_max;

	if (setrlimit(RLIMIT_NOFILE, &rl) == 0)
		return rl.rlim_max - 64;

	if (limit > 128)
		return limit - 64;

	return 32;
}

/* Just a safe strncpy(3) */
static void
xstrlcpy(char *dest, const char *src, size_t n)
{
	while (--n && (*dest++ = *src++));
	if (!n)
		*dest = '\0';
}

/*
 * The poor man's implementation of memrchr(3).
 * We are only looking for '/' in this program.
 */
static void *
xmemrchr(const void *s, unsigned char ch, size_t n)
{
	static unsigned char *p;

	if (!s || !n)
		return NULL;

	p = (unsigned char *)s + n - 1;

	while (n--)
		if ((*p--) == ch)
			return ++p;

	return NULL;
}

/*
 * The following dirname(3) implementation does not
 * modify the input. We use a copy of the original.
 *
 * Modified from the glibc (GNU LGPL) version.
 */
static char *
xdirname(const char *path)
{
	static char buf[PATH_MAX];
	static char *last_slash;

	xstrlcpy(buf, path, PATH_MAX);

	/* Find last '/'. */
	last_slash = strrchr(buf, '/');

	if (last_slash != NULL && last_slash != buf && last_slash[1] == '\0') {
		/* Determine whether all remaining characters are slashes. */
		char *runp;

		for (runp = last_slash; runp != buf; --runp)
			if (runp[-1] != '/')
				break;

		/* The '/' is the last character, we have to look further. */
		if (runp != buf)
			last_slash = xmemrchr(buf, '/', runp - buf);
	}

	if (last_slash != NULL) {
		/* Determine whether all remaining characters are slashes. */
		char *runp;

		for (runp = last_slash; runp != buf; --runp)
			if (runp[-1] != '/')
				break;

		/* Terminate the buffer. */
		if (runp == buf) {
			/* The last slash is the first character in the string.
			   We have to return "/". As a special case we have to
			   return "//" if there are exactly two slashes at the
			   beginning of the string. See XBD 4.10 Path Name
			   Resolution for more information. */
			if (last_slash == buf + 1)
				++last_slash;
			else
				last_slash = buf + 1;
		} else
			last_slash = runp;

		last_slash[0] = '\0';
	} else {
		/* This assignment is ill-designed but the XPG specs require to
		   return a string containing "." in any case no directory part
		   is found and so a static and constant string is required. */
		buf[0] = '.';
		buf[1] = '\0';
	}

	return buf;
}

/*
 * Return number of dots of all chars in a string are dots, else 0
 */
static int
all_dots(const char* ptr)
{
	if (!ptr)
		return FALSE;

	int count = 0;
	while (*ptr == '.') {
		count++;
		ptr++;
	}

	if (*ptr)
		return 0;

	return count;
}

/*
 * Spawns a child process. Behaviour can be controlled using flag:
 * Limited to 2 arguments to a program
 * flag works on bit set:
 *    - 0b1: draw a marker to indicate nnn spawned e.g., a shell
 *    - 0b10: do not wait in parent for child process e.g. DE file manager
 *    - 0b100: suppress stdout and stderr
 *    - 0b1000: restore default SIGINT handler
 */
static void
spawn(char *file, char *arg1, char *arg2, char *dir, unsigned char flag)
{
	pid_t pid;
	int status;

	pid = fork();
	if (pid == 0) {
		if (dir != NULL)
			status = chdir(dir);

		/* Show a marker (to indicate nnn spawned shell) */
		if (flag & 0b1)
			fprintf(stdout, "\n +-++-++-+\n | n n n |\n +-++-++-+\n\n");

		/* Suppress stdout and stderr */
		if (flag & 0b100) {
			int fd = open("/dev/null", O_WRONLY, S_IWUSR);
			dup2(fd, 1);
			dup2(fd, 2);
			close(fd);
		}

		if (flag & 0b1000)
			signal(SIGINT, SIG_DFL);
		execlp(file, file, arg1, arg2, NULL);
		_exit(1);
	} else {
		if (!(flag & 0b10))
			/* Ignore interruptions */
			while (waitpid(pid, &status, 0) == -1)
				DPRINTF_D(status);
		DPRINTF_D(pid);
	}
}

static char *
xgetenv(char *name, char *fallback)
{
	char *value;

	if (name == NULL)
		return fallback;
	value = getenv(name);
	return value && value[0] ? value : fallback;
}

/*
 * We assume none of the strings are NULL.
 *
 * Let's have the logic to sort numeric names in numeric order.
 * E.g., the order '1, 10, 2' doesn't make sense to human eyes.
 *
 * If the absolute numeric values are same, we fallback to alphasort.
 */
static int
xstricmp(char *s1, char *s2)
{
	static char *c1, *c2;

	c1 = s1;
	while (isspace(*c1))
		c1++;
	if (*c1 == '-' || *c1 == '+')
		c1++;
	while (*c1 >= '0' && *c1 <= '9')
		c1++;

	c2 = s2;
	while (isspace(*c2))
		c2++;
	if (*c2 == '-' || *c2 == '+')
		c2++;
	while(*c2 >= '0' && *c2 <= '9')
		c2++;

	if (*c1 == '\0' && *c2 == '\0') {
		static long long num1, num2;

		num1 = strtoll(s1, &c1, 10);
		num2 = strtoll(s2, &c2, 10);
		if (num1 != num2) {
			if (num1 > num2)
				return 1;
			else
				return -1;
		}
	} else if (*c1 == '\0' && *c2 != '\0')
		return -1;
	else if (*c1 != '\0' && *c2 == '\0')
		return 1;

	while (*s2 && *s1 && TOUPPER(*s1) == TOUPPER(*s2))
		s1++, s2++;

	/* In case of alphabetically same names, make sure
	   lower case one comes before upper case one */
	if (!*s1 && !*s2)
		return 1;

	return (int) (TOUPPER(*s1) - TOUPPER(*s2));
}

/* Trim all whitespace from both ends, / from end */
static char *
strstrip(char *s)
{
	if (!s || !*s)
		return s;

	size_t len = strlen(s) - 1;

	while (len != 0 && (isspace(s[len]) || s[len] == '/'))
		len--;
	s[len + 1] = '\0';

	while (*s && isspace(*s))
		s++;

	return s;
}

static char *
getmime(char *file)
{
	regex_t regex;
	unsigned int i;
	static unsigned int len = LEN(assocs);

	for (i = 0; i < len; i++) {
		if (regcomp(&regex, assocs[i].regex,
			    REG_NOSUB | REG_EXTENDED | REG_ICASE) != 0)
			continue;
		if (regexec(&regex, file, 0, NULL, 0) == 0)
			return assocs[i].mime;
	}
	return NULL;
}

static int
setfilter(regex_t *regex, char *filter)
{
	static size_t len;
	static int r;

	r = regcomp(regex, filter, REG_NOSUB | REG_EXTENDED | REG_ICASE);
	if (r != 0 && filter && filter[0] != '\0') {
		len = COLS;
		if (len > LINE_MAX)
			len = LINE_MAX;
		regerror(r, regex, g_buf, len);
		printmsg(g_buf);
	}
	return r;
}

static void
initfilter(int dot, char **ifilter)
{
	*ifilter = dot ? "." : "^[^.]";
}

static int
visible(regex_t *regex, char *file)
{
	return regexec(regex, file, 0, NULL, 0) == 0;
}

static int
entrycmp(const void *va, const void *vb)
{
	static pEntry pa, pb;

	pa = (pEntry)va;
	pb = (pEntry)vb;

	/* Sort directories first */
	if (S_ISDIR(pb->mode) && !S_ISDIR(pa->mode))
		return 1;
	else if (S_ISDIR(pa->mode) && !S_ISDIR(pb->mode))
		return -1;

	/* Do the actual sorting */
	if (mtimeorder)
		return pb->t - pa->t;

	if (sizeorder) {
		if (pb->size > pa->size)
			return 1;
		else if (pb->size < pa->size)
			return -1;
	}

	if (bsizeorder) {
		if (pb->bsize > pa->bsize)
			return 1;
		else if (pb->bsize < pa->bsize)
			return -1;
	}

	return xstricmp(pa->name, pb->name);
}

static void
initcurses(void)
{
	if (initscr() == NULL) {
		char *term = getenv("TERM");
		if (term != NULL)
			fprintf(stderr, "error opening terminal: %s\n", term);
		else
			fprintf(stderr, "failed to initialize curses\n");
		exit(1);
	}
	cbreak();
	noecho();
	nonl();
	intrflush(stdscr, FALSE);
	keypad(stdscr, TRUE);
	curs_set(FALSE); /* Hide cursor */
	start_color();
	use_default_colors();
	timeout(1000); /* One second */
}

static void
exitcurses(void)
{
	endwin(); /* Restore terminal */
}

/* Messages show up at the bottom */
static void
printmsg(char *msg)
{
	move(LINES - 1, 0);
	printw("%s\n", msg);
}

/* Display warning as a message */
static void
printwarn(void)
{
	printmsg(strerror(errno));
}

/* Kill curses and display error before exiting */
static void
printerr(int ret, char *prefix)
{
	exitcurses();
	fprintf(stderr, "%s: %s\n", prefix, strerror(errno));
	exit(ret);
}

/* Clear the last line */
static void
clearprompt(void)
{
	printmsg("");
}

/* Print prompt on the last line */
static void
printprompt(char *str)
{
	clearprompt();
	printw(str);
}

/* Returns SEL_* if key is bound and 0 otherwise.
 * Also modifies the run and env pointers (used on SEL_{RUN,RUNARG}).
 * The next keyboard input can be simulated by presel.
 */
static int
nextsel(char **run, char **env, int *presel)
{
	int c = *presel;
	unsigned int i;
	static unsigned int len = LEN(bindings);

	if (c == 0)
		c = getch();
	else
		*presel = 0;

	if (c == -1)
		idle++;
	else
		idle = 0;

	for (i = 0; i < len; i++)
		if (c == bindings[i].sym) {
			*run = bindings[i].run;
			*env = bindings[i].env;
			return bindings[i].act;
		}

	return 0;
}

/*
 * Move non-matching entries to the end
 */
static void
fill(struct entry **dents,
	 int (*filter)(regex_t *, char *), regex_t *re)
{
	static int count;

	for (count = 0; count < ndents; count++) {
		if (filter(re, (*dents)[count].name) == 0) {
			if (count != --ndents) {
				static struct entry _dent;

				/* Copy count to tmp */
				xstrlcpy(_dent.name, (*dents)[count].name, NAME_MAX);
				_dent.mode = (*dents)[count].mode;
				_dent.t = (*dents)[count].t;
				_dent.size = (*dents)[count].size;
				_dent.bsize = (*dents)[count].bsize;

				/* Copy ndents - 1 to count */
				xstrlcpy((*dents)[count].name, (*dents)[ndents].name, NAME_MAX);
				(*dents)[count].mode = (*dents)[ndents].mode;
				(*dents)[count].t = (*dents)[ndents].t;
				(*dents)[count].size = (*dents)[ndents].size;
				(*dents)[count].bsize = (*dents)[ndents].bsize;

				/* Copy tmp to ndents - 1 */
				xstrlcpy((*dents)[ndents].name, _dent.name, NAME_MAX);
				(*dents)[ndents].mode = _dent.mode;
				(*dents)[ndents].t = _dent.t;
				(*dents)[ndents].size = _dent.size;
				(*dents)[ndents].bsize = _dent.bsize;

				count--;
			}

			continue;
		}
	}
}

static int
matches(char *fltr)
{
	static regex_t re;

	/* Search filter */
	if (setfilter(&re, fltr) != 0)
		return -1;

	fill(&dents, visible, &re);
	qsort(dents, ndents, sizeof(*dents), entrycmp);

	return 0;
}

static int
readln(char *path)
{
	static char ln[LINE_MAX << 2];
	static wchar_t wln[LINE_MAX];
	static wint_t ch[2] = {0};
	int r, total = ndents;
	int oldcur = cur;
	int len = 1;
	char *pln = ln + 1;

	memset(wln, 0, LINE_MAX << 2);
	wln[0] = FILTER;
	ln[0] = FILTER;
	ln[1] = '\0';
	cur = 0;

	timeout(-1);
	echo();
	curs_set(TRUE);
	printprompt(ln);

	while ((r = wget_wch(stdscr, ch)) != ERR) {
		if (r == OK) {
			switch(*ch) {
			case '\r':  // with nonl(), this is ENTER key value
				if (len == 1) {
					cur = oldcur;
					*ch = CONTROL('L');
					goto end;
				}

				if (matches(pln) == -1)
					goto end;

				redraw(path);
				goto end;
			case 127: // handle DEL
				if (len == 1) {
					cur = oldcur;
					*ch = CONTROL('L');
					goto end;
				}

				if (len == 2)
					cur = oldcur;

				wln[--len] = '\0';
				wcstombs(ln, wln, LINE_MAX << 2);
				ndents = total;
				if (matches(pln) == -1) {
					printprompt(ln);
					continue;
				}
				redraw(path);
				printprompt(ln);
				break;
			case CONTROL('L'):
				cur = oldcur; // fallthrough
			case CONTROL('Q'):
				goto end;
			default:
				wln[len++] = (wchar_t)*ch;
				wln[len] = '\0';
				wcstombs(ln, wln, LINE_MAX << 2);
				ndents = total;
				if (matches(pln) == -1)
					continue;
				redraw(path);
				printprompt(ln);
			}
		} else {
			switch(*ch) {
			case KEY_DC: // fallthrough
			case KEY_BACKSPACE:
				if (len == 1) {
					cur = oldcur;
					*ch = CONTROL('L');
					goto end;
				}

				if (len == 2)
					cur = oldcur;

				wln[--len] = '\0';
				wcstombs(ln, wln, LINE_MAX << 2);
				ndents = total;
				if (matches(pln) == -1)
					continue;
				redraw(path);
				printprompt(ln);
				break;
			case KEY_DOWN: // fallthrough
			case KEY_UP: // fallthrough
			case KEY_LEFT: // fallthrough
			case KEY_RIGHT:
				if (len == 1)
					cur = oldcur; // fallthrough
			default:
				goto end;
			}
		}
	}
end:
	noecho();
	curs_set(FALSE);
	timeout(1000);

	/* Return keys for navigation etc. */
	return *ch;
}

static int
canopendir(char *path)
{
	static DIR *dirp;

	dirp = opendir(path);
	if (dirp == NULL)
		return 0;
	closedir(dirp);
	return 1;
}

/*
 * Returns "dir/name or "/name"
 */
static char *
mkpath(char *dir, char *name, char *out, size_t n)
{
	/* Handle absolute path */
	if (name[0] == '/')
		xstrlcpy(out, name, n);
	else {
		/* Handle root case */
		if (strcmp(dir, "/") == 0)
			snprintf(out, n, "/%s", name);
		else
			snprintf(out, n, "%s/%s", dir, name);
	}
	return out;
}

static char *
replace_escape(const char *str)
{
	static char buffer[PATH_MAX];
	static wchar_t wbuf[PATH_MAX];
	static wchar_t *buf;
	buffer[0] = '\0';
	buf = wbuf;

	/* Convert multi-byte to wide char */
	mbstowcs(wbuf, str, PATH_MAX);

	while (*buf) {
		if (*buf <= '\x1f' || *buf == '\x7f')
			*buf = '\?';

		buf++;
	}

	/* Convert wide char to multi-byte */
	wcstombs(buffer, wbuf, PATH_MAX);
	return buffer;
}

static void
printent(struct entry *ent, int active)
{
	static int ncols;

	if (COLS > PATH_MAX + 16)
		ncols = PATH_MAX + 16;
	else
		ncols = COLS;

	if (S_ISDIR(ent->mode))
		snprintf(g_buf, ncols, "%s%s/", CURSYM(active),
			 replace_escape(ent->name));
	else if (S_ISLNK(ent->mode))
		snprintf(g_buf, ncols, "%s%s@", CURSYM(active),
			 replace_escape(ent->name));
	else if (S_ISSOCK(ent->mode))
		snprintf(g_buf, ncols, "%s%s=", CURSYM(active),
			 replace_escape(ent->name));
	else if (S_ISFIFO(ent->mode))
		snprintf(g_buf, ncols, "%s%s|", CURSYM(active),
			 replace_escape(ent->name));
	else if (ent->mode & S_IXUSR)
		snprintf(g_buf, ncols, "%s%s*", CURSYM(active),
			 replace_escape(ent->name));
	else
		snprintf(g_buf, ncols, "%s%s", CURSYM(active),
			 replace_escape(ent->name));

	printw("%s\n", g_buf);
}

static void (*printptr)(struct entry *ent, int active) = &printent;

static char*
coolsize(off_t size)
{
	static const char *size_units[] = {"B", "K", "M", "G", "T", "P", "E", "Z", "Y"};
	static char size_buf[12]; /* Buffer to hold human readable size */
	static int i;
	static off_t tmp;
	static long double rem;

	i = 0;
	rem = 0;

	while (size > 1024) {
		tmp = size;
		size >>= 10;
		rem = tmp - (size << 10);
		i++;
	}

	snprintf(size_buf, 12, "%.*Lf%s", i, size + rem * div_2_pow_10, size_units[i]);
	return size_buf;
}

static void
printent_long(struct entry *ent, int active)
{
	static int ncols;
	static char buf[18];

	if (COLS > PATH_MAX + 32)
		ncols = PATH_MAX + 32;
	else
		ncols = COLS;

	strftime(buf, 18, "%d %m %Y %H:%M", localtime(&ent->t));

	if (active)
		attron(A_REVERSE);

	if (!bsizeorder) {
		if (S_ISDIR(ent->mode))
			snprintf(g_buf, ncols, "%s%-16.16s        /  %s/",
				 CURSYM(active), buf, replace_escape(ent->name));
		else if (S_ISLNK(ent->mode))
			snprintf(g_buf, ncols, "%s%-16.16s        @  %s@",
				 CURSYM(active), buf, replace_escape(ent->name));
		else if (S_ISSOCK(ent->mode))
			snprintf(g_buf, ncols, "%s%-16.16s        =  %s=",
				 CURSYM(active), buf, replace_escape(ent->name));
		else if (S_ISFIFO(ent->mode))
			snprintf(g_buf, ncols, "%s%-16.16s        |  %s|",
				 CURSYM(active), buf, replace_escape(ent->name));
		else if (S_ISBLK(ent->mode))
			snprintf(g_buf, ncols, "%s%-16.16s        b  %s",
				 CURSYM(active), buf, replace_escape(ent->name));
		else if (S_ISCHR(ent->mode))
			snprintf(g_buf, ncols, "%s%-16.16s        c  %s",
				 CURSYM(active), buf, replace_escape(ent->name));
		else if (ent->mode & S_IXUSR)
			snprintf(g_buf, ncols, "%s%-16.16s %8.8s* %s*",
				 CURSYM(active), buf, coolsize(ent->size),
				 replace_escape(ent->name));
		else
			snprintf(g_buf, ncols, "%s%-16.16s %8.8s  %s",
				 CURSYM(active), buf, coolsize(ent->size),
				 replace_escape(ent->name));
	} else {
		if (S_ISDIR(ent->mode))
			snprintf(g_buf, ncols, "%s%-16.16s %8.8s/ %s/",
				 CURSYM(active), buf, coolsize(ent->bsize << 9),
				 replace_escape(ent->name));
		else if (S_ISLNK(ent->mode))
			snprintf(g_buf, ncols, "%s%-16.16s        @  %s@",
				 CURSYM(active), buf,
				 replace_escape(ent->name));
		else if (S_ISSOCK(ent->mode))
			snprintf(g_buf, ncols, "%s%-16.16s        =  %s=",
				 CURSYM(active), buf,
				 replace_escape(ent->name));
		else if (S_ISFIFO(ent->mode))
			snprintf(g_buf, ncols, "%s%-16.16s        |  %s|",
				 CURSYM(active), buf,
				 replace_escape(ent->name));
		else if (S_ISBLK(ent->mode))
			snprintf(g_buf, ncols, "%s%-16.16s        b  %s",
				 CURSYM(active), buf,
				 replace_escape(ent->name));
		else if (S_ISCHR(ent->mode))
			snprintf(g_buf, ncols, "%s%-16.16s        c  %s",
				 CURSYM(active), buf,
				 replace_escape(ent->name));
		else if (ent->mode & S_IXUSR)
			snprintf(g_buf, ncols, "%s%-16.16s %8.8s* %s*",
				 CURSYM(active), buf, coolsize(ent->bsize << 9),
				 replace_escape(ent->name));
		else
			snprintf(g_buf, ncols, "%s%-16.16s %8.8s  %s",
				 CURSYM(active), buf, coolsize(ent->bsize << 9),
				 replace_escape(ent->name));
	}

	printw("%s\n", g_buf);

	if (active)
		attroff(A_REVERSE);
}

static char
get_fileind(mode_t mode, char *desc)
{
	static char c;

	if (S_ISREG(mode)) {
		c = '-';
		sprintf(desc, "%s", "regular file");
		if (mode & S_IXUSR)
			strcat(desc, ", executable");
	} else if (S_ISDIR(mode)) {
		c = 'd';
		sprintf(desc, "%s", "directory");
	} else if (S_ISBLK(mode)) {
		c = 'b';
		sprintf(desc, "%s", "block special device");
	} else if (S_ISCHR(mode)) {
		c = 'c';
		sprintf(desc, "%s", "character special device");
#ifdef S_ISFIFO
	} else if (S_ISFIFO(mode)) {
		c = 'p';
		sprintf(desc, "%s", "FIFO");
#endif  /* S_ISFIFO */
#ifdef S_ISLNK
	} else if (S_ISLNK(mode)) {
		c = 'l';
		sprintf(desc, "%s", "symbolic link");
#endif  /* S_ISLNK */
#ifdef S_ISSOCK
	} else if (S_ISSOCK(mode)) {
		c = 's';
		sprintf(desc, "%s", "socket");
#endif  /* S_ISSOCK */
#ifdef S_ISDOOR
    /* Solaris 2.6, etc. */
	} else if (S_ISDOOR(mode)) {
		c = 'D';
		desc[0] = '\0';
#endif  /* S_ISDOOR */
	} else {
		/* Unknown type -- possibly a regular file? */
		c = '?';
		desc[0] = '\0';
	}

	return(c);
}

/* Convert a mode field into "ls -l" type perms field. */
static char *
get_lsperms(mode_t mode, char *desc)
{
	static const char *rwx[] = {"---", "--x", "-w-", "-wx",
				    "r--", "r-x", "rw-", "rwx"};
	static char bits[11];

	bits[0] = get_fileind(mode, desc);
	strcpy(&bits[1], rwx[(mode >> 6) & 7]);
	strcpy(&bits[4], rwx[(mode >> 3) & 7]);
	strcpy(&bits[7], rwx[(mode & 7)]);

	if (mode & S_ISUID)
		bits[3] = (mode & S_IXUSR) ? 's' : 'S';
	if (mode & S_ISGID)
		bits[6] = (mode & S_IXGRP) ? 's' : 'l';
	if (mode & S_ISVTX)
		bits[9] = (mode & S_IXOTH) ? 't' : 'T';

	bits[10] = '\0';

	return(bits);
}

/*
 * Gets only a single line (that's what we need
 * for now) or shows full command output in pager.
 *
 * If pager is valid, returns NULL
 */
static char *
get_output(char *buf, size_t bytes, char *file, char *arg1, char *arg2, int pager)
{
	pid_t pid;
	int pipefd[2];
	FILE* pf;
	int status;
	char *ret = NULL;

	if (pipe(pipefd) == -1)
		printerr(1, "pipe(2)");

	pid = fork();
	if (pid == 0) {
		/* In child */
		close(pipefd[0]);
		dup2(pipefd[1], STDOUT_FILENO);
		dup2(pipefd[1], STDERR_FILENO);
		execlp(file, file, arg1, arg2, NULL);
		_exit(1);
	}

	/* In parent */
	waitpid(pid, &status, 0);
	close(pipefd[1]);
	if (!pager) {
		if ((pf = fdopen(pipefd[0], "r"))) {
			ret = fgets(buf, bytes, pf);
			close(pipefd[0]);
		}

		return ret;
	}

	pid = fork();
	if (pid == 0) {
		/* Show in pager in child */
		dup2(pipefd[0], STDIN_FILENO);
		execlp("less", "less", NULL);
		close(pipefd[0]);
		_exit(1);
	}

	/* In parent */
	waitpid(pid, &status, 0);

	return NULL;
}

/*
 * Follows the stat(1) output closely
 */
static int
show_stats(char* fpath, char* fname, struct stat *sb)
{
	char *perms = get_lsperms(sb->st_mode, g_buf);
	char *p, *begin = g_buf;

	char tmp[] = "/tmp/nnnXXXXXX";
	int fd = mkstemp(tmp);
	if (fd == -1)
		return -1;

	/* Show file name or 'symlink' -> 'target' */
	if (perms[0] == 'l') {
		/* Note that MAX_CMD_LEN > PATH_MAX */
		ssize_t len = readlink(fpath, g_buf, MAX_CMD_LEN);
		if (len != -1) {
			g_buf[len] = '\0';
			dprintf(fd, "    File: '%s' -> ",
				replace_escape(fname));
			dprintf(fd, "'%s'",
				replace_escape(g_buf));
		}
	} else
		dprintf(fd, "    File: '%s'", replace_escape(fname));

	/* Show size, blocks, file type */
#ifdef __APPLE__
	dprintf(fd, "\n    Size: %-15lld Blocks: %-10lld IO Block: %-6d %s",
#else
	dprintf(fd, "\n    Size: %-15ld Blocks: %-10ld IO Block: %-6ld %s",
#endif
	       sb->st_size, sb->st_blocks, sb->st_blksize, g_buf);

	/* Show containing device, inode, hardlink count */
#ifdef __APPLE__
	sprintf(g_buf, "%xh/%ud", sb->st_dev, sb->st_dev);
	dprintf(fd, "\n  Device: %-15s Inode: %-11llu Links: %-9hu",
#else
	sprintf(g_buf, "%lxh/%lud", sb->st_dev, sb->st_dev);
	dprintf(fd, "\n  Device: %-15s Inode: %-11lu Links: %-9lu",
#endif
	       g_buf, sb->st_ino, sb->st_nlink);

	/* Show major, minor number for block or char device */
	if (perms[0] == 'b' || perms[0] == 'c')
		dprintf(fd, " Device type: %x,%x",
		       major(sb->st_rdev), minor(sb->st_rdev));

	/* Show permissions, owner, group */
	dprintf(fd, "\n  Access: 0%d%d%d/%s Uid: (%u/%s)  Gid: (%u/%s)",
	       (sb->st_mode >> 6) & 7, (sb->st_mode >> 3) & 7, sb->st_mode & 7,
	       perms,
	       sb->st_uid, (getpwuid(sb->st_uid))->pw_name,
	       sb->st_gid, (getgrgid(sb->st_gid))->gr_name);

	/* Show last access time */
	strftime(g_buf, 40, "%a %d-%b-%Y %T %z,%Z", localtime(&sb->st_atime));
	dprintf(fd, "\n\n  Access: %s", g_buf);

	/* Show last modification time */
	strftime(g_buf, 40, "%a %d-%b-%Y %T %z,%Z", localtime(&sb->st_mtime));
	dprintf(fd, "\n  Modify: %s", g_buf);

	/* Show last status change time */
	strftime(g_buf, 40, "%a %d-%b-%Y %T %z,%Z", localtime(&sb->st_ctime));
	dprintf(fd, "\n  Change: %s", g_buf);

	if (S_ISREG(sb->st_mode)) {
		/* Show file(1) output */
		p = get_output(g_buf, MAX_CMD_LEN, "file", "-b", fpath, 0);
		if (p) {
			dprintf(fd, "\n\n ");
			while (*p) {
				if (*p == ',') {
					*p = '\0';
					dprintf(fd, " %s\n", begin);
					begin = p + 1;
				}

				p++;
			}
			dprintf(fd, " %s", begin);
		}

		dprintf(fd, "\n\n");
	} else
		dprintf(fd, "\n\n\n");

	close(fd);

	get_output(NULL, 0, "cat", tmp, NULL, 1);
	unlink(tmp);
	return 0;
}

static int
show_mediainfo(char* fpath, char *arg)
{
	if (get_output(g_buf, MAX_CMD_LEN, "which", "mediainfo", NULL, 0) == NULL)
		return -1;

	exitcurses();
	get_output(NULL, 0, "mediainfo", fpath, arg, 1);
	initcurses();

	return 0;
}

static int
show_help(void)
{
	static char helpstr[] = ("echo \"\
                  Key | Function\n\
                     -+-\n\
            Up, k, ^P | Previous entry\n\
          Down, j, ^N | Next entry\n\
             PgUp, ^U | Scroll half page up\n\
             PgDn, ^D | Scroll half page down\n\
       Home, g, ^, ^A | Jump to first entry\n\
        End, G, $, ^E | Jump to last entry\n\
  Right, Enter, l, ^M | Open file or enter dir\n\
    Left, Bksp, h, ^H | Go to parent dir\n\
               Insert | Toggle navigate-as-you-type mode\n\
                    ~ | Jump to HOME dir\n\
                    & | Jump to initial dir\n\
                    - | Jump to last visited dir\n\
                    / | Filter dir contents\n\
                   ^/ | Search dir in desktop search tool\n\
                    . | Toggle hide .dot files\n\
                    c | Show change dir prompt\n\
                    d | Toggle detail view\n\
                    D | Toggle current file details screen\n\
                    m | Show concise mediainfo\n\
                    M | Show full mediainfo\n\
                    s | Toggle sort by file size\n\
                    S | Toggle disk usage analyzer mode\n\
                    t | Toggle sort by modified time\n\
                    ! | Spawn SHELL in PWD (fallback sh)\n\
                    e | Edit entry in EDITOR (fallback vi)\n\
                    o | Open dir in NNN_DE_FILE_MANAGER\n\
                    p | Open entry in PAGER (fallback less)\n\
                   ^K | Invoke file name copier\n\
                   ^L | Force a redraw\n\
                    ? | Toggle help screen\n\
                    Q | Quit and change directory\n\
		q, ^Q | Quit\n\n\" | less");

	return system(helpstr);
}

static int
sum_bsizes(const char *fpath, const struct stat *sb, int typeflag, struct FTW *ftwbuf)
{
	if (typeflag == FTW_F || typeflag == FTW_D)
		blk_size += sb->st_blocks;

	return 0;
}

static int
getorder(size_t size)
{
	switch (size) {
	case 4096:
		return 12;
	case 512:
		return 9;
	case 8192:
		return 13;
	case 16384:
		return 14;
	case 32768:
		return 15;
	case 65536:
		return 16;
	case 131072:
		return 17;
	case 262144:
		return 18;
	case 524288:
		return 19;
	case 1048576:
		return 20;
	case 2048:
		return 11;
	case 1024:
		return 10;
	default:
		return 0;
	}
}

static int
dentfill(char *path, struct entry **dents,
	 int (*filter)(regex_t *, char *), regex_t *re)
{
	static char newpath[PATH_MAX];
	static DIR *dirp;
	static struct dirent *dp;
	static struct stat sb;
	static int n;
	n = 0;

	dirp = opendir(path);
	if (dirp == NULL)
		return 0;

	while ((dp = readdir(dirp)) != NULL) {
		/* Skip self and parent */
		if ((dp->d_name[0] == '.' && (dp->d_name[1] == '\0' ||
		    (dp->d_name[1] == '.' && dp->d_name[2] == '\0'))))
			continue;

		if (filter(re, dp->d_name) == 0)
			continue;

		if (n == total_dents) {
			total_dents += 64;
			*dents = realloc(*dents, total_dents * sizeof(**dents));
			if (*dents == NULL)
				printerr(1, "realloc");
		}

		xstrlcpy((*dents)[n].name, dp->d_name, NAME_MAX);
		/* Get mode flags */
		mkpath(path, dp->d_name, newpath, PATH_MAX);
		if (lstat(newpath, &sb) == -1) {
			if (*dents)
				free(*dents);
			printerr(1, "lstat");
		}
		(*dents)[n].mode = sb.st_mode;
		(*dents)[n].t = sb.st_mtime;
		(*dents)[n].size = sb.st_size;

		if (bsizeorder) {
			if (S_ISDIR(sb.st_mode)) {
				blk_size = 0;
				if (nftw(newpath, sum_bsizes, open_max, FTW_MOUNT | FTW_PHYS) == -1) {
					printmsg("nftw(3) failed");
					(*dents)[n].bsize = sb.st_blocks;
				} else
					(*dents)[n].bsize = blk_size;
			} else
				(*dents)[n].bsize = sb.st_blocks;
		}

		n++;
	}

	if (bsizeorder) {
		static struct statvfs svb;
		if (statvfs(path, &svb) == -1)
			fs_free = 0;
		else
			fs_free = svb.f_bavail << getorder(svb.f_bsize);
	}

	/* Should never be null */
	if (closedir(dirp) == -1) {
		if (*dents)
			free(*dents);
		printerr(1, "closedir");
	}

	return n;
}

static void
dentfree(struct entry *dents)
{
	free(dents);
}

/* Return the position of the matching entry or 0 otherwise */
static int
dentfind(struct entry *dents, int n, char *path)
{
	if (!path)
		return 0;

	static int i;
	static char *p;

	p = basename(path);
	DPRINTF_S(p);

	for (i = 0; i < n; i++)
		if (strcmp(p, dents[i].name) == 0)
			return i;

	return 0;
}

static int
populate(char *path, char *oldpath, char *fltr)
{
	static regex_t re;

	/* Can fail when permissions change while browsing */
	if (canopendir(path) == 0)
		return -1;

	/* Search filter */
	if (setfilter(&re, fltr) != 0)
		return -1;

	ndents = dentfill(path, &dents, visible, &re);

	qsort(dents, ndents, sizeof(*dents), entrycmp);

	/* Find cur from history */
	cur = dentfind(dents, ndents, oldpath);
	return 0;
}

static void
redraw(char *path)
{
	static char cwd[PATH_MAX];
	static int nlines, i;
	static size_t ncols;

	nlines = MIN(LINES - 4, ndents);

	/* Clean screen */
	erase();

	/* Strip trailing slashes */
	for (i = strlen(path) - 1; i > 0; i--)
		if (path[i] == '/')
			path[i] = '\0';
		else
			break;

	DPRINTF_D(cur);
	DPRINTF_S(path);

	/* No text wrapping in cwd line */
	if (!realpath(path, cwd)) {
		printmsg("Cannot resolve path");
		return;
	}

	ncols = COLS;
	if (ncols > PATH_MAX)
		ncols = PATH_MAX;
	cwd[ncols - strlen(CWD) - 1] = '\0';
	printw(CWD "%s\n\n", cwd);

	/* Print listing */
	if (cur < (nlines >> 1)) {
		for (i = 0; i < nlines; i++)
			printptr(&dents[i], i == cur);
	} else if (cur >= ndents - (nlines >> 1)) {
		for (i = ndents - nlines; i < ndents; i++)
			printptr(&dents[i], i == cur);
	} else {
		static int odd;
		odd = ISODD(nlines);
		nlines >>= 1;
		for (i = cur - nlines; i < cur + nlines + odd; i++)
			printptr(&dents[i], i == cur);
	}

	if (showdetail) {
		if (ndents) {
			static char ind[2] = "\0\0";
			static char sort[17];

			if (mtimeorder)
				sprintf(sort, "by time ");
			else if (sizeorder)
				sprintf(sort, "by size ");
			else
				sort[0] = '\0';

			if (S_ISDIR(dents[cur].mode))
				ind[0] = '/';
			else if (S_ISLNK(dents[cur].mode))
				ind[0] = '@';
			else if (S_ISSOCK(dents[cur].mode))
				ind[0] = '=';
			else if (S_ISFIFO(dents[cur].mode))
				ind[0] = '|';
			else if (dents[cur].mode & S_IXUSR)
				ind[0] = '*';
			else
				ind[0] = '\0';

			if (!bsizeorder)
				sprintf(cwd, "total %d %s[%s%s]", ndents, sort,
					replace_escape(dents[cur].name), ind);
			else
				sprintf(cwd, "total %d by disk usage, %s free [%s%s]",
					ndents, coolsize(fs_free),
					replace_escape(dents[cur].name), ind);

			printmsg(cwd);
		} else
			printmsg("0 items");
	}
}

static void
browse(char *ipath, char *ifilter)
{
	static char path[PATH_MAX], oldpath[PATH_MAX], newpath[PATH_MAX];
	static char lastdir[PATH_MAX];
	static char fltr[LINE_MAX];
	char *mime, *dir, *tmp, *run, *env;
	struct stat sb;
	int r, fd, presel;
	enum action sel = SEL_RUNARG + 1;

	xstrlcpy(path, ipath, PATH_MAX);
	xstrlcpy(fltr, ifilter, LINE_MAX);
	oldpath[0] = '\0';
	newpath[0] = '\0';
	lastdir[0] = '\0'; /* Can't move back from initial directory */

	if (filtermode)
		presel = FILTER;
	else
		presel = 0;

begin:
	if (populate(path, oldpath, fltr) == -1) {
		printwarn();
		goto nochange;
	}

	for (;;) {
		redraw(path);
nochange:
		/* Exit if parent has exited */
		if (getppid() == 1)
			_exit(0);

		sel = nextsel(&run, &env, &presel);

		switch (sel) {
		case SEL_CDQUIT:
		{
			char *tmpfile = "/tmp/nnn";
			if ((tmp = getenv("NNN_TMPFILE")) != NULL)
				tmpfile = tmp;

			FILE *fp = fopen(tmpfile, "w");
			if (fp) {
				fprintf(fp, "cd \"%s\"", path);
				fclose(fp);
			}

			/* Fall through to exit */
		} // fallthrough
		case SEL_QUIT:
			dentfree(dents);
			return;
		case SEL_BACK:
			/* There is no going back */
			if (path[0] == '/' && path[1] == '\0') {
				printmsg("You are at /");
				goto nochange;
			}

			dir = xdirname(path);
			if (canopendir(dir) == 0) {
				printwarn();
				goto nochange;
			}

			/* Save history */
			xstrlcpy(oldpath, path, PATH_MAX);

			/* Save last working directory */
			xstrlcpy(lastdir, path, PATH_MAX);
			xstrlcpy(path, dir, PATH_MAX);
			/* Reset filter */
			xstrlcpy(fltr, ifilter, LINE_MAX);
			if (filtermode)
				presel = FILTER;
			goto begin;
		case SEL_GOIN:
			/* Cannot descend in empty directories */
			if (ndents == 0)
				goto begin;

			mkpath(path, dents[cur].name, newpath, PATH_MAX);
			DPRINTF_S(newpath);

			/* Get path info */
			fd = open(newpath, O_RDONLY | O_NONBLOCK);
			if (fd == -1) {
				printwarn();
				goto nochange;
			}
			r = fstat(fd, &sb);
			if (r == -1) {
				printwarn();
				close(fd);
				goto nochange;
			}
			close(fd);
			DPRINTF_U(sb.st_mode);

			switch (sb.st_mode & S_IFMT) {
			case S_IFDIR:
				if (canopendir(newpath) == 0) {
					printwarn();
					goto nochange;
				}

				/* Save last working directory */
				xstrlcpy(lastdir, path, PATH_MAX);

				xstrlcpy(path, newpath, PATH_MAX);
				oldpath[0] = '\0';
				/* Reset filter */
				xstrlcpy(fltr, ifilter, LINE_MAX);
				if (filtermode)
					presel = FILTER;
				goto begin;
			case S_IFREG:
			{
				/* If NNN_OPENER is set, use it */
				if (opener) {
					spawn(opener, newpath, NULL, NULL, 4);
					continue;
				}

				/* Play with nlay if identified */
				mime = getmime(dents[cur].name);
				if (mime) {
					exitcurses();
					spawn(player, newpath, mime, NULL, 0);
					initcurses();
					continue;
				}

				/* If nlay doesn't handle it, open plain text
				   files with vi, then try NNN_FALLBACK_OPENER */
				if (get_output(g_buf, MAX_CMD_LEN, "file", "-bi",
					       newpath, 0) == NULL)
					continue;

				if (strstr(g_buf, "text/") == g_buf) {
					exitcurses();
					run = xgetenv("EDITOR", "vi");
					spawn(run, newpath, NULL, NULL, 0);
					initcurses();
					continue;
				} else if (fb_opener) {
					spawn(fb_opener, newpath, NULL, NULL, 4);
					continue;
				}

				printmsg("No association");
				goto nochange;
			}
			default:
				printmsg("Unsupported file");
				goto nochange;
			}
		case SEL_FLTR:
			presel = readln(path);
			xstrlcpy(fltr, ifilter, LINE_MAX);
			DPRINTF_S(fltr);
			/* Save current */
			if (ndents > 0)
				mkpath(path, dents[cur].name, oldpath, PATH_MAX);
			goto nochange;
		case SEL_MFLTR:
			filtermode = !filtermode;
			if (filtermode)
				presel = FILTER;
			else
				printmsg("navigate-as-you-type off");
			goto nochange;
		case SEL_SEARCH:
			exitcurses();
			spawn(player, path, "search", NULL, 0);
			initcurses();
			break;
		case SEL_NEXT:
			if (cur < ndents - 1)
				cur++;
			else if (ndents)
				/* Roll over, set cursor to first entry */
				cur = 0;
			break;
		case SEL_PREV:
			if (cur > 0)
				cur--;
			else if (ndents)
				/* Roll over, set cursor to last entry */
				cur = ndents - 1;
			break;
		case SEL_PGDN:
			if (cur < ndents - 1)
				cur += MIN((LINES - 4) / 2, ndents - 1 - cur);
			break;
		case SEL_PGUP:
			if (cur > 0)
				cur -= MIN((LINES - 4) / 2, cur);
			break;
		case SEL_HOME:
			cur = 0;
			break;
		case SEL_END:
			cur = ndents - 1;
			break;
		case SEL_CD:
		{
			static char *tmp, *input;
			static int truecd;

			/* Save the program start dir */
			tmp = getcwd(newpath, PATH_MAX);
			if (tmp == NULL) {
				printwarn();
				goto nochange;
			}

			/* Switch to current path for readline(3) */
			if (chdir(path) == -1) {
				printwarn();
				goto nochange;
			}

			exitcurses();
			tmp = readline("chdir: ");
			initcurses();

			/* Change back to program start dir */
			if (chdir(newpath) == -1)
				printwarn();

			if (tmp[0] == '\0')
				break;
			else
				/* Add to readline(3) history */
				add_history(tmp);

			input = tmp;
			tmp = strstrip(tmp);
			if (tmp[0] == '\0') {
				free(input);
				break;
			}

			truecd = 0;

			if (tmp[0] == '~') {
				/* Expand ~ to HOME absolute path */
				char *home = getenv("HOME");
				if (home)
					snprintf(newpath, PATH_MAX, "%s%s", home, tmp + 1);
				else {
					free(input);
					break;
				}
			} else if (tmp[0] == '-' && tmp[1] == '\0') {
				if (lastdir[0] == '\0') {
					free(input);
					break;
				}

				/* Switch to last visited dir */
				xstrlcpy(newpath, lastdir, PATH_MAX);
				truecd = 1;
			} else if ((r = all_dots(tmp))) {
				if (r == 1) {
					/* Always in the current dir */
					free(input);
					break;
				}

				r--;
				dir = path;

				for (fd = 0; fd < r; fd++) {
					/* Reached / ? */
					if (strcmp(path, "/") == 0 ||
					    strchr(path, '/') == NULL) {
						/* If it's a cd .. at / */
						if (fd == 0) {
							printmsg("You are at /");
							free(input);
							goto nochange;
						}

						/* Can't cd beyond / anyway */
						break;
					} else {
						dir = xdirname(dir);
						if (canopendir(dir) == 0) {
							printwarn();
							free(input);
							goto nochange;
						}
					}
				}

				truecd = 1;

				/* Save the path in case of cd ..
				   We mark the current dir in parent dir */
				if (r == 1) {
					xstrlcpy(oldpath, path, PATH_MAX);
					truecd = 2;
				}

				xstrlcpy(newpath, dir, PATH_MAX);
			} else
				mkpath(path, tmp, newpath, PATH_MAX);

			if (canopendir(newpath) == 0) {
				printwarn();
				free(input);
				break;
			}

			if (truecd == 0) {
				/* Probable change in dir */
				/* No-op if it's the same directory */
				if (strcmp(path, newpath) == 0) {
					free(input);
					break;
				}

				oldpath[0] = '\0';
			} else if (truecd == 1)
				/* Sure change in dir */
				oldpath[0] = '\0';

			/* Save last working directory */
			xstrlcpy(lastdir, path, PATH_MAX);

			/* Save the newly opted dir in path */
			xstrlcpy(path, newpath, PATH_MAX);

			/* Reset filter */
			xstrlcpy(fltr, ifilter, LINE_MAX);
			DPRINTF_S(path);
			free(input);
			if (filtermode)
				presel = FILTER;
			goto begin;
		}
		case SEL_CDHOME:
			tmp = getenv("HOME");
			if (tmp == NULL) {
				clearprompt();
				goto nochange;
			}

			if (canopendir(tmp) == 0) {
				printwarn();
				goto nochange;
			}

			if (strcmp(path, tmp) == 0)
				break;

			/* Save last working directory */
			xstrlcpy(lastdir, path, PATH_MAX);

			xstrlcpy(path, tmp, PATH_MAX);
			oldpath[0] = '\0';
			/* Reset filter */
			xstrlcpy(fltr, ifilter, LINE_MAX);
			DPRINTF_S(path);
			if (filtermode)
				presel = FILTER;
			goto begin;
		case SEL_CDBEGIN:
			if (canopendir(ipath) == 0) {
				printwarn();
				goto nochange;
			}

			if (strcmp(path, ipath) == 0)
				break;

			/* Save last working directory */
			xstrlcpy(lastdir, path, PATH_MAX);

			xstrlcpy(path, ipath, PATH_MAX);
			oldpath[0] = '\0';
			/* Reset filter */
			xstrlcpy(fltr, ifilter, LINE_MAX);
			DPRINTF_S(path);
			if (filtermode)
				presel = FILTER;
			goto begin;
		case SEL_CDLAST:
			if (lastdir[0] == '\0')
				break;

			if (canopendir(lastdir) == 0) {
				printwarn();
				goto nochange;
			}

			xstrlcpy(newpath, lastdir, PATH_MAX);
			xstrlcpy(lastdir, path, PATH_MAX);
			xstrlcpy(path, newpath, PATH_MAX);
			oldpath[0] = '\0';
			/* Reset filter */
			xstrlcpy(fltr, ifilter, LINE_MAX);
			DPRINTF_S(path);
			if (filtermode)
				presel = FILTER;
			goto begin;
		case SEL_TOGGLEDOT:
			showhidden ^= 1;
			initfilter(showhidden, &ifilter);
			xstrlcpy(fltr, ifilter, LINE_MAX);
			goto begin;
		case SEL_DETAIL:
			showdetail = !showdetail;
			showdetail ? (printptr = &printent_long)
				   : (printptr = &printent);
			/* Save current */
			if (ndents > 0)
				mkpath(path, dents[cur].name, oldpath, PATH_MAX);
			goto begin;
		case SEL_STATS:
		{
			struct stat sb;

			if (ndents > 0) {
				mkpath(path, dents[cur].name, oldpath, PATH_MAX);

				r = lstat(oldpath, &sb);
				if (r == -1) {
					if (dents)
						dentfree(dents);
					printerr(1, "lstat");
				} else {
					exitcurses();
					r = show_stats(oldpath, dents[cur].name, &sb);
					initcurses();
					if (r < 0) {
						printmsg(strerror(errno));
						goto nochange;
					}
				}
			}

			break;
		}
		case SEL_MEDIA:
			if (ndents > 0) {
				mkpath(path, dents[cur].name, oldpath, PATH_MAX);

				if(show_mediainfo(oldpath, NULL) == -1) {
					printmsg("mediainfo missing");
					goto nochange;
				}
			}
			break;
		case SEL_FMEDIA:
			if (ndents > 0) {
				mkpath(path, dents[cur].name, oldpath, PATH_MAX);

				if(show_mediainfo(oldpath, "-f") == -1) {
					printmsg("mediainfo missing");
					goto nochange;
				}
			}
			break;
		case SEL_DFB:
			if (!desktop_manager) {
				printmsg("NNN_DE_FILE_MANAGER not set");
				goto nochange;
			}

			spawn(desktop_manager, path, NULL, path, 2);
			break;
		case SEL_FSIZE:
			sizeorder = !sizeorder;
			mtimeorder = 0;
			bsizeorder = 0;
			/* Save current */
			if (ndents > 0)
				mkpath(path, dents[cur].name, oldpath, PATH_MAX);
			goto begin;
		case SEL_BSIZE:
			bsizeorder = !bsizeorder;
			if (bsizeorder) {
				showdetail = 1;
				printptr = &printent_long;
			}
			mtimeorder = 0;
			sizeorder = 0;
			/* Save current */
			if (ndents > 0)
				mkpath(path, dents[cur].name, oldpath, PATH_MAX);
			goto begin;
		case SEL_MTIME:
			mtimeorder = !mtimeorder;
			sizeorder = 0;
			bsizeorder = 0;
			/* Save current */
			if (ndents > 0)
				mkpath(path, dents[cur].name, oldpath, PATH_MAX);
			goto begin;
		case SEL_REDRAW:
			/* Save current */
			if (ndents > 0)
				mkpath(path, dents[cur].name, oldpath, PATH_MAX);
			goto begin;
		case SEL_COPY:
			if (copier && ndents) {
				if (strcmp(path, "/") == 0)
					snprintf(newpath, PATH_MAX, "/%s",
						 dents[cur].name);
				else
					snprintf(newpath, PATH_MAX, "%s/%s",
						 path, dents[cur].name);
				spawn(copier, newpath, NULL, NULL, 0);
				printmsg(newpath);
			} else if (!copier)
					printmsg("NNN_COPIER is not set");
			goto nochange;
		case SEL_HELP:
			exitcurses();
			show_help();
			initcurses();
			break;
		case SEL_RUN:
			run = xgetenv(env, run);
			exitcurses();
			spawn(run, NULL, NULL, path, 1);
			initcurses();
			/* Repopulate as directory content may have changed */
			goto begin;
		case SEL_RUNARG:
			run = xgetenv(env, run);
			exitcurses();
			spawn(run, dents[cur].name, NULL, path, 0);
			initcurses();
			break;
		}
		/* Screensaver */
		if (idletimeout != 0 && idle == idletimeout) {
			idle = 0;
			exitcurses();
			spawn(player, "", "screensaver", NULL, 8);
			initcurses();
		}
	}
}

static void
usage(void)
{
	fprintf(stdout, "usage: nnn [-d] [-i] [-p custom_nlay] [-S] [-v] [-h] [PATH]\n\n\
The missing terminal file browser for X.\n\n\
positional arguments:\n\
  PATH           directory to open [default: current dir]\n\n\
optional arguments:\n\
  -d             start in detail view mode\n\
  -i             start in navigate-as-you-type mode\n\
  -p             path to custom nlay\n\
  -S             start in disk usage analyzer mode\n\
  -v             show program version and exit\n\
  -h             show this help and exit\n\n\
Version: %s\n\
License: BSD 2-Clause\n\
Webpage: https://github.com/jarun/nnn\n", VERSION);

	exit(0);
}

int
main(int argc, char *argv[])
{
	char cwd[PATH_MAX], *ipath;
	char *ifilter;
	int opt = 0;

	/* Confirm we are in a terminal */
	if (!isatty(0) || !isatty(1)) {
		fprintf(stderr, "stdin or stdout is not a tty\n");
		exit(1);
	}

	while ((opt = getopt(argc, argv, "dSip:vh")) != -1) {
		switch (opt) {
		case 'S':
			bsizeorder = 1; // fallthrough
		case 'd':
			/* Open in detail mode, if set */
			showdetail = 1;
			printptr = &printent_long;
			break;
		case 'i':
			filtermode = 1;
			break;
		case 'p':
			player = optarg;
			break;
		case 'v':
			fprintf(stdout, "%s\n", VERSION);
			return 0;
		case 'h': // fallthrough
		default:
			usage();
		}
	}

	if (argc == optind) {
		/* Start in the current directory */
		ipath = getcwd(cwd, PATH_MAX);
		if (ipath == NULL)
			ipath = "/";
	} else {
		ipath = realpath(argv[optind], cwd);
		if (!ipath) {
			fprintf(stderr, "%s: no such dir\n", argv[optind]);
			exit(1);
		}
	}

	open_max = max_openfds();

	if (getuid() == 0)
		showhidden = 1;
	initfilter(showhidden, &ifilter);

	/* Get the default desktop mime opener, if set */
	opener = getenv("NNN_OPENER");

	/* Set player if not set already */
	if (!player)
		player = nlay;

	/* Get the fallback desktop mime opener, if set */
	fb_opener = getenv("NNN_FALLBACK_OPENER");

	/* Get the desktop file browser, if set */
	desktop_manager = getenv("NNN_DE_FILE_MANAGER");

	/* Get screensaver wait time, if set; copier used as tmp var */
	copier = getenv("NNN_IDLE_TIMEOUT");
	if (copier)
		idletimeout = abs(atoi(copier));

	/* Get the default copier, if set */
	copier = getenv("NNN_COPIER");

	signal(SIGINT, SIG_IGN);

	/* Test initial path */
	if (canopendir(ipath) == 0) {
		fprintf(stderr, "%s: %s\n", ipath, strerror(errno));
		exit(1);
	}

	/* Set locale */
	setlocale(LC_ALL, "");

	initcurses();
	browse(ipath, ifilter);
	exitcurses();
	exit(0);
}
