/* See LICENSE file for copyright and license details. */

/*
 * Visual layout:
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

#ifdef __linux__
#include <sys/inotify.h>
#define LINUX_INOTIFY
#endif
#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/statvfs.h>
#if defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__) || defined(__APPLE__)
# include <sys/types.h>
#include <sys/event.h>
#include <sys/time.h>
#define BSD_KQUEUE
#else
# include <sys/sysmacros.h>
#endif
#include <sys/wait.h>

#include <ctype.h>
#ifdef __linux__ /* Fix failure due to mvaddnwstr() */
#ifndef NCURSES_WIDECHAR
#define NCURSES_WIDECHAR 1
#endif
#elif defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__) || defined(__APPLE__)
#ifndef _XOPEN_SOURCE_EXTENDED
#define _XOPEN_SOURCE_EXTENDED
#endif
#endif
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
#include <readline/history.h>
#include <readline/readline.h>
#ifndef __USE_XOPEN_EXTENDED
#define __USE_XOPEN_EXTENDED 1
#endif
#include <ftw.h>

#include "config.h"

#ifdef DEBUGMODE
static int DEBUG_FD;

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

static int
enabledbg()
{
	FILE *fp = fopen("/tmp/nnn_debug", "w");

	if (!fp) {
		fprintf(stderr, "Cannot open debug file\n");
		return -1;
	}

	DEBUG_FD = fileno(fp);
	if (DEBUG_FD == -1) {
		fprintf(stderr, "Cannot open debug file descriptor\n");
		return -1;
	}

	return 0;
}

static void
disabledbg()
{
	close(DEBUG_FD);
}

#define DPRINTF_D(x) xprintf(DEBUG_FD, #x "=%d\n", x)
#define DPRINTF_U(x) xprintf(DEBUG_FD, #x "=%u\n", x)
#define DPRINTF_S(x) xprintf(DEBUG_FD, #x "=%s\n", x)
#define DPRINTF_P(x) xprintf(DEBUG_FD, #x "=0x%p\n", x)
#else
#define DPRINTF_D(x)
#define DPRINTF_U(x)
#define DPRINTF_S(x)
#define DPRINTF_P(x)
#endif /* DEBUGMODE */

/* Macro definitions */
#define VERSION "1.3"
#define LEN(x) (sizeof(x) / sizeof(*(x)))
#undef MIN
#define MIN(x, y) ((x) < (y) ? (x) : (y))
#define ISODD(x) ((x) & 1)
#define TOUPPER(ch) \
	(((ch) >= 'a' && (ch) <= 'z') ? ((ch) - 'a' + 'A') : (ch))
#define MAX_CMD_LEN 5120
#define CWD   "cwd: "
#define CURSR " > "
#define EMPTY "   "
#define CURSYM(flag) (flag ? CURSR : EMPTY)
#define FILTER '/'
#define REGEX_MAX 128
#define BM_MAX 10

/* Macros to define process spawn behaviour as flags */
#define F_NONE     0x00  /* no flag set */
#define F_MARKER   0x01  /* draw marker to indicate nnn spawned (e.g. shell) */
#define F_NOWAIT   0x02  /* don't wait for child process (e.g. file manager) */
#define F_NOTRACE  0x04  /* suppress stdout and strerr (no traces) */
#define F_SIGINT   0x08  /* restore default SIGINT handler */
#define F_NORMAL   0x80  /* spawn child process in non-curses regular mode */

#define exitcurses() endwin()
#define clearprompt() printmsg("")
#define printwarn() printmsg(strerror(errno))
#define istopdir(path) (path[1] == '\0' && path[0] == '/')
#define settimeout() timeout(1000)
#define cleartimeout() timeout(-1)
#define errexit() printerr(__LINE__)

#ifdef LINUX_INOTIFY
#define EVENT_SIZE (sizeof(struct inotify_event))
#define EVENT_BUF_LEN (1024 * (EVENT_SIZE + 16))
#elif defined(BSD_KQUEUE)
#define NUM_EVENT_SLOTS 1
#define NUM_EVENT_FDS 1
#endif

/* TYPE DEFINITIONS */
typedef unsigned long ulong;
typedef unsigned int uint;
typedef unsigned char uchar;

/* STRUCTURES */

/* Directory entry */
typedef struct entry {
	char name[NAME_MAX];
	mode_t mode;
	time_t t;
	off_t size;
	blkcnt_t blocks; /* number of 512B blocks allocated */
} *pEntry;

/* Bookmark */
typedef struct {
	char *key;
	char *loc;
} bm;

/* Settings */
typedef struct {
	uchar filtermode : 1;  /* Set to enter filter mode */
	uchar mtimeorder : 1;  /* Set to sort by time modified */
	uchar sizeorder  : 1;  /* Set to sort by file size */
	uchar blkorder   : 1;  /* Set to sort by blocks used (disk usage) */
	uchar showhidden : 1;  /* Set to show hidden files */
	uchar showdetail : 1;  /* Clear to show fewer file info */
	uchar showcolor  : 1;  /* Set to show dirs in blue */
	uchar dircolor   : 1;  /* Current status of dir color */
} settings;

/* GLOBALS */

/* Configuration */
static settings cfg = {0, 0, 0, 0, 0, 1, 1, 0};

static struct entry *dents;
static int ndents, cur, total_dents;
static uint idle;
static uint idletimeout;
static char *player;
static char *copier;
static char *editor;
static char *desktop_manager;
static char *metaviewer;
static blkcnt_t ent_blocks;
static blkcnt_t dir_blocks;
static ulong num_files;
static uint open_max;
static bm bookmark[BM_MAX];
static uchar color = 4;

#ifdef LINUX_INOTIFY
static int inotify_fd, inotify_wd = -1;
static uint INOTIFY_MASK = IN_ATTRIB | IN_CREATE | IN_DELETE | IN_DELETE_SELF | IN_MODIFY | IN_MOVE_SELF | IN_MOVED_FROM | IN_MOVED_TO;
#elif defined(BSD_KQUEUE)
static int kq, event_fd = -1;
static struct kevent events_to_monitor[NUM_EVENT_FDS];
static uint KQUEUE_FFLAGS = NOTE_DELETE | NOTE_EXTEND | NOTE_LINK | NOTE_RENAME | NOTE_REVOKE | NOTE_WRITE;
static struct timespec gtimeout;
#endif

/* Utilities to open files, run actions */
static char * const utils[] = {
#ifdef __APPLE__
	"/usr/bin/open",
#else
	"/usr/bin/xdg-open",
#endif
	"nlay",
	"mediainfo",
	"exiftool"
};

/* Common message strings */
static char *STR_NFTWFAIL = "nftw(3) failed";
static char *STR_ATROOT = "You are at /";
static char *STR_NOHOME = "HOME not set";

/* For use in functions which are isolated and don't return the buffer */
static char g_buf[MAX_CMD_LEN];

/* Forward declarations */
static void redraw(char *path);

/* Functions */

/* Messages show up at the bottom */
static void
printmsg(char *msg)
{
	mvprintw(LINES - 1, 0, "%s\n", msg);
}

/* Kill curses and display error before exiting */
static void
printerr(int linenum)
{
	exitcurses();
	fprintf(stderr, "line %d: (%d) %s\n", linenum, errno, strerror(errno));
	exit(1);
}

/* Print prompt on the last line */
static void
printprompt(char *str)
{
	clearprompt();
	printw(str);
}

/* Increase the limit on open file descriptors, if possible */
static rlim_t
max_openfds()
{
	struct rlimit rl;
	rlim_t limit = getrlimit(RLIMIT_NOFILE, &rl);

	if (limit != 0)
		return 32;

	limit = rl.rlim_cur;
	rl.rlim_cur = rl.rlim_max;

	/* Return ~75% of max possible */
	if (setrlimit(RLIMIT_NOFILE, &rl) == 0) {
		limit = rl.rlim_max - (rl.rlim_max >> 2);
		/*
		 * 20K is arbitrary> If the limit is set to max possible
		 * value, the memory usage increases to more than double.
		 */
		return limit > 20480 ?  20480 : limit;
	}

	return limit;
}

/*
 * Custom xstrlen()
 */
static size_t
xstrlen(const char *s)
{
	static size_t len;

	if (!s)
		return 0;

	len = 0;
	while (*s)
		++len, ++s;

	return len;
}

/*
 * Just a safe strncpy(3)
 * Always null ('\0') terminates if both src and dest are valid pointers.
 * Returns the number of bytes copied including terminating null byte.
 */
static size_t
xstrlcpy(char *dest, const char *src, size_t n)
{
	static size_t len, blocks;
	static const uint _WSHIFT = (sizeof(ulong) == 8) ? 3 : 2;

	if (!src || !dest)
		return 0;

	len = xstrlen(src) + 1;
	if (n > len)
		n = len;
	else if (len > n)
		/* Save total number of bytes to copy in len */
		len = n;

	blocks = n >> _WSHIFT;
	n -= (blocks << _WSHIFT);

	if (blocks) {
		static ulong *s, *d;

		s = (ulong *)src;
		d = (ulong *)dest;

		while (blocks) {
			*d = *s;
			++d, ++s;
			--blocks;
		}

		if (!n) {
			dest = (char *)d;
			*--dest = '\0';
			return len;
		}

		src = (char *)s;
		dest = (char *)d;
	}

	while (--n && (*dest = *src))
		++dest, ++src;

	if (!n)
		*dest = '\0';

	return len;
}

/*
 * Custom strcmp(), just what we need.
 * Returns 0 if same, else -1
 */
static int
xstrcmp(const char *s1, const char *s2)
{
	if (!s1 || !s2)
		return -1;

	while (*s1 && *s1 == *s2)
		++s1, ++s2;

	if (*s1 != *s2)
		return -1;

	return 0;
}

/*
 * The poor man's implementation of memrchr(3).
 * We are only looking for '/' in this program.
 * Ideally 0 < n <= strlen(s).
 */
static void *
xmemrchr(uchar *s, uchar ch, size_t n)
{
	if (!s || !n)
		return NULL;

	s = s + n - 1;

	while (n) {
		if (*s == ch)
			return s;

		--n, --s;
	}

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
	static char *buf = g_buf;
	static char *last_slash;

	xstrlcpy(buf, path, PATH_MAX);

	/* Find last '/'. */
	last_slash = xmemrchr((uchar *)buf, '/', xstrlen(buf));

	if (last_slash != NULL && last_slash != buf && last_slash[1] == '\0') {
		/* Determine whether all remaining characters are slashes. */
		char *runp;

		for (runp = last_slash; runp != buf; --runp)
			if (runp[-1] != '/')
				break;

		/* The '/' is the last character, we have to look further. */
		if (runp != buf)
			last_slash = xmemrchr((uchar *)buf, '/', runp - buf);
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
			 * We have to return "/". As a special case we have to
			 * return "//" if there are exactly two slashes at the
			 * beginning of the string. See XBD 4.10 Path Name
			 * Resolution for more information.
			 */
			if (last_slash == buf + 1)
				++last_slash;
			else
				last_slash = buf + 1;
		} else
			last_slash = runp;

		last_slash[0] = '\0';
	} else {
		/* This assignment is ill-designed but the XPG specs require to
		 * return a string containing "." in any case no directory part
		 * is found and so a static and constant string is required.
		 */
		buf[0] = '.';
		buf[1] = '\0';
	}

	return buf;
}

/*
 * Return number of dots if all chars in a string are dots, else 0
 */
static int
all_dots(const char *path)
{
	if (!path)
		return FALSE;

	int count = 0;

	while (*path == '.')
		++count, ++path;

	if (*path)
		return 0;

	return count;
}

/* Initialize curses mode */
static void
initcurses(void)
{
	if (initscr() == NULL) {
		char *term = getenv("TERM");

		if (term != NULL)
			fprintf(stderr, "error opening TERM: %s\n", term);
		else
			fprintf(stderr, "initscr() failed\n");
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
	if (cfg.showcolor)
		init_pair(1, color, -1);
	settimeout(); /* One second */
}

/*
 * Spawns a child process. Behaviour can be controlled using flag.
 * Limited to 2 arguments to a program, flag works on bit set.
 */
static void
spawn(char *file, char *arg1, char *arg2, char *dir, uchar flag)
{
	pid_t pid;
	int status;

	if (flag & F_NORMAL)
		exitcurses();

	pid = fork();
	if (pid == 0) {
		if (dir != NULL)
			status = chdir(dir);

		/* Show a marker (to indicate nnn spawned shell) */
		if (flag & F_MARKER) {
			printf("\n +-++-++-+\n | n n n |\n +-++-++-+\n\n");
			printf("Spawned shell level: %d\n", atoi(getenv("SHLVL")) + 1);
		}

		/* Suppress stdout and stderr */
		if (flag & F_NOTRACE) {
			int fd = open("/dev/null", O_WRONLY, 0200);

			dup2(fd, 1);
			dup2(fd, 2);
			close(fd);
		}

		if (flag & F_SIGINT)
			signal(SIGINT, SIG_DFL);
		execlp(file, file, arg1, arg2, NULL);
		_exit(1);
	} else {
		if (!(flag & F_NOWAIT))
			/* Ignore interruptions */
			while (waitpid(pid, &status, 0) == -1)
				DPRINTF_D(status);

		DPRINTF_D(pid);
		if (flag & F_NORMAL)
			initcurses();
	}
}

/* Get program name from env var, else return fallback program */
static char *
xgetenv(char *name, char *fallback)
{
	if (name == NULL)
		return fallback;

	char *value = getenv(name);

	return value && value[0] ? value : fallback;
}

/* Check if a dir exists, IS a dir and is readable */
static bool
xdiraccess(char *path)
{
	static DIR *dirp;

	dirp = opendir(path);
	if (dirp == NULL) {
		printwarn();
		return FALSE;
	}

	closedir(dirp);
	return TRUE;
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
		++c1;
	if (*c1 == '-' || *c1 == '+')
		++c1;
	while (*c1 >= '0' && *c1 <= '9')
		++c1;

	c2 = s2;
	while (isspace(*c2))
		++c2;
	if (*c2 == '-' || *c2 == '+')
		++c2;
	while (*c2 >= '0' && *c2 <= '9')
		++c2;

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
		++s1, ++s2;

	/* In case of alphabetically same names, make sure
	 * lower case one comes before upper case one
	 */
	if (!*s1 && !*s2)
		return 1;

	return (int) (TOUPPER(*s1) - TOUPPER(*s2));
}

/* Return the integer value of a char representing HEX */
static char
xchartohex(char c)
{
	if (c >= '0' && c <= '9')
		return c - '0';

	c = TOUPPER(c);
	if (c >= 'A' && c <= 'F')
		return c - 'A' + 10;

	return c;
}

/* Trim all whitespace from both ends, / from end */
static char *
strstrip(char *s)
{
	if (!s || !*s)
		return s;

	size_t len = xstrlen(s) - 1;

	while (len != 0 && (isspace(s[len]) || s[len] == '/'))
		--len;
	s[len + 1] = '\0';

	while (*s && isspace(*s))
		++s;

	return s;
}

static char *
getmime(char *file)
{
	regex_t regex;
	uint i;
	static uint len = LEN(assocs);

	for (i = 0; i < len; ++i) {
		if (regcomp(&regex, assocs[i].regex, REG_NOSUB | REG_EXTENDED | REG_ICASE) != 0)
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
	if (cfg.mtimeorder)
		return pb->t - pa->t;

	if (cfg.sizeorder) {
		if (pb->size > pa->size)
			return 1;
		else if (pb->size < pa->size)
			return -1;
	}

	if (cfg.blkorder) {
		if (pb->blocks > pa->blocks)
			return 1;
		else if (pb->blocks < pa->blocks)
			return -1;
	}

	return xstricmp(pa->name, pb->name);
}

/*
 * Returns SEL_* if key is bound and 0 otherwise.
 * Also modifies the run and env pointers (used on SEL_{RUN,RUNARG}).
 * The next keyboard input can be simulated by presel.
 */
static int
nextsel(char **run, char **env, int *presel)
{
	static int c;
	static uchar i;
	static uint len = LEN(bindings);
#ifdef LINUX_INOTIFY
	static char inotify_buf[EVENT_BUF_LEN];
#elif defined(BSD_KQUEUE)
	static struct kevent event_data[NUM_EVENT_SLOTS];
#endif

	c = *presel;

	if (c == 0)
		c = getch();
	else {
		*presel = 0;

		/* Unwatch dir if we are still in a filtered view */
#ifdef LINUX_INOTIFY
		if (inotify_wd >= 0) {
			inotify_rm_watch(inotify_fd, inotify_wd);
			inotify_wd = -1;
		}
#elif defined(BSD_KQUEUE)
		if (event_fd >= 0) {
			close(event_fd);
			event_fd = -1;
		}
#endif
	}

	if (c == -1) {
		++idle;

		/* Do not check for directory changes in du
		 * mode. A redraw forces du calculation.
		 * Check for changes every odd second.
		 */
#ifdef LINUX_INOTIFY
		if (!cfg.blkorder && inotify_wd >= 0 && idle & 1 && read(inotify_fd, inotify_buf, EVENT_BUF_LEN) > 0)
#elif defined(BSD_KQUEUE)
		if (!cfg.blkorder && event_fd >= 0 && idle & 1
		    && kevent(kq, events_to_monitor, NUM_EVENT_SLOTS, event_data, NUM_EVENT_FDS, &gtimeout) > 0)
#endif
				c = CONTROL('L');
	} else
		idle = 0;

	for (i = 0; i < len; ++i)
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
fill(struct entry **dents, int (*filter)(regex_t *, char *), regex_t *re)
{
	static int count;

	for (count = 0; count < ndents; ++count) {
		if (filter(re, (*dents)[count].name) == 0) {
			if (count != --ndents) {
				static struct entry _dent, *dentp1, *dentp2;

				dentp1 = &(*dents)[count];
				dentp2 = &(*dents)[ndents];

				/* Copy count to tmp */
				xstrlcpy(_dent.name, dentp1->name, NAME_MAX);
				_dent.mode = dentp1->mode;
				_dent.t = dentp1->t;
				_dent.size = dentp1->size;
				_dent.blocks = dentp1->blocks;

				/* Copy ndents - 1 to count */
				xstrlcpy(dentp1->name, dentp2->name, NAME_MAX);
				dentp1->mode = dentp2->mode;
				dentp1->t = dentp2->t;
				dentp1->size = dentp2->size;
				dentp1->blocks = dentp2->blocks;

				/* Copy tmp to ndents - 1 */
				xstrlcpy(dentp2->name, _dent.name, NAME_MAX);
				dentp2->mode = _dent.mode;
				dentp2->t = _dent.t;
				dentp2->size = _dent.size;
				dentp2->blocks = _dent.blocks;

				--count;
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
filterentries(char *path)
{
	static char ln[REGEX_MAX];
	static wchar_t wln[REGEX_MAX];
	static wint_t ch[2] = {0};
	static int maxlen = REGEX_MAX - 1;
	int r, total = ndents;
	int oldcur = cur;
	int len = 1;
	char *pln = ln + 1;

	ln[0] = wln[0] = FILTER;
	ln[1] = wln[1] = '\0';
	cur = 0;

	cleartimeout();
	echo();
	curs_set(TRUE);
	printprompt(ln);

	while ((r = wget_wch(stdscr, ch)) != ERR)
		if (r == OK)
			switch (*ch) {
			case '\r':  // with nonl(), this is ENTER key value
				if (len == 1) {
					cur = oldcur;
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

				wln[--len] = '\0';
				if (len == 1)
					cur = oldcur;

				wcstombs(ln, wln, REGEX_MAX);
				ndents = total;
				if (matches(pln) == -1) {
					printprompt(ln);
					continue;
				}
				redraw(path);
				printprompt(ln);
				break;
			case CONTROL('L'):
				if (len == 1)
					cur = oldcur; // fallthrough
			case CONTROL('Q'):
				goto end;
			default:
				/* Reset cur in case it's a repeat search */
				if (len == 1)
					cur = 0;

				if (len == maxlen || !isprint(*ch))
					break;

				wln[len] = (wchar_t)*ch;
				wln[++len] = '\0';
				wcstombs(ln, wln, REGEX_MAX);
				ndents = total;
				if (matches(pln) == -1)
					continue;
				redraw(path);
				printprompt(ln);
			}
		else
			switch (*ch) {
			case KEY_DC: // fallthrough
			case KEY_BACKSPACE:
				if (len == 1) {
					cur = oldcur;
					*ch = CONTROL('L');
					goto end;
				}

				wln[--len] = '\0';
				if (len == 1)
					cur = oldcur;

				wcstombs(ln, wln, REGEX_MAX);
				ndents = total;
				if (matches(pln) == -1)
					continue;
				redraw(path);
				printprompt(ln);
				break;
			default:
				if (len == 1)
					cur = oldcur;
				goto end;
			}
end:
	noecho();
	curs_set(FALSE);
	settimeout();

	/* Return keys for navigation etc. */
	return *ch;
}

/* Show a prompt with input string and return the changes */
static char *
xreadline(char *fname)
{
	int old_curs = curs_set(1);
	size_t len, pos;
	int c, x, y;
	wchar_t *buf = (wchar_t *)g_buf;
	size_t buflen = NAME_MAX - 1;

	DPRINTF_S(fname)
	mbstowcs(buf, fname, NAME_MAX);
	len = pos = wcslen(buf);
	/* For future: handle NULL, say for a new name */
	if (len <= 0) {
		buf[0] = '\0';
		len = pos = 0;
	}

	getyx(stdscr, y, x);
	cleartimeout();

	while (1) {
		buf[len] = ' ';
		mvaddnwstr(y, x, buf, len + 1);
		move(y, x + pos);

		c = getch();

		if (c == KEY_ENTER || c == '\n' || c == '\r')
			break;

		if (isprint(c) && pos < buflen) {
			memmove(buf + pos + 1, buf + pos, (len - pos) << 2);
			buf[pos] = c;
			++len, ++pos;
			continue;
		}

		switch (c) {
		case KEY_LEFT:
			if (pos > 0)
				--pos;
			break;
		case KEY_RIGHT:
			if (pos < len)
				++pos;
			break;
		case KEY_BACKSPACE:
			if (pos > 0) {
				memmove(buf + pos - 1, buf + pos, (len - pos) << 2);
				--len, --pos;
			}
			break;
		case KEY_DC:
			if (pos < len) {
				memmove(buf + pos, buf + pos + 1, (len - pos - 1) << 2);
				--len;
			}
			break;
		default:
			break;
		}
	}

	buf[len] = '\0';
	if (old_curs != ERR) curs_set(old_curs);

	settimeout();
	DPRINTF_S(buf)
	wcstombs(g_buf, buf, NAME_MAX);
	return g_buf;
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
		if (istopdir(dir))
			snprintf(out, n, "/%s", name);
		else
			snprintf(out, n, "%s/%s", dir, name);
	}
	return out;
}

static void
parsebmstr(char *bms)
{
	int i = 0;

	while (*bms && i < BM_MAX) {
		bookmark[i].key = bms;

		++bms;
		while (*bms && *bms != ':')
			++bms;

		if (!*bms) {
			bookmark[i].key = NULL;
			break;
		}

		*bms = '\0';

		bookmark[i].loc = ++bms;
		if (bookmark[i].loc[0] == '\0' || bookmark[i].loc[0] == ';') {
			bookmark[i].key = NULL;
			break;
		}

		while (*bms && *bms != ';')
			++bms;

		if (*bms)
			*bms = '\0';
		else
			break;

		++bms;
		++i;
	}
}

static char *
readinput(void)
{
	cleartimeout();
	echo();
	curs_set(TRUE);
	memset(g_buf, 0, LINE_MAX);
	wgetnstr(stdscr, g_buf, LINE_MAX - 1);
	noecho();
	curs_set(FALSE);
	settimeout();
	return g_buf[0] ? g_buf : NULL;
}

/*
 * Replace escape characters in a string with '?'
 */
static char *
unescape(const char *str)
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

		++buf;
	}

	/* Convert wide char to multi-byte */
	wcstombs(buffer, wbuf, PATH_MAX);
	return buffer;
}

static void
printent(struct entry *ent, int sel)
{
	static int ncols;

	if (PATH_MAX + 16 < COLS)
		ncols = PATH_MAX + 16;
	else
		ncols = COLS;

	if (S_ISDIR(ent->mode))
		snprintf(g_buf, ncols, "%s%s/", CURSYM(sel), unescape(ent->name));
	else if (S_ISLNK(ent->mode))
		snprintf(g_buf, ncols, "%s%s@", CURSYM(sel), unescape(ent->name));
	else if (S_ISSOCK(ent->mode))
		snprintf(g_buf, ncols, "%s%s=", CURSYM(sel), unescape(ent->name));
	else if (S_ISFIFO(ent->mode))
		snprintf(g_buf, ncols, "%s%s|", CURSYM(sel), unescape(ent->name));
	else if (ent->mode & 0100)
		snprintf(g_buf, ncols, "%s%s*", CURSYM(sel), unescape(ent->name));
	else
		snprintf(g_buf, ncols, "%s%s", CURSYM(sel), unescape(ent->name));

	/* Dirs are always shown on top */
	if (cfg.dircolor && !S_ISDIR(ent->mode)) {
		attroff(COLOR_PAIR(1) | A_BOLD);
		cfg.dircolor = 0;
	}

	printw("%s\n", g_buf);
}

static char *
coolsize(off_t size)
{
	static const char * const U = "BKMGTPEZY";
	static char size_buf[12]; /* Buffer to hold human readable size */
	static int i;
	static off_t tmp;
	static long double rem;
	static const double div_2_pow_10 = 1.0 / 1024.0;

	i = 0;
	rem = 0;

	while (size > 1024) {
		tmp = size;
		size >>= 10;
		rem = tmp - (size << 10);
		++i;
	}

	snprintf(size_buf, 12, "%.*Lf%c", i, size + rem * div_2_pow_10, U[i]);
	return size_buf;
}

static void
printent_long(struct entry *ent, int sel)
{
	static int ncols;
	static char buf[18];

	if (PATH_MAX + 32 < COLS)
		ncols = PATH_MAX + 32;
	else
		ncols = COLS;

	strftime(buf, 18, "%d-%m-%Y %H:%M", localtime(&ent->t));

	if (sel)
		attron(A_REVERSE);

	if (!cfg.blkorder) {
		if (S_ISDIR(ent->mode))
			snprintf(g_buf, ncols, "%s%-16.16s        /  %s/", CURSYM(sel), buf, unescape(ent->name));
		else if (S_ISLNK(ent->mode))
			snprintf(g_buf, ncols, "%s%-16.16s        @  %s@", CURSYM(sel), buf, unescape(ent->name));
		else if (S_ISSOCK(ent->mode))
			snprintf(g_buf, ncols, "%s%-16.16s        =  %s=", CURSYM(sel), buf, unescape(ent->name));
		else if (S_ISFIFO(ent->mode))
			snprintf(g_buf, ncols, "%s%-16.16s        |  %s|", CURSYM(sel), buf, unescape(ent->name));
		else if (S_ISBLK(ent->mode))
			snprintf(g_buf, ncols, "%s%-16.16s        b  %s", CURSYM(sel), buf, unescape(ent->name));
		else if (S_ISCHR(ent->mode))
			snprintf(g_buf, ncols, "%s%-16.16s        c  %s", CURSYM(sel), buf, unescape(ent->name));
		else if (ent->mode & 0100)
			snprintf(g_buf, ncols, "%s%-16.16s %8.8s* %s*", CURSYM(sel), buf, coolsize(ent->size), unescape(ent->name));
		else
			snprintf(g_buf, ncols, "%s%-16.16s %8.8s  %s", CURSYM(sel), buf, coolsize(ent->size), unescape(ent->name));
	} else {
		if (S_ISDIR(ent->mode))
			snprintf(g_buf, ncols, "%s%-16.16s %8.8s/ %s/", CURSYM(sel), buf, coolsize(ent->blocks << 9), unescape(ent->name));
		else if (S_ISLNK(ent->mode))
			snprintf(g_buf, ncols, "%s%-16.16s        @  %s@", CURSYM(sel), buf, unescape(ent->name));
		else if (S_ISSOCK(ent->mode))
			snprintf(g_buf, ncols, "%s%-16.16s        =  %s=", CURSYM(sel), buf, unescape(ent->name));
		else if (S_ISFIFO(ent->mode))
			snprintf(g_buf, ncols, "%s%-16.16s        |  %s|", CURSYM(sel), buf, unescape(ent->name));
		else if (S_ISBLK(ent->mode))
			snprintf(g_buf, ncols, "%s%-16.16s        b  %s", CURSYM(sel), buf, unescape(ent->name));
		else if (S_ISCHR(ent->mode))
			snprintf(g_buf, ncols, "%s%-16.16s        c  %s", CURSYM(sel), buf, unescape(ent->name));
		else if (ent->mode & 0100)
			snprintf(g_buf, ncols, "%s%-16.16s %8.8s* %s*", CURSYM(sel), buf, coolsize(ent->blocks << 9), unescape(ent->name));
		else
			snprintf(g_buf, ncols, "%s%-16.16s %8.8s  %s", CURSYM(sel), buf, coolsize(ent->blocks << 9), unescape(ent->name));
	}

	/* Dirs are always shown on top */
	if (cfg.dircolor && !S_ISDIR(ent->mode)) {
		attroff(COLOR_PAIR(1) | A_BOLD);
		cfg.dircolor = 0;
	}

	printw("%s\n", g_buf);

	if (sel)
		attroff(A_REVERSE);
}

static void (*printptr)(struct entry *ent, int sel) = &printent_long;

static char
get_fileind(mode_t mode, char *desc)
{
	static char c;

	if (S_ISREG(mode)) {
		c = '-';
		sprintf(desc, "%s", "regular file");
		if (mode & 0100)
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

	return c;
}

/* Convert a mode field into "ls -l" type perms field. */
static char *
get_lsperms(mode_t mode, char *desc)
{
	static const char * const rwx[] = {"---", "--x", "-w-", "-wx", "r--", "r-x", "rw-", "rwx"};
	static char bits[11];

	bits[0] = get_fileind(mode, desc);
	strcpy(&bits[1], rwx[(mode >> 6) & 7]);
	strcpy(&bits[4], rwx[(mode >> 3) & 7]);
	strcpy(&bits[7], rwx[(mode & 7)]);

	if (mode & S_ISUID)
		bits[3] = (mode & 0100) ? 's' : 'S';  /* user executable */
	if (mode & S_ISGID)
		bits[6] = (mode & 0010) ? 's' : 'l';  /* group executable */
	if (mode & S_ISVTX)
		bits[9] = (mode & 0001) ? 't' : 'T';  /* others executable */

	bits[10] = '\0';

	return bits;
}

/*
 * Gets only a single line (that's what we need
 * for now) or shows full command output in pager.
 *
 * If pager is valid, returns NULL
 */
static char *
get_output(char *buf, size_t bytes, char *file, char *arg1, char *arg2,
	   int pager)
{
	pid_t pid;
	int pipefd[2];
	FILE *pf;
	int tmp, flags;
	char *ret = NULL;

	if (pipe(pipefd) == -1)
		errexit();

	for (tmp = 0; tmp < 2; ++tmp) {
		/* Get previous flags */
		flags = fcntl(pipefd[tmp], F_GETFL, 0);

		/* Set bit for non-blocking flag */
		flags |= O_NONBLOCK;

		/* Change flags on fd */
		fcntl(pipefd[tmp], F_SETFL, flags);
	}

	pid = fork();
	if (pid == 0) {
		/* In child */
		close(pipefd[0]);
		dup2(pipefd[1], STDOUT_FILENO);
		dup2(pipefd[1], STDERR_FILENO);
		close(pipefd[1]);
		execlp(file, file, arg1, arg2, NULL);
		_exit(1);
	}

	/* In parent */
	waitpid(pid, &tmp, 0);
	close(pipefd[1]);

	if (!pager) {
		pf = fdopen(pipefd[0], "r");
		if (pf) {
			ret = fgets(buf, bytes, pf);
			close(pipefd[0]);
		}

		return ret;
	}

	pid = fork();
	if (pid == 0) {
		/* Show in pager in child */
		dup2(pipefd[0], STDIN_FILENO);
		close(pipefd[0]);
		execlp("less", "less", NULL);
		_exit(1);
	}

	/* In parent */
	waitpid(pid, &tmp, 0);
	close(pipefd[0]);

	return NULL;
}

/*
 * Follows the stat(1) output closely
 */
static int
show_stats(char *fpath, char *fname, struct stat *sb)
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
			dprintf(fd, "    File: '%s' -> ", unescape(fname));
			dprintf(fd, "'%s'", unescape(g_buf));
			xstrlcpy(g_buf, "symbolic link", MAX_CMD_LEN);
		}
	} else
		dprintf(fd, "    File: '%s'", unescape(fname));

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
		dprintf(fd, " Device type: %x,%x", major(sb->st_rdev), minor(sb->st_rdev));

	/* Show permissions, owner, group */
	dprintf(fd, "\n  Access: 0%d%d%d/%s Uid: (%u/%s)  Gid: (%u/%s)", (sb->st_mode >> 6) & 7, (sb->st_mode >> 3) & 7,
		sb->st_mode & 7, perms, sb->st_uid, (getpwuid(sb->st_uid))->pw_name, sb->st_gid, (getgrgid(sb->st_gid))->gr_name);

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

				++p;
			}
			dprintf(fd, " %s", begin);
		}

		dprintf(fd, "\n\n");
	} else
		dprintf(fd, "\n\n\n");

	close(fd);

	exitcurses();
	get_output(NULL, 0, "cat", tmp, NULL, 1);
	unlink(tmp);
	initcurses();
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

static size_t
get_fs_free(char *path)
{
	static struct statvfs svb;

	if (statvfs(path, &svb) == -1)
		return 0;
	else
		return svb.f_bavail << getorder(svb.f_frsize);
}

static size_t
get_fs_capacity(char *path)
{
	struct statvfs svb;

	if (statvfs(path, &svb) == -1)
		return 0;
	else
		return svb.f_blocks << getorder(svb.f_bsize);
}

static int
show_mediainfo(char *fpath, char *arg)
{
	if (!get_output(g_buf, MAX_CMD_LEN, "which", metaviewer, NULL, 0))
		return -1;

	exitcurses();
	get_output(NULL, 0, metaviewer, fpath, arg, 1);
	initcurses();
	return 0;
}

/*
 * The help string tokens (each line) start with a HEX value
 * which indicates the number of spaces to print before the
 * particular token. This method was chosen instead of a flat
 * string because the number of bytes in help was increasing
 * the binary size by around a hundred bytes. This would only
 * have increased as we keep adding new options.
 */
static int
show_help(char *path)
{
	char tmp[] = "/tmp/nnnXXXXXX";
	int i = 0, fd = mkstemp(tmp);
	char *start, *end;
	static char helpstr[] = (
           "cKey | Function\n"
             "e- + -\n"
      "7↑, k, ^P | Previous entry\n"
      "7↓, j, ^N | Next entry\n"
      "7PgUp, ^U | Scroll half page up\n"
      "7PgDn, ^D | Scroll half page down\n"
"1Home, g, ^, ^A | Jump to first entry\n"
 "2End, G, $, ^E | Jump to last entry\n"
   "4→, ↵, l, ^M | Open file or enter dir\n"
"1←, Bksp, h, ^H | Go to parent dir\n"
        "9Insert | Toggle navigate-as-you-type\n"
             "e~ | Jump to HOME dir\n"
             "e& | Jump to initial dir\n"
             "e- | Jump to last visited dir\n"
             "e/ | Filter dir contents\n"
            "d^/ | Open desktop search tool\n"
             "e. | Toggle hide .dot files\n"
             "eb | Show bookmark key prompt\n"
            "d^B | Mark current dir\n"
            "d^V | Visit marked dir\n"
             "ec | Show change dir prompt\n"
             "ed | Toggle detail view\n"
             "eD | Show current file details\n"
             "em | Show concise media info\n"
             "eM | Show full media info\n"
            "d^R | Rename selected entry\n"
             "es | Toggle sort by file size\n"
             "eS | Toggle disk usage mode\n"
             "et | Toggle sort by mtime\n"
             "e! | Spawn SHELL in current dir\n"
             "ee | Edit entry in EDITOR\n"
             "eo | Open dir in file manager\n"
             "ep | Open entry in PAGER\n"
            "d^K | Invoke file path copier\n"
            "d^L | Force a redraw, unfilter\n"
             "e? | Show help, settings\n"
             "eQ | Quit and change dir\n"
         "aq, ^Q | Quit\n\n");

	if (fd == -1)
		return -1;

	start = end = helpstr;
	while (*end) {
		while (*end != '\n')
			++end;

		if (start == end) {
			++end;
			continue;
		}

		dprintf(fd, "%*c%.*s", xchartohex(*start), ' ', (int)(end - start), start + 1);
		start = ++end;
	}

	dprintf(fd, "\n");

	if (getenv("NNN_BMS")) {
		dprintf(fd, "BOOKMARKS\n");
		for (; i < BM_MAX; ++i)
			if (bookmark[i].key)
				dprintf(fd, " %s: %s\n",
					bookmark[i].key, bookmark[i].loc);
			else
				break;
		dprintf(fd, "\n");
	}

	if (editor)
		dprintf(fd, "NNN_USE_EDITOR: %s\n", editor);

	if (desktop_manager)
		dprintf(fd, "NNN_DE_FILE_MANAGER: %s\n", desktop_manager);

	if (idletimeout)
		dprintf(fd, "NNN_IDLE_TIMEOUT: %d secs\n", idletimeout);

	if (copier)
		dprintf(fd, "NNN_COPIER: %s\n", copier);

	dprintf(fd, "\nVolume: %s of ", coolsize(get_fs_free(path)));
	dprintf(fd, "%s free\n", coolsize(get_fs_capacity(path)));

	dprintf(fd, "\n");
	close(fd);

	exitcurses();
	get_output(NULL, 0, "cat", tmp, NULL, 1);
	unlink(tmp);
	initcurses();
	return 0;
}

static int
sum_bsizes(const char *fpath, const struct stat *sb,
	   int typeflag, struct FTW *ftwbuf)
{
	if (sb->st_blocks && (typeflag == FTW_F || typeflag == FTW_D))
		ent_blocks += sb->st_blocks;

	++num_files;
	return 0;
}

static int
dentfill(char *path, struct entry **dents,
	 int (*filter)(regex_t *, char *), regex_t *re)
{
	static DIR *dirp;
	static struct dirent *dp;
	static struct stat sb_path, sb;
	static int fd, n;
	static char *namep;
	static ulong num_saved;
	static struct entry *dentp;

	dirp = opendir(path);
	if (dirp == NULL)
		return 0;

	fd = dirfd(dirp);

	n = 0;

	if (cfg.blkorder) {
		num_files = 0;
		dir_blocks = 0;

		if (fstatat(fd, ".", &sb_path, 0) == -1) {
			printwarn();
			return 0;
		}
	}

	while ((dp = readdir(dirp)) != NULL) {
		namep = dp->d_name;

		if (filter(re, namep) == 0) {
			if (!cfg.blkorder)
				continue;

			/* Skip self and parent */
			if ((namep[0] == '.' && (namep[1] == '\0' || (namep[1] == '.' && namep[2] == '\0'))))
				continue;

			if (fstatat(fd, namep, &sb, AT_SYMLINK_NOFOLLOW)
					== -1)
				continue;

			if (S_ISDIR(sb.st_mode)) {
				if (sb_path.st_dev == sb.st_dev) {
					ent_blocks = 0;
					mkpath(path, namep, g_buf, PATH_MAX);

					if (nftw(g_buf, sum_bsizes, open_max, FTW_MOUNT | FTW_PHYS) == -1) {
						printmsg(STR_NFTWFAIL);
						dir_blocks += sb.st_blocks;
					} else
						dir_blocks += ent_blocks;
				}
			} else {
				if (sb.st_blocks)
					dir_blocks += sb.st_blocks;

				++num_files;
			}

			continue;
		}

		/* Skip self and parent */
		if ((namep[0] == '.' && (namep[1] == '\0' ||
		    (namep[1] == '.' && namep[2] == '\0'))))
			continue;

		if (fstatat(fd, namep, &sb, AT_SYMLINK_NOFOLLOW) == -1) {
			if (*dents)
				free(*dents);
			errexit();
		}

		if (n == total_dents) {
			total_dents += 64;
			*dents = realloc(*dents, total_dents * sizeof(**dents));
			if (*dents == NULL)
				errexit();
		}

		dentp = &(*dents)[n];
		xstrlcpy(dentp->name, namep, NAME_MAX);

		dentp->mode = sb.st_mode;
		dentp->t = sb.st_mtime;
		dentp->size = sb.st_size;

		if (cfg.blkorder) {
			if (S_ISDIR(sb.st_mode)) {
				ent_blocks = 0;
				num_saved = num_files + 1;
				mkpath(path, namep, g_buf, PATH_MAX);

				if (nftw(g_buf, sum_bsizes, open_max, FTW_MOUNT | FTW_PHYS) == -1) {
					printmsg(STR_NFTWFAIL);
					dentp->blocks = sb.st_blocks;
				} else
					dentp->blocks = ent_blocks;

				if (sb_path.st_dev == sb.st_dev)
					dir_blocks += dentp->blocks;
				else
					num_files = num_saved;
			} else {
				dentp->blocks = sb.st_blocks;
				dir_blocks += dentp->blocks;
				++num_files;
			}
		}

		++n;
	}

	/* Should never be null */
	if (closedir(dirp) == -1) {
		if (*dents)
			free(*dents);
		errexit();
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

	for (i = 0; i < n; ++i)
		if (xstrcmp(p, dents[i].name) == 0)
			return i;

	return 0;
}

static int
populate(char *path, char *oldpath, char *fltr)
{
	static regex_t re;

	/* Can fail when permissions change while browsing.
	 * It's assumed that path IS a directory when we are here.
	 */
	if (access(path, R_OK) == -1)
		return -1;

	/* Search filter */
	if (setfilter(&re, fltr) != 0)
		return -1;

	if (cfg.blkorder) {
		printmsg("Calculating...");
		refresh();
	}

	ndents = dentfill(path, &dents, visible, &re);

	qsort(dents, ndents, sizeof(*dents), entrycmp);

	/* Find cur from history */
	cur = dentfind(dents, ndents, oldpath);
	return 0;
}

static void
redraw(char *path)
{
	static int nlines, i;
	static size_t ncols;

	nlines = MIN(LINES - 4, ndents);

	/* Clean screen */
	erase();

	/* Strip trailing slashes */
	for (i = xstrlen(path) - 1; i > 0; --i)
		if (path[i] == '/')
			path[i] = '\0';
		else
			break;

	DPRINTF_D(cur);
	DPRINTF_S(path);

	/* No text wrapping in cwd line */
	if (!realpath(path, g_buf)) {
		printwarn();
		return;
	}

	ncols = COLS;
	if (ncols > PATH_MAX)
		ncols = PATH_MAX;
	/* - xstrlen(CWD) - 1 = 6 */
	g_buf[ncols - 6] = '\0';
	printw(CWD "%s\n\n", g_buf);

	if (cfg.showcolor) {
		attron(COLOR_PAIR(1) | A_BOLD);
		cfg.dircolor = 1;
	}

	/* Print listing */
	if (cur < (nlines >> 1)) {
		for (i = 0; i < nlines; ++i)
			printptr(&dents[i], i == cur);
	} else if (cur >= ndents - (nlines >> 1)) {
		for (i = ndents - nlines; i < ndents; ++i)
			printptr(&dents[i], i == cur);
	} else {
		static int odd;

		odd = ISODD(nlines);
		nlines >>= 1;
		for (i = cur - nlines; i < cur + nlines + odd; ++i)
			printptr(&dents[i], i == cur);
	}

	/* Must reset e.g. no files in dir */
	if (cfg.dircolor) {
		attroff(COLOR_PAIR(1) | A_BOLD);
		cfg.dircolor = 0;
	}

	if (cfg.showdetail) {
		if (ndents) {
			static char ind[2] = "\0\0";
			static char sort[9];

			if (cfg.mtimeorder)
				sprintf(sort, "by time ");
			else if (cfg.sizeorder)
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
			else if (dents[cur].mode & 0100)
				ind[0] = '*';
			else
				ind[0] = '\0';

			/* We need to show filename as it may
			 * be truncated in directory listing
			 */
			if (!cfg.blkorder)
				sprintf(g_buf, "total %d %s[%s%s]", ndents, sort, unescape(dents[cur].name), ind);
			else {
				i = sprintf(g_buf, "du: %s (%lu files) ", coolsize(dir_blocks << 9), num_files);
				sprintf(g_buf + i, "vol: %s free [%s%s]", coolsize(get_fs_free(path)), unescape(dents[cur].name), ind);
			}

			printmsg(g_buf);
		} else
			printmsg("0 items");
	}
}

static void
browse(char *ipath, char *ifilter)
{
	static char path[PATH_MAX], oldpath[PATH_MAX], newpath[PATH_MAX], lastdir[PATH_MAX], mark[PATH_MAX];
	static char fltr[LINE_MAX];
	char *dir, *tmp, *run, *env, *dstdir = NULL;
	struct stat sb;
	int r, fd, presel;
	enum action sel = SEL_RUNARG + 1;

	xstrlcpy(path, ipath, PATH_MAX);
	xstrlcpy(fltr, ifilter, LINE_MAX);
	oldpath[0] = newpath[0] = lastdir[0] = mark[0] = '\0';

	if (cfg.filtermode)
		presel = FILTER;
	else
		presel = 0;

begin:
#ifdef LINUX_INOTIFY
	if (inotify_wd >= 0)
		inotify_rm_watch(inotify_fd, inotify_wd);
#elif defined(BSD_KQUEUE)
	if (event_fd >= 0)
		close(event_fd);
#endif

	if (populate(path, oldpath, fltr) == -1) {
		printwarn();
		goto nochange;
	}

#ifdef LINUX_INOTIFY
	inotify_wd = inotify_add_watch(inotify_fd, path, INOTIFY_MASK);
#elif defined(BSD_KQUEUE)
	event_fd = open(path, O_EVTONLY);
	if (event_fd >= 0)
		EV_SET(&events_to_monitor[0], event_fd, EVFILT_VNODE, EV_ADD | EV_CLEAR, KQUEUE_FFLAGS, 0, path);
#endif

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

			tmp = getenv("NNN_TMPFILE");
			if (tmp)
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
			if (istopdir(path)) {
				printmsg(STR_ATROOT);
				goto nochange;
			}

			dir = xdirname(path);
			if (access(dir, R_OK) == -1) {
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
			if (cfg.filtermode)
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
				if (access(newpath, R_OK) == -1) {
					printwarn();
					goto nochange;
				}

				/* Save last working directory */
				xstrlcpy(lastdir, path, PATH_MAX);

				xstrlcpy(path, newpath, PATH_MAX);
				oldpath[0] = '\0';
				/* Reset filter */
				xstrlcpy(fltr, ifilter, LINE_MAX);
				if (cfg.filtermode)
					presel = FILTER;
				goto begin;
			case S_IFREG:
			{
				/* If NNN_USE_EDITOR is set,
				 * open text in EDITOR
				 */
				if (editor) {
					if (getmime(dents[cur].name)) {
						spawn(editor, newpath, NULL, NULL, F_NORMAL);
						continue;
					}

					/* Recognize and open plain
					 * text files with vi
					 */
					if (get_output(g_buf, MAX_CMD_LEN, "file", "-bi", newpath, 0) == NULL)
						continue;

					if (strstr(g_buf, "text/") == g_buf) {
						spawn(editor, newpath, NULL, NULL, F_NORMAL);
						continue;
					}
				}

				/* Invoke desktop opener as last resort */
				spawn(utils[0], newpath, NULL, NULL, F_NOTRACE);
				continue;
			}
			default:
				printmsg("Unsupported file");
				goto nochange;
			}
		case SEL_FLTR:
			presel = filterentries(path);
			xstrlcpy(fltr, ifilter, LINE_MAX);
			DPRINTF_S(fltr);
			/* Save current */
			if (ndents > 0)
				mkpath(path, dents[cur].name, oldpath, PATH_MAX);
			goto nochange;
		case SEL_MFLTR:
			cfg.filtermode ^= 1;
			if (cfg.filtermode)
				presel = FILTER;
			else
				printmsg("navigate-as-you-type off");
			goto nochange;
		case SEL_SEARCH:
			spawn(player, path, "search", NULL, F_NORMAL);
			break;
		case SEL_NEXT:
			if (cur < ndents - 1)
				++cur;
			else if (ndents)
				/* Roll over, set cursor to first entry */
				cur = 0;
			break;
		case SEL_PREV:
			if (cur > 0)
				--cur;
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
			char *input;
			int truecd;

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
					printmsg(STR_NOHOME);
					goto nochange;
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

				/* Show a message if already at / */
				if (istopdir(path)) {
					printmsg(STR_ATROOT);
					free(input);
					goto nochange;
				}

				--r; /* One . for the current dir */
				dir = path;

				/* Note: fd is used as a tmp variable here */
				for (fd = 0; fd < r; ++fd) {
					/* Reached / ? */
					if (istopdir(path)) {
						/* Can't cd beyond / */
						break;
					}

					dir = xdirname(dir);
					if (access(dir, R_OK) == -1) {
						printwarn();
						free(input);
						goto nochange;
					}
				}

				truecd = 1;

				/* Save the path in case of cd ..
				 * We mark the current dir in parent dir
				 */
				if (r == 1) {
					xstrlcpy(oldpath, path, PATH_MAX);
					truecd = 2;
				}

				xstrlcpy(newpath, dir, PATH_MAX);
			} else
				mkpath(path, tmp, newpath, PATH_MAX);

			free(input);

			if (!xdiraccess(newpath))
				goto nochange;

			if (truecd == 0) {
				/* Probable change in dir */
				/* No-op if it's the same directory */
				if (xstrcmp(path, newpath) == 0)
					break;

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
			if (cfg.filtermode)
				presel = FILTER;
			goto begin;
		}
		case SEL_CDHOME:
			dstdir = getenv("HOME");
			if (dstdir == NULL) {
				clearprompt();
				goto nochange;
			} // fallthrough
		case SEL_CDBEGIN:
			if (!dstdir)
				dstdir = ipath;

			if (!xdiraccess(dstdir)) {
				dstdir = NULL;
				goto nochange;
			}

			if (xstrcmp(path, dstdir) == 0) {
				dstdir = NULL;
				break;
			}

			/* Save last working directory */
			xstrlcpy(lastdir, path, PATH_MAX);

			xstrlcpy(path, dstdir, PATH_MAX);
			oldpath[0] = '\0';
			/* Reset filter */
			xstrlcpy(fltr, ifilter, LINE_MAX);
			DPRINTF_S(path);
			if (cfg.filtermode)
				presel = FILTER;
			dstdir = NULL;
			goto begin;
		case SEL_CDLAST: // fallthrough
		case SEL_VISIT:
			if (sel == SEL_VISIT) {
				if (xstrcmp(mark, path) == 0)
					break;

				tmp = mark;
			} else
				tmp = lastdir;

			if (tmp[0] == '\0') {
				printmsg("Not set...");
				goto nochange;
			}

			if (!xdiraccess(tmp))
				goto nochange;

			xstrlcpy(newpath, tmp, PATH_MAX);
			xstrlcpy(lastdir, path, PATH_MAX);
			xstrlcpy(path, newpath, PATH_MAX);
			oldpath[0] = '\0';
			/* Reset filter */
			xstrlcpy(fltr, ifilter, LINE_MAX);
			DPRINTF_S(path);
			if (cfg.filtermode)
				presel = FILTER;
			goto begin;
		case SEL_CDBM:
			printprompt("key: ");
			tmp = readinput();
			clearprompt();
			if (tmp == NULL)
				break;

			for (r = 0; bookmark[r].key && r < BM_MAX; ++r) {
				if (xstrcmp(bookmark[r].key, tmp) == -1)
					continue;

				if (bookmark[r].loc[0] == '~') {
					/* Expand ~ to HOME */
					char *home = getenv("HOME");

					if (home)
						snprintf(newpath, PATH_MAX, "%s%s", home, bookmark[r].loc + 1);
					else {
						printmsg(STR_NOHOME);
						goto nochange;
					}
				} else
					mkpath(path, bookmark[r].loc,
					       newpath, PATH_MAX);

				if (!xdiraccess(newpath))
					goto nochange;

				if (xstrcmp(path, newpath) == 0)
					break;

				oldpath[0] = '\0';
				break;
			}

			if (!bookmark[r].key) {
				printmsg("No matching bookmark");
				goto nochange;
			}

			/* Save last working directory */
			xstrlcpy(lastdir, path, PATH_MAX);

			/* Save the newly opted dir in path */
			xstrlcpy(path, newpath, PATH_MAX);

			/* Reset filter */
			xstrlcpy(fltr, ifilter, LINE_MAX);
			DPRINTF_S(path);

			if (cfg.filtermode)
				presel = FILTER;
			goto begin;
		case SEL_MARK:
			xstrlcpy(mark, path, PATH_MAX);
			printmsg(mark);
			goto nochange;
		case SEL_TOGGLEDOT:
			cfg.showhidden ^= 1;
			initfilter(cfg.showhidden, &ifilter);
			xstrlcpy(fltr, ifilter, LINE_MAX);
			goto begin;
		case SEL_DETAIL:
			cfg.showdetail ^= 1;
			cfg.showdetail ? (printptr = &printent_long) : (printptr = &printent);
			/* Save current */
			if (ndents > 0)
				mkpath(path, dents[cur].name, oldpath, PATH_MAX);
			goto begin;
		case SEL_STATS:
			if (ndents > 0) {
				mkpath(path, dents[cur].name, oldpath, PATH_MAX);

				r = lstat(oldpath, &sb);
				if (r == -1) {
					if (dents)
						dentfree(dents);
					errexit();
				} else {
					r = show_stats(oldpath, dents[cur].name, &sb);
					if (r < 0) {
						printwarn();
						goto nochange;
					}
				}
			}
			break;
		case SEL_MEDIA: // fallthrough
		case SEL_FMEDIA:
			if (ndents > 0) {
				mkpath(path, dents[cur].name, oldpath, PATH_MAX);

				if (show_mediainfo(oldpath, run) == -1) {
					sprintf(g_buf, "%s missing", metaviewer);
					printmsg(g_buf);
					goto nochange;
				}
			}
			break;
		case SEL_DFB:
			if (!desktop_manager) {
				printmsg("NNN_DE_FILE_MANAGER not set");
				goto nochange;
			}

			spawn(desktop_manager, path, NULL, path, F_NOTRACE | F_NOWAIT);
			break;
		case SEL_FSIZE:
			cfg.sizeorder ^= 1;
			cfg.mtimeorder = 0;
			cfg.blkorder = 0;
			/* Save current */
			if (ndents > 0)
				mkpath(path, dents[cur].name, oldpath, PATH_MAX);
			goto begin;
		case SEL_BSIZE:
			cfg.blkorder ^= 1;
			if (cfg.blkorder) {
				cfg.showdetail = 1;
				printptr = &printent_long;
			}
			cfg.mtimeorder = 0;
			cfg.sizeorder = 0;
			/* Save current */
			if (ndents > 0)
				mkpath(path, dents[cur].name, oldpath, PATH_MAX);
			goto begin;
		case SEL_MTIME:
			cfg.mtimeorder ^= 1;
			cfg.sizeorder = 0;
			cfg.blkorder = 0;
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
				if (istopdir(path))
					snprintf(newpath, PATH_MAX, "/%s", dents[cur].name);
				else
					snprintf(newpath, PATH_MAX, "%s/%s", path, dents[cur].name);
				spawn(copier, newpath, NULL, NULL, F_NONE);
				printmsg(newpath);
			} else if (!copier)
				printmsg("NNN_COPIER is not set");
			goto nochange;
		case SEL_RENAME:
			if (ndents <= 0)
				break;

			printprompt("> ");
			tmp = xreadline(dents[cur].name);
			clearprompt();
			if (tmp == NULL || tmp[0] == '\0')
				break;

			/* Allow only relative paths */
			if (tmp[0] == '/' || basename(tmp) != tmp) {
				printmsg("relative paths only");
				goto nochange;
			}

			/* Skip renaming to same name */
			if (xstrcmp(tmp, dents[cur].name) == 0)
				break;

			/* Open the descriptor to currently open directory */
			fd = open(path, O_RDONLY | O_DIRECTORY);
			if (fd == -1) {
				printwarn();
				goto nochange;
			}

			/* Check if another file with same name exists */
			if (faccessat(fd, tmp, F_OK, AT_SYMLINK_NOFOLLOW) != -1) {
				/* File with the same name exists */
				printprompt("Press 'y' to overwrite: ");
				cleartimeout();
				r = getch();
				settimeout();
				if (r != 'y') {
					close(fd);
					break;
				}
			}

			/* Rename the file */
			r = renameat(fd, dents[cur].name, fd, tmp);
			if (r != 0) {
				printwarn();
				close(fd);
				goto nochange;
			}

			close(fd);
			mkpath(path, tmp, oldpath, PATH_MAX);
			goto begin;
		case SEL_HELP:
			show_help(path);
			break;
		case SEL_RUN:
			run = xgetenv(env, run);
			spawn(run, NULL, NULL, path, F_NORMAL | F_MARKER);
			/* Repopulate as directory content may have changed */
			goto begin;
		case SEL_RUNARG:
			run = xgetenv(env, run);
			spawn(run, dents[cur].name, NULL, path, F_NORMAL);
			break;
		}
		/* Screensaver */
		if (idletimeout != 0 && idle == idletimeout) {
			idle = 0;
			spawn(player, "", "screensaver", NULL, F_NORMAL | F_SIGINT);
		}
	}
}

static void
usage(void)
{
	printf("usage: nnn [-c N] [-e] [-i] [-l] [-p nlay] [-S]\n\
           [-v] [-h] [PATH]\n\n\
The missing terminal file browser for X.\n\n\
positional arguments:\n\
  PATH   directory to open [default: current dir]\n\n\
optional arguments:\n\
 -c N    specify dir color, disables if N>7\n\
 -e      use exiftool instead of mediainfo\n\
 -i      start in navigate-as-you-type mode\n\
 -l      start in light mode (fewer details)\n\
 -p nlay path to custom nlay\n\
 -S      start in disk usage analyzer mode\n\
 -v      show program version and exit\n\
 -h      show this help and exit\n\n\
Version: %s\n\
License: BSD 2-Clause\n\
Webpage: https://github.com/jarun/nnn\n", VERSION);
	exit(0);
}

int
main(int argc, char *argv[])
{
	static char cwd[PATH_MAX];
	char *ipath, *ifilter, *bmstr;
	int opt;

	/* Confirm we are in a terminal */
	if (!isatty(0) || !isatty(1)) {
		fprintf(stderr, "stdin or stdout is not a tty\n");
		exit(1);
	}

	while ((opt = getopt(argc, argv, "Slic:ep:vh")) != -1) {
		switch (opt) {
		case 'S':
			cfg.blkorder = 1;
			break;
		case 'l':
			cfg.showdetail = 0;
			printptr = &printent;
			break;
		case 'i':
			cfg.filtermode = 1;
			break;
		case 'c':
			color = (uchar)atoi(optarg);
			if (color > 7)
				cfg.showcolor = 0;
			break;
		case 'e':
			metaviewer = utils[3];
			break;
		case 'p':
			player = optarg;
			break;
		case 'v':
			printf("%s\n", VERSION);
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

	/* Increase current open file descriptor limit */
	open_max = max_openfds();

	if (getuid() == 0)
		cfg.showhidden = 1;
	initfilter(cfg.showhidden, &ifilter);

#ifdef LINUX_INOTIFY
	/* Initialize inotify */
	inotify_fd = inotify_init1(IN_NONBLOCK);
	if (inotify_fd < 0) {
		fprintf(stderr, "Cannot initialize inotify: %s\n", strerror(errno));
		exit(1);
	}
#elif defined(BSD_KQUEUE)
	kq = kqueue();
	if (kq < 0) {
		fprintf(stderr, "Cannot initialize kqueue: %s\n", strerror(errno));
		exit(1);
	}

	gtimeout.tv_sec = 0;
	gtimeout.tv_nsec = 50; /* 50 ns delay */
#endif

	/* Parse bookmarks string, if available */
	bmstr = getenv("NNN_BMS");
	if (bmstr)
		parsebmstr(bmstr);

	/* Edit text in EDITOR, if opted */
	if (getenv("NNN_USE_EDITOR"))
		editor = xgetenv("EDITOR", "vi");

	/* Set metadata viewer if not set */
	if (!metaviewer)
		metaviewer = utils[2];

	/* Set player if not set already */
	if (!player)
		player = utils[1];

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
	if (!xdiraccess(ipath)) {
		fprintf(stderr, "%s: %s\n", ipath, strerror(errno));
		exit(1);
	}

	/* Set locale */
	setlocale(LC_ALL, "");
#ifdef DEBUGMODE
	enabledbg();
#endif
	initcurses();
	browse(ipath, ifilter);
	exitcurses();

#ifdef LINUX_INOTIFY
	/* Shutdown inotify */
	if (inotify_wd >= 0)
		inotify_rm_watch(inotify_fd, inotify_wd);
	close(inotify_fd);
#elif defined(BSD_KQUEUE)
	if (event_fd >= 0)
		close(event_fd);
	close(kq);
#endif

#ifdef DEBUGMODE
	disabledbg();
#endif
	exit(0);
}
