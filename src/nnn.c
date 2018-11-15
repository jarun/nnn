/*
 * BSD 2-Clause License
 *
 * Copyright (c) 2014-2016, Lazaros Koromilas <lostd@2f30.org>
 * Copyright (c) 2014-2016, Dimitris Papastamos <sin@2f30.org>
 * Copyright (c) 2016-2018, Arun Prakash Jana <engineerarun@gmail.com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * * Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 *
 * * Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
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
#include <strings.h>
#include <time.h>
#include <unistd.h>
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
		fprintf(stderr, "debug: open failed! (1)\n");

		fp = fopen("./nnn_debug", "w");
		if (!fp) {
			fprintf(stderr, "debug: open failed! (2)\n");
			return -1;
		}
	}

	DEBUG_FD = fileno(fp);
	if (DEBUG_FD == -1) {
		fprintf(stderr, "debug: open fd failed!\n");
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
#define VERSION "2.0"
#define GENERAL_INFO "License: BSD 2-Clause\nWebpage: https://github.com/jarun/nnn"

#define LEN(x) (sizeof(x) / sizeof(*(x)))
#undef MIN
#define MIN(x, y) ((x) < (y) ? (x) : (y))
#define ISODD(x) ((x) & 1)
#define TOUPPER(ch) \
	(((ch) >= 'a' && (ch) <= 'z') ? ((ch) - 'a' + 'A') : (ch))
#define MAX_CMD_LEN 5120
#define CURSR " > "
#define EMPTY "   "
#define CURSYM(flag) ((flag) ? CURSR : EMPTY)
#define FILTER '/'
#define REGEX_MAX 128
#define BM_MAX 10
#define ENTRY_INCR 64 /* Number of dir 'entry' structures to allocate per shot */
#define NAMEBUF_INCR 0x1000 /* 64 dir entries at a time, avg. 64 chars per filename = 64*64B = 4KB */
#define DESCRIPTOR_LEN 32
#define _ALIGNMENT 0x10
#define _ALIGNMENT_MASK 0xF
#define SYMLINK_TO_DIR 0x1
#define MAX_HOME_LEN 64
#define MAX_CTX 4
#define DOT_FILTER_LEN 8

/* Macros to define process spawn behaviour as flags */
#define F_NONE     0x00  /* no flag set */
#define F_MARKER   0x01  /* draw marker to indicate nnn spawned (e.g. shell) */
#define F_NOWAIT   0x02  /* don't wait for child process (e.g. file manager) */
#define F_NOTRACE  0x04  /* suppress stdout and strerr (no traces) */
#define F_SIGINT   0x08  /* restore default SIGINT handler */
#define F_NORMAL   0x80  /* spawn child process in non-curses regular CLI mode */

/* CRC8 macros */
#define WIDTH  (sizeof(unsigned char) << 3)
#define TOPBIT (1 << (WIDTH - 1))
#define POLYNOMIAL 0xD8  /* 11011 followed by 0's */
#define CRC8_TABLE_LEN 256

/* Volume info */
#define FREE 0
#define CAPACITY 1

/* Function macros */
#define exitcurses() endwin()
#define clearprompt() printmsg("")
#define printwarn() printmsg(strerror(errno))
#define istopdir(path) ((path)[1] == '\0' && (path)[0] == '/')
#define copycurname() xstrlcpy(lastname, dents[cur].name, NAME_MAX + 1)
#define settimeout() timeout(1000)
#define cleartimeout() timeout(-1)
#define errexit() printerr(__LINE__)
#define setdirwatch() (cfg.filtermode ? (presel = FILTER) : (dir_changed = TRUE))

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
typedef unsigned short ushort;

/* STRUCTURES */

/* Directory entry */
typedef struct entry {
	char *name;
	time_t t;
	off_t size;
	blkcnt_t blocks; /* number of 512B blocks allocated */
	mode_t mode;
	ushort nlen; /* Length of file name; can be uchar (< NAME_MAX + 1) */
	uchar flags; /* Flags specific to the file */
} __attribute__ ((packed, aligned(_ALIGNMENT))) *pEntry;

/* Bookmark */
typedef struct {
	int key;
	char *loc;
} bm;

/* Settings */
typedef struct {
	uint filtermode : 1;  /* Set to enter filter mode */
	uint mtimeorder : 1;  /* Set to sort by time modified */
	uint sizeorder  : 1;  /* Set to sort by file size */
	uint apparentsz : 1;  /* Set to sort by apparent size (disk usage) */
	uint blkorder   : 1;  /* Set to sort by blocks used (disk usage) */
	uint showhidden : 1;  /* Set to show hidden files */
	uint copymode   : 1;  /* Set when copying files */
	uint autoselect : 1;  /* Auto-select dir in nav-as-you-type mode */
	uint showdetail : 1;  /* Clear to show fewer file info */
	uint showcolor  : 1;  /* Set to show dirs in blue */
	uint dircolor   : 1;  /* Current status of dir color */
	uint metaviewer : 1;  /* Index of metadata viewer in utils[] */
	uint quote      : 1;  /* Copy paths within quotes */
	uint color      : 3;  /* Color code for directories */
	uint ctxactive  : 1;  /* Context active or not */
	uint reserved   : 13;
	/* The following settings are global */
	uint curctx     : 2;  /* Current context number */
} settings;

/* Contexts or workspaces */
typedef struct {
	char c_path[PATH_MAX]; /* Current dir */
	char c_init[PATH_MAX]; /* Initial dir */
	char c_last[PATH_MAX]; /* Last visited dir */
	char c_name[NAME_MAX + 1]; /* Current file name */
	settings c_cfg; /* Current configuration */
	char c_fltr[DOT_FILTER_LEN]; /* Hidden filter */
} context;

/* GLOBALS */

/* Configuration, contexts */
static settings cfg = {0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 4, 1, 0, 0};
static context g_ctx[MAX_CTX] __attribute__ ((aligned));

static struct entry *dents;
static char *pnamebuf, *pcopybuf;
static int ndents, cur, total_dents = ENTRY_INCR;
static uint idle;
static uint idletimeout, copybufpos, copybuflen;
static char *copier;
static char *editor;
static blkcnt_t ent_blocks;
static blkcnt_t dir_blocks;
static ulong num_files;
static uint open_max;
static bm bookmark[BM_MAX];
static size_t g_tmpfplen; /* path to tmp files for copy without X, keybind help and file stats */
static uchar g_crc;
static uchar BLK_SHIFT = 9;

/* CRC data */
static uchar crc8table[CRC8_TABLE_LEN] __attribute__ ((aligned));

/* For use in functions which are isolated and don't return the buffer */
static char g_buf[MAX_CMD_LEN] __attribute__ ((aligned));

/* Buffer for file path copy file */
static char g_cppath[MAX_HOME_LEN] __attribute__ ((aligned));

/* Buffer to store tmp file path */
static char g_tmpfpath[MAX_HOME_LEN] __attribute__ ((aligned));

#ifdef LINUX_INOTIFY
static int inotify_fd, inotify_wd = -1;
static uint INOTIFY_MASK = IN_ATTRIB | IN_CREATE | IN_DELETE | IN_DELETE_SELF | IN_MODIFY | IN_MOVE_SELF | IN_MOVED_FROM | IN_MOVED_TO;
#elif defined(BSD_KQUEUE)
static int kq, event_fd = -1;
static struct kevent events_to_monitor[NUM_EVENT_FDS];
static uint KQUEUE_FFLAGS = NOTE_DELETE | NOTE_EXTEND | NOTE_LINK | NOTE_RENAME | NOTE_REVOKE | NOTE_WRITE;
static struct timespec gtimeout;
#endif

/* Macros for utilities */
#define MEDIAINFO 0
#define EXIFTOOL 1
#define OPENER 2
#define ATOOL 3
#define APACK 4
#define VIDIR 5
#define LOCKER 6
#define UNKNOWN 7

/* Utilities to open files, run actions */
static char * const utils[] = {
	"mediainfo",
	"exiftool",
#ifdef __APPLE__
	"/usr/bin/open",
#elif defined __CYGWIN__
	"cygstart",
#else
	"xdg-open",
#endif
	"atool",
	"apack",
	"vidir",
#ifdef __APPLE__
	"bashlock",
#elif defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__)
	"lock",
#else
	"vlock",
#endif
	"UNKNOWN"
};

/* Common strings */
#define STR_NFTWFAIL_ID 0
#define STR_NOHOME_ID 1
#define STR_INPUT_ID 2
#define STR_INVBM_KEY 3
#define STR_COPY_ID 4
#define STR_DATE_ID 5

static const char messages[][16] = {
	"nftw failed",
	"HOME not set",
	"no traversal",
	"invalid key",
	"copy not set",
	"%F %T %z",
};

/* Forward declarations */
static void redraw(char *path);
static char *get_output(char *buf, size_t bytes, char *file, char *arg1, char *arg2, int pager);
int (*nftw_fn) (const char *fpath, const struct stat *sb, int typeflag, struct FTW *ftwbuf);

/* Functions */

/*
 * CRC8 source:
 *   https://barrgroup.com/Embedded-Systems/How-To/CRC-Calculation-C-Code
 */
static void crc8init()
{
	uchar remainder, bit;
	uint dividend;

	/* Compute the remainder of each possible dividend  */
	for (dividend = 0; dividend < CRC8_TABLE_LEN; ++dividend) {
		/* Start with the dividend followed by zeros */
		remainder = dividend << (WIDTH - 8);

		/* Perform modulo-2 division, a bit at a time */
		for (bit = 8; bit > 0; --bit) {
			/* Try to divide the current data bit */
			if (remainder & TOPBIT)
				remainder = (remainder << 1) ^ POLYNOMIAL;
			else
				remainder = (remainder << 1);
		}

		/* Store the result into the table */
		crc8table[dividend] = remainder;
	}
}

static uchar crc8fast(uchar const message[], size_t n)
{
	static uchar data, remainder;
	static size_t byte;

	/* Divide the message by the polynomial, a byte at a time */
	for (remainder = byte = 0; byte < n; ++byte) {
		data = message[byte] ^ (remainder >> (WIDTH - 8));
		remainder = crc8table[data] ^ (remainder << 8);
	}

	/* The final remainder is the CRC */
	return remainder;
}

/* Get platform block shift */
static int get_blk_shift(void)
{
	int shift;
	for (shift = 0; shift < 32; ++shift)
	{
		if ((1<<shift) & S_BLKSIZE)
			break;
	}
	return shift;
}

/* Messages show up at the bottom */
static void printmsg(const char *msg)
{
	mvprintw(LINES - 1, 0, "%s\n", msg);
}

/* Kill curses and display error before exiting */
static void printerr(int linenum)
{
	exitcurses();
	fprintf(stderr, "line %d: (%d) %s\n", linenum, errno, strerror(errno));
	if (g_cppath[0])
		unlink(g_cppath);
	exit(1);
}

/* Print prompt on the last line */
static void printprompt(const char *str)
{
	clearprompt();
	printw(str);
}

static int get_input(const char *prompt)
{
	if (prompt)
		printprompt(prompt);
	cleartimeout();
	int r = getch();
	settimeout();
	return r;
}

/* Increase the limit on open file descriptors, if possible */
static rlim_t max_openfds()
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
 * Wrapper to realloc()
 * Frees current memory if realloc() fails and returns NULL.
 *
 * As per the docs, the *alloc() family is supposed to be memory aligned:
 * Ubuntu: http://manpages.ubuntu.com/manpages/xenial/man3/malloc.3.html
 * OS X: https://developer.apple.com/legacy/library/documentation/Darwin/Reference/ManPages/man3/malloc.3.html
 */
static void *xrealloc(void *pcur, size_t len)
{
	static void *pmem;

	pmem = realloc(pcur, len);
	if (!pmem)
		free(pcur);

	return pmem;
}

/*
 * Just a safe strncpy(3)
 * Always null ('\0') terminates if both src and dest are valid pointers.
 * Returns the number of bytes copied including terminating null byte.
 */
static size_t xstrlcpy(char *dest, const char *src, size_t n)
{
	static ulong *s, *d;
	static size_t len, blocks;
	static const uint lsize = sizeof(ulong);
	static const uint _WSHIFT = (sizeof(ulong) == 8) ? 3 : 2;

	if (!src || !dest || !n)
		return 0;

	len = strlen(src) + 1;
	if (n > len)
		n = len;
	else if (len > n)
		/* Save total number of bytes to copy in len */
		len = n;

	/*
	 * To enable -O3 ensure src and dest are 16-byte aligned
	 * More info: http://www.felixcloutier.com/x86/MOVDQA.html
	 */
	if ((n >= lsize) && (((ulong)src & _ALIGNMENT_MASK) == 0 && ((ulong)dest & _ALIGNMENT_MASK) == 0)) {
		s = (ulong *)src;
		d = (ulong *)dest;
		blocks = n >> _WSHIFT;
		n &= lsize - 1;

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
 * The poor man's implementation of memrchr(3).
 * We are only looking for '/' in this program.
 * And we are NOT expecting a '/' at the end.
 * Ideally 0 < n <= strlen(s).
 */
static void *xmemrchr(uchar *s, uchar ch, size_t n)
{
	static uchar *ptr;

	if (!s || !n)
		return NULL;

	ptr = s + n;

	do {
		--ptr;

		if (*ptr == ch)
			return ptr;
	} while (s != ptr);

	return NULL;
}

/*
 * The following dirname(3) implementation does not
 * modify the input. We use a copy of the original.
 *
 * Modified from the glibc (GNU LGPL) version.
 */
static char *xdirname(const char *path)
{
	static char * const buf = g_buf, *last_slash, *runp;

	xstrlcpy(buf, path, PATH_MAX);

	/* Find last '/'. */
	last_slash = xmemrchr((uchar *)buf, '/', strlen(buf));

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

static char *xbasename(char *path)
{
	static char *base;

	base = xmemrchr((uchar *)path, '/', strlen(path));
	return base ? base + 1 : path;
}

/* Writes buflen char(s) from buf to a file */
static void writecp(const char *buf, const size_t buflen)
{
	if (!g_cppath[0]) {
		printmsg(messages[STR_COPY_ID]);
		return;
	}

	FILE *fp = fopen(g_cppath, "w");

	if (fp) {
		fwrite(buf, 1, buflen, fp);
		fclose(fp);
	} else
		printwarn();
}

static bool appendfpath(const char *path, const size_t len)
{
	if ((copybufpos >= copybuflen) || ((len + 3) > (copybuflen - copybufpos))) {
		copybuflen += PATH_MAX;
		pcopybuf = xrealloc(pcopybuf, copybuflen);
		if (!pcopybuf) {
			printmsg("no memory!");
			return FALSE;
		}
	}

	if (copybufpos) {
		pcopybuf[copybufpos - 1] = '\n';
		if (cfg.quote) {
			pcopybuf[copybufpos] = '\'';
			++copybufpos;
		}
	} else if (cfg.quote) {
		pcopybuf[copybufpos] = '\'';
		++copybufpos;
	}

	copybufpos += xstrlcpy(pcopybuf + copybufpos, path, len);
	if (cfg.quote) {
		pcopybuf[copybufpos - 1] = '\'';
		pcopybuf[copybufpos] = '\0';
		++copybufpos;
	}

	return TRUE;
}

static bool showcplist()
{
	ssize_t len;

	if (!copybufpos)
		return FALSE;

	if (g_tmpfpath[0])
		xstrlcpy(g_tmpfpath + g_tmpfplen - 1, "/.nnnXXXXXX", MAX_HOME_LEN - g_tmpfplen);
	else {
		printmsg(messages[STR_NOHOME_ID]);
		return -1;
	}

	int fd = mkstemp(g_tmpfpath);
	if (fd == -1)
		return FALSE;

	len = write(fd, pcopybuf, copybufpos - 1);
	close(fd);

	exitcurses();
	if (len == copybufpos - 1)
		get_output(NULL, 0, "cat", g_tmpfpath, NULL, 1);
	unlink(g_tmpfpath);
	refresh();
	return TRUE;
}

/* Initialize curses mode */
static void initcurses(void)
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
static void spawn(const char *file, const char *arg1, const char *arg2, const char *dir, uchar flag)
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
			fprintf(stdout, "\n +-++-++-+\n | n n n |\n +-++-++-+\n\n");
			fprintf(stdout, "Next shell level: %d\n", atoi(shlvl) + 1);
		}

		/* Suppress stdout and stderr */
		if (flag & F_NOTRACE) {
			int fd = open("/dev/null", O_WRONLY, 0200);

			dup2(fd, 1);
			dup2(fd, 2);
			close(fd);
		}

		if (flag & F_NOWAIT) {
			signal(SIGHUP, SIG_IGN);
			signal(SIGPIPE, SIG_IGN);
			setsid();
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
			refresh();
	}
}

/* Get program name from env var, else return fallback program */
static char *xgetenv(const char *name, char *fallback)
{
	static char *value;

	if (name == NULL)
		return fallback;

	value = getenv(name);

	return value && value[0] ? value : fallback;
}

/* Check if a dir exists, IS a dir and is readable */
static bool xdiraccess(const char *path)
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
static int xstricmp(const char * const s1, const char * const s2)
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

			return -1;
		}
	}

	return strcoll(s1, s2);
}

/* Return the integer value of a char representing HEX */
static char xchartohex(char c)
{
	if (c >= '0' && c <= '9')
		return c - '0';

	c = TOUPPER(c);
	if (c >= 'A' && c <= 'F')
		return c - 'A' + 10;

	return c;
}

static char *getmime(const char *file)
{
	static regex_t regex;
	static uint i;
	static const uint len = LEN(assocs);

	for (i = 0; i < len; ++i) {
		if (regcomp(&regex, assocs[i].regex, REG_NOSUB | REG_EXTENDED | REG_ICASE) != 0)
			continue;

		if (regexec(&regex, file, 0, NULL, 0) == 0) {
			regfree(&regex);
			return assocs[i].mime;
		}
	}

	regfree(&regex);
	return NULL;
}

static int setfilter(regex_t *regex, char *filter)
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

static void initfilter(int dot, char *hfltr)
{
	dot ? (hfltr[0] = '.', hfltr[1] = '\0') : xstrlcpy(hfltr, "^[^.]", DOT_FILTER_LEN);
}

static int visible(regex_t *regex, char *file)
{
	return regexec(regex, file, 0, NULL, 0) == 0;
}

static int entrycmp(const void *va, const void *vb)
{
	static pEntry pa, pb;

	pa = (pEntry)va;
	pb = (pEntry)vb;

	/* Sort directories first */
	if (S_ISDIR(pb->mode) && !S_ISDIR(pa->mode))
		return 1;

	if (S_ISDIR(pa->mode) && !S_ISDIR(pb->mode))
		return -1;

	/* Do the actual sorting */
	if (cfg.mtimeorder)
		return pb->t - pa->t;

	if (cfg.sizeorder) {
		if (pb->size > pa->size)
			return 1;

		if (pb->size < pa->size)
			return -1;
	}

	if (cfg.blkorder) {
		if (pb->blocks > pa->blocks)
			return 1;

		if (pb->blocks < pa->blocks)
			return -1;
	}

	return xstricmp(pa->name, pb->name);
}

/*
 * Returns SEL_* if key is bound and 0 otherwise.
 * Also modifies the run and env pointers (used on SEL_{RUN,RUNARG}).
 * The next keyboard input can be simulated by presel.
 */
static int nextsel(char **run, char **env, int *presel)
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
		/* Unwatch dir if we are still in a filtered view */
#ifdef LINUX_INOTIFY
		if (*presel == FILTER && inotify_wd >= 0) {
			inotify_rm_watch(inotify_fd, inotify_wd);
			inotify_wd = -1;
		}
#elif defined(BSD_KQUEUE)
		if (*presel == FILTER && event_fd >= 0) {
			close(event_fd);
			event_fd = -1;
		}
#endif
		*presel = 0;
	}

	if (c == -1) {
		++idle;

		/*
		 * Do not check for directory changes in du mode. A redraw forces du calculation.
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
static int fill(struct entry **dents, int (*filter)(regex_t *, char *), regex_t *re)
{
	static int count;
	static struct entry _dent, *pdent1, *pdent2;

	for (count = 0; count < ndents; ++count) {
		if (filter(re, (*dents)[count].name) == 0) {
			if (count != --ndents) {
				pdent1 = &(*dents)[count];
				pdent2 = &(*dents)[ndents];

				*(&_dent) = *pdent1;
				*pdent1 = *pdent2;
				*pdent2 = *(&_dent);
				--count;
			}

			continue;
		}
	}

	return ndents;
}

static int matches(char *fltr)
{
	static regex_t re;

	/* Search filter */
	if (setfilter(&re, fltr) != 0)
		return -1;

	ndents = fill(&dents, visible, &re);
	regfree(&re);
	if (!ndents)
		return 0;

	qsort(dents, ndents, sizeof(*dents), entrycmp);

	return 0;
}

static int filterentries(char *path)
{
	static char ln[REGEX_MAX] __attribute__ ((aligned));
	static wchar_t wln[REGEX_MAX] __attribute__ ((aligned));
	static wint_t ch[2] = {0};
	int r, total = ndents, oldcur = cur, len = 1;
	char *pln = ln + 1;

	ln[0] = wln[0] = FILTER;
	ln[1] = wln[1] = '\0';
	cur = 0;

	cleartimeout();
	curs_set(TRUE);
	printprompt(ln);

	while ((r = get_wch(ch)) != ERR) {
		switch (*ch) {
		case KEY_DC: // fallthrough
		case  KEY_BACKSPACE: // fallthrough
		case '\b': // fallthrough
		case CONTROL('L'): // fallthrough
		case 127: /* handle DEL */
			if (len == 1 && *ch != CONTROL('L')) {
				cur = oldcur;
				*ch = CONTROL('L');
				goto end;
			}

			if (*ch == CONTROL('L'))
				while (len > 1)
					wln[--len] = '\0';
			else
				wln[--len] = '\0';

			if (len == 1)
				cur = oldcur;

			wcstombs(ln, wln, REGEX_MAX);
			ndents = total;
			if (matches(pln) != -1)
				redraw(path);

			printprompt(ln);
			continue;
		case 27: /* Exit filter mode on Escape */
			cur = oldcur;
			*ch = CONTROL('L');
			goto end;
		}

		if (r == OK) {
			/* Handle all control chars in main loop */
			if (keyname(*ch)[0] == '^') {
				if (len == 1)
					cur = oldcur;
				goto end;
			}

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
			case '?':  // '?' is an invalid regex, show help instead
				if (len == 1) {
					cur = oldcur;
					goto end;
				} // fallthrough
			default:
				/* Reset cur in case it's a repeat search */
				if (len == 1)
					cur = 0;

				if (len == REGEX_MAX - 1)
					break;

				wln[len] = (wchar_t)*ch;
				wln[++len] = '\0';
				wcstombs(ln, wln, REGEX_MAX);

				/* Forward-filtering optimization:
				 * - new matches can only be a subset of current matches.
				 */
				/* ndents = total; */

				if (matches(pln) == -1)
					continue;

				/* If the only match is a dir, auto-select and cd into it */
				if (ndents == 1 && cfg.filtermode && cfg.autoselect && S_ISDIR(dents[0].mode)) {
					*ch = KEY_ENTER;
					cur = 0;
					goto end;
				}

				/*
				 * redraw() should be above the auto-select optimization, for
				 * the case where there's an issue with dir auto-select, say,
				 * due to a permission problem. The transition is _jumpy_ in
				 * case of such an error. However, we optimize for successful
				 * cases where the dir has permissions. This skips a redraw().
				 */
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
	curs_set(FALSE);
	settimeout();

	/* Return keys for navigation etc. */
	return *ch;
}

/* Show a prompt with input string and return the changes */
static char *xreadline(char *fname, char *prompt)
{
	size_t len, pos;
	int x, y, r;
	wint_t ch[2] = {0};
	static wchar_t * const buf = (wchar_t *)g_buf;

	cleartimeout();
	printprompt(prompt);

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
	curs_set(TRUE);

	while (1) {
		buf[len] = ' ';
		mvaddnwstr(y, x, buf, len + 1);
		move(y, x + wcswidth(buf, pos));

		r = get_wch(ch);
		if (r != ERR) {
			if (r == OK) {
				switch (*ch) {
				case KEY_ENTER: //fallthrough
				case '\n': //fallthrough
				case '\r':
					goto END;
				case '\b': /* some old curses (e.g. rhel25) still send '\b' for backspace */
					if (pos > 0) {
						memmove(buf + pos - 1, buf + pos, (len - pos) << 2);
						--len, --pos;
					} //fallthrough
				case '\t': /* TAB breaks cursor position, ignore it */
					continue;
				case CONTROL('L'):
					clearprompt();
					printprompt(prompt);
					len = pos = 0;
					continue;
				case CONTROL('A'):
					pos = 0;
					continue;
				case CONTROL('E'):
					pos = len;
					continue;
				case CONTROL('U'):
					clearprompt();
					printprompt(prompt);
					memmove(buf, buf + pos, (len - pos) << 2);
					len -= pos;
					pos = 0;
					continue;
				case 27: /* Exit prompt on Escape */
					len = 0;
					goto END;
				}

				/* Filter out all other control chars */
				if (keyname(*ch)[0] == '^')
					continue;

				if (pos < NAME_MAX - 1) {
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

END:
	curs_set(FALSE);
	settimeout();
	clearprompt();

	buf[len] = '\0';
	DPRINTF_S(buf);
	wcstombs(g_buf + ((NAME_MAX + 1) << 4), buf, NAME_MAX);
	return g_buf + ((NAME_MAX + 1) << 4);
}

/*
 * Updates out with "dir/name or "/name"
 * Returns the number of bytes copied including the terminating NULL byte
 */
static size_t mkpath(char *dir, char *name, char *out, size_t n)
{
	static size_t len;

	/* Handle absolute path */
	if (name[0] == '/')
		return xstrlcpy(out, name, n);

	/* Handle root case */
	if (istopdir(dir))
		len = 1;
	else
		len = xstrlcpy(out, dir, n);

	out[len - 1] = '/';
	return (xstrlcpy(out + len, name, n - len) + len);
}

static int parsebmstr()
{
	int i = 0;
	char *bms = getenv("NNN_BMS");
	if (!bms)
		return 0;

	while (*bms && i < BM_MAX) {
		bookmark[i].key = *bms;

		if (!*++bms) {
			bookmark[i].key = '\0';
			break;
		}

		if (*bms != ':')
			return -1; /* We support single char keys only */

		bookmark[i].loc = ++bms;
		if (bookmark[i].loc[0] == '\0' || bookmark[i].loc[0] == ';') {
			bookmark[i].key = '\0';
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

	return 0;
}

/*
 * Get the real path to a bookmark
 *
 * NULL is returned in case of no match, path resolution failure etc.
 * buf would be modified, so check return value before access
 */
static char *get_bm_loc(int key, char *buf)
{
	int r;

	for (r = 0; bookmark[r].key && r < BM_MAX; ++r) {
		if (bookmark[r].key == key) {
			if (bookmark[r].loc[0] == '~') {
				char *home = getenv("HOME");

				if (!home) {
					DPRINTF_S(messages[STR_NOHOME_ID]);
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

static void resetdircolor(mode_t mode)
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
 * Interestingly, note that unescape() uses g_buf. What happens if
 * str also points to g_buf? In this case we assume that the caller
 * acknowledges that it's OK to lose the data in g_buf after this
 * call to unescape().
 * The API, on its part, first converts str to multibyte (after which
 * it doesn't touch str anymore). Only after that it starts modifying
 * g_buf. This is a phased operation.
 */
static char *unescape(const char *str, uint maxcols)
{
	static wchar_t wbuf[PATH_MAX] __attribute__ ((aligned));
	static wchar_t *buf;
	static size_t len;

	/* Convert multi-byte to wide char */
	len = mbstowcs(wbuf, str, PATH_MAX);

	g_buf[0] = '\0';
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
	wcstombs(g_buf, wbuf, PATH_MAX);
	return g_buf;
}

static char *coolsize(off_t size)
{
	static const char * const U = "BKMGTPEZY";
	static char size_buf[12]; /* Buffer to hold human readable size */
	static off_t rem;
	static int i;

	i = 0;
	rem = 0;

	while (size > 1024) {
		rem = size & (0x3FF); /* 1024 - 1 = 0x3FF */
		size >>= 10;
		++i;
	}

	if (i == 1) {
		rem = (rem * 1000) >> 10;

		rem /= 10;
		if (rem % 10 >= 5) {
			rem = (rem / 10) + 1;
			if (rem == 10) {
				++size;
				rem = 0;
			}
		} else
			rem /= 10;
	} else if (i == 2) {
		rem = (rem * 1000) >> 10;

		if (rem % 10 >= 5) {
			rem = (rem / 10) + 1;
			if (rem == 100) {
				++size;
				rem = 0;
			}
		} else
			rem /= 10;
	} else if (i > 0) {
		rem = (rem * 10000) >> 10;

		if (rem % 10 >= 5) {
			rem = (rem / 10) + 1;
			if (rem == 1000) {
				++size;
				rem = 0;
			}
		} else
			rem /= 10;
	}

	if (i > 0 && i < 6)
		snprintf(size_buf, 12, "%lu.%0*lu%c", (ulong)size, i, (ulong)rem, U[i]);
	else
		snprintf(size_buf, 12, "%lu%c", (ulong)size, U[i]);

	return size_buf;
}

static char *get_file_sym(mode_t mode)
{
	static char ind[2] = "\0\0";

	if (S_ISDIR(mode))
		ind[0] = '/';
	else if (S_ISLNK(mode))
		ind[0] = '@';
	else if (S_ISSOCK(mode))
		ind[0] = '=';
	else if (S_ISFIFO(mode))
		ind[0] = '|';
	else if (mode & 0100)
		ind[0] = '*';
	else
		ind[0] = '\0';

	return ind;
}

static void printent(struct entry *ent, int sel, uint namecols)
{
	static char *pname;

	pname = unescape(ent->name, namecols);

	/* Directories are always shown on top */
	resetdircolor(ent->mode);

	printw("%s%s%s\n", CURSYM(sel), pname, get_file_sym(ent->mode));
}

static void printent_long(struct entry *ent, int sel, uint namecols)
{
	static char buf[18], *pname;

	strftime(buf, 18, "%F %R", localtime(&ent->t));
	pname = unescape(ent->name, namecols);

	/* Directories are always shown on top */
	resetdircolor(ent->mode);

	if (sel)
		attron(A_REVERSE);

	if (S_ISDIR(ent->mode)) {
		if (cfg.blkorder)
			printw("%s%-16.16s %8.8s/ %s/\n", CURSYM(sel), buf, coolsize(ent->blocks << BLK_SHIFT), pname);
		else
			printw("%s%-16.16s        /  %s/\n", CURSYM(sel), buf, pname);
	} else if (S_ISLNK(ent->mode)) {
		if (ent->flags & SYMLINK_TO_DIR)
			printw("%s%-16.16s       @/  %s@\n", CURSYM(sel), buf, pname);
		else
			printw("%s%-16.16s        @  %s@\n", CURSYM(sel), buf, pname);
	} else if (S_ISSOCK(ent->mode))
		printw("%s%-16.16s        =  %s=\n", CURSYM(sel), buf, pname);
	else if (S_ISFIFO(ent->mode))
		printw("%s%-16.16s        |  %s|\n", CURSYM(sel), buf, pname);
	else if (S_ISBLK(ent->mode))
		printw("%s%-16.16s        b  %s\n", CURSYM(sel), buf, pname);
	else if (S_ISCHR(ent->mode))
		printw("%s%-16.16s        c  %s\n", CURSYM(sel), buf, pname);
	else if (ent->mode & 0100) {
		if (cfg.blkorder)
			printw("%s%-16.16s %8.8s* %s*\n", CURSYM(sel), buf, coolsize(ent->blocks << BLK_SHIFT), pname);
		else
			printw("%s%-16.16s %8.8s* %s*\n", CURSYM(sel), buf, coolsize(ent->size), pname);
	} else {
		if (cfg.blkorder)
			printw("%s%-16.16s %8.8s  %s\n", CURSYM(sel), buf, coolsize(ent->blocks << BLK_SHIFT), pname);
		else
			printw("%s%-16.16s %8.8s  %s\n", CURSYM(sel), buf, coolsize(ent->size), pname);
	}

	if (sel)
		attroff(A_REVERSE);
}

static void (*printptr)(struct entry *ent, int sel, uint namecols) = &printent_long;

static char get_fileind(mode_t mode, char *desc)
{
	static char c;

	if (S_ISREG(mode)) {
		c = '-';
		xstrlcpy(desc, "regular file", DESCRIPTOR_LEN);
		if (mode & 0100)
			xstrlcpy(desc + 12, ", executable", DESCRIPTOR_LEN - 12); /* Length of string "regular file" is 12 */
	} else if (S_ISDIR(mode)) {
		c = 'd';
		xstrlcpy(desc, "directory", DESCRIPTOR_LEN);
	} else if (S_ISBLK(mode)) {
		c = 'b';
		xstrlcpy(desc, "block special device", DESCRIPTOR_LEN);
	} else if (S_ISCHR(mode)) {
		c = 'c';
		xstrlcpy(desc, "character special device", DESCRIPTOR_LEN);
#ifdef S_ISFIFO
	} else if (S_ISFIFO(mode)) {
		c = 'p';
		xstrlcpy(desc, "FIFO", DESCRIPTOR_LEN);
#endif  /* S_ISFIFO */
#ifdef S_ISLNK
	} else if (S_ISLNK(mode)) {
		c = 'l';
		xstrlcpy(desc, "symbolic link", DESCRIPTOR_LEN);
#endif  /* S_ISLNK */
#ifdef S_ISSOCK
	} else if (S_ISSOCK(mode)) {
		c = 's';
		xstrlcpy(desc, "socket", DESCRIPTOR_LEN);
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
static char *get_lsperms(mode_t mode, char *desc)
{
	static const char * const rwx[] = {"---", "--x", "-w-", "-wx", "r--", "r-x", "rw-", "rwx"};
	static char bits[11] = {'\0'};

	bits[0] = get_fileind(mode, desc);
	xstrlcpy(&bits[1], rwx[(mode >> 6) & 7], 4);
	xstrlcpy(&bits[4], rwx[(mode >> 3) & 7], 4);
	xstrlcpy(&bits[7], rwx[(mode & 7)], 4);

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
static char *get_output(char *buf, size_t bytes, char *file, char *arg1, char *arg2, int pager)
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

static char *xgetpwuid(uid_t uid)
{
	struct passwd *pwd = getpwuid(uid);

	if (!pwd)
		return utils[UNKNOWN];

	return pwd->pw_name;
}

static char *xgetgrgid(gid_t gid)
{
	struct group *grp = getgrgid(gid);

	if (!grp)
		return utils[UNKNOWN];

	return grp->gr_name;
}

/*
 * Follows the stat(1) output closely
 */
static int show_stats(char *fpath, char *fname, struct stat *sb)
{
	char desc[DESCRIPTOR_LEN];
	char *perms = get_lsperms(sb->st_mode, desc);
	char *p, *begin = g_buf;

	if (g_tmpfpath[0])
		xstrlcpy(g_tmpfpath + g_tmpfplen - 1, "/.nnnXXXXXX", MAX_HOME_LEN - g_tmpfplen);
	else {
		printmsg(messages[STR_NOHOME_ID]);
		return -1;
	}

	int fd = mkstemp(g_tmpfpath);

	if (fd == -1)
		return -1;

	dprintf(fd, "    File: '%s'", unescape(fname, 0));

	/* Show file name or 'symlink' -> 'target' */
	if (perms[0] == 'l') {
		/* Note that MAX_CMD_LEN > PATH_MAX */
		ssize_t len = readlink(fpath, g_buf, MAX_CMD_LEN);

		if (len != -1) {
			struct stat tgtsb;
			if (!stat(fpath, &tgtsb) && S_ISDIR(tgtsb.st_mode))
				g_buf[len++] = '/';

			g_buf[len] = '\0';

			/*
			 * We pass g_buf but unescape() operates on g_buf too!
			 * Read the API notes for information on how this works.
			 */
			dprintf(fd, " -> '%s'", unescape(g_buf, 0));
		}
	}

	/* Show size, blocks, file type */
#if defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__) || defined(__APPLE__)
	dprintf(fd, "\n    Size: %-15lld Blocks: %-10lld IO Block: %-6d %s",
	       (long long)sb->st_size, (long long)sb->st_blocks, sb->st_blksize, desc);
#else
	dprintf(fd, "\n    Size: %-15ld Blocks: %-10ld IO Block: %-6ld %s",
	       sb->st_size, sb->st_blocks, (long)sb->st_blksize, desc);
#endif

	/* Show containing device, inode, hardlink count */
#if defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__) || defined(__APPLE__)
	snprintf(g_buf, 32, "%xh/%ud", sb->st_dev, sb->st_dev);
	dprintf(fd, "\n  Device: %-15s Inode: %-11llu Links: %-9hu",
		g_buf, (unsigned long long)sb->st_ino, sb->st_nlink);
#else
	snprintf(g_buf, 32, "%lxh/%lud", (ulong)sb->st_dev, (ulong)sb->st_dev);
	dprintf(fd, "\n  Device: %-15s Inode: %-11lu Links: %-9lu",
		g_buf, sb->st_ino, (ulong)sb->st_nlink);
#endif

	/* Show major, minor number for block or char device */
	if (perms[0] == 'b' || perms[0] == 'c')
		dprintf(fd, " Device type: %x,%x", major(sb->st_rdev), minor(sb->st_rdev));

	/* Show permissions, owner, group */
	dprintf(fd, "\n  Access: 0%d%d%d/%s Uid: (%u/%s)  Gid: (%u/%s)", (sb->st_mode >> 6) & 7, (sb->st_mode >> 3) & 7,
		sb->st_mode & 7, perms, sb->st_uid, xgetpwuid(sb->st_uid), sb->st_gid, xgetgrgid(sb->st_gid));

	/* Show last access time */
	strftime(g_buf, 40, messages[STR_DATE_ID], localtime(&sb->st_atime));
	dprintf(fd, "\n\n  Access: %s", g_buf);

	/* Show last modification time */
	strftime(g_buf, 40, messages[STR_DATE_ID], localtime(&sb->st_mtime));
	dprintf(fd, "\n  Modify: %s", g_buf);

	/* Show last status change time */
	strftime(g_buf, 40, messages[STR_DATE_ID], localtime(&sb->st_ctime));
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
	get_output(NULL, 0, "cat", g_tmpfpath, NULL, 1);
	unlink(g_tmpfpath);
	refresh();
	return 0;
}

static size_t get_fs_info(const char *path, bool type)
{
	static struct statvfs svb;

	if (statvfs(path, &svb) == -1)
		return 0;

	if (type == CAPACITY)
		return svb.f_blocks << ffs(svb.f_bsize >> 1);

	return svb.f_bavail << ffs(svb.f_frsize >> 1);
}

static int show_mediainfo(char *fpath, char *arg)
{
	if (!get_output(g_buf, MAX_CMD_LEN, "which", utils[cfg.metaviewer], NULL, 0))
		return -1;

	exitcurses();
	get_output(NULL, 0, utils[cfg.metaviewer], fpath, arg, 1);
	refresh();
	return 0;
}

static int handle_archive(char *fpath, char *arg, char *dir)
{
	if (!get_output(g_buf, MAX_CMD_LEN, "which", utils[ATOOL], NULL, 0))
		return -1;

	if (arg[1] == 'x')
		spawn(utils[ATOOL], arg, fpath, dir, F_NORMAL);
	else {
		exitcurses();
		get_output(NULL, 0, utils[ATOOL], arg, fpath, 1);
		refresh();
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
static int show_help(char *path)
{
	if (g_tmpfpath[0])
		xstrlcpy(g_tmpfpath + g_tmpfplen - 1, "/.nnnXXXXXX", MAX_HOME_LEN - g_tmpfplen);
	else {
		printmsg(messages[STR_NOHOME_ID]);
		return -1;
	}

	int i = 0, fd = mkstemp(g_tmpfpath);
	char *start, *end;
	static char helpstr[] = {
"0\n"
"1NAVIGATION\n"
      "7↑, k, ^P  Up             PgUp, ^U  Scroll up\n"
      "7↓, j, ^N  Down           PgDn, ^D  Scroll down\n"
"1Home, g, ^, ^A  First entry           ~  Go HOME\n"
 "2End, G, $, ^E  Last entry            &  Start dir\n"
"1←, Bksp, h, ^H  Parent dir            -  Last visited dir\n"
   "4→, ↵, l, ^M  Open file/enter dir   .  Toggle show hidden\n"
             "e/  Filter          Ins, ^I  Toggle nav-as-you-type\n"
             "eb  Pin current dir      ^B  Go to pinned dir\n"
         "a`, ^/  Leader key      LeaderN  Switch to context N\n"
           "cEsc  Exit prompt          ^L  Redraw, clear prompt\n"
            "d^G  Quit and cd           q  Quit context\n"
         "aQ, ^Q  Quit                  ?  Help, settings\n"
"1FILES\n"
            "d^O  Open with...          n  Create new\n"
             "eD  File details          d  Toggle detail view\n"
            "d^R  Rename entry          r  Open dir in vidir\n"
            "d^Y  Toggle selection      y  List selection\n"
         "a⎵, ^K  Copy entry path      ^T  Toggle path quote\n"
             "eP  Copy selection       ^X  Delete selection\n"
             "eV  Move selection        X  Delete entry\n"
             "ef  Archive entry         F  List archive\n"
            "d^F  Extract archive    m, M  Brief/full media info\n"
             "ee  Edit in EDITOR        p  Open in PAGER\n"
"1ORDER\n"
            "d^J  Toggle du mode        S  Toggle apparent size\n"
             "es  Toggle sort by size   t  Toggle sort by mtime\n"
"1MISC\n"
             "eo  Launch GUI app    !, ^]  Spawn SHELL in dir\n"
             "eR  Run custom script     L  Lock terminal\n"};

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

	dprintf(fd, "\nVOLUME: %s of ", coolsize(get_fs_info(path, FREE)));
	dprintf(fd, "%s free\n\n", coolsize(get_fs_info(path, CAPACITY)));

	if (getenv("NNN_BMS")) {
		dprintf(fd, "BOOKMARKS\n");
		for (; i < BM_MAX; ++i)
			if (bookmark[i].key)
				dprintf(fd, " %c: %s\n", (char)bookmark[i].key, bookmark[i].loc);
			else
				break;
		dprintf(fd, "\n");
	}

	if (editor)
		dprintf(fd, "NNN_USE_EDITOR: %s\n", editor);
	if (idletimeout)
		dprintf(fd, "NNN_IDLE_TIMEOUT: %d secs\n", idletimeout);
	if (copier)
		dprintf(fd, "NNN_COPIER: %s\n", copier);
	else if (g_cppath[0])
		dprintf(fd, "copy file: %s\n", g_cppath);
	if (getenv("NNN_SCRIPT"))
		dprintf(fd, "NNN_SCRIPT: %s\n", getenv("NNN_SCRIPT"));
	if (getenv("NNN_MULTISCRIPT"))
		dprintf(fd, "NNN_MULTISCRIPT: %s\n", getenv("NNN_MULTISCRIPT"));
	if (getenv("NNN_SHOW_HIDDEN"))
		dprintf(fd, "NNN_SHOW_HIDDEN: %s\n", getenv("NNN_SHOW_HIDDEN"));

	dprintf(fd, "\n");

	if (getenv("PWD"))
		dprintf(fd, "PWD: %s\n", getenv("PWD"));
	if (getenv("SHELL"))
		dprintf(fd, "SHELL: %s\n", getenv("SHELL"));
	if (getenv("SHLVL"))
		dprintf(fd, "SHLVL: %s\n", getenv("SHLVL"));
	if (getenv("VISUAL"))
		dprintf(fd, "VISUAL: %s\n", getenv("VISUAL"));
	else if (getenv("EDITOR"))
		dprintf(fd, "EDITOR: %s\n", getenv("EDITOR"));
	if (getenv("PAGER"))
		dprintf(fd, "PAGER: %s\n", getenv("PAGER"));

	dprintf(fd, "\nVersion: %s\n%s\n", VERSION, GENERAL_INFO);
	close(fd);

	exitcurses();
	get_output(NULL, 0, "cat", g_tmpfpath, NULL, 1);
	unlink(g_tmpfpath);
	refresh();
	return 0;
}

static int sum_bsizes(const char *fpath, const struct stat *sb,
	   int typeflag, struct FTW *ftwbuf)
{
	if (sb->st_blocks && (typeflag == FTW_F || typeflag == FTW_D))
		ent_blocks += sb->st_blocks;

	++num_files;
	return 0;
}

static int sum_sizes(const char *fpath, const struct stat *sb,
	   int typeflag, struct FTW *ftwbuf)
{
	if (sb->st_size && (typeflag == FTW_F || typeflag == FTW_D))
		ent_blocks += sb->st_size;

	++num_files;
	return 0;
}

static void dentfree(struct entry *dents)
{
	free(pnamebuf);
	free(dents);
}

static int dentfill(char *path, struct entry **dents,
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

					if (nftw(g_buf, nftw_fn, open_max, FTW_MOUNT | FTW_PHYS) == -1) {
						printmsg(messages[STR_NFTWFAIL_ID]);
						dir_blocks += (cfg.apparentsz ? sb.st_size : sb.st_blocks);
					} else
						dir_blocks += ent_blocks;
				}
			} else {
				dir_blocks += (cfg.apparentsz ? sb.st_size : sb.st_blocks);
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
			*dents = xrealloc(*dents, total_dents * sizeof(**dents));
			if (*dents == NULL) {
				free(pnamebuf);
				errexit();
			}
			DPRINTF_P(*dents);
		}

		/* If there's not enough bytes left to copy a file name of length NAME_MAX, re-allocate */
		if (namebuflen - off < NAME_MAX + 1) {
			namebuflen += NAMEBUF_INCR;

			pnb = pnamebuf;
			pnamebuf = (char *)xrealloc(pnamebuf, namebuflen);
			if (pnamebuf == NULL) {
				free(*dents);
				errexit();
			}
			DPRINTF_P(pnamebuf);

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

				if (nftw(g_buf, nftw_fn, open_max, FTW_MOUNT | FTW_PHYS) == -1) {
					printmsg(messages[STR_NFTWFAIL_ID]);
					dentp->blocks = (cfg.apparentsz ? sb.st_size : sb.st_blocks);
				} else
					dentp->blocks = ent_blocks;

				if (sb_path.st_dev == sb.st_dev)
					dir_blocks += dentp->blocks;
				else
					num_files = num_saved;
			} else {
				dentp->blocks = (cfg.apparentsz ? sb.st_size : sb.st_blocks);
				dir_blocks += dentp->blocks;
				++num_files;
			}
		}

		/* Flag if this is a symlink to a dir */
		if (S_ISLNK(sb.st_mode))
			if (!fstatat(fd, namep, &sb, 0)) {
				if (S_ISDIR(sb.st_mode))
					dentp->flags |= SYMLINK_TO_DIR;
				else
					dentp->flags &= ~SYMLINK_TO_DIR;
			}

		++n;
	}

	/* Should never be null */
	if (closedir(dirp) == -1) {
		dentfree(*dents);
		errexit();
	}

	return n;
}

/* Return the position of the matching entry or 0 otherwise */
static int dentfind(struct entry *dents, const char *fname, int n)
{
	static int i;

	if (!fname)
		return 0;

	DPRINTF_S(fname);

	for (i = 0; i < n; ++i)
		if (strcmp(fname, dents[i].name) == 0)
			return i;

	return 0;
}

static int populate(char *path, char *lastname, char *fltr)
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
		printmsg("calculating...");
		refresh();
	}

#ifdef DEBUGMODE
	struct timespec ts1, ts2;

	clock_gettime(CLOCK_REALTIME, &ts1); /* Use CLOCK_MONOTONIC on FreeBSD */
#endif

	ndents = dentfill(path, &dents, visible, &re);
	regfree(&re);
	if (!ndents)
		return 0;

	qsort(dents, ndents, sizeof(*dents), entrycmp);

#ifdef DEBUGMODE
	clock_gettime(CLOCK_REALTIME, &ts2);
	DPRINTF_U(ts2.tv_nsec - ts1.tv_nsec);
#endif

	/* Find cur from history */
	cur = dentfind(dents, lastname, ndents);
	return 0;
}

static void redraw(char *path)
{
	static char buf[NAME_MAX + 65] __attribute__ ((aligned));
	static size_t ncols;
	static int nlines, i;
	static bool mode_changed;

	mode_changed = FALSE;
	nlines = MIN(LINES - 4, ndents);

	/* Clear screen */
	erase();
	if (cfg.copymode)
		if (g_crc != crc8fast((uchar *)dents, ndents * sizeof(struct entry))) {
			cfg.copymode = 0;
			DPRINTF_S("copymode off");
		}

	/* Fail redraw if < than 11 columns, context info prints 10 chars */
	if (COLS < 11) {
		printmsg("too few columns!");
		return;
	}

	/* Strip trailing slashes */
	for (i = strlen(path) - 1; i > 0; --i)
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

	printw("[");
	for (i = 0; i < MAX_CTX; ++i) {
		/* Print current context in reverse */
		if (cfg.curctx == i) {
			attron(A_REVERSE);
			printw("%d", i + 1);
			attroff(A_REVERSE);
			printw(" ");
		} else if (g_ctx[i].c_cfg.ctxactive) {
			attron(A_UNDERLINE);
			printw("%d", i + 1);
			attroff(A_UNDERLINE);
			printw(" ");
		} else
			printw("%d ", i + 1);
	}
	printw("\b] "); /* 10 chars printed in total for contexts - "[1 2 3 4] " */

	attron(A_UNDERLINE);
	/* No text wrapping in cwd line */
	g_buf[ncols - 11] = '\0';
	printw("%s\n\n", g_buf);
	attroff(A_UNDERLINE);

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
			static char sort[9];

			if (cfg.mtimeorder)
				xstrlcpy(sort, "by time ", 9);
			else if (cfg.sizeorder)
				xstrlcpy(sort, "by size ", 9);
			else
				sort[0] = '\0';

			/* We need to show filename as it may be truncated in directory listing */
			if (!cfg.blkorder)
				snprintf(buf, NAME_MAX + 65, "%d/%d %s[%s%s]",
					 cur + 1, ndents, sort, unescape(dents[cur].name, NAME_MAX), get_file_sym(dents[cur].mode));
			else {
				i = snprintf(buf, 64, "%d/%d ", cur + 1, ndents);

				if (cfg.apparentsz)
					buf[i++] = 'a';
				else
					buf[i++] = 'd';

				i += snprintf(buf + i, 64, "u: %s (%lu files) ", coolsize(dir_blocks << BLK_SHIFT), num_files);
				snprintf(buf + i, NAME_MAX, "vol: %s free [%s%s]",
					 coolsize(get_fs_info(path, FREE)), unescape(dents[cur].name, NAME_MAX), get_file_sym(dents[cur].mode));
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

static void browse(char *ipath)
{
	static char newpath[PATH_MAX] __attribute__ ((aligned));
	static char mark[PATH_MAX] __attribute__ ((aligned));
	char *path, *lastdir, *lastname, *hfltr;
	char *dir, *tmp, *run = NULL, *env = NULL;
	struct stat sb;
	int r, fd, presel, ncp = 0, copystartid = 0, copyendid = 0;
	enum action sel = SEL_RUNARG + 1;
	bool dir_changed = FALSE;

	/* setup first context */
	xstrlcpy(g_ctx[0].c_path, ipath, PATH_MAX); /* current directory */
	path = g_ctx[0].c_path;
	xstrlcpy(g_ctx[0].c_init, ipath, PATH_MAX); /* start directory */
	g_ctx[0].c_last[0] = g_ctx[0].c_name[0] = newpath[0] = mark[0] = '\0';
	lastdir = g_ctx[0].c_last; /* last visited directory */
	lastname = g_ctx[0].c_name; /* last visited filename */
	g_ctx[0].c_cfg = cfg; /* current configuration */
	initfilter(cfg.showhidden, g_ctx[0].c_fltr); /* Show hidden filter */
	hfltr = g_ctx[0].c_fltr;

	if (cfg.filtermode)
		presel = FILTER;
	else
		presel = 0;

	dents = xrealloc(dents, total_dents * sizeof(struct entry));
	if (dents == NULL)
		errexit();
	DPRINTF_P(dents);

	/* Allocate buffer to hold names */
	pnamebuf = (char *)xrealloc(pnamebuf, NAMEBUF_INCR);
	if (pnamebuf == NULL) {
		free(dents);
		errexit();
	}
	DPRINTF_P(pnamebuf);

begin:
#ifdef LINUX_INOTIFY
	if ((presel == FILTER || dir_changed) && inotify_wd >= 0) {
		inotify_rm_watch(inotify_fd, inotify_wd);
		inotify_wd = -1;
		dir_changed = FALSE;
	}
#elif defined(BSD_KQUEUE)
	if ((presel == FILTER || dir_changed) && event_fd >= 0) {
		close(event_fd);
		event_fd = -1;
		dir_changed = FALSE;
	}
#endif

	if (populate(path, lastname, hfltr) == -1) {
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
				/* Continue in navigate-as-you-type mode, if enabled */
				if (cfg.filtermode)
					presel = FILTER;
				goto nochange;
			}

			dir = xdirname(path);
			if (access(dir, R_OK) == -1) {
				printwarn();
				goto nochange;
			}

			/* Save history */
			xstrlcpy(lastname, xbasename(path), NAME_MAX + 1);

			/* Save last working directory */
			xstrlcpy(lastdir, path, PATH_MAX);

			xstrlcpy(path, dir, PATH_MAX);

			setdirwatch();
			goto begin;
		case SEL_GOIN:
			/* Cannot descend in empty directories */
			if (!ndents)
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

				xstrlcpy(path, newpath, PATH_MAX);
				lastname[0] = '\0';
				setdirwatch();
				goto begin;
			case S_IFREG:
			{
				/* If NNN_USE_EDITOR is set,
				 * open text in EDITOR
				 */
				if (editor) {
					if (getmime(dents[cur].name)) {
						spawn(editor, newpath, NULL, path, F_NORMAL);
						continue;
					}

					/* Recognize and open plain
					 * text files with vi
					 */
					if (get_output(g_buf, MAX_CMD_LEN, "file", "-bi", newpath, 0) == NULL)
						continue;

					if (strstr(g_buf, "text/") == g_buf) {
						spawn(editor, newpath, NULL, path, F_NORMAL);
						continue;
					}
				}

				/* Invoke desktop opener as last resort */
				spawn(utils[OPENER], newpath, NULL, NULL, F_NOWAIT | F_NOTRACE);
				continue;
			}
			default:
				printmsg("unsupported file");
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
		case SEL_CDHOME:
			dir = getenv("HOME");
			if (dir == NULL) {
				clearprompt();
				goto nochange;
			} // fallthrough
		case SEL_CDBEGIN:
			if (sel == SEL_CDBEGIN)
				dir = ipath;

			if (!xdiraccess(dir))
				goto nochange;

			if (strcmp(path, dir) == 0)
				break;

			/* Save last working directory */
			xstrlcpy(lastdir, path, PATH_MAX);

			xstrlcpy(path, dir, PATH_MAX);
			lastname[0] = '\0';
			DPRINTF_S(path);
			setdirwatch();
			goto begin;
		case SEL_CDLAST: // fallthrough
		case SEL_VISIT:
			if (sel == SEL_VISIT) {
				if (strcmp(mark, path) == 0)
					break;

				tmp = mark;
			} else
				tmp = lastdir;

			if (tmp[0] == '\0') {
				printmsg("not set");
				goto nochange;
			}

			if (!xdiraccess(tmp))
				goto nochange;

			xstrlcpy(newpath, tmp, PATH_MAX);
			xstrlcpy(lastdir, path, PATH_MAX);
			xstrlcpy(path, newpath, PATH_MAX);
			lastname[0] = '\0';
			DPRINTF_S(path);
			setdirwatch();
			goto begin;
		case SEL_LEADER:
			fd = get_input(NULL);
			switch (fd) {
			case 'q': //fallthrough
			case '~': //fallthrough
			case '-': //fallthrough
			case '&':
				presel = fd;
				goto nochange;
			case '>':
			case '.':
			case '<':
			case ',':
				r = cfg.curctx;
				if (fd == '>' || fd == '.')
					do
						(r == MAX_CTX - 1) ? (r = 0) : ++r;
					while (!g_ctx[r].c_cfg.ctxactive);
				else
					do
						(r == 0) ? (r = MAX_CTX - 1) : --r;
					while (!g_ctx[r].c_cfg.ctxactive); //fallthrough
				fd = '1' + r; //fallthrough
			case '1': //fallthrough
			case '2': //fallthrough
			case '3': //fallthrough
			case '4':
				r = fd - '1'; /* Save the next context id */
				if (cfg.curctx == r)
					continue;

				g_crc = 0;

				/* Save current context */
				xstrlcpy(g_ctx[cfg.curctx].c_name, dents[cur].name, NAME_MAX + 1);
				g_ctx[cfg.curctx].c_cfg = cfg;

				if (g_ctx[r].c_cfg.ctxactive) /* Switch to saved context */
					cfg = g_ctx[r].c_cfg;
				else { /* Setup a new context from current context */
					g_ctx[r].c_cfg.ctxactive = 1;
					xstrlcpy(g_ctx[r].c_path, path, PATH_MAX);
					xstrlcpy(g_ctx[r].c_init, path, PATH_MAX);
					g_ctx[r].c_last[0] = '\0';
					xstrlcpy(g_ctx[r].c_name, dents[cur].name, NAME_MAX + 1);
					g_ctx[r].c_cfg = cfg;
					xstrlcpy(g_ctx[r].c_fltr, hfltr, DOT_FILTER_LEN);
				}

				/* Reset the pointers */
				path = g_ctx[r].c_path;
				ipath = g_ctx[r].c_init;
				lastdir = g_ctx[r].c_last;
				lastname = g_ctx[r].c_name;
				hfltr = g_ctx[r].c_fltr;

				cfg.curctx = r;
				setdirwatch();
				goto begin;
			}

			if (get_bm_loc(fd, newpath) == NULL) {
				printmsg(messages[STR_INVBM_KEY]);
				goto nochange;
			}

			if (!xdiraccess(newpath))
				goto nochange;

			if (strcmp(path, newpath) == 0)
				break;

			lastname[0] = '\0';

			/* Save last working directory */
			xstrlcpy(lastdir, path, PATH_MAX);

			/* Save the newly opted dir in path */
			xstrlcpy(path, newpath, PATH_MAX);
			DPRINTF_S(path);

			setdirwatch();
			goto begin;
		case SEL_PIN:
			xstrlcpy(mark, path, PATH_MAX);
			printmsg(mark);
			goto nochange;
		case SEL_FLTR:
			presel = filterentries(path);
			/* Save current */
			if (ndents)
				copycurname();
			goto nochange;
		case SEL_MFLTR:
			cfg.filtermode ^= 1;
			if (cfg.filtermode) {
				presel = FILTER;
				goto nochange;
			}

			/* Save current */
			if (ndents)
				copycurname();

			dir_changed = TRUE;
			/* Start watching the directory */
			goto begin;
		case SEL_TOGGLEDOT:
			cfg.showhidden ^= 1;
			initfilter(cfg.showhidden, hfltr);
			/* Save current */
			if (ndents)
				copycurname();
			goto begin;
		case SEL_DETAIL:
			cfg.showdetail ^= 1;
			cfg.showdetail ? (printptr = &printent_long) : (printptr = &printent);
			/* Save current */
			if (ndents)
				copycurname();
			goto begin;
		case SEL_STATS:
			if (!ndents)
				break;

			mkpath(path, dents[cur].name, newpath, PATH_MAX);

			if (lstat(newpath, &sb) == -1) {
				dentfree(dents);
				errexit();
			}

			if (show_stats(newpath, dents[cur].name, &sb) < 0) {
				printwarn();
				goto nochange;
			}
			break;
		case SEL_LIST: // fallthrough
		case SEL_EXTRACT: // fallthrough
		case SEL_MEDIA: // fallthrough
		case SEL_FMEDIA:
			if (!ndents)
				break;

			mkpath(path, dents[cur].name, newpath, PATH_MAX);

			if (sel == SEL_MEDIA || sel == SEL_FMEDIA)
				r = show_mediainfo(newpath, run);
			else
				r = handle_archive(newpath, run, path);

			if (r == -1) {
				xstrlcpy(newpath, "missing ", PATH_MAX);
				if (sel == SEL_MEDIA || sel == SEL_FMEDIA)
					xstrlcpy(newpath + 8, utils[cfg.metaviewer], 32);
				else
					xstrlcpy(newpath + 8, utils[ATOOL], 32);

				printmsg(newpath);
				goto nochange;
			}

			/* In case of successful archive extract, reload contents */
			if (sel == SEL_EXTRACT) {
				/* Continue in navigate-as-you-type mode, if enabled */
				if (cfg.filtermode)
					presel = FILTER;

				/* Save current */
				copycurname();

				/* Repopulate as directory content may have changed */
				goto begin;
			}
			break;
		case SEL_FSIZE:
			cfg.sizeorder ^= 1;
			cfg.mtimeorder = 0;
			cfg.apparentsz = 0;
			cfg.blkorder = 0;
			cfg.copymode = 0;
			/* Save current */
			if (ndents)
				copycurname();
			goto begin;
		case SEL_ASIZE:
			cfg.apparentsz ^= 1;
			if (cfg.apparentsz) {
				nftw_fn = &sum_sizes;
				cfg.blkorder = 1;
				BLK_SHIFT = 0;
			} else
				cfg.blkorder = 0; // fallthrough
		case SEL_BSIZE:
			if (sel == SEL_BSIZE) {
				if (!cfg.apparentsz)
					cfg.blkorder ^= 1;
				nftw_fn = &sum_bsizes;
				cfg.apparentsz = 0;
				BLK_SHIFT = get_blk_shift();
			}

			if (cfg.blkorder) {
				cfg.showdetail = 1;
				printptr = &printent_long;
			}
			cfg.mtimeorder = 0;
			cfg.sizeorder = 0;
			cfg.copymode = 0;
			/* Save current */
			if (ndents)
				copycurname();
			goto begin;
		case SEL_MTIME:
			cfg.mtimeorder ^= 1;
			cfg.sizeorder = 0;
			cfg.apparentsz = 0;
			cfg.blkorder = 0;
			cfg.copymode = 0;
			/* Save current */
			if (ndents)
				copycurname();
			goto begin;
		case SEL_REDRAW:
			/* Save current */
			if (ndents)
				copycurname();
			goto begin;
		case SEL_COPY:
			if (!ndents)
				goto nochange;

			if (cfg.copymode) {
				/*
				 * Clear the tmp copy path file on first copy.
				 *
				 * This ensures that when the first file path is
				 * copied into memory (but not written to tmp file
				 * yet to save on writes), the tmp file is cleared.
				 * The user may be in the middle of selection mode op
				 * and issue a cp, mv of multi-rm assuming the files
				 * in the copy list would be affected. However, these
				 * ops read the source file paths from the tmp file.
				 */
				if (!ncp)
					writecp(NULL, 0);

				r = mkpath(path, dents[cur].name, newpath, PATH_MAX);
				if (!appendfpath(newpath, r))
					goto nochange;

				++ncp;
				printmsg(newpath);
			} else if (cfg.quote) {
				g_buf[0] = '\'';
				r = mkpath(path, dents[cur].name, g_buf + 1, PATH_MAX);
				/* Keep the copy buf in sync */
				copybufpos = 0;
				appendfpath(g_buf + 1, r);

				g_buf[r] = '\'';
				g_buf[r + 1] = '\0';

				writecp(g_buf, r + 1); /* Truncate NULL from end */
				if (copier)
					spawn(copier, g_buf, NULL, NULL, F_NOTRACE);

				g_buf[r] = '\0';
				printmsg(g_buf + 1);
			} else {
				r = mkpath(path, dents[cur].name, newpath, PATH_MAX);
				/* Keep the copy buf in sync */
				copybufpos = 0;
				appendfpath(newpath, r);

				writecp(newpath, r - 1); /* Truncate NULL from end */
				if (copier)
					spawn(copier, newpath, NULL, NULL, F_NOTRACE);

				printmsg(newpath);
			}
			goto nochange;
		case SEL_COPYMUL:
			if (!ndents)
				goto nochange;

			cfg.copymode ^= 1;
			if (cfg.copymode) {
				g_crc = crc8fast((uchar *)dents, ndents * sizeof(struct entry));
				copystartid = cur;
				copybufpos = 0;
				ncp = 0;
				printmsg("selection on");
				DPRINTF_S("copymode on");
				goto nochange;
			}

			if (!ncp) { /* Handle range selection */
				if (cur < copystartid) {
					copyendid = copystartid;
					copystartid = cur;
				} else
					copyendid = cur;

				if (copystartid < copyendid) {
					for (r = copystartid; r <= copyendid; ++r)
						if (!appendfpath(newpath, mkpath(path, dents[r].name, newpath, PATH_MAX)))
							goto nochange;

					snprintf(newpath, PATH_MAX, "%d files copied", copyendid - copystartid + 1);
					printmsg(newpath);
				}
			}

			if (copybufpos) { /* File path(s) written to the buffer */
				writecp(pcopybuf, copybufpos - 1); /* Truncate NULL from end */
				if (copier)
					spawn(copier, pcopybuf, NULL, NULL, F_NOTRACE);

				if (ncp) { /* Some files cherry picked */
					snprintf(newpath, PATH_MAX, "%d files copied", ncp);
					printmsg(newpath);
				}
			} else
				printmsg("selection off");
			goto nochange;
		case SEL_COPYLIST:
			if (copybufpos)
				showcplist();
			else
				printmsg("selection off");
			goto nochange;
		case SEL_CP:
		case SEL_MV:
		case SEL_RMMUL:
		{
			if (!g_cppath[0]) {
				printmsg("copy file not found");
				goto nochange;
			}

			if (sel == SEL_CP)
				snprintf(g_buf, MAX_CMD_LEN, "xargs -0 -d \'\n\' -a %s cp -ir --preserve=all -t .", g_cppath);
			else if (sel == SEL_MV)
				snprintf(g_buf, MAX_CMD_LEN, "xargs -0 -d \'\n\' -a %s mv -i -t .", g_cppath);
			else /* SEL_RMMUL */
				snprintf(g_buf, MAX_CMD_LEN, "xargs -0 -d \'\n\' -a %s rm -Ir", g_cppath);

			spawn("sh", "-c", g_buf, path, F_NORMAL | F_SIGINT);

			copycurname();
			if (cfg.filtermode)
				presel = FILTER;
			goto begin;
		}
		case SEL_RM:
			if (!ndents)
				break;

			mkpath(path, dents[cur].name, newpath, PATH_MAX);
			spawn("rm", "-Ir", newpath, NULL, F_NORMAL | F_SIGINT);

			lastname[0] = '\0';
			if (cfg.filtermode)
				presel = FILTER;
			goto begin;
		case SEL_QUOTE:
			cfg.quote ^= 1;
			DPRINTF_D(cfg.quote);
			if (cfg.quote)
				printmsg("quotes on");
			else
				printmsg("quotes off");
			goto nochange;
		case SEL_OPEN: // fallthrough
		case SEL_ARCHIVE:
			if (!ndents)
				break; // fallthrough
		case SEL_LAUNCH: // fallthrough
		case SEL_NEW:
			if (sel == SEL_OPEN)
				tmp = xreadline(NULL, "open with: ");
			else if (sel == SEL_LAUNCH)
				tmp = xreadline(NULL, "launch: ");
			else if (sel == SEL_ARCHIVE)
				tmp = xreadline(dents[cur].name, "name: ");
			else
				tmp = xreadline(NULL, "name: ");

			if (tmp == NULL || tmp[0] == '\0')
				break;

			/* Allow only relative, same dir paths */
			if ((sel != SEL_LAUNCH) &&
			    (tmp[0] == '/' || strcmp(xbasename(tmp), tmp) != 0)) {
				printmsg(messages[STR_INPUT_ID]);
				goto nochange;
			}

			if (sel == SEL_OPEN) {
				r = get_input("press 'c' for cli mode");
				if (r == 'c')
					r = F_NORMAL;
				else
					r = F_NOWAIT | F_NOTRACE;

				mkpath(path, dents[cur].name, newpath, PATH_MAX);
				spawn(tmp, newpath, NULL, path, r);
				continue;
			}

			if (sel == SEL_LAUNCH) {
				uint args = 0;
				char *ptr = tmp, *ptr1 = NULL, *ptr2 = NULL;

				while (*ptr) {
					if (*ptr == ' ') {
						*ptr = '\0';
						if (args == 0)
							ptr1 = ptr + 1;
						else if (args == 1)
							ptr2 = ptr + 1;
						else
							break;

						++args;
					}

					++ptr;
				}

				spawn(tmp, ptr1, ptr2, path, F_NOWAIT | F_NOTRACE);
				break;
			}

			if (sel == SEL_ARCHIVE) {
				/* newpath is used as temporary buffer */
				if (!get_output(newpath, PATH_MAX, "which", utils[APACK], NULL, 0)) {
					printmsg("apack missing");
					continue;
				}

				spawn(utils[APACK], tmp, dents[cur].name, path, F_NORMAL);

				/* Continue in navigate-as-you-type mode, if enabled */
				if (cfg.filtermode)
					presel = FILTER;

				/* Save current */
				copycurname();

				/* Repopulate as directory content may have changed */
				goto begin;
			}

			/* Open the descriptor to currently open directory */
			fd = open(path, O_RDONLY | O_DIRECTORY);
			if (fd == -1) {
				printwarn();
				goto nochange;
			}

			/* Check if another file with same name exists */
			if (faccessat(fd, tmp, F_OK, AT_SYMLINK_NOFOLLOW) != -1) {
				printmsg("entry exists");
				goto nochange;
			}

			/* Check if it's a dir or file */
			r = get_input("press 'f'(ile) or 'd'(ir)");
			if (r == 'f') {
				r = openat(fd, tmp, O_CREAT, 0666);
				close(r);
			} else if (r == 'd')
				r = mkdirat(fd, tmp, 0777);
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
			xstrlcpy(lastname, tmp, NAME_MAX + 1);
			goto begin;
		case SEL_RENAME:
			if (!ndents)
				break;

			tmp = xreadline(dents[cur].name, "");
			if (tmp == NULL || tmp[0] == '\0')
				break;

			/* Allow only relative, same dir paths */
			if (tmp[0] == '/' || strcmp(xbasename(tmp), tmp) != 0) {
				printmsg(messages[STR_INPUT_ID]);
				goto nochange;
			}

			/* Skip renaming to same name */
			if (strcmp(tmp, dents[cur].name) == 0)
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
				if (get_input("press 'y' to overwrite") != 'y') {
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
			xstrlcpy(lastname, tmp, NAME_MAX + 1);
			goto begin;
		case SEL_RENAMEALL:
			if (!get_output(g_buf, MAX_CMD_LEN, "which", utils[VIDIR], NULL, 0)) {
				printmsg("vidir missing");
				goto nochange;
			}

			spawn(utils[VIDIR], ".", NULL, path, F_NORMAL);

			/* Save current */
			if (ndents)
				copycurname();
			goto begin;
		case SEL_HELP:
			show_help(path);

			/* Continue in navigate-as-you-type mode, if enabled */
			if (cfg.filtermode)
				presel = FILTER;
			break;
		case SEL_RUN: // fallthrough
		case SEL_RUNSCRIPT:
			run = xgetenv(env, run);

			if (sel == SEL_RUNSCRIPT) {
				tmp = getenv("NNN_SCRIPT");
				if (tmp) {
					if (getenv("NNN_MULTISCRIPT")) {
						size_t _len = xstrlcpy(newpath, tmp, PATH_MAX);

						tmp = xreadline(NULL, "script suffix: ");
						if (tmp && tmp[0])
							xstrlcpy(newpath + _len - 1, tmp, PATH_MAX - _len);

						tmp = newpath;
					}

					char *curfile = NULL;

					if (ndents)
						curfile = dents[cur].name;

					spawn(run, tmp, curfile, path, F_NORMAL | F_SIGINT);
				} else
					printmsg("set NNN_SCRIPT");
			} else {
				spawn(run, NULL, NULL, path, F_NORMAL | F_MARKER);

				/* Continue in navigate-as-you-type mode, if enabled */
				if (cfg.filtermode)
					presel = FILTER;
			}

			/* Save current */
			if (ndents)
				copycurname();

			/* Repopulate as directory content may have changed */
			goto begin;
		case SEL_RUNARG:
			run = xgetenv(env, run);
			if ((!run || !run[0]) && (strcmp("VISUAL", env) == 0))
				run = editor ? editor : xgetenv("EDITOR", "vi");
			spawn(run, dents[cur].name, NULL, path, F_NORMAL);
			break;
		case SEL_LOCK:
			spawn(utils[LOCKER], NULL, NULL, NULL, F_NORMAL | F_SIGINT);
			break;
		case SEL_QUITCTX:
		{
			uint iter = 1;
			r = cfg.curctx;
			while (iter < MAX_CTX) {
				(r == MAX_CTX - 1) ? (r = 0) : ++r;
				if (g_ctx[r].c_cfg.ctxactive) {
					g_ctx[cfg.curctx].c_cfg.ctxactive = 0;

					/* Switch to next active context */
					path = g_ctx[r].c_path;
					ipath = g_ctx[r].c_init;
					lastdir = g_ctx[r].c_last;
					lastname = g_ctx[r].c_name;
					cfg = g_ctx[r].c_cfg;
					hfltr = g_ctx[r].c_fltr;

					cfg.curctx = r;
					setdirwatch();
					goto begin;
				}

				++iter;
			}

			dentfree(dents);
			return;
		}
		case SEL_CDQUIT: // fallthrough
		case SEL_QUIT:
			for (r = 0; r < MAX_CTX; ++r)
				if (r != cfg.curctx && g_ctx[r].c_cfg.ctxactive) {
					r = get_input("Quit all contexts? ('Enter' confirms)");
					break;
				}

			if (!(r == MAX_CTX || r == 13))
				break;

			if (sel == SEL_CDQUIT) {
				tmp = getenv("NNN_TMPFILE");
				if (!tmp) {
					printmsg("set NNN_TMPFILE");
					goto nochange;
				}

				FILE *fp = fopen(tmp, "w");

				if (fp) {
					fprintf(fp, "cd \"%s\"", path);
					fclose(fp);
				}
			}

			dentfree(dents);
			return;
		} /* switch (sel) */

		/* Screensaver */
		if (idletimeout != 0 && idle == idletimeout) {
			idle = 0;
			spawn(utils[LOCKER], NULL, NULL, NULL, F_NORMAL | F_SIGINT);
		}
	}
}

static void usage(void)
{
	fprintf(stdout,
		"usage: nnn [-b key] [-c N] [-e] [-i] [-l]\n"
		"           [-S] [-v] [-h] [PATH]\n\n"
		"The missing terminal file manager for X.\n\n"
		"positional args:\n"
		"  PATH   start dir [default: current dir]\n\n"
		"optional args:\n"
		" -b key  bookmark key to open\n"
		" -c N    dir color, disables if N>7\n"
		" -e      use exiftool instead of mediainfo\n"
		" -i      start in navigate-as-you-type mode\n"
		" -l      start in light mode\n"
		" -S      start in disk usage analyser mode\n"
		" -v      show program version\n"
		" -h      show this help\n\n"
		"Version: %s\n%s\n", VERSION, GENERAL_INFO);
	exit(0);
}

int main(int argc, char *argv[])
{
	static char cwd[PATH_MAX] __attribute__ ((aligned));
	char *ipath = NULL;
	int opt;

	// Get platform block shift
	BLK_SHIFT = get_blk_shift();

	/* Confirm we are in a terminal */
	if (!isatty(0) || !isatty(1)) {
		fprintf(stderr, "stdin or stdout is not a tty\n");
		exit(1);
	}

	while ((opt = getopt(argc, argv, "Slib:c:evh")) != -1) {
		switch (opt) {
		case 'S':
			cfg.blkorder = 1;
			nftw_fn = sum_bsizes;
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
			cfg.metaviewer = EXIFTOOL;
			break;
		case 'v':
			fprintf(stdout, "%s\n", VERSION);
			return 0;
		case 'h': // fallthrough
		default:
			usage();
		}
	}

	/* Parse bookmarks string */
	 if (parsebmstr() < 0) {
		fprintf(stderr, "ERROR parsing NNN_BMS: set single-char bookmark keys only\n");
		exit(1);
	 }

	if (ipath) { /* Open a bookmark directly */
		if (ipath[1] || get_bm_loc(*ipath, cwd) == NULL) {
			fprintf(stderr, "%s\n", messages[STR_INVBM_KEY]);
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

	if (getuid() == 0 || getenv("NNN_SHOW_HIDDEN"))
		cfg.showhidden = 1;

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
#endif

	/* Edit text in EDITOR, if opted */
	if (getenv("NNN_USE_EDITOR")) {
		editor = xgetenv("VISUAL", NULL);
		if (!editor)
			editor = xgetenv("EDITOR", "vi");
	}

	/* Get locker wait time, if set; copier used as tmp var */
	copier = getenv("NNN_IDLE_TIMEOUT");
	if (copier) {
		opt = atoi(copier);
		idletimeout = opt * ((opt > 0) - (opt < 0));
	}

	/* Get the default copier, if set */
	copier = getenv("NNN_COPIER");

	/* Enable quotes if opted */
	if (getenv("NNN_QUOTE_ON"))
		cfg.quote = 1;

	if (getenv("HOME"))
		g_tmpfplen = xstrlcpy(g_tmpfpath, getenv("HOME"), MAX_HOME_LEN);
	else if (getenv("TMPDIR"))
		g_tmpfplen = xstrlcpy(g_tmpfpath, getenv("TMPDIR"), MAX_HOME_LEN);
	else if (xdiraccess("/tmp"))
		g_tmpfplen = xstrlcpy(g_tmpfpath, "/tmp", MAX_HOME_LEN);

	if (g_tmpfplen) {
		xstrlcpy(g_cppath, g_tmpfpath, MAX_HOME_LEN);
		xstrlcpy(g_cppath + g_tmpfplen - 1, "/.nnncp", MAX_HOME_LEN - g_tmpfplen);
	}

	/* Disable auto-select if opted */
	if (getenv("NNN_NO_AUTOSELECT"))
		cfg.autoselect = 0;

	signal(SIGINT, SIG_IGN);
	signal(SIGQUIT, SIG_IGN);

	/* Test initial path */
	if (!xdiraccess(ipath)) {
		fprintf(stderr, "%s: %s\n", ipath, strerror(errno));
		exit(1);
	}

	/* Set locale */
	setlocale(LC_ALL, "");
	crc8init();

#ifdef DEBUGMODE
	enabledbg();
#endif
	initcurses();
	browse(ipath);
	exitcurses();

	if (g_cppath[0])
		unlink(g_cppath);

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
