/* See LICENSE file for copyright and license details. */

/*
 * Visual layout:
 * .---------
 * | DIR: /mnt/path
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
#ifdef __i386__
#define _FILE_OFFSET_BITS 64 /* Support large files on 32-bit Linux */
#endif
#include <sys/inotify.h>
#define LINUX_INOTIFY
#if !defined(__GLIBC__)
#include <sys/types.h>
#endif
#endif
#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/statvfs.h>
#if defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__) || defined(__APPLE__)
#include <sys/types.h>
#include <sys/event.h>
#include <sys/time.h>
#define BSD_KQUEUE
#else
#include <sys/sysmacros.h>
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
#ifndef __USE_XOPEN /* Fix failure due to wcswidth(), ncursesw/curses.h includes whcar.h on Ubuntu 14.04 */
#define __USE_XOPEN
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
#include <readline/history.h>
#include <readline/readline.h>
#ifndef __USE_XOPEN_EXTENDED
#define __USE_XOPEN_EXTENDED 1
#endif
#include <ftw.h>
#include <wchar.h>

#include "nnn.h"

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
#define DPRINTF_P(x) xprintf(DEBUG_FD, #x "=%p\n", x)
#else
#define DPRINTF_D(x)
#define DPRINTF_U(x)
#define DPRINTF_S(x)
#define DPRINTF_P(x)
#endif /* DEBUGMODE */

/* Macro definitions */
#define VERSION "1.5"
#define GENERAL_INFO "License: BSD 2-Clause\nWebpage: https://github.com/jarun/nnn"

#define LEN(x) (sizeof(x) / sizeof(*(x)))
#undef MIN
#define MIN(x, y) ((x) < (y) ? (x) : (y))
#define ISODD(x) ((x) & 1)
#define TOUPPER(ch) \
	(((ch) >= 'a' && (ch) <= 'z') ? ((ch) - 'a' + 'A') : (ch))
#define MAX_CMD_LEN 5120
#define CWD   "DIR: "
#define CURSR " > "
#define EMPTY "   "
#define CURSYM(flag) (flag ? CURSR : EMPTY)
#define FILTER '/'
#define REGEX_MAX 128
#define BM_MAX 10
#define ENTRY_INCR 64 /* Number of dir 'entry' structures to allocate per shot */
#define NAMEBUF_INCR 0x1000 /* 64 dir entries at a time, avg. 64 chars per filename = 64*64B = 4KB */

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
#define copyfilter() xstrlcpy(fltr, ifilter, NAME_MAX)
#define copycurname() xstrlcpy(oldname, dents[cur].name, NAME_MAX + 1)
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
	char *name;
	time_t t;
	off_t size;
	blkcnt_t blocks; /* number of 512B blocks allocated */
	mode_t mode;
	uint nlen; /* Length of file name; can be uchar (< NAME_MAX + 1) */
} *pEntry;

/* Bookmark */
typedef struct {
	char *key;
	char *loc;
} bm;

/* Settings */
typedef struct {
	ushort filtermode : 1;  /* Set to enter filter mode */
	ushort mtimeorder : 1;  /* Set to sort by time modified */
	ushort sizeorder  : 1;  /* Set to sort by file size */
	ushort blkorder   : 1;  /* Set to sort by blocks used (disk usage) */
	ushort showhidden : 1;  /* Set to show hidden files */
	ushort showdetail : 1;  /* Clear to show fewer file info */
	ushort showcolor  : 1;  /* Set to show dirs in blue */
	ushort dircolor   : 1;  /* Current status of dir color */
	ushort metaviewer : 1;  /* Index of metadata viewer in utils[] */
	ushort color      : 3;  /* Color code for directories */
} settings;

/* GLOBALS */

/* Configuration */
static settings cfg = {0, 0, 0, 0, 0, 1, 1, 0, 0, 4};

static struct entry *dents;
static char *pnamebuf;
static int ndents, cur, total_dents;
static uint idle;
static uint idletimeout;
static char *player;
static char *copier;
static char *editor;
static char *desktop_manager;
static char nowait = F_NOTRACE;
static blkcnt_t ent_blocks;
static blkcnt_t dir_blocks;
static ulong num_files;
static uint open_max;
static bm bookmark[BM_MAX];

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
	"mediainfo",
	"exiftool",
#ifdef __APPLE__
	"/usr/bin/open",
#else
	"/usr/bin/xdg-open",
#endif
	"nlay",
	"atool"
};

/* Common message strings */
static const char *STR_NFTWFAIL = "nftw(3) failed";
static const char *STR_ATROOT = "You are at /";
static const char *STR_NOHOME = "HOME not set";
static const char *STR_INPUT = "No traversal delimiter allowed";
static const char *STR_INVBM = "Invalid bookmark";

/* For use in functions which are isolated and don't return the buffer */
static char g_buf[MAX_CMD_LEN];

/* Forward declarations */
static void redraw(char *path);

/* Functions */

/* Messages show up at the bottom */
static void
printmsg(const char *msg)
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
		 * 20K is arbitrary. If the limit is set to max possible
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
	static const uint lsize = sizeof(ulong);
	static const uint _WSHIFT = (sizeof(ulong) == 8) ? 3 : 2;

	if (!src || !dest)
		return 0;

	len = xstrlen(src) + 1;
	if (n > len)
		n = len;
	else if (len > n)
		/* Save total number of bytes to copy in len */
		len = n;

	if (n >= lsize) {
		blocks = n >> _WSHIFT;
		n -= (blocks << _WSHIFT);
	} else
		blocks = 0;

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
 * Returns 0 if same, -ve if s1 < s2, +ve if s1 > s2.
 */
static int
xstrcmp(const char *s1, const char *s2)
{
	if (!s1 || !s2)
		return -1;

	while (*s1 && *s1 == *s2)
		++s1, ++s2;

	return *s1 - *s2;
}

/*
 * The poor man's implementation of memrchr(3).
 * We are only looking for '/' in this program.
 * And we are NOT expecting a '/' at the end.
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
	static char * const buf = g_buf, *last_slash, *runp;

	xstrlcpy(buf, path, PATH_MAX);

	/* Find last '/'. */
	last_slash = xmemrchr((uchar *)buf, '/', xstrlen(buf));

	if (last_slash != NULL && last_slash != buf && last_slash[1] == '\0') {
		/* Determine whether all remaining characters are slashes. */
		for (runp = last_slash; runp != buf; --runp)
			if (runp[-1] != '/')
				break;

		/* The '/' is the last character, we have to look further. */
		if (runp != buf)
			last_slash = xmemrchr((uchar *)buf, '/', runp - buf);
	}

	if (last_slash != NULL) {
		/* Determine whether all remaining characters are slashes. */
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

static char *
xbasename(char *path)
{
	static char *base;

	base = xmemrchr((uchar *)path, '/', xstrlen(path));
	return base ? base + 1 : path;
}

/*
 * Return number of dots if all chars in a string are dots, else 0
 */
static int
all_dots(const char *path)
{
	int count = 0;

	if (!path)
		return FALSE;

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
		init_pair(1, cfg.color, -1);
	settimeout(); /* One second */
}

/*
 * Spawns a child process. Behaviour can be controlled using flag.
 * Limited to 2 arguments to a program, flag works on bit set.
 */
static void
spawn(const char *file, const char *arg1, const char *arg2, const char *dir, uchar flag)
{
	static char *shlvl;
	static pid_t pid;
	static int status;

	if (flag & F_NORMAL)
		exitcurses();

	pid = fork();
	if (pid == 0) {
		if (dir != NULL)
			status = chdir(dir);

		shlvl = getenv("SHLVL");

		/* Show a marker (to indicate nnn spawned shell) */
		if (flag & F_MARKER && shlvl != NULL) {
			printf("\n +-++-++-+\n | n n n |\n +-++-++-+\n\n");
			printf("Spawned shell level: %d\n", atoi(shlvl) + 1);
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
xgetenv(const char *name, char *fallback)
{
	static char *value;

	if (name == NULL)
		return fallback;

	value = getenv(name);

	return value && value[0] ? value : fallback;
}

/* Check if a dir exists, IS a dir and is readable */
static bool
xdiraccess(const char *path)
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
xstricmp(const char * const s1, const char * const s2)
{
	static const char *c1, *c2;

	c1 = s1;
	while (isspace(*c1))
		++c1;

	c2 = s2;
	while (isspace(*c2))
		++c2;

	if (*c1 == '-' || *c1 == '+')
		++c1;

	if (*c2 == '-' || *c2 == '+')
		++c2;

	if (isdigit(*c1) && isdigit(*c2)) {
		while (*c1 >= '0' && *c1 <= '9')
			++c1;
		while (isspace(*c1))
			++c1;

		while (*c2 >= '0' && *c2 <= '9')
			++c2;
		while (isspace(*c2))
			++c2;
	}

	if (!*c1 && !*c2) {
		static long long num1, num2;

		num1 = strtoll(s1, NULL, 10);
		num2 = strtoll(s2, NULL, 10);
		if (num1 != num2) {
			if (num1 > num2)
				return 1;
			else
				return -1;
		}
	}

	return strcoll(s1, s2);
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
getmime(const char *file)
{
	static regex_t regex;
	static uint i;
	static const uint len = LEN(assocs);

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
		if (len > NAME_MAX)
			len = NAME_MAX;
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
	static uint i;
	static const uint len = LEN(bindings);
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
	static struct entry _dent, *dentp1, *dentp2;

	for (count = 0; count < ndents; ++count) {
		if (filter(re, (*dents)[count].name) == 0) {
			if (count != --ndents) {
				dentp1 = &(*dents)[count];
				dentp2 = &(*dents)[ndents];

				memcpy(&_dent, dentp1, sizeof(struct entry));
				memcpy(dentp1, dentp2, sizeof(struct entry));
				memcpy(dentp2, &_dent, sizeof(struct entry));
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
	int r, total = ndents, oldcur = cur, len = 1;
	char *pln = ln + 1;

	ln[0] = wln[0] = FILTER;
	ln[1] = wln[1] = '\0';
	cur = 0;

	cleartimeout();
	echo();
	curs_set(TRUE);
	printprompt(ln);

	while ((r = get_wch(ch)) != ERR) {
		if (*ch == 127 /* handle DEL */ || *ch == KEY_DC || *ch == KEY_BACKSPACE) {
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
			continue;
		}

		if (r == OK) {
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
			case CONTROL('L'):
				if (len == 1)
					cur = oldcur; // fallthrough
			case CONTROL('Q'):
				goto end;
			default:
				/* Reset cur in case it's a repeat search */
				if (len == 1)
					cur = 0;

				if (len == REGEX_MAX - 1)
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
		} else {
			if (len == 1)
				cur = oldcur;
			goto end;
		}
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
	int x, y, r;
	wint_t ch[2] = {0};
	wchar_t *buf = (wchar_t *)g_buf;
	size_t buflen = NAME_MAX - 1;

	if (fname) {
		DPRINTF_S(fname);
		len = pos = mbstowcs(buf, fname, NAME_MAX);
	} else
		len = (size_t)-1;

	if (len == (size_t)-1) {
		buf[0] = '\0';
		len = pos = 0;
	}

	getyx(stdscr, y, x);
	cleartimeout();

	while (1) {
		buf[len] = ' ';
		mvaddnwstr(y, x, buf, len + 1);
		move(y, x + wcswidth(buf, pos));

		if ((r = get_wch(ch)) != ERR) {
			if (r == OK) {
				if (*ch == KEY_ENTER || *ch == '\n' || *ch == '\r')
					break;

				if (*ch == CONTROL('L')) {
					clearprompt();
					len = pos = 0;
					continue;
				}

				/* TAB breaks cursor position, ignore it */
				if (*ch == TAB || *ch == '\t')
					continue;

				if (pos < buflen) {
					memmove(buf + pos + 1, buf + pos, (len - pos) << 2);
					buf[pos] = *ch;
					++len, ++pos;
					continue;
				}
			} else {
				switch (*ch) {
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
		}
	}

	buf[len] = '\0';
	if (old_curs != ERR) curs_set(old_curs);

	settimeout();
	DPRINTF_S(buf);
	wcstombs(g_buf, buf, NAME_MAX);
	return g_buf;
}

static char *
readinput(void)
{
	cleartimeout();
	echo();
	curs_set(TRUE);
	memset(g_buf, 0, NAME_MAX + 1);
	wgetnstr(stdscr, g_buf, NAME_MAX);
	noecho();
	curs_set(FALSE);
	settimeout();
	return g_buf[0] ? g_buf : NULL;
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

/*
 * Get the real path to a bookmark
 *
 * NULL is returned in case of no match, path resolution failure etc.
 * buf would be modified, so check return value before access
 */
static char *
get_bm_loc(char *key, char *buf)
{
	if (!key || !key[0])
		return NULL;

	for (int r = 0; bookmark[r].key && r < BM_MAX; ++r) {
		if (xstrcmp(bookmark[r].key, key) == 0) {
			if (bookmark[r].loc[0] == '~') {
				char *home = getenv("HOME");
				if (!home) {
					DPRINTF_S(STR_NOHOME);
					return NULL;
				}

				snprintf(buf, PATH_MAX, "%s%s", home, bookmark[r].loc + 1);
			} else
				xstrlcpy(buf, bookmark[r].loc, PATH_MAX);

			return buf;
		}
	}

	DPRINTF_S("Invalid key");
	return NULL;
}

static void
resetdircolor(mode_t mode)
{
	if (cfg.dircolor && !S_ISDIR(mode)) {
		attroff(COLOR_PAIR(1) | A_BOLD);
		cfg.dircolor = 0;
	}
}

/*
 * Replace escape characters in a string with '?'
 * Adjust string length to maxcols if > 0;
 *
 * Interestingly, note that buffer points to g_buf. What happens if
 * str also points to g_buf? In this case we assume that the caller
 * acknowledges that it's OK to lose the data in g_buf after this
 * call to unescape().
 * The API, on its part, first converts str to multibyte (after which
 * it doesn't touch str anymore). Only after that it starts modifying
 * buffer. This works like a phased operation.
 */
static char *
unescape(const char *str, uint maxcols)
{
	static wchar_t wbuf[PATH_MAX];
	static char *buffer;
	static wchar_t *buf;
	static size_t len;

	/* Convert multi-byte to wide char */
	len = mbstowcs(wbuf, str, PATH_MAX);

	buffer = g_buf;
	buffer[0] = '\0';
	buf = wbuf;

	if (maxcols && len > maxcols) {
		len = wcswidth(wbuf, len);

		if (len > maxcols)
			wbuf[maxcols] = 0;
	}

	while (*buf) {
		if (*buf <= '\x1f' || *buf == '\x7f')
			*buf = '\?';

		++buf;
	}

	/* Convert wide char to multi-byte */
	wcstombs(buffer, wbuf, PATH_MAX);
	return buffer;
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
printent(struct entry *ent, int sel, uint namecols)
{
	static char *pname;

	pname = unescape(ent->name, namecols);

	/* Directories are always shown on top */
	resetdircolor(ent->mode);

	if (S_ISDIR(ent->mode))
		printw("%s%s/\n", CURSYM(sel), pname);
	else if (S_ISLNK(ent->mode))
		printw("%s%s@\n", CURSYM(sel), pname);
	else if (S_ISSOCK(ent->mode))
		printw("%s%s=\n", CURSYM(sel), pname);
	else if (S_ISFIFO(ent->mode))
		printw("%s%s|\n", CURSYM(sel), pname);
	else if (ent->mode & 0100)
		printw("%s%s*\n", CURSYM(sel), pname);
	else
		printw("%s%s\n", CURSYM(sel), pname);
}

static void
printent_long(struct entry *ent, int sel, uint namecols)
{
	static char buf[18], *pname;

	strftime(buf, 18, "%d-%m-%Y %H:%M", localtime(&ent->t));
	pname = unescape(ent->name, namecols);

	/* Directories are always shown on top */
	resetdircolor(ent->mode);

	if (sel)
		attron(A_REVERSE);

	if (!cfg.blkorder) {
		if (S_ISDIR(ent->mode))
			printw("%s%-16.16s        /  %s/\n", CURSYM(sel), buf, pname);
		else if (S_ISLNK(ent->mode))
			printw("%s%-16.16s        @  %s@\n", CURSYM(sel), buf, pname);
		else if (S_ISSOCK(ent->mode))
			printw("%s%-16.16s        =  %s=\n", CURSYM(sel), buf, pname);
		else if (S_ISFIFO(ent->mode))
			printw("%s%-16.16s        |  %s|\n", CURSYM(sel), buf, pname);
		else if (S_ISBLK(ent->mode))
			printw("%s%-16.16s        b  %s\n", CURSYM(sel), buf, pname);
		else if (S_ISCHR(ent->mode))
			printw("%s%-16.16s        c  %s\n", CURSYM(sel), buf, pname);
		else if (ent->mode & 0100)
			printw("%s%-16.16s %8.8s* %s*\n", CURSYM(sel), buf, coolsize(ent->size), pname);
		else
			printw("%s%-16.16s %8.8s  %s\n", CURSYM(sel), buf, coolsize(ent->size), pname);
	} else {
		if (S_ISDIR(ent->mode))
			printw("%s%-16.16s %8.8s/ %s/\n", CURSYM(sel), buf, coolsize(ent->blocks << 9), pname);
		else if (S_ISLNK(ent->mode))
			printw("%s%-16.16s        @  %s@\n", CURSYM(sel), buf, pname);
		else if (S_ISSOCK(ent->mode))
			printw("%s%-16.16s        =  %s=\n", CURSYM(sel), buf, pname);
		else if (S_ISFIFO(ent->mode))
			printw("%s%-16.16s        |  %s|\n", CURSYM(sel), buf, pname);
		else if (S_ISBLK(ent->mode))
			printw("%s%-16.16s        b  %s\n", CURSYM(sel), buf, pname);
		else if (S_ISCHR(ent->mode))
			printw("%s%-16.16s        c  %s\n", CURSYM(sel), buf, pname);
		else if (ent->mode & 0100)
			printw("%s%-16.16s %8.8s* %s*\n", CURSYM(sel), buf, coolsize(ent->blocks << 9), pname);
		else
			printw("%s%-16.16s %8.8s  %s\n", CURSYM(sel), buf, coolsize(ent->blocks << 9), pname);
	}

	if (sel)
		attroff(A_REVERSE);
}

static void (*printptr)(struct entry *ent, int sel, uint namecols) = &printent_long;

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
	static char bits[11] = {'\0'};

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

	return bits;
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
	char desc[32];
	char *perms = get_lsperms(sb->st_mode, desc);
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
			dprintf(fd, "    File: '%s' -> ", unescape(fname, 0));

			/*
			 * We pass g_buf but unescape() operates on g_buf too!
			 * Read the API notes for information on how this works.
			 */
			dprintf(fd, "'%s'", unescape(g_buf, 0));
		}
	} else
		dprintf(fd, "    File: '%s'", unescape(fname, 0));

	/* Show size, blocks, file type */
#if defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__) || defined(__APPLE__)
	dprintf(fd, "\n    Size: %-15lld Blocks: %-10lld IO Block: %-6d %s",
#else
	dprintf(fd, "\n    Size: %-15ld Blocks: %-10ld IO Block: %-6ld %s",
#endif
	       sb->st_size, sb->st_blocks, sb->st_blksize, desc);

	/* Show containing device, inode, hardlink count */
#if defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__) || defined(__APPLE__)
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

/*
 * Get the order of 2 for this size
 * In brief - return the number of trailing zeroes
 */
static int
getorder(size_t size)
{
	static int count, mask;

	for (mask = 1, count = 0; count < 32; mask <<= 1, ++count)
		if ((size & mask) != 0)
			return count;

	return 32;
}

static size_t
get_fs_free(const char *path)
{
	static struct statvfs svb;

	if (statvfs(path, &svb) == -1)
		return 0;
	else
		return svb.f_bavail << getorder(svb.f_frsize);
}

static size_t
get_fs_capacity(const char *path)
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
	if (!get_output(g_buf, MAX_CMD_LEN, "which", utils[cfg.metaviewer], NULL, 0))
		return -1;

	exitcurses();
	get_output(NULL, 0, utils[cfg.metaviewer], fpath, arg, 1);
	initcurses();
	return 0;
}

static int
handle_archive(char *fpath, char *arg, char *dir)
{
	if (!get_output(g_buf, MAX_CMD_LEN, "which", utils[4], NULL, 0))
		return -1;

	if (arg[1] == 'x')
		spawn(utils[4], arg, fpath, dir, F_NORMAL);
	else {
		exitcurses();
		get_output(NULL, 0, utils[4], arg, fpath, 1);
		initcurses();
	}

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
"1Home, g, ^, ^A | First entry\n"
 "2End, G, $, ^E | Last entry\n"
   "4→, ↵, l, ^M | Open file or enter dir\n"
"1←, Bksp, h, ^H | Go to parent dir\n"
        "9Insert | Toggle navigate-as-you-type\n"
             "e~ | Go HOME\n"
             "e& | Go to initial dir\n"
             "e- | Go to last visited dir\n"
             "e/ | Filter dir contents\n"
            "d^/ | Open desktop search tool\n"
             "e. | Toggle hide . files\n"
             "eb | Bookmark prompt\n"
            "d^B | Pin current dir\n"
            "d^V | Go to pinned dir\n"
             "ec | Change dir prompt\n"
             "ed | Toggle detail view\n"
             "eD | File details\n"
             "em | Brief media info\n"
             "eM | Full media info\n"
	     "en | Create new\n"
            "d^R | Rename entry\n"
             "es | Toggle sort by size\n"
             "eS | Toggle du mode\n"
             "et | Toggle sort by mtime\n"
             "e! | Spawn SHELL in dir\n"
             "ee | Edit entry in EDITOR\n"
             "eo | Open dir in file manager\n"
             "ep | Open entry in PAGER\n"
             "eF | List archive\n"
            "d^X | Extract archive\n"
            "d^K | Invoke file path copier\n"
            "d^L | Redraw, clear prompt\n"
             "e? | Help, settings\n"
             "eQ | Quit and cd\n"
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
				dprintf(fd, " %s: %s\n", bookmark[i].key, bookmark[i].loc);
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

	dprintf(fd, "\nVersion: %s\n%s\n", VERSION, GENERAL_INFO);
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
	static char *namep, *pnb;
	static struct entry *dentp;
	static size_t off, namebuflen = NAMEBUF_INCR;
	static ulong num_saved;
	static int fd, n, count;
	static struct stat sb_path, sb;

	off = 0;

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

			if (fstatat(fd, namep, &sb, AT_SYMLINK_NOFOLLOW) == -1)
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
			DPRINTF_S(namep);
			continue;
		}

		if (n == total_dents) {
			total_dents += ENTRY_INCR;
			*dents = realloc(*dents, total_dents * sizeof(**dents));
			if (*dents == NULL) {
				if (pnamebuf)
					free(pnamebuf);
				errexit();
			}
		}

		/* If there's not enough bytes left to copy a file name of length NAME_MAX, re-allocate */
		if (namebuflen - off < NAME_MAX + 1) {
			namebuflen += NAMEBUF_INCR;

			pnb = pnamebuf;
			pnamebuf = (char *)realloc(pnamebuf, namebuflen);
			DPRINTF_P(pnamebuf);
			if (pnamebuf == NULL) {
				free(*dents);
				errexit();
			}

			/* realloc() may result in memory move, we must re-adjust if that happens */
			if (pnb != pnamebuf) {
				dentp = *dents;
				dentp->name = pnamebuf;

				for (count = 1; count < n; ++dentp, ++count)
					/* Current filename starts at last filename start + length */
					(dentp + 1)->name = (char *)((size_t)dentp->name + dentp->nlen);
			}
		}

		dentp = *dents + n;

		/* Copy file name */
		dentp->name = (char *)((size_t)pnamebuf + off);
		dentp->nlen = xstrlcpy(dentp->name, namep, NAME_MAX + 1);
		off += dentp->nlen;

		/* Copy other fields */
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
		if (*dents) {
			free(pnamebuf);
			free(*dents);
		}
		errexit();
	}

	return n;
}

static void
dentfree(struct entry *dents)
{
	free(pnamebuf);
	free(dents);
}

/* Return the position of the matching entry or 0 otherwise */
static int
dentfind(struct entry *dents, const char *fname, int n)
{
	static int i;

	if (!fname)
		return 0;

	DPRINTF_S(fname);

	for (i = 0; i < n; ++i)
		if (xstrcmp(fname, dents[i].name) == 0)
			return i;

	return 0;
}

static int
populate(char *path, char *oldname, char *fltr)
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

#ifdef DEBUGMODE
	struct timespec ts1, ts2;
	clock_gettime(CLOCK_REALTIME, &ts1); /* Use CLOCK_MONOTONIC on FreeBSD */
#endif

	ndents = dentfill(path, &dents, visible, &re);

	qsort(dents, ndents, sizeof(*dents), entrycmp);

#ifdef DEBUGMODE
	clock_gettime(CLOCK_REALTIME, &ts2);
	DPRINTF_U(ts2.tv_nsec - ts1.tv_nsec);
#endif

	/* Find cur from history */
	cur = dentfind(dents, oldname, ndents);
	return 0;
}

static void
redraw(char *path)
{
	static char buf[(NAME_MAX + 1) << 1];
	static size_t ncols;
	static int nlines, i;
	static bool mode_changed;

	mode_changed = FALSE;
	nlines = MIN(LINES - 4, ndents);

	/* Clean screen */
	erase();

	/* Fail redraw if < than 10 columns */
	if (COLS < 10) {
		printmsg("Too few columns!");
		return;
	}

	/* Strip trailing slashes */
	for (i = xstrlen(path) - 1; i > 0; --i)
		if (path[i] == '/')
			path[i] = '\0';
		else
			break;

	DPRINTF_D(cur);
	DPRINTF_S(path);

	if (!realpath(path, g_buf)) {
		printwarn();
		return;
	}

	ncols = COLS;
	if (ncols > PATH_MAX)
		ncols = PATH_MAX;

	/* No text wrapping in cwd line */
	/* Show CWD: - xstrlen(CWD) - 1 = 6 */
	g_buf[ncols - 6] = '\0';
	printw(CWD "%s\n\n", g_buf);

	/* Fallback to light mode if less than 35 columns */
	if (ncols < 35 && cfg.showdetail) {
		cfg.showdetail ^= 1;
		printptr = &printent;
		mode_changed = TRUE;
	}

	/* Calculate the number of cols available to print entry name */
	if (cfg.showdetail)
		ncols -= 32;
	else
		ncols -= 5;

	if (cfg.showcolor) {
		attron(COLOR_PAIR(1) | A_BOLD);
		cfg.dircolor = 1;
	}

	/* Print listing */
	if (cur < (nlines >> 1)) {
		for (i = 0; i < nlines; ++i)
			printptr(&dents[i], i == cur, ncols);
	} else if (cur >= ndents - (nlines >> 1)) {
		for (i = ndents - nlines; i < ndents; ++i)
			printptr(&dents[i], i == cur, ncols);
	} else {
		static int odd;

		odd = ISODD(nlines);
		nlines >>= 1;
		for (i = cur - nlines; i < cur + nlines + odd; ++i)
			printptr(&dents[i], i == cur, ncols);
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

			/* We need to show filename as it may be truncated in directory listing */
			if (!cfg.blkorder)
				sprintf(buf, "%d/%d %s[%s%s]", cur + 1, ndents, sort, unescape(dents[cur].name, 0), ind);
			else {
				i = sprintf(buf, "%d/%d du: %s (%lu files) ", cur + 1, ndents, coolsize(dir_blocks << 9), num_files);
				sprintf(buf + i, "vol: %s free [%s%s]", coolsize(get_fs_free(path)), unescape(dents[cur].name, 0), ind);
			}

			printmsg(buf);
		} else
			printmsg("0 items");
	}

	if (mode_changed) {
		cfg.showdetail ^= 1;
		printptr = &printent_long;
	}
}

static void
browse(char *ipath, char *ifilter)
{
	static char path[PATH_MAX], newpath[PATH_MAX], lastdir[PATH_MAX], mark[PATH_MAX];
	static char fltr[NAME_MAX + 1];
	static char oldname[NAME_MAX + 1];
	char *dir, *tmp, *run = NULL, *env = NULL;
	struct stat sb;
	int r, fd, presel;
	enum action sel = SEL_RUNARG + 1;
	bool dir_changed = FALSE;

	xstrlcpy(path, ipath, PATH_MAX);
	copyfilter();
	oldname[0] = newpath[0] = lastdir[0] = mark[0] = '\0';

	if (cfg.filtermode)
		presel = FILTER;
	else
		presel = 0;

	/* Allocate buffer to hold names */
	pnamebuf = (char *)malloc(NAMEBUF_INCR);
	if (pnamebuf == NULL)
		errexit();

begin:
#ifdef LINUX_INOTIFY
	if (dir_changed && inotify_wd >= 0) {
		inotify_rm_watch(inotify_fd, inotify_wd);
		inotify_wd = -1;
		dir_changed = FALSE;
	}
#elif defined(BSD_KQUEUE)
	if (dir_changed && event_fd >= 0) {
		close(event_fd);
		event_fd = -1;
		dir_changed = FALSE;
	}
#endif

	if (populate(path, oldname, fltr) == -1) {
		printwarn();
		goto nochange;
	}

#ifdef LINUX_INOTIFY
	if (inotify_wd == -1)
		inotify_wd = inotify_add_watch(inotify_fd, path, INOTIFY_MASK);
#elif defined(BSD_KQUEUE)
	if (event_fd == -1) {
#if defined(O_EVTONLY)
		event_fd = open(path, O_EVTONLY);
#else
		event_fd = open(path, O_RDONLY);
#endif
		if (event_fd >= 0)
			EV_SET(&events_to_monitor[0], event_fd, EVFILT_VNODE, EV_ADD | EV_CLEAR, KQUEUE_FFLAGS, 0, path);
	}
#endif

	for (;;) {
		redraw(path);
nochange:
		/* Exit if parent has exited */
		if (getppid() == 1)
			_exit(0);

		sel = nextsel(&run, &env, &presel);

		switch (sel) {
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
			xstrlcpy(oldname, xbasename(path), NAME_MAX + 1);

			/* Save last working directory */
			xstrlcpy(lastdir, path, PATH_MAX);
			dir_changed = TRUE;

			xstrlcpy(path, dir, PATH_MAX);
			/* Reset filter */
			copyfilter();
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
			if (fstat(fd, &sb) == -1) {
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
				dir_changed = TRUE;

				xstrlcpy(path, newpath, PATH_MAX);
				oldname[0] = '\0';
				/* Reset filter */
				copyfilter();
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
				spawn(utils[2], newpath, NULL, NULL, nowait);
				continue;
			}
			default:
				printmsg("Unsupported file");
				goto nochange;
			}
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
					xstrlcpy(oldname, xbasename(path), NAME_MAX + 1);
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

				oldname[0] = '\0';
			} else if (truecd == 1)
				/* Sure change in dir */
				oldname[0] = '\0';

			/* Save last working directory */
			xstrlcpy(lastdir, path, PATH_MAX);
			dir_changed = TRUE;

			/* Save the newly opted dir in path */
			xstrlcpy(path, newpath, PATH_MAX);

			/* Reset filter */
			copyfilter();
			DPRINTF_S(path);
			if (cfg.filtermode)
				presel = FILTER;
			goto begin;
		}
		case SEL_CDHOME:
			dir = getenv("HOME");
			if (dir == NULL) {
				clearprompt();
				goto nochange;
			} // fallthrough
		case SEL_CDBEGIN:
			if (sel == SEL_CDBEGIN)
				dir = ipath;

			if (!xdiraccess(dir)) {
				goto nochange;
			}

			if (xstrcmp(path, dir) == 0) {
				break;
			}

			/* Save last working directory */
			xstrlcpy(lastdir, path, PATH_MAX);
			dir_changed = TRUE;

			xstrlcpy(path, dir, PATH_MAX);
			oldname[0] = '\0';
			/* Reset filter */
			copyfilter();
			DPRINTF_S(path);
			if (cfg.filtermode)
				presel = FILTER;
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
			dir_changed = TRUE;
			xstrlcpy(path, newpath, PATH_MAX);
			oldname[0] = '\0';
			/* Reset filter */
			copyfilter();
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

			if (get_bm_loc(tmp, newpath) == NULL) {
				printmsg(STR_INVBM);
				goto nochange;
			}

			if (!xdiraccess(newpath))
				goto nochange;

			if (xstrcmp(path, newpath) == 0)
				break;

			oldname[0] = '\0';

			/* Save last working directory */
			xstrlcpy(lastdir, path, PATH_MAX);
			dir_changed = TRUE;

			/* Save the newly opted dir in path */
			xstrlcpy(path, newpath, PATH_MAX);

			/* Reset filter */
			copyfilter();
			DPRINTF_S(path);

			if (cfg.filtermode)
				presel = FILTER;
			goto begin;
		case SEL_PIN:
			xstrlcpy(mark, path, PATH_MAX);
			printmsg(mark);
			goto nochange;
		case SEL_FLTR:
			presel = filterentries(path);
			copyfilter();
			DPRINTF_S(fltr);
			/* Save current */
			if (ndents > 0)
				copycurname();
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
		case SEL_TOGGLEDOT:
			cfg.showhidden ^= 1;
			initfilter(cfg.showhidden, &ifilter);
			copyfilter();
			goto begin;
		case SEL_DETAIL:
			cfg.showdetail ^= 1;
			cfg.showdetail ? (printptr = &printent_long) : (printptr = &printent);
			/* Save current */
			if (ndents > 0)
				copycurname();
			goto begin;
		case SEL_STATS:
			if (ndents > 0) {
				mkpath(path, dents[cur].name, newpath, PATH_MAX);

				if (lstat(newpath, &sb) == -1) {
					if (dents)
						dentfree(dents);
					errexit();
				} else {
					if (show_stats(newpath, dents[cur].name, &sb) < 0) {
						printwarn();
						goto nochange;
					}
				}
			}
			break;
		case SEL_LIST: // fallthrough
		case SEL_EXTRACT: // fallthrough
		case SEL_MEDIA: // fallthrough
		case SEL_FMEDIA:
			if (ndents > 0) {
				mkpath(path, dents[cur].name, newpath, PATH_MAX);

				if (sel == SEL_MEDIA || sel == SEL_FMEDIA)
					r = show_mediainfo(newpath, run);
				else
					r = handle_archive(newpath, run, path);

				if (r == -1) {
					if (sel == SEL_MEDIA || sel == SEL_FMEDIA)
						sprintf(g_buf, "%s missing", utils[cfg.metaviewer]);
					else
						sprintf(g_buf, "%s missing", utils[4]);

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
				copycurname();
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
				copycurname();
			goto begin;
		case SEL_MTIME:
			cfg.mtimeorder ^= 1;
			cfg.sizeorder = 0;
			cfg.blkorder = 0;
			/* Save current */
			if (ndents > 0)
				copycurname();
			goto begin;
		case SEL_REDRAW:
			/* Save current */
			if (ndents > 0)
				copycurname();
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
		case SEL_NEW:
			printprompt("name: ");
			tmp = xreadline(NULL);
			clearprompt();
			if (tmp == NULL || tmp[0] == '\0')
				break;

			/* Allow only relative, same dir paths */
			if (tmp[0] == '/' || xstrcmp(xbasename(tmp), tmp) != 0) {
				printmsg(STR_INPUT);
				goto nochange;
			}

			/* Open the descriptor to currently open directory */
			fd = open(path, O_RDONLY | O_DIRECTORY);
			if (fd == -1) {
				printwarn();
				goto nochange;
			}

			/* Check if another file with same name exists */
			if (faccessat(fd, tmp, F_OK, AT_SYMLINK_NOFOLLOW) != -1) {
				printmsg("Entry exists");
				goto nochange;
			}

			/* Check if it's a dir or file */
			printprompt("Press 'f' for file or 'd' for dir");
			cleartimeout();
			r = getch();
			settimeout();
			if (r == 'f') {
				r = openat(fd, tmp, O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
				close(r);
			} else if (r == 'd')
				r = mkdirat(fd, tmp, S_IRWXU | S_IRWXG | S_IRWXO);
			else {
				close(fd);
				break;
			}

			if (r == -1) {
				printwarn();
				close(fd);
				goto nochange;
			}

			close(fd);
			xstrlcpy(oldname, tmp, NAME_MAX + 1);
			goto begin;
		case SEL_RENAME:
			if (ndents <= 0)
				break;

			printprompt("");
			tmp = xreadline(dents[cur].name);
			clearprompt();
			if (tmp == NULL || tmp[0] == '\0')
				break;

			/* Allow only relative, same dir paths */
			if (tmp[0] == '/' || xstrcmp(xbasename(tmp), tmp) != 0) {
				printmsg(STR_INPUT);
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
				printprompt("Press 'y' to overwrite");
				cleartimeout();
				r = getch();
				settimeout();
				if (r != 'y') {
					close(fd);
					break;
				}
			}

			/* Rename the file */
			if (renameat(fd, dents[cur].name, fd, tmp) != 0) {
				printwarn();
				close(fd);
				goto nochange;
			}

			close(fd);
			xstrlcpy(oldname, tmp, NAME_MAX + 1);
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
	printf("usage: nnn [-b key] [-c N] [-e] [-i] [-l]\n\
           [-p nlay] [-S] [-v] [-h] [PATH]\n\n\
The missing terminal file browser for X.\n\n\
positional arguments:\n\
  PATH   start dir [default: current dir]\n\n\
optional arguments:\n\
 -b key  specify bookmark key to open\n\
 -c N    specify dir color, disables if N>7\n\
 -e      use exiftool instead of mediainfo\n\
 -i      start in navigate-as-you-type mode\n\
 -l      start in light mode (fewer details)\n\
 -p nlay path to custom nlay\n\
 -S      start in disk usage analyzer mode\n\
 -v      show program version and exit\n\
 -h      show this help and exit\n\n\
Version: %s\n%s\n", VERSION, GENERAL_INFO);
	exit(0);
}

int
main(int argc, char *argv[])
{
	static char cwd[PATH_MAX];
	char *ipath = NULL, *ifilter, *bmstr;
	int opt;

	/* Confirm we are in a terminal */
	if (!isatty(0) || !isatty(1)) {
		fprintf(stderr, "stdin or stdout is not a tty\n");
		exit(1);
	}

	while ((opt = getopt(argc, argv, "Slib:c:ep:vh")) != -1) {
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
		case 'b':
			ipath = optarg;
			break;
		case 'c':
			if (atoi(optarg) > 7)
				cfg.showcolor = 0;
			else
				cfg.color = (uchar)atoi(optarg);
			break;
		case 'e':
			cfg.metaviewer = 1;
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

	/* Parse bookmarks string, if available */
	bmstr = getenv("NNN_BMS");
	if (bmstr)
		parsebmstr(bmstr);

	if (ipath) { /* Open a bookmark directly */
		if (get_bm_loc(ipath, cwd) == NULL) {
			fprintf(stderr, "%s\n", STR_INVBM);
			exit(1);
		}

		ipath = cwd;
	} else if (argc == optind) {
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
		fprintf(stderr, "inotify init! %s\n", strerror(errno));
		exit(1);
	}
#elif defined(BSD_KQUEUE)
	kq = kqueue();
	if (kq < 0) {
		fprintf(stderr, "kqueue init! %s\n", strerror(errno));
		exit(1);
	}

	gtimeout.tv_sec = 0;
	gtimeout.tv_nsec = 0;
#endif

	/* Edit text in EDITOR, if opted */
	if (getenv("NNN_USE_EDITOR"))
		editor = xgetenv("EDITOR", "vi");

	/* Set player if not set already */
	if (!player)
		player = utils[3];

	/* Get the desktop file browser, if set */
	desktop_manager = getenv("NNN_DE_FILE_MANAGER");

	/* Get screensaver wait time, if set; copier used as tmp var */
	copier = getenv("NNN_IDLE_TIMEOUT");
	if (copier)
		idletimeout = abs(atoi(copier));

	/* Get the default copier, if set */
	copier = getenv("NNN_COPIER");

	/* Get nowait flag */
	nowait |= getenv("NNN_NOWAIT") ? F_NOWAIT : 0;

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
