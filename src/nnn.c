/*
 * BSD 2-Clause License
 *
 * Copyright (C) 2014-2016, Lazaros Koromilas <lostd@2f30.org>
 * Copyright (C) 2014-2016, Dimitris Papastamos <sin@2f30.org>
 * Copyright (C) 2016-2019, Arun Prakash Jana <engineerarun@gmail.com>
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
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#if defined(__arm__) || defined(__i386__)
#define _FILE_OFFSET_BITS 64 /* Support large files on 32-bit */
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

#ifdef __linux__ /* Fix failure due to mvaddnwstr() */
#ifndef NCURSES_WIDECHAR
#define NCURSES_WIDECHAR 1
#endif
#elif defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__) || defined(__APPLE__)
#ifndef _XOPEN_SOURCE_EXTENDED
#define _XOPEN_SOURCE_EXTENDED
#endif
#endif
#ifndef __USE_XOPEN /* Fix wcswidth() failure, ncursesw/curses.h includes whcar.h on Ubuntu 14.04 */
#define __USE_XOPEN
#endif
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <libgen.h>
#include <limits.h>
#ifdef __gnu_hurd__
#define PATH_MAX 4096
#endif
#include <locale.h>
#include <stdio.h>
#ifndef NORL
#include <readline/history.h>
#include <readline/readline.h>
#endif
#include <regex.h>
#include <signal.h>
#include <stdarg.h>
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
#include "dbg.h"

/* Macro definitions */
#define VERSION "2.6"
#define GENERAL_INFO "BSD 2-Clause\nhttps://github.com/jarun/nnn"

#ifndef S_BLKSIZE
#define S_BLKSIZE 512 /* S_BLKSIZE is missing on Android NDK (Termux) */
#endif

#define LEN(x) (sizeof(x) / sizeof(*(x)))
#undef MIN
#define MIN(x, y) ((x) < (y) ? (x) : (y))
#undef MAX
#define MAX(x, y) ((x) > (y) ? (x) : (y))
#define ISODD(x) ((x) & 1)
#define ISBLANK(x) ((x) == ' ' || (x) == '\t')
#define TOUPPER(ch) \
	(((ch) >= 'a' && (ch) <= 'z') ? ((ch) - 'a' + 'A') : (ch))
#define CMD_LEN_MAX (PATH_MAX + ((NAME_MAX + 1) << 1))
#define FILTER '/'
#define MSGWAIT '$'
#define REGEX_MAX 48
#define BM_MAX 10
#define PLUGIN_MAX 10
#define ENTRY_INCR 64 /* Number of dir 'entry' structures to allocate per shot */
#define NAMEBUF_INCR 0x800 /* 64 dir entries at once, avg. 32 chars per filename = 64*32B = 2KB */
#define DESCRIPTOR_LEN 32
#define _ALIGNMENT 0x10 /* 16-byte alignment */
#define _ALIGNMENT_MASK 0xF
#define TMP_LEN_MAX 64
#define CTX_MAX 4
#define DOT_FILTER_LEN 7
#define ASCII_MAX 128
#define EXEC_ARGS_MAX 8
#define SCROLLOFF 3
#define LONG_SIZE sizeof(ulong)
#define ARCHIVE_CMD_LEN 16

/* Program return codes */
#define _SUCCESS 0
#define _FAILURE !_SUCCESS

/* Entry flags */
#define DIR_OR_LINK_TO_DIR 0x1
#define FILE_COPIED 0x10

/* Macros to define process spawn behaviour as flags */
#define F_NONE    0x00  /* no flag set */
#define F_MULTI   0x01  /* first arg can be combination of args; to be used with F_NORMAL */
#define F_NOWAIT  0x02  /* don't wait for child process (e.g. file manager) */
#define F_NOTRACE 0x04  /* suppress stdout and strerr (no traces) */
#define F_NORMAL  0x08  /* spawn child process in non-curses regular CLI mode */
#define F_CMD     0x10  /* run command - show results before exit (must have F_NORMAL) */
#define F_CLI     (F_NORMAL | F_MULTI)

/* CRC8 macros */
#define UCHAR_BIT_WIDTH  (sizeof(unsigned char) << 3)
#define TOPBIT (1 << (UCHAR_BIT_WIDTH - 1))
#define POLYNOMIAL 0xD8  /* 11011 followed by 0's */
#define CRC8_TABLE_LEN 256

/* Version compare macros */
/*
 * states: S_N: normal, S_I: comparing integral part, S_F: comparing
 *         fractional parts, S_Z: idem but with leading Zeroes only
 */
#define S_N 0x0
#define S_I 0x3
#define S_F 0x6
#define S_Z 0x9

/* result_type: VCMP: return diff; VLEN: compare using len_diff/diff */
#define VCMP 2
#define VLEN 3

/* Volume info */
#define FREE 0
#define CAPACITY 1

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
} __attribute__ ((aligned(_ALIGNMENT))) *pEntry;

/* Key-value pairs from env */
typedef struct {
	int key;
	char *val;
} kv;

/*
 * Settings
 * NOTE: update default values if changing order
 */
typedef struct {
	uint filtermode : 1;  /* Set to enter filter mode */
	uint mtimeorder : 1;  /* Set to sort by time modified */
	uint sizeorder  : 1;  /* Set to sort by file size */
	uint apparentsz : 1;  /* Set to sort by apparent size (disk usage) */
	uint blkorder   : 1;  /* Set to sort by blocks used (disk usage) */
	uint extnorder  : 1;  /* Order by extension */
	uint showhidden : 1;  /* Set to show hidden files */
	uint copymode   : 1;  /* Set when copying files */
	uint showdetail : 1;  /* Clear to show fewer file info */
	uint ctxactive  : 1;  /* Context active or not */
	uint reserved   : 7;
	/* The following settings are global */
	uint curctx     : 2;  /* Current context number */
	uint dircolor   : 1;  /* Current status of dir color */
	uint picker     : 1;  /* Write selection to user-specified file */
	uint pickraw    : 1;  /* Write selection to sdtout before exit */
	uint nonavopen  : 1;  /* Open file on right arrow or `l` */
	uint autoselect : 1;  /* Auto-select dir in nav-as-you-type mode */
	uint metaviewer : 1;  /* Index of metadata viewer in utils[] */
	uint useeditor  : 1;  /* Use VISUAL to open text files */
	uint runplugin  : 1;  /* Choose plugin mode */
	uint runctx     : 2;  /* The context in which plugin is to be run */
	uint filter_re  : 1;  /* Use regex filters */
	uint wild       : 1;  /* Do not sort entries on dir load */
	uint trash      : 1;  /* Move removed files to trash */
} settings;

/* Contexts or workspaces */
typedef struct {
	char c_path[PATH_MAX]; /* Current dir */
	char c_last[PATH_MAX]; /* Last visited dir */
	char c_name[NAME_MAX + 1]; /* Current file name */
	char c_fltr[REGEX_MAX]; /* Current filter */
	settings c_cfg; /* Current configuration */
	uint color; /* Color code for directories */
} context;

/* GLOBALS */

/* Configuration, contexts */
static settings cfg = {
	0, /* filtermode */
	0, /* mtimeorder */
	0, /* sizeorder */
	0, /* apparentsz */
	0, /* blkorder */
	0, /* extnorder */
	0, /* showhidden */
	0, /* copymode */
	0, /* showdetail */
	1, /* ctxactive */
	0, /* reserved */
	0, /* curctx */
	0, /* dircolor */
	0, /* picker */
	0, /* pickraw */
	0, /* nonavopen */
	1, /* autoselect */
	0, /* metaviewer */
	0, /* useeditor */
	0, /* runplugin */
	0, /* runctx */
	1, /* filter_re */
	0, /* wild */
	0, /* trash */
};

static context g_ctx[CTX_MAX] __attribute__ ((aligned));

static int ndents, cur, curscroll, total_dents = ENTRY_INCR;
static int xlines, xcols;
static uint idle;
static uint idletimeout, copybufpos, copybuflen;
static char *bmstr;
static char *pluginstr;
static char *opener;
static char *copier;
static char *editor;
static char *pager;
static char *shell;
static char *home;
static char *initpath;
static char *cfgdir;
static char *g_cppath;
static char *plugindir;
static char *pnamebuf, *pcopybuf;
static struct entry *dents;
static blkcnt_t ent_blocks;
static blkcnt_t dir_blocks;
static ulong num_files;
static kv bookmark[BM_MAX];
static kv plug[PLUGIN_MAX];
static size_t g_tmpfplen;
static uchar g_crc;
static uchar BLK_SHIFT = 9;
static bool interrupted = FALSE;

/* Retain old signal handlers */
#ifdef __linux__
static sighandler_t oldsighup; /* old value of hangup signal */
static sighandler_t oldsigtstp; /* old value of SIGTSTP */
#else
static sig_t oldsighup;
static sig_t oldsigtstp;
#endif

/* For use in functions which are isolated and don't return the buffer */
static char g_buf[CMD_LEN_MAX] __attribute__ ((aligned));

/* Buffer to store tmp file path to show selection, file stats and help */
static char g_tmpfpath[TMP_LEN_MAX] __attribute__ ((aligned));

/* Replace-str for xargs on different platforms */
#if defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__) || defined(__APPLE__)
#define REPLACE_STR 'J'
#elif defined(__linux__) || defined(__CYGWIN__)
#define REPLACE_STR 'I'
#else
#define REPLACE_STR 'I'
#endif

/* Options to identify file mime */
#ifdef __APPLE__
#define FILE_OPTS "-bIL"
#else
#define FILE_OPTS "-biL"
#endif

/* Macros for utilities */
#define MEDIAINFO 0
#define EXIFTOOL 1
#define OPENER 2
#define ATOOL 3
#define BSDTAR 4
#define UNZIP 5
#define TAR 6
#define LOCKER 7
#define CMATRIX 8
#define NLAUNCH 9
#define UNKNOWN 10

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
	"bsdtar",
	"unzip",
	"tar",
#ifdef __APPLE__
	"bashlock",
#elif defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__)
	"lock",
#else
	"vlock",
#endif
	"cmatrix",
	"nlaunch",
	"UNKNOWN"
};

#ifdef __linux__
static char cp[] = "cpg -giRp";
static char mv[] = "mvg -gi";
#endif

/* Common strings */
#define STR_INPUT_ID 0
#define STR_INVBM_KEY 1
#define STR_DATE_ID 2
#define STR_TMPFILE 3
#define NONE_SELECTED 4
#define UTIL_MISSING 5

static const char * const messages[] = {
	"no traversal",
	"invalid key",
	"%F %T %z",
	"/.nnnXXXXXX",
	"empty selection",
	"utility missing",
};

/* Supported configuration environment variables */
#define NNN_BMS 0
#define NNN_OPENER 1
#define NNN_CONTEXT_COLORS 2
#define NNN_IDLE_TIMEOUT 3
#define NNN_COPIER 4
#define NNN_NOTE 5
#define NNNLVL 6 /* strings end here */
#define NNN_USE_EDITOR 7 /* flags begin here */
#define NNN_TRASH 8
#ifdef __linux__
#define NNN_OPS_PROG 9
#endif

static const char * const env_cfg[] = {
	"NNN_BMS",
	"NNN_OPENER",
	"NNN_CONTEXT_COLORS",
	"NNN_IDLE_TIMEOUT",
	"NNN_COPIER",
	"NNN_NOTE",
	"NNNLVL",
	"NNN_USE_EDITOR",
	"NNN_TRASH",
#ifdef __linux__
	"NNN_OPS_PROG",
#endif
};

/* Required environment variables */
#define SHELL 0
#define VISUAL 1
#define EDITOR 2
#define PAGER 3

static const char * const envs[] = {
	"SHELL",
	"VISUAL",
	"EDITOR",
	"PAGER",
};

/* Event handling */
#ifdef LINUX_INOTIFY
#define NUM_EVENT_SLOTS 16 /* Make room for 16 events */
#define EVENT_SIZE (sizeof(struct inotify_event))
#define EVENT_BUF_LEN (EVENT_SIZE * NUM_EVENT_SLOTS)
static int inotify_fd, inotify_wd = -1;
static uint INOTIFY_MASK = /* IN_ATTRIB | */ IN_CREATE | IN_DELETE | IN_DELETE_SELF
			   | IN_MODIFY | IN_MOVE_SELF | IN_MOVED_FROM | IN_MOVED_TO;
#elif defined(BSD_KQUEUE)
#define NUM_EVENT_SLOTS 1
#define NUM_EVENT_FDS 1
static int kq, event_fd = -1;
static struct kevent events_to_monitor[NUM_EVENT_FDS];
static uint KQUEUE_FFLAGS = NOTE_DELETE | NOTE_EXTEND | NOTE_LINK
			    | NOTE_RENAME | NOTE_REVOKE | NOTE_WRITE;
static struct timespec gtimeout;
#endif

/* Function macros */
#define exitcurses() endwin()
#define clearprompt() printmsg("")
#define printwarn(presel) printwait(strerror(errno), presel)
#define istopdir(path) ((path)[1] == '\0' && (path)[0] == '/')
#define copycurname() xstrlcpy(lastname, dents[cur].name, NAME_MAX + 1)
#define settimeout() timeout(1000)
#define cleartimeout() timeout(-1)
#define errexit() printerr(__LINE__)
#define setdirwatch() (cfg.filtermode ? (presel = FILTER) : (dir_changed = TRUE))
/* We don't care about the return value from strcmp() */
#define xstrcmp(a, b)  (*(a) != *(b) ? -1 : strcmp((a), (b)))
/* A faster version of xisdigit */
#define xisdigit(c) ((unsigned int) (c) - '0' <= 9)
#define xerror() perror(xitoa(__LINE__))

/* Forward declarations */
static void redraw(char *path);
static int spawn(char *file, char *arg1, char *arg2, const char *dir, uchar flag);
static int (*nftw_fn)(const char *fpath, const struct stat *sb, int typeflag, struct FTW *ftwbuf);
static int dentfind(const char *fname, int n);
static void move_cursor(int target, int ignore_scrolloff);
static bool getutil(char *util);

/* Functions */

/*
 * CRC8 source:
 *   https://barrgroup.com/Embedded-Systems/How-To/CRC-Calculation-C-Code
 */
static uchar crc8fast(const uchar * const message, size_t n)
{
	uchar data, remainder = 0;
	size_t byte = 0;

	/* CRC data */
	static const uchar crc8table[CRC8_TABLE_LEN] __attribute__ ((aligned)) = {
		  0,  94, 188, 226,  97,  63, 221, 131, 194, 156, 126,  32, 163, 253,  31,  65,
		157, 195,  33, 127, 252, 162,  64,  30,  95,   1, 227, 189,  62,  96, 130, 220,
		 35, 125, 159, 193,  66,  28, 254, 160, 225, 191,  93,   3, 128, 222,  60,  98,
		190, 224,   2,  92, 223, 129,  99,  61, 124,  34, 192, 158,  29,  67, 161, 255,
		 70,  24, 250, 164,  39, 121, 155, 197, 132, 218,  56, 102, 229, 187,  89,   7,
		219, 133, 103,  57, 186, 228,   6,  88,  25,  71, 165, 251, 120,  38, 196, 154,
		101,  59, 217, 135,   4,  90, 184, 230, 167, 249,  27,  69, 198, 152, 122,  36,
		248, 166,  68,  26, 153, 199,  37, 123,  58, 100, 134, 216,  91,   5, 231, 185,
		140, 210,  48, 110, 237, 179,  81,  15,  78,  16, 242, 172,  47, 113, 147, 205,
		 17,  79, 173, 243, 112,  46, 204, 146, 211, 141, 111,  49, 178, 236,  14,  80,
		175, 241,  19,  77, 206, 144, 114,  44, 109,  51, 209, 143,  12,  82, 176, 238,
		 50, 108, 142, 208,  83,  13, 239, 177, 240, 174,  76,  18, 145, 207,  45, 115,
		202, 148, 118,  40, 171, 245,  23,  73,   8,  86, 180, 234, 105,  55, 213, 139,
		 87,   9, 235, 181,  54, 104, 138, 212, 149, 203,  41, 119, 244, 170,  72,  22,
		233, 183,  85,  11, 136, 214,  52, 106,  43, 117, 151, 201,  74,  20, 246, 168,
		116,  42, 200, 150,  21,  75, 169, 247, 182, 232,  10,  84, 215, 137, 107,  53,
	};

	/* Divide the message by the polynomial, a byte at a time */
	while (byte < n) {
		data = message[byte] ^ (remainder >> (UCHAR_BIT_WIDTH - 8));
		remainder = crc8table[data] ^ (remainder << 8);
		++byte;
	}

	/* The final remainder is the CRC */
	return remainder;
}

static void sigint_handler(int sig)
{
	interrupted = TRUE;
}

static uint xatoi(const char *str)
{
	int val = 0;

	if (!str)
		return 0;

	while (xisdigit(*str)) {
		val = val * 10 + (*str - '0');
		++str;
	}

	return val;
}

static char *xitoa(uint val)
{
	static char ascbuf[32] = {0};
	int i;

	for (i = 30; val && i; --i, val /= 10)
		ascbuf[i] = '0' + (val % 10);

	return &ascbuf[++i];
}

#ifdef KEY_RESIZE
/* Clear the old prompt */
static inline void clearoldprompt()
{
	move(xlines - 1, 0);
	clrtoeol();
}
#endif

/* Messages show up at the bottom */
static inline void printmsg(const char *msg)
{
	mvprintw(xlines - 1, 0, "%s\n", msg);
}

static void printwait(const char *msg, int *presel)
{
	printmsg(msg);
	if (presel)
		*presel = MSGWAIT;
}

/* Kill curses and display error before exiting */
static void printerr(int linenum)
{
	exitcurses();
	perror(xitoa(linenum));
	if (!cfg.picker && g_cppath)
		unlink(g_cppath);
	free(pcopybuf);
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
	int r;

	if (prompt)
		printprompt(prompt);
	cleartimeout();
#ifdef KEY_RESIZE
	do {
		r = getch();
		if ( r == KEY_RESIZE) {
			if (prompt) {
				clearoldprompt();
				xlines = LINES;
				printprompt(prompt);
			}
		}
	} while ( r == KEY_RESIZE);
#else
	r = getch();
#endif
	settimeout();
	return r;
}

static void xdelay(void)
{
	refresh();
	usleep(350000); /* 350 ms delay */
}

static char confirm_force(void)
{
	int r = get_input("use force? [y/Y confirms]");

	if (r == 'y' || r == 'Y')
		return 'f'; /* forceful */
	return 'i'; /* interactive */
}

/* Increase the limit on open file descriptors, if possible */
static rlim_t max_openfds(void)
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
 * macOS: https://developer.apple.com/legacy/library/documentation/Darwin/Reference/ManPages/man3/malloc.3.html
 */
static void *xrealloc(void *pcur, size_t len)
{
	void *pmem = realloc(pcur, len);

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
	if (!src || !dest || !n)
		return 0;

	ulong *s, *d;
	size_t len = strlen(src) + 1, blocks;
	const uint _WSHIFT = (LONG_SIZE == 8) ? 3 : 2;

	if (n > len)
		n = len;
	else if (len > n)
		/* Save total number of bytes to copy in len */
		len = n;

	/*
	 * To enable -O3 ensure src and dest are 16-byte aligned
	 * More info: http://www.felixcloutier.com/x86/MOVDQA.html
	 */
	if ((n >= LONG_SIZE) && (((ulong)src & _ALIGNMENT_MASK) == 0 &&
	    ((ulong)dest & _ALIGNMENT_MASK) == 0)) {
		s = (ulong *)src;
		d = (ulong *)dest;
		blocks = n >> _WSHIFT;
		n &= LONG_SIZE - 1;

		while (blocks) {
			*d = *s; // NOLINT
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

	while (--n && (*dest = *src)) // NOLINT
		++dest, ++src;

	if (!n)
		*dest = '\0';

	return len;
}

static bool is_suffix(const char *str, const char *suffix)
{
	if (!str || !suffix)
		return FALSE;

	size_t lenstr = strlen(str);
	size_t lensuffix = strlen(suffix);

	if (lensuffix > lenstr)
		return FALSE;

	return (xstrcmp(str + (lenstr - lensuffix), suffix) == 0);
}

/*
 * The poor man's implementation of memrchr(3).
 * We are only looking for '/' in this program.
 * And we are NOT expecting a '/' at the end.
 * Ideally 0 < n <= strlen(s).
 */
static void *xmemrchr(uchar *s, uchar ch, size_t n)
{
	if (!s || !n)
		return NULL;

	uchar *ptr = s + n;

	do {
		--ptr;

		if (*ptr == ch)
			return ptr;
	} while (s != ptr);

	return NULL;
}

static char *xbasename(char *path)
{
	char *base = xmemrchr((uchar *)path, '/', strlen(path)); // NOLINT

	return base ? base + 1 : path;
}

static int create_tmp_file()
{
	xstrlcpy(g_tmpfpath + g_tmpfplen - 1, messages[STR_TMPFILE], TMP_LEN_MAX - g_tmpfplen);
	return mkstemp(g_tmpfpath);
}

/* Writes buflen char(s) from buf to a file */
static void writecp(const char *buf, const size_t buflen)
{
	if (cfg.pickraw || !g_cppath)
		return;

	FILE *fp = fopen(g_cppath, "w");
	if (fp) {
		if (fwrite(buf, 1, buflen, fp) != buflen)
			printwarn(NULL);
		fclose(fp);
	} else
		printwarn(NULL);
}

static void appendfpath(const char *path, const size_t len)
{
	if ((copybufpos >= copybuflen) || ((len + 3) > (copybuflen - copybufpos))) {
		copybuflen += PATH_MAX;
		pcopybuf = xrealloc(pcopybuf, copybuflen);
		if (!pcopybuf)
			errexit();
	}

	copybufpos += xstrlcpy(pcopybuf + copybufpos, path, len);
}

/* Write selected file paths to fd, linefeed separated */
static size_t selectiontofd(int fd, uint *pcount)
{
	uint lastpos, count = 0;
	char *pbuf = pcopybuf;
	size_t pos = 0, len;
	ssize_t r;

	if (pcount)
		*pcount = 0;

	if (!copybufpos)
		return 0;

	lastpos = copybufpos - 1;

	while (pos <= lastpos) {
		len = strlen(pbuf);
		pos += len;

		r = write(fd, pbuf, len);
		if (r != (ssize_t)len)
			return pos;

		if (pos <= lastpos) {
			if (write(fd, "\n", 1) != 1)
				return pos;
			pbuf += len + 1;
		}
		++pos;
		++count;
	}

	if (pcount)
		*pcount = count;

	return pos;
}

/* List selection from selection buffer */
static bool showcplist(void)
{
	int fd;
	size_t pos;

	if (!copybufpos)
		return FALSE;

	fd = create_tmp_file();
	if (fd == -1) {
		DPRINTF_S("mkstemp failed!");
		return FALSE;
	}

	pos = selectiontofd(fd, NULL);

	close(fd);
	if (pos && pos == copybufpos)
		spawn(pager, g_tmpfpath, NULL, NULL, F_CLI);
	unlink(g_tmpfpath);

	return TRUE;
}

/* List selection from selection file (another instance) */
static bool showcpfile(void)
{
	struct stat sb;

	if (stat(g_cppath, &sb) == -1)
		return FALSE;

	/* Nothing selected if file size is 0 */
	if (!sb.st_size)
		return FALSE;

	snprintf(g_buf, CMD_LEN_MAX, "cat %s | tr \'\\0\' \'\\n\'", g_cppath);
	spawn("sh", "-c", g_buf, NULL, F_NORMAL | F_CMD);

	return TRUE;
}

static bool cpsafe(void)
{
	/* Fail if selection file path not generated */
	if (!g_cppath) {
		printmsg("selection file not found");
		return FALSE;
	}

	/* Warn if selection not completed */
	if (cfg.copymode) {
		printmsg("finish selection first");
		return FALSE;
	}

	/* Fail if selection file path isn't accessible */
	if (access(g_cppath, R_OK | W_OK) == -1) {
		errno == ENOENT ? printmsg(messages[NONE_SELECTED]) : printwarn(NULL);
		return FALSE;
	}

	return TRUE;
}

/* Reset copy indicators */
static void resetcpind(void)
{
	int r = 0;

	/* Reset copy indicators */
	for (; r < ndents; ++r)
		if (dents[r].flags & FILE_COPIED)
			dents[r].flags &= ~FILE_COPIED;
}

/* Initialize curses mode */
static bool initcurses(void)
{
	short i;

	if (cfg.picker) {
		if (!newterm(NULL, stderr, stdin)) {
			fprintf(stderr, "newterm!\n");
			return FALSE;
		}
	} else if (!initscr()) {
		char *term = getenv("TERM");

		if (term)
			fprintf(stderr, "error opening TERM: %s\n", term);
		else
			fprintf(stderr, "initscr!\n");
		return FALSE;
	}

	cbreak();
	noecho();
	nonl();
	//intrflush(stdscr, FALSE);
	keypad(stdscr, TRUE);
#if NCURSES_MOUSE_VERSION <= 1
	mousemask(BUTTON1_CLICKED | BUTTON1_DOUBLE_CLICKED, NULL);
#else
	mousemask(BUTTON1_CLICKED | BUTTON1_DOUBLE_CLICKED | BUTTON4_PRESSED | BUTTON5_PRESSED, NULL);
#endif
	mouseinterval(400);
	curs_set(FALSE); /* Hide cursor */
	start_color();
	use_default_colors();

	/* Initialize default colors */
	for (i = 0; i <  CTX_MAX; ++i)
		init_pair(i + 1, g_ctx[i].color, -1);

	settimeout(); /* One second */
	set_escdelay(25);
	return TRUE;
}

/* No NULL check here as spawn() guards against it */
static int parseargs(char *line, char **argv)
{
	int count = 0;

	argv[count++] = line;

	while (*line) { // NOLINT
		if (ISBLANK(*line)) {
			*line++ = '\0';

			if (!*line) // NOLINT
				return count;

			argv[count++] = line;
			if (count == EXEC_ARGS_MAX)
				return -1;
		}

		++line;
	}

	return count;
}

static pid_t xfork(uchar flag)
{
	pid_t p = fork();

	if (p > 0) {
		/* the parent ignores the interrupt, quit and hangup signals */
		oldsighup = signal(SIGHUP, SIG_IGN);
		oldsigtstp = signal(SIGTSTP, SIG_DFL);
	} else if (p == 0) {
		/* so they can be used to stop the child */
		signal(SIGHUP, SIG_DFL);
		signal(SIGINT, SIG_DFL);
		signal(SIGQUIT, SIG_DFL);
		signal(SIGTSTP, SIG_DFL);

		if (flag & F_NOWAIT)
			setsid();
	}

	if (p == -1)
		perror("fork");
	return p;
}

static int join(pid_t p, uchar flag)
{
	int status = 0xFFFF;

	if (!(flag & F_NOWAIT)) {
		/* wait for the child to exit */
		do {
		} while (waitpid(p, &status, 0) == -1);

		if (WIFEXITED(status)) {
			status = WEXITSTATUS(status);
			DPRINTF_D(status);
		}
	}

	/* restore parent's signal handling */
	signal(SIGHUP, oldsighup);
	signal(SIGTSTP, oldsigtstp);

	return status;
}

/*
 * Spawns a child process. Behaviour can be controlled using flag.
 * Limited to 2 arguments to a program, flag works on bit set.
 */
static int spawn(char *file, char *arg1, char *arg2, const char *dir, uchar flag)
{
	pid_t pid;
	int status, retstatus = 0xFFFF;
	char *argv[EXEC_ARGS_MAX] = {0};
	char *cmd = NULL;

	if (!file || !*file)
		return retstatus;

	/* Swap args if the first arg is NULL and second isn't */
	if (!arg1 && arg2) {
		arg1 = arg2;
		arg2 = NULL;
	}

	if (flag & F_MULTI) {
		size_t len = strlen(file) + 1;

		cmd = (char *)malloc(len);
		if (!cmd) {
			DPRINTF_S("malloc()!");
			return retstatus;
		}

		xstrlcpy(cmd, file, len);
		status = parseargs(cmd, argv);
		if (status == -1 || status > (EXEC_ARGS_MAX - 3)) { /* arg1, arg2 and last NULL */
			free(cmd);
			DPRINTF_S("NULL or too many args");
			return retstatus;
		}

		argv[status++] = arg1;
		argv[status] = arg2;
	} else {
		argv[0] = file;
		argv[1] = arg1;
		argv[2] = arg2;
	}

	if (flag & F_NORMAL)
		exitcurses();

	pid = xfork(flag);
	if (pid == 0) {
		if (dir && chdir(dir) == -1)
			_exit(1);

		/* Suppress stdout and stderr */
		if (flag & F_NOTRACE) {
			int fd = open("/dev/null", O_WRONLY, 0200);

			dup2(fd, 1);
			dup2(fd, 2);
			close(fd);
		}

		execvp(*argv, argv);
		_exit(1);
	} else {
		retstatus = join(pid, flag);

		DPRINTF_D(pid);
		if (flag & F_NORMAL) {
			if (flag & F_CMD) {
				printf("\nPress Enter to continue");
				getchar();
			}

			refresh();
		}

		free(cmd);
	}

	return retstatus;
}

/* Get program name from env var, else return fallback program */
static char *xgetenv(const char *name, char *fallback)
{
	char *value = getenv(name);

	return value && value[0] ? value : fallback;
}

/* Checks if an env variable is set to 1 */
static bool xgetenv_set(const char *name)
{
	char *value = getenv(name);

	if (value && value[0] == '1' && !value[1])
		return TRUE;

	return FALSE;
}

/* Check if a dir exists, IS a dir and is readable */
static bool xdiraccess(const char *path)
{
	DIR *dirp = opendir(path);

	if (!dirp) {
		printwarn(NULL);
		return FALSE;
	}

	closedir(dirp);
	return TRUE;
}

static void cpstr(char *buf)
{
	snprintf(buf, CMD_LEN_MAX,
#ifdef __linux__
		 "xargs -0 -a %s -%c src %s src .", g_cppath, REPLACE_STR, cp);
#else
		 "cat %s | xargs -0 -o -%c src cp -iRp src .", g_cppath, REPLACE_STR);
#endif
}

static void mvstr(char *buf)
{
	snprintf(buf, CMD_LEN_MAX,
#ifdef __linux__
		 "xargs -0 -a %s -%c src %s src .", g_cppath, REPLACE_STR, mv);
#else
		 "cat %s | xargs -0 -o -%c src mv -i src .", g_cppath, REPLACE_STR);
#endif
}

static void rmmulstr(char *buf)
{
	if (cfg.trash) {
		snprintf(buf, CMD_LEN_MAX,
#ifdef __linux__
			 "xargs -0 -a %s trash-put", g_cppath);
#else
			 "cat %s | xargs -0 trash-put", g_cppath);
#endif
	} else {
		snprintf(buf, CMD_LEN_MAX,
#ifdef __linux__
			 "xargs -0 -a %s rm -%cr", g_cppath, confirm_force());
#else
			 "cat %s | xargs -0 -o rm -%cr", g_cppath, confirm_force());
#endif
	}
}

static void xrm(char *path)
{
	if (cfg.trash)
		spawn("trash-put", path, NULL, NULL, F_NORMAL);
	else {
		char rm_opts[] = "-ir";

		rm_opts[1] = confirm_force();
		spawn("rm", rm_opts, path, NULL, F_NORMAL);
	}
}

static bool batch_rename(const char *path)
{
	int fd1, fd2, i;
	uint count = 0, lines = 0;
	bool dir = FALSE, ret = FALSE;
	const char renamecmd[] = "paste -d'\n' %s %s | sed 'N; /^\\(.*\\)\\n\\1$/!p;d' | tr '\n' '\\0' | xargs -0 -n2 mv 2>/dev/null";
	char foriginal[TMP_LEN_MAX] = {0};
	char buf[sizeof(renamecmd) + (PATH_MAX << 1)];

	if ((fd1 = create_tmp_file()) == -1)
		return ret;

	xstrlcpy(foriginal, g_tmpfpath, strlen(g_tmpfpath)+1);

	if ((fd2 = create_tmp_file()) == -1) {
		unlink(foriginal);
		close(fd1);
		return ret;
	}

	if (!copybufpos) {
		if (!ndents)
			return TRUE;

		for (i = 0; i < ndents; ++i)
			appendfpath(dents[i].name, NAME_MAX);

		dir = TRUE;
	}

	selectiontofd(fd1, &count);
	selectiontofd(fd2, NULL);
	close(fd2);

	if (dir)
		copybufpos = 0;

	spawn(editor, g_tmpfpath, NULL, path, F_CLI);

	/* Reopen file descriptor to get updated contents */
	if ((fd2 = open(g_tmpfpath, O_RDONLY)) == -1)
		goto finish;

	while ((i = read(fd2, buf, sizeof(buf))) > 0) {
		while (i)
			lines += (buf[--i] == '\n');
	}

	if (i < 0)
		goto finish;

	DPRINTF_U(count);
	DPRINTF_U(lines);

	if (count != lines) {
		DPRINTF_S("cannot delete files");
		goto finish;
	}

	snprintf(buf, sizeof(buf), renamecmd, foriginal, g_tmpfpath);
	spawn("sh", "-c", buf, path, F_NORMAL);
	ret = TRUE;

finish:
	if (fd1 >= 0)
		close(fd1);
	unlink(foriginal);

	if (fd2 >= 0)
		close(fd2);
	unlink(g_tmpfpath);

	return ret;
}

static void get_archive_cmd(char *cmd, char *archive)
{
	if (getutil(utils[ATOOL]))
		xstrlcpy(cmd, "atool -a", ARCHIVE_CMD_LEN);
	else if (getutil(utils[BSDTAR]))
		xstrlcpy(cmd, "bsdtar -acvf", ARCHIVE_CMD_LEN);
	else if (is_suffix(archive, ".zip"))
		xstrlcpy(cmd, "zip -r", ARCHIVE_CMD_LEN);
	else
		xstrlcpy(cmd, "tar -acvf", ARCHIVE_CMD_LEN);
}

static void archive_selection(const char *cmd, const char *archive, const char *curpath)
{
	char *buf = (char *)malloc(CMD_LEN_MAX * sizeof(char));
	snprintf(buf, CMD_LEN_MAX,
#ifdef __linux__
		"sed -ze 's|^%s/||' '%s' | xargs -0 %s %s", curpath, g_cppath, cmd, archive);
#else
		"cat '%s' | tr '\\0' '\n' | sed -e 's|^%s/||' | tr '\n' '\\0' | xargs -0 %s %s",
		g_cppath, curpath, cmd, archive);
#endif
	spawn("sh", "-c", buf, curpath, F_NORMAL);
	free(buf);
}

static bool write_lastdir(const char *curpath)
{
	bool ret = TRUE;
	size_t len = strlen(cfgdir);

	xstrlcpy(cfgdir + len, "/.lastd", 8);
	DPRINTF_S(cfgdir);

	FILE *fp = fopen(cfgdir, "w");

	if (fp) {
		if (fprintf(fp, "cd \"%s\"", curpath) < 0)
			ret = FALSE;

		fclose(fp);
	} else
		ret = FALSE;

	return ret;
}

static int digit_compare(const char *a, const char *b)
{
	while (*a && *b && *a == *b)
		++a, ++b;

	return *a - *b;
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
	const char *c1 = s1, *c2 = s2, *m1, *m2;
	int count1 = 0, count2 = 0, bias;
	char sign[2] = {'+', '+'};

	while (ISBLANK(*c1))
		++c1;

	while (ISBLANK(*c2))
		++c2;

	if (*c1 == '-' || *c1 == '+') {
		if (*c1 == '-')
			sign[0] = '-';
		++c1;
	}

	if (*c2 == '-' || *c2 == '+') {
		if (*c2 == '-')
			sign[1] = '-';
		++c2;
	}

	if (xisdigit(*c1) && xisdigit(*c2)) {
		while (*c1 == '0')
			++c1;
		m1 = c1;

		while (*c2 == '0')
			++c2;
		m2 = c2;

		while (xisdigit(*c1)) {
			++count1;
			++c1;
		}
		while (ISBLANK(*c1))
			++c1;

		while (xisdigit(*c2)) {
			++count2;
			++c2;
		}
		while (ISBLANK(*c2))
			++c2;

		if (*c1 && !*c2)
			return 1;

		if (!*c1 && *c2)
			return -1;

		if (!*c1 && !*c2) {
			if (sign[0] != sign[1])
				return ((sign[0] == '+') ? 1 : -1);

			if (count1 > count2)
				return 1;

			if (count1 < count2)
				return -1;

			bias = digit_compare(m1, m2);
			if (bias)
				return bias;
		}
	}

	return strcoll(s1, s2);
}

/*
 * Version comparison
 *
 * The code for version compare is a modified version of the GLIBC
 * and uClibc implementation of strverscmp(). The source is here:
 * https://elixir.bootlin.com/uclibc-ng/latest/source/libc/string/strverscmp.c
 */

/*
 * Compare S1 and S2 as strings holding indices/version numbers,
 * returning less than, equal to or greater than zero if S1 is less than,
 * equal to or greater than S2 (for more info, see the texinfo doc).
 *
 * Ignores case.
 */
static int xstrverscasecmp(const char * const s1, const char * const s2)
{
	const uchar *p1 = (const uchar *)s1;
	const uchar *p2 = (const uchar *)s2;
	uchar c1, c2;
	int state, diff;

	/*
	 * Symbol(s)    0       [1-9]   others
	 * Transition   (10) 0  (01) d  (00) x
	 */
	static const uint8_t next_state[] = {
		/* state    x    d    0  */
		/* S_N */  S_N, S_I, S_Z,
		/* S_I */  S_N, S_I, S_I,
		/* S_F */  S_N, S_F, S_F,
		/* S_Z */  S_N, S_F, S_Z
	};

	static const int8_t result_type[] __attribute__ ((aligned)) = {
		/* state   x/x  x/d  x/0  d/x  d/d  d/0  0/x  0/d  0/0  */

		/* S_N */  VCMP, VCMP, VCMP, VCMP, VLEN, VCMP, VCMP, VCMP, VCMP,
		/* S_I */  VCMP,   -1,   -1,    1, VLEN, VLEN,    1, VLEN, VLEN,
		/* S_F */  VCMP, VCMP, VCMP, VCMP, VCMP, VCMP, VCMP, VCMP, VCMP,
		/* S_Z */  VCMP,    1,    1,   -1, VCMP, VCMP,   -1, VCMP, VCMP
	};

	if (p1 == p2)
		return 0;

	c1 = TOUPPER(*p1);
	++p1;
	c2 = TOUPPER(*p2);
	++p2;

	/* Hint: '0' is a digit too.  */
	state = S_N + ((c1 == '0') + (xisdigit(c1) != 0));

	while ((diff = c1 - c2) == 0) {
		if (c1 == '\0')
			return diff;

		state = next_state[state];
		c1 = TOUPPER(*p1);
		++p1;
		c2 = TOUPPER(*p2);
		++p2;
		state += (c1 == '0') + (xisdigit(c1) != 0);
	}

	state = result_type[state * 3 + (((c2 == '0') + (xisdigit(c2) != 0)))];

	switch (state) {
	case VCMP:
		return diff;
	case VLEN:
		while (xisdigit(*p1++))
			if (!xisdigit(*p2++))
				return 1;
		return xisdigit(*p2) ? -1 : diff;
	default:
		return state;
	}
}

static int (*cmpfn)(const char * const s1, const char * const s2) = &xstricmp;

/* Return the integer value of a char representing HEX */
static char xchartohex(char c)
{
	if (xisdigit(c))
		return c - '0';

	c = TOUPPER(c);
	if (c >= 'A' && c <= 'F')
		return c - 'A' + 10;

	return c;
}

static int setfilter(regex_t *regex, const char *filter)
{
	int r = regcomp(regex, filter, REG_NOSUB | REG_EXTENDED | REG_ICASE);

	if (r != 0 && filter && filter[0] != '\0')
		mvprintw(xlines - 1, 0, "regex error: %d\n", r);

	return r;
}

static int visible_re(regex_t *regex, const char *fname, const char *fltr)
{
	return regexec(regex, fname, 0, NULL, 0) == 0;
}

static int visible_str(regex_t *regex, const char *fname, const char *fltr)
{
	return strcasestr(fname, fltr) != NULL;
}

static int (*filterfn)(regex_t *regex, const char *fname, const char *fltr) = &visible_re;

static int entrycmp(const void *va, const void *vb)
{
	const struct entry *pa = (pEntry)va;
	const struct entry *pb = (pEntry)vb;

	if ((pb->flags & DIR_OR_LINK_TO_DIR) != (pa->flags & DIR_OR_LINK_TO_DIR)) {
		if (pb->flags & DIR_OR_LINK_TO_DIR)
			return 1;
		return -1;
	}

	/* Sort based on specified order */
	if (cfg.mtimeorder) {
		if (pb->t > pa->t)
			return 1;
		if (pb->t < pa->t)
			return -1;
	} else if (cfg.sizeorder) {
		if (pb->size > pa->size)
			return 1;
		if (pb->size < pa->size)
			return -1;
	} else if (cfg.blkorder) {
		if (pb->blocks > pa->blocks)
			return 1;
		if (pb->blocks < pa->blocks)
			return -1;
	} else if (cfg.extnorder && !(pb->flags & DIR_OR_LINK_TO_DIR)) {
		char *extna = xmemrchr((uchar *)pa->name, '.', strlen(pa->name));
		char *extnb = xmemrchr((uchar *)pb->name, '.', strlen(pb->name));

		if (extna || extnb) {
			if (!extna)
				return 1;

			if (!extnb)
				return -1;

			int ret = strcasecmp(extna, extnb);

			if (ret)
				return ret;
		}
	}

	return cmpfn(pa->name, pb->name);
}

/*
 * Returns SEL_* if key is bound and 0 otherwise.
 * Also modifies the run and env pointers (used on SEL_{RUN,RUNARG}).
 * The next keyboard input can be simulated by presel.
 */
static int nextsel(int presel)
{
	int c = presel;
	uint i;
	const uint len = LEN(bindings);
#ifdef LINUX_INOTIFY
	struct inotify_event *event;
	char inotify_buf[EVENT_BUF_LEN];
	memset((void *)inotify_buf, 0x0, EVENT_BUF_LEN);
#elif defined(BSD_KQUEUE)
	struct kevent event_data[NUM_EVENT_SLOTS];
	memset((void *)event_data, 0x0, sizeof(struct kevent) * NUM_EVENT_SLOTS);
#endif

	if (c == 0 || c == MSGWAIT) {
		c = getch();
		DPRINTF_D(c);

		if (presel == MSGWAIT) {
			if (cfg.filtermode)
				c = FILTER;
			else
				c = CONTROL('L');
		}
	}

	if (c == -1) {
		++idle;

		/*
		 * Do not check for directory changes in du mode.
		 * A redraw forces du calculation.
		 * Check for changes every odd second.
		 */
#ifdef LINUX_INOTIFY
		if (!cfg.blkorder && inotify_wd >= 0 && (idle & 1)) {
			i = read(inotify_fd, inotify_buf, EVENT_BUF_LEN);
			if (i > 0) {
				char *ptr;

				for (ptr = inotify_buf;
				     ptr + ((struct inotify_event *)ptr)->len < inotify_buf + i;
				     ptr += sizeof(struct inotify_event) + event->len) {
					event = (struct inotify_event *) ptr;
					DPRINTF_D(event->wd);
					DPRINTF_D(event->mask);
					if (!event->wd)
						break;

					if (event->mask & INOTIFY_MASK) {
						c = CONTROL('L');
						DPRINTF_S("issue refresh");
						break;
					}
				}
				DPRINTF_S("inotify read done");
			}
		}
#elif defined(BSD_KQUEUE)
		if (!cfg.blkorder && event_fd >= 0 && idle & 1
		    && kevent(kq, events_to_monitor, NUM_EVENT_SLOTS,
			      event_data, NUM_EVENT_FDS, &gtimeout) > 0)
			c = CONTROL('L');
#endif
	} else
		idle = 0;

	for (i = 0; i < len; ++i)
		if (c == bindings[i].sym)
			return bindings[i].act;

	return 0;
}

static inline void swap_ent(int id1, int id2)
{
	struct entry _dent, *pdent1 = &dents[id1], *pdent2 =  &dents[id2];

	*(&_dent) = *pdent1;
	*pdent1 = *pdent2;
	*pdent2 = *(&_dent);
}

/*
 * Move non-matching entries to the end
 */
static int fill(const char *fltr, regex_t *re)
{
	int count = 0;

	for (; count < ndents; ++count) {
		if (filterfn(re, dents[count].name, fltr) == 0) {
			if (count != --ndents) {
				swap_ent(count, ndents);
				--count;
			}

			continue;
		}
	}

	return ndents;
}

static int matches(const char *fltr)
{
	regex_t re;

	/* Search filter */
	if (cfg.filter_re && setfilter(&re, fltr) != 0)
		return -1;

	ndents = fill(fltr, &re);
	if (cfg.filter_re)
		regfree(&re);
	if (!ndents)
		return 0;

	qsort(dents, ndents, sizeof(*dents), entrycmp);

	return 0;
}

static int filterentries(char *path)
{
	wchar_t *wln = (wchar_t *)alloca(sizeof(wchar_t) * REGEX_MAX);
	char *ln = g_ctx[cfg.curctx].c_fltr;
	wint_t ch[2] = {0};
	int r, total = ndents, oldcur = cur, len;
	char *pln = g_ctx[cfg.curctx].c_fltr + 1;

	cur = 0;

	if (ndents && ln[0] == FILTER && *pln) {
		if (matches(pln) != -1)
			redraw(path);

		len = mbstowcs(wln, ln, REGEX_MAX);
	} else {
		ln[0] = wln[0] = FILTER;
		ln[1] = wln[1] = '\0';
		len = 1;
	}

	cleartimeout();
	curs_set(TRUE);
	printprompt(ln);

	while ((r = get_wch(ch)) != ERR) {
		switch (*ch) {
#ifdef KEY_RESIZE
		case KEY_RESIZE:
			clearoldprompt();
			if (len == 1) {
				cur = oldcur;
				redraw(path);
				cur = 0;
			} else
				redraw(path);
			printprompt(ln);
			continue;
#endif
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
		case KEY_MOUSE: // fallthrough
		case 27: /* Exit filter mode on Escape */
			if (len == 1)
				cur = oldcur;
			goto end;
		}

		if (r == OK) {
			/* Handle all control chars in main loop */
			if (*ch < ASCII_MAX && keyname(*ch)[0] == '^' && *ch != '^') {
				if (len == 1)
					cur = oldcur;
				goto end;
			}

			switch (*ch) {
			case '\r':  /* with nonl(), this is ENTER key value */
				if (len == 1) {
					cur = oldcur;
					goto end;
				}

				if (matches(pln) == -1)
					goto end;

				redraw(path);
				goto end;
			case '/': /* works as Leader key in filter mode */
				*ch = CONTROL('_'); // fallthrough
				if (len == 1)
					cur = oldcur;
				goto end;
			case '?':  /* '?' is an invalid regex, show help instead */
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
				if (ndents == 1 && cfg.filtermode
				    && cfg.autoselect && S_ISDIR(dents[0].mode)) {
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
	if (*ch != '\t')
		g_ctx[cfg.curctx].c_fltr[0] = g_ctx[cfg.curctx].c_fltr[1] = '\0';

	move_cursor(cur, 0);

	curs_set(FALSE);
	settimeout();

	/* Return keys for navigation etc. */
	return *ch;
}

/* Show a prompt with input string and return the changes */
static char *xreadline(char *prefill, char *prompt)
{
	size_t len, pos;
	int x, r;
	const int WCHAR_T_WIDTH = sizeof(wchar_t);
	wint_t ch[2] = {0};
	wchar_t * const buf = malloc(sizeof(wchar_t) * CMD_LEN_MAX);

	if (!buf)
		errexit();

	cleartimeout();
	printprompt(prompt);

	if (prefill) {
		DPRINTF_S(prefill);
		len = pos = mbstowcs(buf, prefill, CMD_LEN_MAX);
	} else
		len = (size_t)-1;

	if (len == (size_t)-1) {
		buf[0] = '\0';
		len = pos = 0;
	}

	x = getcurx(stdscr);
	curs_set(TRUE);

	while (1) {
		buf[len] = ' ';
		mvaddnwstr(xlines - 1, x, buf, len + 1);
		move(xlines - 1, x + wcswidth(buf, pos));

		r = get_wch(ch);
		if (r != ERR) {
			if (r == OK) {
				switch (*ch) {
				case KEY_ENTER: // fallthrough
				case '\n': // fallthrough
				case '\r':
					goto END;
				case 127: // fallthrough
				case '\b': /* rhel25 sends '\b' for backspace */
					if (pos > 0) {
						memmove(buf + pos - 1, buf + pos,
							(len - pos) * WCHAR_T_WIDTH);
						--len, --pos;
					} // fallthrough
				case '\t': /* TAB breaks cursor position, ignore it */
					continue;
				case CONTROL('L'):
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
					printprompt(prompt);
					memmove(buf, buf + pos, (len - pos) * WCHAR_T_WIDTH);
					len -= pos;
					pos = 0;
					continue;
				case 27: /* Exit prompt on Escape */
					len = 0;
					goto END;
				}

				/* Filter out all other control chars */
				if (*ch < ASCII_MAX && keyname(*ch)[0] == '^')
					continue;

				if (pos < CMD_LEN_MAX - 1) {
					memmove(buf + pos + 1, buf + pos,
						(len - pos) * WCHAR_T_WIDTH);
					buf[pos] = *ch;
					++len, ++pos;
					continue;
				}
			} else {
				switch (*ch) {
#ifdef KEY_RESIZE
				case KEY_RESIZE:
					clearoldprompt();
					xlines = LINES;
					printprompt(prompt);
					break;
#endif
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
						memmove(buf + pos - 1, buf + pos,
							(len - pos) * WCHAR_T_WIDTH);
						--len, --pos;
					}
					break;
				case KEY_DC:
					if (pos < len) {
						memmove(buf + pos, buf + pos + 1,
							(len - pos - 1) * WCHAR_T_WIDTH);
						--len;
					}
					break;
				case KEY_END:
					pos = len;
					break;
				case KEY_HOME:
					pos = 0;
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

	pos = wcstombs(g_buf, buf, CMD_LEN_MAX - 1);
	if (pos >= CMD_LEN_MAX - 1)
		g_buf[CMD_LEN_MAX - 1] = '\0';

	free(buf);
	return g_buf;
}

#ifndef NORL
/*
 * Caller should check the value of presel to confirm if it needs to wait to show warning
 */
static char *getreadline(char *prompt, char *path, char *curpath, int *presel)
{
	/* Switch to current path for readline(3) */
	if (chdir(path) == -1) {
		printwarn(presel);
		return NULL;
	}

	exitcurses();

	char *input = readline(prompt);

	refresh();

	if (chdir(curpath) == -1) {
		printwarn(presel);
		free(input);
		return NULL;
	}

	if (input && input[0]) {
		add_history(input);
		xstrlcpy(g_buf, input, CMD_LEN_MAX);
		free(input);
		return g_buf;
	}

	free(input);
	return NULL;
}
#endif

/*
 * Updates out with "dir/name or "/name"
 * Returns the number of bytes copied including the terminating NULL byte
 */
static size_t mkpath(char *dir, char *name, char *out)
{
	size_t len;

	/* Handle absolute path */
	if (name[0] == '/')
		return xstrlcpy(out, name, PATH_MAX);

	/* Handle root case */
	if (istopdir(dir))
		len = 1;
	else
		len = xstrlcpy(out, dir, PATH_MAX);

	out[len - 1] = '/'; // NOLINT
	return (xstrlcpy(out + len, name, PATH_MAX - len) + len);
}

/*
 * Create symbolic/hard link(s) to file(s) in selection list
 * Returns the number of links created
 */
static int xlink(char *suffix, char *path, char *buf, int *presel, int type)
{
	int count = 0;
	char *pbuf = pcopybuf, *fname;
	size_t pos = 0, len, r;
	int (*link_fn)(const char *, const char *) = NULL;

	/* Check if selection is empty */
	if (!copybufpos) {
		printwait(messages[NONE_SELECTED], presel);
		return -1;
	}

	if (type == 's') /* symbolic link */
		link_fn = &symlink;
	else /* hard link */
		link_fn = &link;

	while (pos < copybufpos) {
		len = strlen(pbuf);
		fname = xbasename(pbuf);
		r = mkpath(path, fname, buf);
		xstrlcpy(buf + r - 1, suffix, PATH_MAX - r - 1);

		if (!link_fn(pbuf, buf))
			++count;

		pos += len + 1;
		pbuf += len + 1;
	}

	if (!count)
		printwait("none created", presel);

	return count;
}

static bool parsekvpair(kv *kvarr, char **envcpy, const char *cfgstr, uchar maxitems)
{
	int i = 0;
	char *nextkey;
	char *ptr = getenv(cfgstr);

	if (!ptr || !*ptr)
		return TRUE;

	*envcpy = strdup(ptr);
	ptr = *envcpy;
	nextkey = ptr;

	while (*ptr && i < maxitems) {
		if (ptr == nextkey) {
			kvarr[i].key = *ptr;
			if (*++ptr != ':')
				return FALSE;
			if (*++ptr == '\0')
				return FALSE;
			kvarr[i].val = ptr;
			++i;
		}

		if (*ptr == ';') {
			/* Remove trailing space */
			if (i > 0 && *(ptr - 1) == '/')
				*(ptr - 1) = '\0';

			*ptr = '\0';
			nextkey = ptr + 1;
		}

		++ptr;
	}

	if (i < maxitems) {
		if (*kvarr[i - 1].val == '\0')
			return FALSE;
		kvarr[i].key = '\0';
	}

	return TRUE;
}

/*
 * Get the value corresponding to a key
 *
 * NULL is returned in case of no match, path resolution failure etc.
 * buf would be modified, so check return value before access
 */
static char *get_kv_val(kv *kvarr, char *buf, int key, uchar max, bool path)
{
	int r = 0;

	for (; kvarr[r].key && r < max; ++r) {
		if (kvarr[r].key == key) {
			if (!path)
				return kvarr[r].val;

			if (kvarr[r].val[0] == '~') {
				ssize_t len = strlen(home);
				ssize_t loclen = strlen(kvarr[r].val);

				if (!buf)
					buf = (char *)malloc(len + loclen);

				xstrlcpy(buf, home, len + 1);
				xstrlcpy(buf + len, kvarr[r].val + 1, loclen);
				return buf;
			}

			return realpath(kvarr[r].val, buf);
		}
	}

	DPRINTF_S("Invalid key");
	return NULL;
}

static inline void resetdircolor(int flags)
{
	if (cfg.dircolor && !(flags & DIR_OR_LINK_TO_DIR)) {
		attroff(COLOR_PAIR(cfg.curctx + 1) | A_BOLD);
		cfg.dircolor = 0;
	}
}

/*
 * Replace escape characters in a string with '?'
 * Adjust string length to maxcols if > 0;
 * Max supported str length: NAME_MAX;
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
	static wchar_t wbuf[NAME_MAX + 1] __attribute__ ((aligned));
	wchar_t *buf = wbuf;
	size_t lencount = 0;

	/* Convert multi-byte to wide char */
	size_t len = mbstowcs(wbuf, str, NAME_MAX);

	while (*buf && lencount <= maxcols) {
		if (*buf <= '\x1f' || *buf == '\x7f')
			*buf = '\?';

		++buf;
		++lencount;
	}

	len = lencount = wcswidth(wbuf, len);

	/* Reduce number of wide chars to max columns */
	if (len > maxcols) {
		lencount = maxcols + 1;

		/* Reduce wide chars one by one till it fits */
		while (len > maxcols)
			len = wcswidth(wbuf, --lencount);

		wbuf[lencount] = L'\0';
	}

	/* Convert wide char to multi-byte */
	wcstombs(g_buf, wbuf, NAME_MAX);
	return g_buf;
}

static char *coolsize(off_t size)
{
	const char * const U = "BKMGTPEZY";
	static char size_buf[12]; /* Buffer to hold human readable size */
	off_t rem = 0;
	size_t ret;
	int i = 0;

	while (size >= 1024) {
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

	if (i > 0 && i < 6 && rem) {
		ret = xstrlcpy(size_buf, xitoa(size), 12);
		size_buf[ret - 1] = '.';

		char *frac = xitoa(rem);
		size_t toprint = i > 3 ? 3 : i;
		size_t len = strlen(frac);

		if (len < toprint) {
			size_buf[ret] = size_buf[ret + 1] = size_buf[ret + 2] = '0';
			xstrlcpy(size_buf + ret + (toprint - len), frac, len + 1);
		} else
			xstrlcpy(size_buf + ret, frac, toprint + 1);

		ret += toprint;
	} else {
		ret = xstrlcpy(size_buf, size ? xitoa(size) : "0", 12);
		--ret;
	}

	size_buf[ret] = U[i];
	size_buf[ret + 1] = '\0';

	return size_buf;
}

static void printent(const struct entry *ent, int sel, uint namecols)
{
	const char *pname = unescape(ent->name, namecols);
	const char cp = (ent->flags & FILE_COPIED) ? '+' : ' ';
	char ind[2] = {'\0', '\0'};
	mode_t mode = ent->mode;

	switch (mode & S_IFMT) {
	case S_IFREG:
		if (mode & 0100)
			ind[0] = '*';
		break;
	case S_IFDIR:
		ind[0] = '/';
		break;
	case S_IFLNK:
		ind[0] = '@';
		break;
	case S_IFSOCK:
		ind[0] = '=';
		break;
	case S_IFIFO:
		ind[0] = '|';
		break;
	case S_IFBLK: // fallthrough
	case S_IFCHR:
		break;
	default:
		ind[0] = '?';
		break;
	}

	/* Directories are always shown on top */
	resetdircolor(ent->flags);

	if (sel)
		attron(A_REVERSE);

	printw("%c%s%s\n", cp, pname, ind);

	if (sel)
		attroff(A_REVERSE);
}

static void printent_long(const struct entry *ent, int sel, uint namecols)
{
	char timebuf[18], permbuf[4], ind1 = '\0', ind2[] = "\0\0";
	const char cp = (ent->flags & FILE_COPIED) ? '+' : ' ';

	/* Timestamp */
	strftime(timebuf, 18, "%F %R", localtime(&ent->t));

	/* Permissions */
	permbuf[0] = '0' + ((ent->mode >> 6) & 7);
	permbuf[1] = '0' + ((ent->mode >> 3) & 7);
	permbuf[2] = '0' + (ent->mode & 7);
	permbuf[3] = '\0';

	/* Trim escape chars from name */
	const char *pname = unescape(ent->name, namecols);

	/* Directories are always shown on top */
	resetdircolor(ent->flags);

	if (sel)
		attron(A_REVERSE);

	switch (ent->mode & S_IFMT) {
	case S_IFREG:
		if (ent->mode & 0100)
			printw("%c%-16.16s  %s %8.8s* %s*\n", cp, timebuf, permbuf,
			       coolsize(cfg.blkorder ? ent->blocks << BLK_SHIFT : ent->size), pname);
		else
			printw("%c%-16.16s  %s %8.8s  %s\n", cp, timebuf, permbuf,
			       coolsize(cfg.blkorder ? ent->blocks << BLK_SHIFT : ent->size), pname);
		break;
	case S_IFDIR:
		if (cfg.blkorder)
			printw("%c%-16.16s  %s %8.8s/ %s/\n",
			       cp, timebuf, permbuf, coolsize(ent->blocks << BLK_SHIFT), pname);
		else
			printw("%c%-16.16s  %s        /  %s/\n", cp, timebuf, permbuf, pname);
		break;
	case S_IFLNK:
		printw("%c%-16.16s  %s        @  %s@\n", cp, timebuf, permbuf, pname);
		break;
	case S_IFSOCK:
		ind1 = ind2[0] = '='; // fallthrough
	case S_IFIFO:
		if (!ind1)
			ind1 = ind2[0] = '|'; // fallthrough
	case S_IFBLK:
		if (!ind1)
			ind1 = 'b'; // fallthrough
	case S_IFCHR:
		if (!ind1)
			ind1 = 'c'; // fallthrough
	default:
		if (!ind1)
			ind1 = ind2[0] = '?';
		printw("%c%-16.16s  %s        %c  %s%s\n", cp, timebuf, permbuf, ind1, pname, ind2);
		break;
	}

	if (sel)
		attroff(A_REVERSE);
}

static void (*printptr)(const struct entry *ent, int sel, uint namecols) = &printent;

static void savecurctx(settings *curcfg, char *path, char *curname, int r /* next context num */)
{
	settings cfg = *curcfg;
	bool copymode = cfg.copymode ? TRUE : FALSE;

#ifdef DIR_LIMITED_COPY
	g_crc = 0;
#endif
	/* Save current context */
	xstrlcpy(g_ctx[cfg.curctx].c_name, curname, NAME_MAX + 1);
	g_ctx[cfg.curctx].c_cfg = cfg;

	if (g_ctx[r].c_cfg.ctxactive) { /* Switch to saved context */
		/* Switch light/detail mode */
		if (cfg.showdetail != g_ctx[r].c_cfg.showdetail)
			/* set the reverse */
			printptr = cfg.showdetail ? &printent : &printent_long;

		cfg = g_ctx[r].c_cfg;
	} else { /* Setup a new context from current context */
		g_ctx[r].c_cfg.ctxactive = 1;
		xstrlcpy(g_ctx[r].c_path, path, PATH_MAX);
		g_ctx[r].c_last[0] = '\0';
		xstrlcpy(g_ctx[r].c_name, curname, NAME_MAX + 1);
		g_ctx[r].c_fltr[0] = g_ctx[r].c_fltr[1] = '\0';
		g_ctx[r].c_cfg = cfg;
		g_ctx[r].c_cfg.runplugin = 0;
	}

	/* Continue copy mode */
	cfg.copymode = copymode;
	cfg.curctx = r;

	*curcfg = cfg;
}

/*
 * Gets only a single line (that's what we need
 * for now) or shows full command output in pager.
 *
 * If page is valid, returns NULL
 */
static char *get_output(char *buf, const size_t bytes, const char *file,
			const char *arg1, const char *arg2, const bool page)
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

	if (!page) {
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
		spawn(pager, NULL, NULL, NULL, F_CLI);
		_exit(1);
	}

	/* In parent */
	waitpid(pid, &tmp, 0);
	close(pipefd[0]);

	return NULL;
}

static inline bool getutil(char *util)
{
	return spawn("which", util, NULL, NULL, F_NORMAL | F_NOTRACE) == 0;
}

/*
 * Follows the stat(1) output closely
 */
static bool show_stats(const char *fpath, const char *fname, const struct stat *sb)
{
	int fd;
	char *p, *begin = g_buf;
	size_t r;
	FILE *fp;

	fd = create_tmp_file();
	if (fd == -1)
		return FALSE;

	r = xstrlcpy(g_buf, "stat \"", PATH_MAX);
	r += xstrlcpy(g_buf + r - 1, fpath, PATH_MAX);
	g_buf[r - 2] = '\"';
	g_buf[r - 1] = '\0';
	DPRINTF_S(g_buf);

	fp = popen(g_buf, "r");
	if (fp) {
		while (fgets(g_buf, CMD_LEN_MAX - 1, fp))
			dprintf(fd, "%s", g_buf);
		pclose(fp);
	}

	if (S_ISREG(sb->st_mode)) {
		/* Show file(1) output */
		p = get_output(g_buf, CMD_LEN_MAX, "file", "-b", fpath, FALSE);
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
	}

	dprintf(fd, "\n\n");
	close(fd);

	spawn(pager, g_tmpfpath, NULL, NULL, F_CLI);
	unlink(g_tmpfpath);
	return TRUE;
}

static size_t get_fs_info(const char *path, bool type)
{
	struct statvfs svb;

	if (statvfs(path, &svb) == -1)
		return 0;

	if (type == CAPACITY)
		return svb.f_blocks << ffs((int)(svb.f_bsize >> 1));

	return svb.f_bavail << ffs((int)(svb.f_frsize >> 1));
}

static bool show_mediainfo(const char *fpath, const char *arg)
{
	if (!getutil(utils[cfg.metaviewer]))
		return FALSE;

	exitcurses();
	get_output(NULL, 0, utils[cfg.metaviewer], fpath, arg, TRUE);
	refresh();
	return TRUE;
}

/* List or extract archive */
static void handle_archive(char *fpath, const char *dir, char op)
{
	char arg[] = "-tvf"; /* options for tar/bsdtar to list files */
	char *util;

	if (getutil(utils[ATOOL])) {
		util = utils[ATOOL];
		arg[1] = op;
		arg[2] = '\0';
	} else if (getutil(utils[BSDTAR])) {
		util = utils[BSDTAR];
		if (op == 'x')
			arg[1] = op;
	} else if (is_suffix(fpath, ".zip")) {
		util = utils[UNZIP];
		arg[1] = (op == 'l') ? 'v' /* verbose listing */ : '\0';
		arg[2] = '\0';
	} else {
		util = utils[TAR];
		if (op == 'x')
			arg[1] = op;
	}

	if (op == 'x') { /* extract */
		spawn(util, arg, fpath, dir, F_NORMAL);
	} else { /* list */
		exitcurses();
		get_output(NULL, 0, util, arg, fpath, TRUE);
		refresh();
	}
}

static char *visit_parent(char *path, char *newpath, int *presel)
{
	char *dir;

	/* There is no going back */
	if (istopdir(path)) {
		/* Continue in navigate-as-you-type mode, if enabled */
		if (cfg.filtermode)
			*presel = FILTER;
		return NULL;
	}

	/* Use a copy as dirname() may change the string passed */
	xstrlcpy(newpath, path, PATH_MAX);

	dir = dirname(newpath);
	if (access(dir, R_OK) == -1) {
		printwarn(presel);
		return NULL;
	}

	return dir;
}

static bool execute_file(int cur, char *path, char *newpath, int *presel)
{
	if (!ndents)
		return FALSE;

	/* Check if this is a directory */
	if (!S_ISREG(dents[cur].mode)) {
		printwait("not regular file", presel);
		return FALSE;
	}

	/* Check if file is executable */
	if (!(dents[cur].mode & 0100)) {
		printwait("permission denied", presel);
		return FALSE;
	}

	mkpath(path, dents[cur].name, newpath);
	spawn(newpath, NULL, NULL, path, F_NORMAL);

	return TRUE;
}

static bool create_dir(const char *path)
{
	if (!xdiraccess(path)) {
		if (errno != ENOENT)
			return FALSE;

		if (mkdir(path, 0755) == -1)
			return FALSE;
	}

	return TRUE;
}

static bool sshfs_mount(char *path, char *newpath, int *presel)
{
	uchar flag = F_NORMAL;
	int r;
	char *tmp, *env, *cmd = "sshfs";

	if (!getutil(cmd)) {
		printwait(messages[UTIL_MISSING], presel);
		return FALSE;
	}

	tmp = xreadline(NULL, "host: ");
	if (!tmp[0])
		return FALSE;

	/* Create the mount point */
	mkpath(cfgdir, tmp, newpath);
	if (!create_dir(newpath)) {
		printwait(strerror(errno), presel);
		return FALSE;
	}

	/* Convert "Host" to "Host:" */
	r = strlen(tmp);
	tmp[r] = ':';
	tmp[r + 1] = '\0';

	env = getenv("NNN_SSHFS_OPTS");
	if (env)
		flag |= F_MULTI;
	else
		env = cmd;

	/* Connect to remote */
	if (spawn(env, tmp, newpath, NULL, flag)) {
		printwait("mount failed", presel);
		return FALSE;
	}

	return TRUE;
}

static bool sshfs_unmount(char *path, char *newpath, int *presel)
{
	static char cmd[] = "fusermount3"; /* Arch Linux utility */
	static bool found = FALSE;
	char *tmp;

	/* On Ubuntu it's fusermount */
	if (!found && !getutil(cmd)) {
		cmd[10] = '\0';
		found = TRUE;
	}

	tmp = xreadline(NULL, "host: ");
	if (!tmp[0])
		return FALSE;

	/* Create the mount point */
	mkpath(cfgdir, tmp, newpath);
	if (!xdiraccess(newpath)) {
		*presel = MSGWAIT;
		return FALSE;
	}

	if (spawn(cmd, "-u", newpath, NULL, F_NORMAL)) {
		printwait("unmount failed", presel);
		return FALSE;
	}

	return TRUE;
}

static void lock_terminal(void)
{
	char *tmp = utils[LOCKER];
	if (!getutil(tmp))
		tmp = utils[CMATRIX];;

	spawn(tmp, NULL, NULL, NULL, F_NORMAL);
}

static void printkv(kv *kvarr, int fd, uchar max)
{
	uchar i = 0;

	for (; i < max && kvarr[i].key; ++i)
		dprintf(fd, " %c: %s\n", (char)kvarr[i].key, kvarr[i].val);
}

/*
 * The help string tokens (each line) start with a HEX value
 * which indicates the number of spaces to print before the
 * particular token. This method was chosen instead of a flat
 * string because the number of bytes in help was increasing
 * the binary size by around a hundred bytes. This would only
 * have increased as we keep adding new options.
 */
static bool show_help(const char *path)
{
	int i, fd;
	const char *start, *end;
	const char helpstr[] = {
		"0\n"
		"1NAVIGATION\n"
		"a↑ k  Up          PgUp ^U  Scroll up\n"
		"a↓ j  Down        PgDn ^D  Scroll down\n"
		"a← h  Parent dir  ~ ` @ -  HOME, /, start, last\n"
	      "8↵ → l  Open file/dir     .  Toggle show hidden\n"
	       "9g ^A  First entry    G ^E  Last entry\n"
		  "cb  Pin current dir  ^B  Go to pinned dir\n"
	     "7Tab ^I  Next context      d  Toggle detail view\n"
	       "9, ^/  Lead key    N LeadN  Context N\n"
		  "c/  Filter/Lead  Ins ^T  Toggle nav-as-you-type\n"
		"aEsc  Exit prompt      ^L  Redraw/clear prompt\n"
		  "cq  Quit context  Lead'  First file\n"
	       "9Q ^Q  Quit  ^G  QuitCD  ?  Help, config\n"
		"1FILES\n"
		 "b^O  Open with...      n  Create new/link\n"
		  "cD  File details     ^R  Rename/duplicate\n"
	   "5⎵ ^K / Y  Select entry/all  r  Batch rename\n"
	       "9K ^Y  Toggle selection  y  List selection\n"
		  "cP  Copy selection    X  Delete selection\n"
		  "cV  Move selection   ^X  Delete entry\n"
		  "cf  Create archive  m M  Brief/full mediainfo\n"
		 "b^F  Extract archive   F  List archive\n"
		  "ce  Edit in EDITOR    p  Open in PAGER\n"
		"1ORDER TOGGLES\n"
		 "b^J  du      E  Extn   S  Apparent du\n"
		 "b^W  Random  s  Size   t  Time modified\n"
		"1MISC\n"
	       "9! ^]  Shell   L  Lock   C  Execute entry\n"
	       "9R ^V  Pick plugin      xK  Run plugin key K\n"
	          "cc  SSHFS mount       u  Unmount\n"
		 "b^P  Prompt  ^N  Note  =  Launcher\n"};

	fd = create_tmp_file();
	if (fd == -1)
		return FALSE;

	start = end = helpstr;
	while (*end) {
		if (*end == '\n') {
			dprintf(fd, "%*c%.*s",
				xchartohex(*start), ' ', (int)(end - start), start + 1);
			start = end + 1;
		}

		++end;
	}

	dprintf(fd, "\nVOLUME: %s of ", coolsize(get_fs_info(path, FREE)));
	dprintf(fd, "%s free\n\n", coolsize(get_fs_info(path, CAPACITY)));

	if (bookmark[0].val) {
		dprintf(fd, "BOOKMARKS\n");
		printkv(bookmark, fd, BM_MAX);
		dprintf(fd, "\n");
	}

	if (plug[0].val) {
		dprintf(fd, "PLUGIN KEYS\n");
		printkv(plug, fd, PLUGIN_MAX);
		dprintf(fd, "\n");
	}

	for (i = NNN_OPENER; i <= NNN_TRASH; ++i) {
		start = getenv(env_cfg[i]);
		if (start)
			dprintf(fd, "%s: %s\n", env_cfg[i], start);
	}

	if (g_cppath)
		dprintf(fd, "SELECTION FILE: %s\n", g_cppath);

	dprintf(fd, "\nv%s\n%s\n", VERSION, GENERAL_INFO);
	close(fd);

	spawn(pager, g_tmpfpath, NULL, NULL, F_CLI);
	unlink(g_tmpfpath);
	return TRUE;
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

static void dentfree(void)
{
	free(pnamebuf);
	free(dents);
}

static int dentfill(char *path, struct entry **dents)
{
	static uint open_max;
	int n = 0, count, flags = 0;
	ulong num_saved;
	struct dirent *dp;
	char *namep, *pnb, *buf = NULL;
	struct entry *dentp;
	size_t off = 0, namebuflen = NAMEBUF_INCR;
	struct stat sb_path, sb;
	DIR *dirp = opendir(path);

	if (!dirp)
		return 0;

	int fd = dirfd(dirp);

	if (cfg.blkorder) {
		num_files = 0;
		dir_blocks = 0;
		buf = (char *)alloca(strlen(path) + NAME_MAX + 2);

		if (fstatat(fd, path, &sb_path, 0) == -1) {
			closedir(dirp);
			printwarn(NULL);
			return 0;
		}

		/* Increase current open file descriptor limit */
		if (!open_max)
			open_max = max_openfds();
	}

	dp = readdir(dirp);
	if (!dp)
		goto exit;

	if (cfg.blkorder || dp->d_type == DT_UNKNOWN) {
		/*
		 * Optimization added for filesystems which support dirent.d_type
		 * see readdir(3)
		 * Known drawbacks:
		 * - the symlink size is set to 0
		 * - the modification time of the symlink is set to that of the target file
		 */
		flags = AT_SYMLINK_NOFOLLOW;
	}

	do {
		namep = dp->d_name;

		/* Skip self and parent */
		if ((namep[0] == '.' && (namep[1] == '\0' || (namep[1] == '.' && namep[2] == '\0'))))
			continue;

		if (!cfg.showhidden && namep[0] == '.') {
			if (!cfg.blkorder)
				continue;

			if (fstatat(fd, namep, &sb, AT_SYMLINK_NOFOLLOW) == -1)
				continue;

			if (S_ISDIR(sb.st_mode)) {
				if (sb_path.st_dev == sb.st_dev) {
					ent_blocks = 0;
					mkpath(path, namep, buf);

					mvprintw(xlines - 1, 0, "scanning %s [^C aborts]\n",
						 xbasename(buf));
					refresh();
					if (nftw(buf, nftw_fn, open_max,
						 FTW_MOUNT | FTW_PHYS) == -1) {
						DPRINTF_S("nftw failed");
						dir_blocks += (cfg.apparentsz
							       ? sb.st_size
							       : sb.st_blocks);
					} else
						dir_blocks += ent_blocks;

					if (interrupted) {
						closedir(dirp);
						return n;
					}
				}
			} else {
				dir_blocks += (cfg.apparentsz ? sb.st_size : sb.st_blocks);
				++num_files;
			}

			continue;
		}

		if (fstatat(fd, namep, &sb, flags) == -1) {
			/* List a symlink with target missing */
			if (!flags && errno == ENOENT) {
				if (fstatat(fd, namep, &sb, AT_SYMLINK_NOFOLLOW) == -1) {
					DPRINTF_S(namep);
					DPRINTF_S(strerror(errno));
					continue;
				}
			} else {
				DPRINTF_S(namep);
				DPRINTF_S(strerror(errno));
				continue;
			}
		}

		if (n == total_dents) {
			total_dents += ENTRY_INCR;
			*dents = xrealloc(*dents, total_dents * sizeof(**dents));
			if (!*dents) {
				free(pnamebuf);
				closedir(dirp);
				errexit();
			}
			DPRINTF_P(*dents);
		}

		/* If not enough bytes left to copy a file name of length NAME_MAX, re-allocate */
		if (namebuflen - off < NAME_MAX + 1) {
			namebuflen += NAMEBUF_INCR;

			pnb = pnamebuf;
			pnamebuf = (char *)xrealloc(pnamebuf, namebuflen);
			if (!pnamebuf) {
				free(*dents);
				closedir(dirp);
				errexit();
			}
			DPRINTF_P(pnamebuf);

			/* realloc() may result in memory move, we must re-adjust if that happens */
			if (pnb != pnamebuf) {
				dentp = *dents;
				dentp->name = pnamebuf;

				for (count = 1; count < n; ++dentp, ++count)
					/* Current filename starts at last filename start + length */
					(dentp + 1)->name = (char *)((size_t)dentp->name
							    + dentp->nlen);
			}
		}

		dentp = *dents + n;

		/* Selection file name */
		dentp->name = (char *)((size_t)pnamebuf + off);
		dentp->nlen = xstrlcpy(dentp->name, namep, NAME_MAX + 1);
		off += dentp->nlen;

		/* Copy other fields */
		dentp->t = sb.st_mtime;
		if (dp->d_type == DT_LNK && !flags) { /* Do not add sizes for links */
			dentp->mode = (sb.st_mode & ~S_IFMT) | S_IFLNK;
			dentp->size = 0;
		} else {
			dentp->mode = sb.st_mode;
			dentp->size = sb.st_size;
		}
		dentp->flags = 0;

		if (cfg.blkorder) {
			if (S_ISDIR(sb.st_mode)) {
				ent_blocks = 0;
				num_saved = num_files + 1;
				mkpath(path, namep, buf);

				mvprintw(xlines - 1, 0, "scanning %s [^C aborts]\n", xbasename(buf));
				refresh();
				if (nftw(buf, nftw_fn, open_max, FTW_MOUNT | FTW_PHYS) == -1) {
					DPRINTF_S("nftw failed");
					dentp->blocks = (cfg.apparentsz ? sb.st_size : sb.st_blocks);
				} else
					dentp->blocks = ent_blocks;

				if (sb_path.st_dev == sb.st_dev) // NOLINT
					dir_blocks += dentp->blocks;
				else
					num_files = num_saved;

				if (interrupted) {
					closedir(dirp);
					return n;
				}
			} else {
				dentp->blocks = (cfg.apparentsz ? sb.st_size : sb.st_blocks);
				dir_blocks += dentp->blocks;
				++num_files;
			}
		}

		if (flags) {
			/* Flag if this is a dir or symlink to a dir */
			if (S_ISLNK(sb.st_mode)) {
				sb.st_mode = 0;
				fstatat(fd, namep, &sb, 0);
			}

			if (S_ISDIR(sb.st_mode))
				dentp->flags |= DIR_OR_LINK_TO_DIR;
		} else if (dp->d_type == DT_DIR || (dp->d_type == DT_LNK && S_ISDIR(sb.st_mode)))
			dentp->flags |= DIR_OR_LINK_TO_DIR;

		++n;
	} while ((dp = readdir(dirp)));

exit:
	/* Should never be null */
	if (closedir(dirp) == -1) {
		dentfree();
		errexit();
	}

	return n;
}

/*
 * Return the position of the matching entry or 0 otherwise
 * Note there's no NULL check for fname
 */
static int dentfind(const char *fname, int n)
{
	int i = 0;

	for (; i < n; ++i)
		if (xstrcmp(fname, dents[i].name) == 0)
			return i;

	return 0;
}

static void populate(char *path, char *lastname)
{
#ifdef DBGMODE
	struct timespec ts1, ts2;

	clock_gettime(CLOCK_REALTIME, &ts1); /* Use CLOCK_MONOTONIC on FreeBSD */
#endif

	ndents = dentfill(path, &dents);
	if (!ndents)
		return;

	if (!cfg.wild)
		qsort(dents, ndents, sizeof(*dents), entrycmp);

#ifdef DBGMODE
	clock_gettime(CLOCK_REALTIME, &ts2);
	DPRINTF_U(ts2.tv_nsec - ts1.tv_nsec);
#endif

	/* Find cur from history */
	/* No NULL check for lastname, always points to an array */
	if (!*lastname)
		move_cursor(0, 0);
	else
		move_cursor(dentfind(lastname, ndents), 0);
}

static void move_cursor(int target, int ignore_scrolloff)
{
	int delta, scrolloff, onscreen = xlines - 4;
	target = MAX(0, MIN(ndents - 1, target));
	delta = target - cur;
	cur = target;
	if (!ignore_scrolloff) {
		scrolloff = MIN(SCROLLOFF, onscreen >> 1);
		/*
		 * When ignore_scrolloff is 1, the cursor can jump into the scrolloff
		 * margin area, but when ignore_scrolloff is 0, act like a boa
		 * constrictor and squeeze the cursor towards the middle region of the
		 * screen by allowing it to move inward and disallowing it to move
		 * outward (deeper into the scrolloff margin area).
		 */
		if (cur < curscroll + scrolloff && delta < 0)
			curscroll += delta;
		else if (cur > curscroll + onscreen - scrolloff - 1 && delta > 0)
			curscroll += delta;
	}
	curscroll = MIN(curscroll, MIN(cur, ndents - onscreen));
	curscroll = MAX(curscroll, MAX(cur - (onscreen - 1), 0));
}

static void redraw(char *path)
{
	xlines = LINES;
	xcols = COLS;

	int ncols = (xcols <= PATH_MAX) ? xcols : PATH_MAX;
	int lastln = xlines, onscreen = xlines - 4;
	int i, attrs;
	char buf[12];
	char c;

	--lastln;

	/* Clear screen */
	erase();

	/* Enforce scroll/cursor invariants */
	move_cursor(cur, 1);

#ifdef DIR_LIMITED_COPY
	if (cfg.copymode)
		if (g_crc != crc8fast((uchar *)dents, ndents * sizeof(struct entry))) {
			cfg.copymode = 0;
			DPRINTF_S("selection off");
		}
#endif

	/* Fail redraw if < than 11 columns, context info prints 10 chars */
	if (ncols < 11) {
		printmsg("too few columns!");
		return;
	}

	DPRINTF_D(cur);
	DPRINTF_S(path);

	printw("[");
	for (i = 0; i < CTX_MAX; ++i) {
		if (!g_ctx[i].c_cfg.ctxactive)
			printw("%d ", i + 1);
		else if (cfg.curctx != i) {
			attrs = COLOR_PAIR(i + 1) | A_BOLD | A_UNDERLINE;
			attron(attrs);
			printw("%d", i + 1);
			attroff(attrs);
			printw(" ");
		} else {
			/* Print current context in reverse */
			attrs = COLOR_PAIR(i + 1) | A_BOLD | A_REVERSE;
			attron(attrs);
			printw("%d", i + 1);
			attroff(attrs);
			printw(" ");
		}
	}

	printw("\b] "); /* 10 chars printed in total for contexts - "[1 2 3 4] " */

	attron(A_UNDERLINE);
	/* No text wrapping in cwd line, store the truncating char in c */
	c = path[ncols - 11];
	path[ncols - 11] = '\0';
	printw("%s\n\n", path);
	attroff(A_UNDERLINE);
	path[ncols - 11] = c; /* Restore c */

	/* Calculate the number of cols available to print entry name */
	if (cfg.showdetail) {
		/* Fallback to light mode if less than 35 columns */
		if (ncols < 36) {
			cfg.showdetail ^= 1;
			printptr = &printent;
			ncols -= 5;
		} else
			ncols -= 35;
	} else
		ncols -= 5;

	if (!cfg.wild) {
		attron(COLOR_PAIR(cfg.curctx + 1) | A_BOLD);
		cfg.dircolor = 1;
	}

	/* Print listing */
	for (i = curscroll; i < ndents && i < curscroll + onscreen; ++i) {
		printptr(&dents[i], i == cur, ncols);
	}

	/* Must reset e.g. no files in dir */
	if (cfg.dircolor) {
		attroff(COLOR_PAIR(cfg.curctx + 1) | A_BOLD);
		cfg.dircolor = 0;
	}

	if (ndents) {
		char sort[] = "\0 ";

		if (cfg.mtimeorder)
			sort[0] = 'T';
		else if (cfg.sizeorder)
			sort[0] = 'S';
		else if (cfg.extnorder)
			sort[0] = 'E';

		/* We need to show filename as it may be truncated in directory listing */
		if (!cfg.showdetail || !cfg.blkorder)
			mvprintw(lastln, 0, "%d/%d %s[%s]\n", cur + 1, ndents, sort,
				 unescape(dents[cur].name, NAME_MAX));
		else {
			xstrlcpy(buf, coolsize(dir_blocks << BLK_SHIFT), 12);
			c = cfg.apparentsz ? 'a' : 'd';

			mvprintw(lastln, 0, "%d/%d %cu: %s (%lu files) free: %s [%s]\n",
				 cur + 1, ndents, c, buf, num_files,
				 coolsize(get_fs_info(path, FREE)),
				 unescape(dents[cur].name, NAME_MAX));
		}
	} else
		printmsg("0/0");
}

static void browse(char *ipath)
{
	char newpath[PATH_MAX] __attribute__ ((aligned));
	char mark[PATH_MAX] __attribute__ ((aligned));
	char rundir[PATH_MAX] __attribute__ ((aligned));
	char runfile[NAME_MAX + 1] __attribute__ ((aligned));
	int r = -1, fd, presel, ncp = 0, copystartid = 0, copyendid = 0, onscreen;
	enum action sel;
	bool dir_changed = FALSE;
	struct stat sb;
	char *path, *lastdir, *lastname, *dir, *tmp;
	MEVENT event;

	atexit(dentfree);

	/* setup first context */
	xstrlcpy(g_ctx[0].c_path, ipath, PATH_MAX); /* current directory */
	path = g_ctx[0].c_path;
	g_ctx[0].c_last[0] = g_ctx[0].c_name[0] = newpath[0] = mark[0] = '\0';
	rundir[0] = runfile[0] = '\0';
	lastdir = g_ctx[0].c_last; /* last visited directory */
	lastname = g_ctx[0].c_name; /* last visited filename */
	g_ctx[0].c_fltr[0] = g_ctx[0].c_fltr[1] = '\0';
	g_ctx[0].c_cfg = cfg; /* current configuration */

	cfg.filtermode ?  (presel = FILTER) : (presel = 0);

	dents = xrealloc(dents, total_dents * sizeof(struct entry));
	if (!dents)
		errexit();

	/* Allocate buffer to hold names */
	pnamebuf = (char *)xrealloc(pnamebuf, NAMEBUF_INCR);
	if (!pnamebuf)
		errexit();

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

	/* Can fail when permissions change while browsing.
	 * It's assumed that path IS a directory when we are here.
	 */
	if (access(path, R_OK) == -1)
		printwarn(&presel);

	populate(path, lastname);
	if (interrupted) {
		interrupted = FALSE;
		cfg.apparentsz = 0;
		cfg.blkorder = 0;
		BLK_SHIFT = 9;
		presel = CONTROL('L');
	}

#ifdef LINUX_INOTIFY
	if (presel != FILTER && inotify_wd == -1)
		inotify_wd = inotify_add_watch(inotify_fd, path, INOTIFY_MASK);
#elif defined(BSD_KQUEUE)
	if (presel != FILTER && event_fd == -1) {
#if defined(O_EVTONLY)
		event_fd = open(path, O_EVTONLY);
#else
		event_fd = open(path, O_RDONLY);
#endif
		if (event_fd >= 0)
			EV_SET(&events_to_monitor[0], event_fd, EVFILT_VNODE,
			       EV_ADD | EV_CLEAR, KQUEUE_FFLAGS, 0, path);
	}
#endif

	while (1) {
		redraw(path);
nochange:
		/* Exit if parent has exited */
		if (getppid() == 1)
			_exit(0);

		/* If CWD is deleted or moved or perms changed, find an accessible parent */
		if (access(path, F_OK)) {
			DPRINTF_S("directory inaccessible");

			/* Save history */
			xstrlcpy(lastname, xbasename(path), NAME_MAX + 1);

			xstrlcpy(newpath, path, PATH_MAX);
			while (true) {
				dir = visit_parent(path, newpath, &presel);
				if (istopdir(path) || istopdir(newpath)) {
					if (!dir)
						dir = dirname(newpath);
					break;
				}
				if (!dir) {
					xstrlcpy(path, newpath, PATH_MAX);
					continue;
				}
				break;
			}

			xstrlcpy(path, dir, PATH_MAX);

			setdirwatch();
			mvprintw(xlines - 1, 0, "cannot access directory\n");
			xdelay();
			goto begin;
		}

		/* If STDIN is no longer a tty (closed) we should exit */
		if (!isatty(STDIN_FILENO) && !cfg.picker)
			return;

		sel = nextsel(presel);
		if (presel)
			presel = 0;

		switch (sel) {
		case SEL_CLICK:
			if (getmouse(&event) != OK)
				goto nochange; // fallthrough
		case SEL_BACK:
			/* Handle clicking on a context at the top */
			if (sel == SEL_CLICK && event.bstate == BUTTON1_CLICKED && event.y == 0) {
				/* Get context from: "[1 2 3 4]..." */
				r = event.x >> 1;

				/* If clicked after contexts, go to parent */
				if (r >= CTX_MAX)
					sel = SEL_BACK;
				else if (0 <= r && r < CTX_MAX && r != cfg.curctx) {
					savecurctx(&cfg, path, dents[cur].name, r);

					/* Reset the pointers */
					path = g_ctx[r].c_path;
					lastdir = g_ctx[r].c_last;
					lastname = g_ctx[r].c_name;

					setdirwatch();
					goto begin;
				}
			}

			if (sel == SEL_BACK) {
				dir = visit_parent(path, newpath, &presel);
				if (!dir)
					goto nochange;

				/* Save last working directory */
				xstrlcpy(lastdir, path, PATH_MAX);

				/* Save history */
				xstrlcpy(lastname, xbasename(path), NAME_MAX + 1);

				xstrlcpy(path, dir, PATH_MAX);

				setdirwatch();
				goto begin;
			}

#if NCURSES_MOUSE_VERSION > 1
			/* Scroll up */
			if (event.bstate == BUTTON4_PRESSED && ndents) {
				move_cursor((cur + ndents - 1) % ndents, 0);
				break;
			}

			/* Scroll down */
			if (event.bstate == BUTTON5_PRESSED && ndents) {
				move_cursor((cur + 1) % ndents, 0);
				break;
			}
#endif

			/* Toggle filter mode on left click on last 2 lines */
			if (event.y >= xlines - 2) {
				cfg.filtermode ^= 1;
				if (cfg.filtermode) {
					presel = FILTER;
					goto nochange;
				}

				/* Start watching the directory */
				dir_changed = TRUE;

				if (ndents)
					copycurname();
				goto begin;
			}

			/* Handle clicking on a file */
			if (2 <= event.y && event.y <= ndents + 1) {
				r = curscroll + (event.y - 2);
				move_cursor(r, 1);

				/*Single click just selects, double click also opens */
				if (event.bstate != BUTTON1_DOUBLE_CLICKED)
					break;
			} else {
				if (cfg.filtermode)
					presel = FILTER;
				goto nochange; // fallthrough
			}
		case SEL_NAV_IN: // fallthrough
		case SEL_GOIN:
			/* Cannot descend in empty directories */
			if (!ndents)
				goto begin;

			mkpath(path, dents[cur].name, newpath);
			DPRINTF_S(newpath);

			/* Cannot use stale data in entry, file may be missing by now */
			if (stat(newpath, &sb) == -1) {
				printwarn(&presel);
				goto nochange;
			}
			DPRINTF_U(sb.st_mode);

			switch (sb.st_mode & S_IFMT) {
			case S_IFDIR:
				if (access(newpath, R_OK) == -1) {
					printwarn(&presel);
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
				/* If opened as vim plugin and Enter/^M pressed, pick */
				if (cfg.picker && sel == SEL_GOIN) {
					r = mkpath(path, dents[cur].name, newpath);
					appendfpath(newpath, r);
					writecp(pcopybuf, copybufpos - 1);
					return;
				}

				/* If open file is disabled on right arrow or `l`, return */
				if (cfg.nonavopen && sel == SEL_NAV_IN)
					continue;

				/* Handle plugin selection mode */
				if (cfg.runplugin) {
					if (!plugindir || (cfg.runctx != cfg.curctx)
					    /* Must be in plugin directory to select plugin */
					    || (strcmp(path, plugindir) != 0))
						continue;

					mkpath(path, dents[cur].name, newpath);
					/* Copy to path so we can return back to earlier dir */
					xstrlcpy(path, rundir, PATH_MAX);
					if (runfile[0]) {
						xstrlcpy(lastname, runfile, NAME_MAX);
						spawn(newpath, lastname, NULL, path, F_NORMAL);
						runfile[0] = '\0';
					} else
						spawn(newpath, NULL, NULL, path, F_NORMAL);
					rundir[0] = '\0';
					cfg.runplugin = 0;
					setdirwatch();
					goto begin;
				}

				/* If NNN_USE_EDITOR is set, open text in EDITOR */
				if (cfg.useeditor &&
				    get_output(g_buf, CMD_LEN_MAX, "file", FILE_OPTS, newpath, FALSE)
				    && g_buf[0] == 't' && g_buf[1] == 'e' && g_buf[2] == 'x'
				    && g_buf[3] == g_buf[0] && g_buf[4] == '/') {
					spawn(editor, newpath, NULL, path, F_CLI);
					continue;
				}

				if (!sb.st_size) {
					printwait("empty: use edit or open with", &presel);
					goto nochange;
				}

				/* Invoke desktop opener as last resort */
				spawn(opener, newpath, NULL, NULL, F_NOTRACE | F_NOWAIT);
				continue;
			}
			default:
				printwait("unsupported file", &presel);
				goto nochange;
			}
		case SEL_NEXT:
			if (ndents)
				move_cursor((cur + 1) % ndents, 0);
			break;
		case SEL_PREV:
			if (ndents)
				move_cursor((cur + ndents - 1) % ndents, 0);
			break;
		case SEL_PGDN: // fallthrough
			onscreen = xlines - 4;
			move_cursor(curscroll + (onscreen - 1), 1);
			curscroll += onscreen - 1;
			break;
		case SEL_CTRL_D:
			onscreen = xlines - 4;
			move_cursor(curscroll + (onscreen - 1), 1);
			curscroll += onscreen >> 1;
			break;
		case SEL_PGUP: // fallthrough
			onscreen = xlines - 4;
			move_cursor(curscroll, 1);
			curscroll -= onscreen - 1;
			break;
		case SEL_CTRL_U:
			onscreen = xlines - 4;
			move_cursor(curscroll, 1);
			curscroll -= onscreen >> 1;
			break;
		case SEL_HOME:
			move_cursor(0, 1);
			break;
		case SEL_END:
			move_cursor(ndents - 1, 1);
			break;
		case SEL_CDHOME: // fallthrough
		case SEL_CDBEGIN: // fallthrough
		case SEL_CDLAST: // fallthrough
		case SEL_CDROOT: // fallthrough
		case SEL_VISIT:
			switch (sel) {
			case SEL_CDHOME:
				dir = home;
				break;
			case SEL_CDBEGIN:
				dir = ipath;
				break;
			case SEL_CDLAST:
				dir = lastdir;
				break;
			case SEL_CDROOT:
				dir = "/";
				break;
			default: /* case SEL_VISIT */
				dir = mark;
				break;
			}

			if (dir[0] == '\0') {
				printwait("not set", &presel);
				goto nochange;
			}

			if (!xdiraccess(dir)) {
				presel = MSGWAIT;
				goto nochange;
			}

			if (strcmp(path, dir) == 0)
				goto nochange;

			/* SEL_CDLAST: dir pointing to lastdir */
			xstrlcpy(newpath, dir, PATH_MAX);

			/* Save last working directory */
			xstrlcpy(lastdir, path, PATH_MAX);

			xstrlcpy(path, newpath, PATH_MAX);
			lastname[0] = '\0';
			DPRINTF_S(path);
			setdirwatch();
			goto begin;
		case SEL_LEADER: // fallthrough
		case SEL_CYCLE: // fallthrough
		case SEL_CTX1: // fallthrough
		case SEL_CTX2: // fallthrough
		case SEL_CTX3: // fallthrough
		case SEL_CTX4:
			if (sel == SEL_CYCLE)
				fd = '>';
			else if (sel >= SEL_CTX1 && sel <= SEL_CTX4)
				fd = sel - SEL_CTX1 + '1';
			else
				fd = get_input(NULL);

			switch (fd) {
			case 'q': // fallthrough
			case '~': // fallthrough
			case '`': // fallthrough
			case '-': // fallthrough
			case '@':
				presel = fd;
				goto nochange;
			case '\'':
				for (r = 0; r < ndents; ++r) {
					if (!(dents[r].flags & DIR_OR_LINK_TO_DIR)) {
						move_cursor((r) % ndents, 0);
						break;
					}
				}
				if (r != ndents)
					continue;;
				goto nochange;
			case '>': // fallthrough
			case '.': // fallthrough
			case '<': // fallthrough
			case ',':
				r = cfg.curctx;
				if (fd == '>' || fd == '.')
					do
						r = (r + 1) & ~CTX_MAX;
					while (!g_ctx[r].c_cfg.ctxactive);
				else
					do
						r = (r + (CTX_MAX - 1)) & (CTX_MAX - 1);
					while (!g_ctx[r].c_cfg.ctxactive); // fallthrough
				fd = '1' + r; // fallthrough
			case '1': // fallthrough
			case '2': // fallthrough
			case '3': // fallthrough
			case '4':
				r = fd - '1'; /* Save the next context id */
				if (cfg.curctx == r) {
					if (sel != SEL_CYCLE)
						continue;

					(r == CTX_MAX - 1) ? (r = 0) : ++r;
					snprintf(newpath, PATH_MAX,
						 "Create context %d? [Enter]", r + 1);
					fd = get_input(newpath);
					if (fd != '\r')
						continue;
				}

				savecurctx(&cfg, path, dents[cur].name, r);

				/* Reset the pointers */
				path = g_ctx[r].c_path;
				lastdir = g_ctx[r].c_last;
				lastname = g_ctx[r].c_name;

				setdirwatch();
				goto begin;
			}

			if (!get_kv_val(bookmark, newpath, fd, BM_MAX, TRUE)) {
				printwait(messages[STR_INVBM_KEY], &presel);
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
			printwait(mark, &presel);
			goto nochange;
		case SEL_FLTR:
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
			presel = filterentries(path);

			/* Save current */
			if (ndents)
				copycurname();

			if (presel == 27) {
				presel = 0;
				break;
			}

			goto nochange;
		case SEL_MFLTR: // fallthrough
		case SEL_TOGGLEDOT: // fallthrough
		case SEL_DETAIL: // fallthrough
		case SEL_FSIZE: // fallthrough
		case SEL_ASIZE: // fallthrough
		case SEL_BSIZE: // fallthrough
		case SEL_EXTN: // fallthrough
		case SEL_MTIME: // fallthrough
		case SEL_WILD:
			switch (sel) {
			case SEL_MFLTR:
				cfg.filtermode ^= 1;
				if (cfg.filtermode) {
					presel = FILTER;
					goto nochange;
				}

				/* Start watching the directory */
				dir_changed = TRUE;
				break;
			case SEL_TOGGLEDOT:
				cfg.showhidden ^= 1;
				setdirwatch();
				break;
			case SEL_DETAIL:
				cfg.showdetail ^= 1;
				cfg.showdetail ? (printptr = &printent_long) : (printptr = &printent);
				continue;
			case SEL_FSIZE:
				cfg.sizeorder ^= 1;
				cfg.mtimeorder = 0;
				cfg.apparentsz = 0;
				cfg.blkorder = 0;
				cfg.extnorder = 0;
				cfg.copymode = 0;
				cfg.wild = 0;
				break;
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
					BLK_SHIFT = ffs(S_BLKSIZE) - 1;
				}

				if (cfg.blkorder) {
					cfg.showdetail = 1;
					printptr = &printent_long;
				}
				cfg.mtimeorder = 0;
				cfg.sizeorder = 0;
				cfg.extnorder = 0;
				cfg.copymode = 0;
				cfg.wild = 0;
				break;
			case SEL_EXTN:
				cfg.extnorder ^= 1;
				cfg.sizeorder = 0;
				cfg.mtimeorder = 0;
				cfg.apparentsz = 0;
				cfg.blkorder = 0;
				cfg.copymode = 0;
				cfg.wild = 0;
				break;
			case SEL_MTIME:
				cfg.mtimeorder ^= 1;
				cfg.sizeorder = 0;
				cfg.apparentsz = 0;
				cfg.blkorder = 0;
				cfg.extnorder = 0;
				cfg.copymode = 0;
				cfg.wild = 0;
				break;
			default: /* SEL_WILD */
				cfg.wild ^= 1;
				cfg.mtimeorder = 0;
				cfg.sizeorder = 0;
				cfg.apparentsz = 0;
				cfg.blkorder = 0;
				cfg.extnorder = 0;
				cfg.copymode = 0;
				setdirwatch();
				goto nochange;
			}

			/* Save current */
			if (ndents)
				copycurname();
			goto begin;
		case SEL_STATS:
			if (!ndents)
				break;

			mkpath(path, dents[cur].name, newpath);
			if (lstat(newpath, &sb) == -1 || !show_stats(newpath, dents[cur].name, &sb)) {
				printwarn(&presel);
				goto nochange;
			}
			break;
		case SEL_MEDIA: // fallthrough
		case SEL_FMEDIA: // fallthrough
		case SEL_ARCHIVELS: // fallthrough
		case SEL_EXTRACT: // fallthrough
		case SEL_RUNEDIT: // fallthrough
		case SEL_RUNPAGE:
			if (!ndents)
				break; // fallthrough
		case SEL_REDRAW: // fallthrough
		case SEL_RENAMEALL: // fallthrough
		case SEL_HELP: // fallthrough
		case SEL_NOTE: // fallthrough
		case SEL_LOCK:
		{
			if (ndents)
				mkpath(path, dents[cur].name, newpath);
			r = TRUE;

			switch (sel) {
			case SEL_MEDIA: // fallthrough
			case SEL_FMEDIA:
				tmp = (sel == SEL_FMEDIA) ? "-f" : NULL;
				show_mediainfo(newpath, tmp);
				setdirwatch();
				goto nochange;
			case SEL_ARCHIVELS:
				handle_archive(newpath, path, 'l');
				break;
			case SEL_EXTRACT:
				handle_archive(newpath, path, 'x');
				break;
			case SEL_REDRAW:
				if (ndents)
					copycurname();
				goto begin;
			case SEL_RENAMEALL:
				if (!batch_rename(path)) {
					printwait("batch rename failed", &presel);
					goto nochange;
				}
				break;
			case SEL_HELP:
				r = show_help(path);
				break;
			case SEL_RUNEDIT:
				spawn(editor, dents[cur].name, NULL, path, F_CLI);
				break;
			case SEL_RUNPAGE:
				spawn(pager, dents[cur].name, NULL, path, F_CLI);
				break;
			case SEL_NOTE:
			{
				static char *notepath;

				notepath = notepath ? notepath : getenv(env_cfg[NNN_NOTE]);
				if (!notepath) {
					printwait("set NNN_NOTE", &presel);
					goto nochange;
				}

				spawn(editor, notepath, NULL, path, F_CLI);
				break;
			}
			default: /* SEL_LOCK */
				lock_terminal();
				break;
			}

			if (!r) {
				printwait(messages[UTIL_MISSING], &presel);
				goto nochange;
			}

			/* In case of successful operation, reload contents */

			/* Continue in navigate-as-you-type mode, if enabled */
			if (cfg.filtermode)
				presel = FILTER;

			/* Save current */
			if (ndents)
				copycurname();

			/* Repopulate as directory content may have changed */
			goto begin;
		}
		case SEL_SEL:
			if (!ndents)
				goto nochange;

			if (cfg.copymode) {
				/*
				 * Clear the selection file on first copy.
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

				/* Do not select if already selected */
				if (!(dents[cur].flags & FILE_COPIED)) {
					r = mkpath(path, dents[cur].name, newpath);
					appendfpath(newpath, r);

					++ncp;
					dents[cur].flags |= FILE_COPIED;
				}

				/* move cursor to the next entry if this is not the last entry */
				if (cur != ndents - 1)
					move_cursor((cur + 1) % ndents, 0);
			} else {
				r = mkpath(path, dents[cur].name, newpath);

				if (copybufpos) {
					resetcpind();

					/* Keep the copy buf in sync */
					copybufpos = 0;
				}
				appendfpath(newpath, r);

				writecp(newpath, r - 1); /* Truncate NULL from end */
				spawn(copier, NULL, NULL, NULL, F_NOTRACE);

				dents[cur].flags |= FILE_COPIED;
			}
			break;
		case SEL_SELMUL:
			cfg.copymode ^= 1;
			if (cfg.copymode) {
				if (copybufpos) {
					resetcpind();
					writecp(NULL, 0);
					copybufpos = 0;
				}
				g_crc = crc8fast((uchar *)dents, ndents * sizeof(struct entry));
				copystartid = cur;
				ncp = 0;
				mvprintw(xlines - 1, 0, "selection on\n");
				xdelay();
				continue;
			}

			if (!ncp) { /* Handle range selection */
#ifndef DIR_LIMITED_COPY
				if (g_crc != crc8fast((uchar *)dents,
						      ndents * sizeof(struct entry))) {
					cfg.copymode = 0;
					printwait("dir/content changed", &presel);
					goto nochange;
				}
#endif
				if (cur < copystartid) {
					copyendid = copystartid;
					copystartid = cur;
				} else
					copyendid = cur;
			} // fallthrough
		case SEL_SELALL:
			if (sel == SEL_SELALL) {
				if (!ndents)
					goto nochange;

				cfg.copymode = 0;
				copybufpos = 0;
				ncp = 0; /* Override single/multi path selection */
				copystartid = 0;
				copyendid = ndents - 1;
			}

			if ((!ncp && copystartid < copyendid) || sel == SEL_SELALL) {
				for (r = copystartid; r <= copyendid; ++r) {
					appendfpath(newpath, mkpath(path, dents[r].name, newpath));
					dents[r].flags |= FILE_COPIED;
				}

				ncp = copyendid - copystartid + 1;
				mvprintw(xlines - 1, 0, "%d selected\n", ncp);
				xdelay();
			}

			if (copybufpos) { /* File path(s) written to the buffer */
				writecp(pcopybuf, copybufpos - 1); /* Truncate NULL from end */
				spawn(copier, NULL, NULL, NULL, F_NOTRACE);

				if (ncp) { /* Some files cherry picked */
					mvprintw(xlines - 1, 0, "%d selected\n", ncp);
					xdelay();
				}
			} else {
				printwait("selection off", &presel);
				goto nochange;
			}
			continue;
		case SEL_SELLST:
			if (showcplist() || showcpfile()) {
				if (cfg.filtermode)
					presel = FILTER;
				break;
			}

			printwait(messages[NONE_SELECTED], &presel);
			goto nochange;
		case SEL_CP:
		case SEL_MV:
		case SEL_RMMUL:
		{
			if (!cpsafe()) {
				presel = MSGWAIT;
				goto nochange;
			}

			switch (sel) {
			case SEL_CP:
				cpstr(g_buf);
				break;
			case SEL_MV:
				mvstr(g_buf);
				break;
			default: /* SEL_RMMUL */
				rmmulstr(g_buf);
				break;
			}

			spawn("sh", "-c", g_buf, path, F_NORMAL);

			if (ndents)
				copycurname();
			if (cfg.filtermode)
				presel = FILTER;
			goto begin;
		}
		case SEL_RM:
		{
			if (!ndents)
				break;

			mkpath(path, dents[cur].name, newpath);
			xrm(newpath);

			/* Don't optimize cur if filtering is on */
			if (!cfg.filtermode && cur && access(newpath, F_OK) == -1)
				move_cursor(cur - 1, 0);

			/* We reduce cur only if it is > 0, so it's at least 0 */
			copycurname();

			if (cfg.filtermode)
				presel = FILTER;
			goto begin;
		}
		case SEL_OPENWITH: // fallthrough
		case SEL_RENAME:
			if (!ndents)
				break; // fallthrough
		case SEL_ARCHIVE: // fallthrough
		case SEL_NEW:
		{
			int dup = 'n';

			switch (sel) {
			case SEL_ARCHIVE:
				r = get_input("archive selection (else current)? [y/Y confirms]");
				if (r == 'y' || r == 'Y') {
					if (!cpsafe()) {
						presel = MSGWAIT;
						goto nochange;
					}
					tmp = NULL;
				} else if (!ndents) {
					printwait("no files", &presel);
					goto nochange;
				} else
					tmp = dents[cur].name;
				tmp = xreadline(tmp, "archive name: ");
				break;
			case SEL_OPENWITH:
#ifdef NORL
				tmp = xreadline(NULL, "open with: ");
#else
				presel = 0;
				tmp = getreadline("open with: ", path, ipath, &presel);
				if (presel == MSGWAIT)
					goto nochange;
#endif
				break;
			case SEL_NEW:
				tmp = xreadline(NULL, "name/link suffix [@ for none]: ");
				break;
			default: /* SEL_RENAME */
				tmp = xreadline(dents[cur].name, "");
				break;
			}

			if (!tmp || !*tmp)
				break;

			/* Allow only relative, same dir paths */
			if (tmp[0] == '/' || xstrcmp(xbasename(tmp), tmp) != 0) {
				printwait(messages[STR_INPUT_ID], &presel);
				goto nochange;
			}

			/* Confirm if app is CLI or GUI */
			if (sel == SEL_OPENWITH) {
				r = get_input("cli mode? [y/Y confirms]");
				(r == 'y' || r == 'Y') ? (r = F_CLI)
						       : (r = F_NOWAIT | F_NOTRACE | F_MULTI);
			}

			switch (sel) {
			case SEL_ARCHIVE:
			{
				char cmd[ARCHIVE_CMD_LEN];

				get_archive_cmd(cmd, tmp);

				(r == 'y' || r == 'Y') ? archive_selection(cmd, tmp, path)
						       : spawn(cmd, tmp, dents[cur].name,
							       path, F_NORMAL | F_MULTI);
				break;
			}
			case SEL_OPENWITH:
				mkpath(path, dents[cur].name, newpath);
				spawn(tmp, newpath, NULL, path, r);
				break;
			case SEL_RENAME:
				/* Skip renaming to same name */
				if (strcmp(tmp, dents[cur].name) == 0) {
					tmp = xreadline(dents[cur].name, "copy name: ");
					if (strcmp(tmp, dents[cur].name) == 0)
						goto nochange;

					dup = 'd';
				}
				break;
			default:
				break;
			}

			/* Complete OPEN, LAUNCH, ARCHIVE operations */
			if (sel != SEL_NEW && sel != SEL_RENAME) {
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
				printwarn(&presel);
				goto nochange;
			}

			/* Check if another file with same name exists */
			if (faccessat(fd, tmp, F_OK, AT_SYMLINK_NOFOLLOW) != -1) {
				if (sel == SEL_RENAME) {
					/* Overwrite file with same name? */
					r = get_input("overwrite? [y/Y confirms]");
					if (r != 'y' && r != 'Y') {
						close(fd);
						break;
					}
				} else {
					/* Do nothing in case of NEW */
					close(fd);
					printwait("entry exists", &presel);
					goto nochange;
				}
			}

			if (sel == SEL_RENAME) {
				/* Rename the file */
				if (dup == 'd')
					spawn("cp -r", dents[cur].name, tmp, path, F_CLI | F_NOTRACE);
				else if (renameat(fd, dents[cur].name, fd, tmp) != 0) {
					close(fd);
					printwarn(&presel);
					goto nochange;
				}
			} else {
				/* Check if it's a dir or file */
				r = get_input("create 'f'(ile) / 'd'(ir) / 's'(ym) / 'h'(ard)?");
				if (r == 'f') {
					r = openat(fd, tmp, O_CREAT, 0666);
					close(r);
				} else if (r == 'd') {
					r = mkdirat(fd, tmp, 0777);
				} else if (r == 's' || r == 'h') {
					if (tmp[0] == '@' && tmp[1] == '\0')
						tmp[0] = '\0';
					r = xlink(tmp, path, newpath, &presel, r);
					close(fd);

					if (r <= 0)
						goto nochange;

					if (cfg.filtermode)
						presel = FILTER;
					if (ndents)
						copycurname();
					goto begin;
				} else {
					close(fd);
					break;
				}

				/* Check if file creation failed */
				if (r == -1) {
					printwarn(&presel);
					close(fd);
					goto nochange;
				}
			}

			close(fd);
			xstrlcpy(lastname, tmp, NAME_MAX + 1);
			goto begin;
		}
		case SEL_EXEC: // fallthrough
		case SEL_SHELL: // fallthrough
		case SEL_PLUGKEY: // fallthrough
		case SEL_PLUGIN: // fallthrough
		case SEL_LAUNCH: // fallthrough
		case SEL_RUNCMD:
			switch (sel) {
			case SEL_EXEC:
				if (!execute_file(cur, path, newpath, &presel))
					goto nochange;
				break;
			case SEL_SHELL:
				spawn(shell, NULL, NULL, path, F_CLI);
				break;
			case SEL_PLUGKEY: // fallthrough
			case SEL_PLUGIN:
				if (!plugindir) {
					printwait("plugins dir missing", &presel);
					goto nochange;
				}

				if (stat(plugindir, &sb) == -1) {
					printwarn(&presel);
					goto nochange;
				}

				/* Must be a directory */
				if (!S_ISDIR(sb.st_mode))
					break;

				if (sel == SEL_PLUGKEY)
				{
					r = get_input("");
					tmp = get_kv_val(plug, NULL, r, PLUGIN_MAX, FALSE);
					if (!tmp)
						goto nochange;

					mkpath(plugindir, tmp, newpath);
					if (ndents)
						spawn(newpath, dents[cur].name, NULL, path, F_NORMAL);
					else
						spawn(newpath, NULL, NULL, path, F_NORMAL);
					break;
				}

				cfg.runplugin ^= 1;
				if (!cfg.runplugin && rundir[0]) {
					/*
					 * If toggled, and still in the plugin dir,
					 * switch to original directory
					 */
					if (strcmp(path, plugindir) == 0) {
						xstrlcpy(path, rundir, PATH_MAX);
						xstrlcpy(lastname, runfile, NAME_MAX);
						rundir[0] = runfile[0] = '\0';
						setdirwatch();
						goto begin;
					}
					break;
				}

				/* Check if directory is accessible */
				if (!xdiraccess(plugindir))
					goto nochange;

				xstrlcpy(rundir, path, PATH_MAX);
				xstrlcpy(path, plugindir, PATH_MAX);
				if (ndents)
					xstrlcpy(runfile, dents[cur].name, NAME_MAX);
				cfg.runctx = cfg.curctx;
				lastname[0] = '\0';
				setdirwatch();
				goto begin;
			case SEL_LAUNCH:
				if (getutil(utils[NLAUNCH])) {
					spawn(utils[NLAUNCH], "0", NULL, path, F_NORMAL);
					break;
				} // fallthrough
			default: /* SEL_RUNCMD */
#ifndef NORL
				if (cfg.picker) {
#endif
					tmp = xreadline(NULL, "> ");
#ifndef NORL
				} else {
					presel = 0;
					tmp = getreadline("> ", path, ipath, &presel);
					if (presel == MSGWAIT)
						goto nochange;
				}
#endif
				if (tmp && tmp[0]) // NOLINT
					spawn(shell, "-c", tmp, path, F_CLI | F_CMD);
			}

			/* Continue in navigate-as-you-type mode, if enabled */
			if (cfg.filtermode)
				presel = FILTER;

			/* Save current */
			if (ndents)
				copycurname();

			/* Repopulate as directory content may have changed */
			goto begin;
		case SEL_SSHFS:
			if (!sshfs_mount(path, newpath, &presel))
				goto nochange;

			lastname[0] = '\0';

			/* Save last working directory */
			xstrlcpy(lastdir, path, PATH_MAX);

			/* Switch to mount point */
			xstrlcpy(path, newpath, PATH_MAX);

			setdirwatch();
			goto begin;
		case SEL_UMOUNT:
			sshfs_unmount(path, newpath, &presel);
			goto nochange;
		case SEL_QUITCD: // fallthrough
		case SEL_QUIT:
			for (r = 0; r < CTX_MAX; ++r)
				if (r != cfg.curctx && g_ctx[r].c_cfg.ctxactive) {
					r = get_input("Quit all contexts? [Enter]");
					break;
				}

			if (!(r == CTX_MAX || r == '\r'))
				break;

			if (sel == SEL_QUITCD) {
				/* In vim picker mode, clear selection and exit */
				if (cfg.picker) {
					/* Picker mode: reset buffer or clear file */
					if (copybufpos)
						cfg.pickraw ? copybufpos = 0 : writecp(NULL, 0);
				} else if (!write_lastdir(path)) {
					presel = MSGWAIT;
					goto nochange;
				}
			}
			return;
		case SEL_QUITCTX:
			fd = cfg.curctx; /* fd used as tmp var */
			for (r = (fd + 1) & ~CTX_MAX;
			     (r != fd) && !g_ctx[r].c_cfg.ctxactive;
			     r = ((r + 1) & ~CTX_MAX)) {
			};

			if (r != fd) {
				bool copymode = cfg.copymode ? TRUE : FALSE;

				g_ctx[fd].c_cfg.ctxactive = 0;

				/* Switch to next active context */
				path = g_ctx[r].c_path;
				lastdir = g_ctx[r].c_last;
				lastname = g_ctx[r].c_name;

				/* Switch light/detail mode */
				if (cfg.showdetail != g_ctx[r].c_cfg.showdetail)
					/* Set the reverse */
					printptr = cfg.showdetail ? &printent : &printent_long;

				cfg = g_ctx[r].c_cfg;

				/* Continue copy mode */
				cfg.copymode = copymode;
				cfg.curctx = r;
				setdirwatch();
				goto begin;
			}
			return;
		default:
			if (xlines != LINES || xcols != COLS) {
				idle = 0;
				setdirwatch();
				if (ndents)
					copycurname();
				goto begin;
			}

			/* Locker */
			if (idletimeout && idle == idletimeout) {
				idle = 0;
				lock_terminal();
				if (ndents)
					copycurname();
				goto begin;
			}

			goto nochange;
		} /* switch (sel) */
	}
}

static void usage(void)
{
	fprintf(stdout,
		"%s: nnn [-b key] [-d] [-e] [-H] [-i] [-n] [-o]\n"
		"           [-p file] [-s] [-S] [-t] [-v] [-w] [-h] [PATH]\n\n"
		"The missing terminal file manager for X.\n\n"
		"positional args:\n"
		"  PATH   start dir [default: current dir]\n\n"
		"optional args:\n"
		" -b key  open bookmark key\n"
		" -d      detail mode\n"
		" -e      use exiftool for media info\n"
		" -H      show hidden files\n"
		" -i      nav-as-you-type mode\n"
		" -n      version sort\n"
		" -o      press Enter to open files\n"
		" -p file selection file (stdout if '-')\n"
		" -s      string filters [default: regex]\n"
		" -S      du mode\n"
		" -t      disable dir auto-select\n"
		" -v      show version\n"
		" -w      wild load\n"
		" -h      show help\n\n"
		"v%s\n%s\n", __func__, VERSION, GENERAL_INFO);
}

static bool setup_config(void)
{
	size_t r, len;
	char *xdgcfg = getenv("XDG_CONFIG_HOME");
	bool xdg = FALSE;

	/* Set up configuration file paths */
	if (xdgcfg && xdgcfg[0]) {
		DPRINTF_S(xdgcfg);
		if (xdgcfg[0] == '~') {
			r = xstrlcpy(g_buf, home, PATH_MAX);
			xstrlcpy(g_buf + r - 1, xdgcfg + 1, PATH_MAX);
			xdgcfg = g_buf;
			DPRINTF_S(xdgcfg);
		}

		if (!xdiraccess(xdgcfg)) {
			xerror();
			return FALSE;
		}

		len = strlen(xdgcfg) + 1 + 12; /* add length of "/nnn/plugins" */
		xdg = TRUE;
	}

	if (!xdg)
		len = strlen(home) + 1 + 20; /* add length of "/.config/nnn/plugins" */

	cfgdir = (char *)malloc(len);
	plugindir = (char *)malloc(len);
	if (!cfgdir || !plugindir) {
		xerror();
		return FALSE;
	}

	if (xdg) {
		xstrlcpy(cfgdir, xdgcfg, len);
		r = len - 12;
	} else {
		r = xstrlcpy(cfgdir, home, len);

		/* Create ~/.config */
		xstrlcpy(cfgdir + r - 1, "/.config", len - r);
		DPRINTF_S(cfgdir);
		if (!create_dir(cfgdir)) {
			xerror();
			return FALSE;
		}

		r += 8; /* length of "/.config" */
	}

	/* Create ~/.config/nnn */
	xstrlcpy(cfgdir + r - 1, "/nnn", len - r);
	DPRINTF_S(cfgdir);
	if (!create_dir(cfgdir)) {
		xerror();
		return FALSE;
	}

	/* Create ~/.config/nnn/plugins */
	xstrlcpy(cfgdir + r + 4 - 1, "/plugins", 9);
	DPRINTF_S(cfgdir);

	xstrlcpy(plugindir, cfgdir, len);
	DPRINTF_S(plugindir);

	if (!create_dir(cfgdir)) {
		xerror();
		return FALSE;
	}

	/* Reset to config path */
	cfgdir[r + 3] = '\0';
	DPRINTF_S(cfgdir);

	/* Set selection file path */
	if (!cfg.picker) {
		/* Length of "/.config/nnn/.selection" */
		g_cppath = (char *)malloc(len + 3);
		r = xstrlcpy(g_cppath, cfgdir, len + 3);
		xstrlcpy(g_cppath + r - 1, "/.selection", 12);
		DPRINTF_S(g_cppath);
	}

	return TRUE;
}

static bool set_tmp_path()
{
	char *path;

	if (xdiraccess("/tmp"))
		g_tmpfplen = xstrlcpy(g_tmpfpath, "/tmp", TMP_LEN_MAX);
	else {
		path = getenv("TMPDIR");
		if (path)
			g_tmpfplen = xstrlcpy(g_tmpfpath, path, TMP_LEN_MAX);
		else {
			fprintf(stderr, "set TMPDIR\n");
			return FALSE;
		}
	}

	return TRUE;
}

static void cleanup(void)
{
	free(g_cppath);
	free(plugindir);
	free(cfgdir);
	free(initpath);
	free(bmstr);
	free(pluginstr);

#ifdef DBGMODE
	disabledbg();
#endif
}

int main(int argc, char *argv[])
{
	char *arg = NULL;
	int opt;

	while ((opt = getopt(argc, argv, "HSib:denop:stvwh")) != -1) {
		switch (opt) {
		case 'S':
			cfg.blkorder = 1;
			nftw_fn = sum_bsizes;
			BLK_SHIFT = ffs(S_BLKSIZE) - 1;
			break;
		case 'd':
			cfg.showdetail = 1;
			printptr = &printent_long;
			break;
		case 'i':
			cfg.filtermode = 1;
			break;
		case 'b':
			arg = optarg;
			break;
		case 'e':
			cfg.metaviewer = EXIFTOOL;
			break;
		case 'H':
			cfg.showhidden = 1;
			break;
		case 'n':
			cmpfn = &xstrverscasecmp;
			break;
		case 'o':
			cfg.nonavopen = 1;
			break;
		case 'p':
			cfg.picker = 1;
			if (optarg[0] == '-' && optarg[1] == '\0')
				cfg.pickraw = 1;
			else {
				int fd = open(optarg, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);

				if (fd == -1) {
					xerror();
					return _FAILURE;
				}

				close(fd);
				g_cppath = realpath(optarg, NULL);
				unlink(g_cppath);
			}
			break;
		case 's':
			cfg.filter_re = 0;
			filterfn = &visible_str;
			break;
		case 't':
			cfg.autoselect = 0;
			break;
		case 'v':
			fprintf(stdout, "%s\n", VERSION);
			return _SUCCESS;
		case 'w':
			cfg.wild = 1;
			break;
		case 'h':
			usage();
			return _SUCCESS;
		default:
			usage();
			return _FAILURE;
		}
	}

	/* Confirm we are in a terminal */
	if (!cfg.picker && !(isatty(STDIN_FILENO) && isatty(STDOUT_FILENO)))
		exit(1);

	/* Get the context colors; copier used as tmp var */
	copier = xgetenv(env_cfg[NNN_CONTEXT_COLORS], "4444");
	opt = 0;
	while (opt < CTX_MAX) {
		if (*copier) {
			if (*copier < '0' || *copier > '7') {
				fprintf(stderr, "0 <= code <= 7\n");
				return _FAILURE;
			}

			g_ctx[opt].color = *copier - '0';
			++copier;
		} else
			g_ctx[opt].color = 4;

		++opt;
	}

#ifdef DBGMODE
	enabledbg();
#endif

	atexit(cleanup);

	home = getenv("HOME");
	if (!home) {
		fprintf(stderr, "set HOME\n");
		return _FAILURE;
	}
	DPRINTF_S(home);

	if (!setup_config())
		return _FAILURE;

	/* Get custom opener, if set */
	opener = xgetenv(env_cfg[NNN_OPENER], utils[OPENER]);
	DPRINTF_S(opener);

	/* Parse bookmarks string */
	if (!parsekvpair(bookmark, &bmstr, env_cfg[NNN_BMS], BM_MAX)) {
		fprintf(stderr, "%s\n", env_cfg[NNN_BMS]);
		return _FAILURE;
	}

	/* Parse plugins string */
	if (!parsekvpair(plug, &pluginstr, "NNN_PLUG", PLUGIN_MAX)) {
		fprintf(stderr, "%s\n", "NNN_PLUG");
		return _FAILURE;
	}

	if (arg) { /* Open a bookmark directly */
		if (arg[1] || (initpath = get_kv_val(bookmark, NULL, *arg, BM_MAX, TRUE)) == NULL) {
			fprintf(stderr, "%s\n", messages[STR_INVBM_KEY]);
			return _FAILURE;
		}
	} else if (argc == optind) {
		/* Start in the current directory */
		initpath = getcwd(NULL, PATH_MAX);
		if (!initpath)
			initpath = "/";
	} else {
		arg = argv[optind];
		if (strlen(arg) > 7 && arg[0] == 'f' && arg[1] == 'i' && arg[2] == 'l'
		    && arg[3] == 'e' && arg[4] == ':' && arg[5] == '/' && arg[6] == '/')
			arg = arg + 7;
		initpath = realpath(arg, NULL);
		DPRINTF_S(initpath);
		if (!initpath) {
			xerror();
			return _FAILURE;
		}

		/*
		 * If nnn is set as the file manager, applications may try to open
		 * files by invoking nnn. In that case pass the file path to the
		 * desktop opener and exit.
		 */
		struct stat sb;

		if (stat(initpath, &sb) == -1) {
			xerror();
			return _FAILURE;
		}

		if (S_ISREG(sb.st_mode)) {
			execlp(opener, opener, arg, NULL);
			return _SUCCESS;
		}
	}

	/* Edit text in EDITOR, if opted */
	if (xgetenv_set(env_cfg[NNN_USE_EDITOR]))
		cfg.useeditor = 1;

	/* Get VISUAL/EDITOR */
	editor = xgetenv(envs[VISUAL], xgetenv(envs[EDITOR], "vi"));
	DPRINTF_S(getenv(envs[VISUAL]));
	DPRINTF_S(getenv(envs[EDITOR]));
	DPRINTF_S(editor);

	/* Get PAGER */
	pager = xgetenv(envs[PAGER], "less");
	DPRINTF_S(pager);

	/* Get SHELL */
	shell = xgetenv(envs[SHELL], "sh");
	DPRINTF_S(shell);

	DPRINTF_S(getenv("PWD"));

#ifdef LINUX_INOTIFY
	/* Initialize inotify */
	inotify_fd = inotify_init1(IN_NONBLOCK);
	if (inotify_fd < 0) {
		xerror();
		return _FAILURE;
	}
#elif defined(BSD_KQUEUE)
	kq = kqueue();
	if (kq < 0) {
		xerror();
		return _FAILURE;
	}
#endif

	/* Set nnn nesting level, idletimeout used as tmp var */
	idletimeout = xatoi(getenv(env_cfg[NNNLVL]));
	setenv(env_cfg[NNNLVL], xitoa(++idletimeout), 1);

	/* Get locker wait time, if set */
	idletimeout = xatoi(getenv(env_cfg[NNN_IDLE_TIMEOUT]));
	DPRINTF_U(idletimeout);

	if (xgetenv_set(env_cfg[NNN_TRASH]))
		cfg.trash = 1;

	/* Prefix for temporary files */
	if (!set_tmp_path())
		return _FAILURE;

	/* Get the clipboard copier, if set */
	copier = getenv(env_cfg[NNN_COPIER]);

#ifdef __linux__
	if (!xgetenv_set(env_cfg[NNN_OPS_PROG])) {
		cp[5] = cp[4];
		cp[2] = cp[4] = ' ';

		mv[5] = mv[4];
		mv[2] = mv[4] = ' ';
	}
#endif

	/* Ignore/handle certain signals */
	struct sigaction act = {.sa_handler = sigint_handler};

	if (sigaction(SIGINT, &act, NULL) < 0) {
		xerror();
		return _FAILURE;
	}
	signal(SIGQUIT, SIG_IGN);

	/* Test initial path */
	if (!xdiraccess(initpath)) {
		xerror();
		return _FAILURE;
	}

	/* Set locale */
	setlocale(LC_ALL, "");

#ifndef NORL
#if RL_READLINE_VERSION >= 0x0603
	/* readline would overwrite the WINCH signal hook */
	rl_change_environment = 0;
#endif
	/* Bind TAB to cycling */
	rl_variable_bind("completion-ignore-case", "on");
#ifdef __linux__
	rl_bind_key('\t', rl_menu_complete);
#else
	rl_bind_key('\t', rl_complete);
#endif
	read_history(NULL);
#endif

	if (!initcurses())
		return _FAILURE;

	browse(initpath);
	exitcurses();

#ifndef NORL
	write_history(NULL);
#endif

	if (cfg.pickraw) {
		if (copybufpos) {
			opt = selectiontofd(1, NULL);
			if (opt != (int)(copybufpos))
				xerror();
		}
	} else if (!cfg.picker && g_cppath)
		unlink(g_cppath);

	/* Free the copy buffer */
	free(pcopybuf);

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

	return _SUCCESS;
}
