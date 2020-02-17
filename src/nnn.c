/*
 * BSD 2-Clause License
 *
 * Copyright (C) 2014-2016, Lazaros Koromilas <lostd@2f30.org>
 * Copyright (C) 2014-2016, Dimitris Papastamos <sin@2f30.org>
 * Copyright (C) 2016-2020, Arun Prakash Jana <engineerarun@gmail.com>
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
#elif defined(__HAIKU__)
#include "../misc/haiku/haiku_interop.h"
#define HAIKU_NM
#else
#include <sys/sysmacros.h>
#endif
#include <sys/wait.h>

#ifdef __linux__ /* Fix failure due to mvaddnwstr() */
#ifndef NCURSES_WIDECHAR
#define NCURSES_WIDECHAR 1
#endif
#elif defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__) || defined(__APPLE__) || defined(__sun)
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
#ifndef NOLOCALE
#include <locale.h>
#endif
#include <stdio.h>
#ifndef NORL
#include <readline/history.h>
#include <readline/readline.h>
#endif
#ifdef PCRE
#include <pcre.h>
#else
#include <regex.h>
#endif
#include <signal.h>
#include <stdarg.h>
#include <stdlib.h>
#ifdef __sun
#include <alloca.h>
#endif
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
#define VERSION "3.0"
#define GENERAL_INFO "BSD 2-Clause\nhttps://github.com/jarun/nnn"
#define SESSIONS_VERSION 1

#ifndef S_BLKSIZE
#define S_BLKSIZE 512 /* S_BLKSIZE is missing on Android NDK (Termux) */
#endif

/*
 * NAME_MAX and PATH_MAX may not exist, e.g. with dirent.c_name being a
 * flexible array on Illumos. Use somewhat accomodating fallback values.
 */
#ifndef NAME_MAX
#define NAME_MAX 255
#endif

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

#define _ABSSUB(N, M) (((N) <= (M)) ? ((M) - (N)) : ((N) - (M)))
#define DOUBLECLICK_INTERVAL_NS (400000000)
#define XDELAY_INTERVAL_MS (350000) /* 350 ms delay */
#define ELEMENTS(x) (sizeof(x) / sizeof(*(x)))
#undef MIN
#define MIN(x, y) ((x) < (y) ? (x) : (y))
#undef MAX
#define MAX(x, y) ((x) > (y) ? (x) : (y))
#define ISODD(x) ((x) & 1)
#define ISBLANK(x) ((x) == ' ' || (x) == '\t')
#define TOUPPER(ch) (((ch) >= 'a' && (ch) <= 'z') ? ((ch) - 'a' + 'A') : (ch))
#define CMD_LEN_MAX (PATH_MAX + ((NAME_MAX + 1) << 1))
#define READLINE_MAX 256
#define FILTER '/'
#define RFILTER '\\'
#define CASE ':'
#define MSGWAIT '$'
#define REGEX_MAX 48
#define BM_MAX 10
#define PLUGIN_MAX 15
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
#define LIST_FILES_MAX (1 << 16)
#define SCROLLOFF 3
#define MIN_DISPLAY_COLS 10
#define LONG_SIZE sizeof(ulong)
#define ARCHIVE_CMD_LEN 16
#define BLK_SHIFT_512 9

/* Program return codes */
#define _SUCCESS 0
#define _FAILURE !_SUCCESS

/* Entry flags */
#define DIR_OR_LINK_TO_DIR 0x1
#define FILE_SELECTED 0x10

/* Macros to define process spawn behaviour as flags */
#define F_NONE    0x00  /* no flag set */
#define F_MULTI   0x01  /* first arg can be combination of args; to be used with F_NORMAL */
#define F_NOWAIT  0x02  /* don't wait for child process (e.g. file manager) */
#define F_NOTRACE 0x04  /* suppress stdout and strerr (no traces) */
#define F_NORMAL  0x08  /* spawn child process in non-curses regular CLI mode */
#define F_CONFIRM 0x10  /* run command - show results before exit (must have F_NORMAL) */
#define F_CLI     (F_NORMAL | F_MULTI)
#define F_SILENT  (F_CLI | F_NOTRACE)

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
typedef long long ll;

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
} *pEntry;

/* Key-value pairs from env */
typedef struct {
	int key;
	char *val;
} kv;

typedef struct {
#ifdef PCRE
	const pcre *pcrex;
#else
	const regex_t *regex;
#endif
	const char *str;
} fltrexp_t;

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
	uint selmode    : 1;  /* Set when selecting files */
	uint showdetail : 1;  /* Clear to show fewer file info */
	uint ctxactive  : 1;  /* Context active or not */
	uint reserved   : 2;
	/* The following settings are global */
	uint forcequit  : 1;  /* Do not confirm when quitting program */
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
	uint regex      : 1;  /* Use regex filters */
	uint x11        : 1;  /* Copy to system clipboard and show notis */
	uint trash      : 1;  /* Move removed files to trash */
	uint mtime      : 1;  /* Use modification time (else access time) */
	uint cliopener  : 1;  /* All-CLI app opener */
	uint waitedit   : 1;  /* For ops that can't be detached, used EDITOR */
	uint rollover   : 1;  /* Roll over at edges */
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

typedef struct {
	size_t ver;
	size_t pathln[CTX_MAX];
	size_t lastln[CTX_MAX];
	size_t nameln[CTX_MAX];
	size_t fltrln[CTX_MAX];
} session_header_t;

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
	0, /* selmode */
	0, /* showdetail */
	1, /* ctxactive */
	0, /* reserved */
	0, /* forcequit */
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
	0, /* regex */
	0, /* x11 */
	0, /* trash */
	1, /* mtime */
	0, /* cliopener */
	0, /* waitedit */
	1, /* rollover */
};

static context g_ctx[CTX_MAX] __attribute__ ((aligned));

static int ndents, cur, last, curscroll, last_curscroll, total_dents = ENTRY_INCR;
static int xlines, xcols;
static int nselected;
static uint idle;
static uint idletimeout, selbufpos, lastappendpos, selbuflen;
static char *bmstr;
static char *pluginstr;
static char *opener;
static char *editor;
static char *enveditor;
static char *pager;
static char *shell;
static char *home;
static char *initpath;
static char *cfgdir;
static char *g_selpath;
static char *g_listpath;
static char *g_prefixpath;
static char *plugindir;
static char *sessiondir;
static char *pnamebuf, *pselbuf;
static struct entry *dents;
static blkcnt_t ent_blocks;
static blkcnt_t dir_blocks;
static ulong num_files;
static kv bookmark[BM_MAX];
static kv plug[PLUGIN_MAX];
static uchar g_tmpfplen;
static uchar blk_shift = BLK_SHIFT_512;
static const uint _WSHIFT = (LONG_SIZE == 8) ? 3 : 2;
#ifdef PCRE
static pcre *archive_pcre;
#else
static regex_t archive_re;
#endif

/* Retain old signal handlers */
#ifdef __linux__
static sighandler_t oldsighup; /* old value of hangup signal */
static sighandler_t oldsigtstp; /* old value of SIGTSTP */
#else
/* note: no sig_t on Solaris-derivs */
static void (*oldsighup)(int);
static void (*oldsigtstp)(int);
#endif

/* For use in functions which are isolated and don't return the buffer */
static char g_buf[CMD_LEN_MAX] __attribute__ ((aligned));

/* Buffer to store tmp file path to show selection, file stats and help */
static char g_tmpfpath[TMP_LEN_MAX] __attribute__ ((aligned));

/* Buffer to store plugins control pipe location */
static char g_pipepath[TMP_LEN_MAX] __attribute__ ((aligned));

/* MISC NON-PERSISTENT INTERNAL BINARY STATES */

/* Plugin control initialization status */
#define STATE_PLUGIN_INIT 0x1
#define STATE_INTERRUPTED 0x2
#define STATE_RANGESEL 0x4
#define STATE_MOVE_OP 0x8
#define STATE_AUTONEXT 0x10
#define STATE_MSG 0x20

static uchar g_states;

/* Options to identify file mime */
#if defined(__APPLE__)
#define FILE_MIME_OPTS "-bIL"
#elif !defined(__sun) /* no mime option for 'file' */
#define FILE_MIME_OPTS "-biL"
#endif

/* Macros for utilities */
#define UTIL_OPENER 0
#define UTIL_ATOOL 1
#define UTIL_BSDTAR 2
#define UTIL_UNZIP 3
#define UTIL_TAR 4
#define UTIL_LOCKER 5
#define UTIL_CMATRIX 6
#define UTIL_LAUNCH 7
#define UTIL_SH_EXEC 8
#define UTIL_ARCHIVEMOUNT 9
#define UTIL_SSHFS 10
#define UTIL_RCLONE 11
#define UTIL_VI 12
#define UTIL_LESS 13
#define UTIL_SH 14
#define UTIL_FZF 15
#define UTIL_FZY 16
#define UTIL_NTFY 17
#define UTIL_CBCP 18

/* Utilities to open files, run actions */
static char * const utils[] = {
#ifdef __APPLE__
	"/usr/bin/open",
#elif defined __CYGWIN__
	"cygstart",
#elif defined __HAIKU__
	"open",
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
#elif defined __HAIKU__
	"peaclock",
#else
	"vlock",
#endif
	"cmatrix",
	"launch",
	"sh -c",
	"archivemount",
	"sshfs",
	"rclone",
	"vi",
	"less",
	"sh",
	"fzf",
	"fzy",
	".ntfy",
	".cbcp",
};

/* Common strings */
#define MSG_NO_TRAVERSAL 0
#define MSG_INVALID_KEY 1
#define STR_TMPFILE 2
#define MSG_0_SELECTED 3
#define MSG_UTIL_MISSING 4
#define MSG_FAILED 5
#define MSG_SSN_NAME 6
#define MSG_CP_MV_AS 7
#define MSG_CUR_SEL_OPTS 8
#define MSG_FORCE_RM 9
#define MSG_CREATE_CTX 10
#define MSG_NEW_OPTS 11
#define MSG_CLI_MODE 12
#define MSG_OVERWRITE 13
#define MSG_SSN_OPTS 14
#define MSG_QUIT_ALL 15
#define MSG_HOSTNAME 16
#define MSG_ARCHIVE_NAME 17
#define MSG_OPEN_WITH 18
#define MSG_REL_PATH 19
#define MSG_LINK_PREFIX 20
#define MSG_COPY_NAME 21
#define MSG_CONTINUE 22
#define MSG_SEL_MISSING 23
#define MSG_ACCESS 24
#define MSG_EMPTY_FILE 25
#define MSG_UNSUPPORTED 26
#define MSG_NOT_SET 27
#define MSG_EXISTS 28
#define MSG_FEW_COLUMNS 29
#define MSG_REMOTE_OPTS 30
#define MSG_RCLONE_DELAY 31
#define MSG_APP_NAME 32
#define MSG_ARCHIVE_OPTS 33
#define MSG_PLUGIN_KEYS 34
#define MSG_BOOKMARK_KEYS 35
#define MSG_INVALID_REG 36
#define MSG_ORDER 37
#define MSG_LAZY 38
#define MSG_IGNORED 39
#ifndef DIR_LIMITED_SELECTION
#define MSG_DIR_CHANGED 40 /* Must be the last entry */
#endif

static const char * const messages[] = {
	"no traversal",
	"invalid key",
	"/.nnnXXXXXX",
	"0 selected",
	"missing util",
	"failed!",
	"session name: ",
	"'c'p / 'm'v as?",
	"'c'urrent / 's'el?",
	"forcibly remove %s file%s (unrecoverable)?",
	"create context %d?",
	"'f'ile / 'd'ir / 's'ym / 'h'ard?",
	"'c'li / 'g'ui?",
	"overwrite?",
	"'s'ave / 'l'oad / 'r'estore?",
	"Quit all contexts?",
	"remote name: ",
	"archive name: ",
	"open with: ",
	"relative path: ",
	"link prefix [@ for none]: ",
	"copy name: ",
	"\nPress Enter to continue",
	"open failed",
	"dir inaccessible",
	"empty: edit or open with",
	"unsupported file",
	"not set",
	"entry exists",
	"too few columns!",
	"'s'shfs / 'r'clone?",
	"may take a while, try refresh",
	"app name: ",
	"'d'efault / e'x'tract / 'l'ist / 'm'ount?",
	"plugin keys:",
	"bookmark keys:",
	"invalid regex",
	"toggle 'a'u / 'd'u / 'e'xtn / 'r'everse / 's'ize / 't'ime / 'v'ersion?",
	"unmount failed! try lazy?",
	"ignoring invalid paths...",
#ifndef DIR_LIMITED_SELECTION
	"dir changed, range sel off", /* Must be the last entry */
#endif
};

/* Supported configuration environment variables */
#define NNN_OPTS 0
#define NNN_BMS 1
#define NNN_PLUG 2
#define NNN_OPENER 3
#define NNN_COLORS 4
#define NNNLVL 5
#define NNN_PIPE 6
#define NNN_ARCHIVE 7 /* strings end here */
#define NNN_TRASH 8 /* flags begin here */

static const char * const env_cfg[] = {
	"NNN_OPTS",
	"NNN_BMS",
	"NNN_PLUG",
	"NNN_OPENER",
	"NNN_COLORS",
	"NNNLVL",
	"NNN_PIPE",
	"NNN_ARCHIVE",
	"NNN_TRASH",
};

/* Required environment variables */
#define ENV_SHELL 0
#define ENV_VISUAL 1
#define ENV_EDITOR 2
#define ENV_PAGER 3
#define ENV_NCUR 4

static const char * const envs[] = {
	"SHELL",
	"VISUAL",
	"EDITOR",
	"PAGER",
	"nnn",
};

#ifdef __linux__
static char cp[] = "cp   -iRp";
static char mv[] = "mv   -i";
#else
static char cp[] = "cp -iRp";
static char mv[] = "mv -i";
#endif

static const char cpmvformatcmd[] = "sed -i 's|^\\(\\(.*/\\)\\(.*\\)$\\)|#\\1\\n\\3|' %s";
static const char cpmvrenamecmd[] = "sed 's|^\\([^#][^/]\\?.*\\)$|%s/\\1|;s|^#\\(/.*\\)$|\\1|' "
				    "%s | tr '\\n' '\\0' | xargs -0 -n2 sh -c '%s \"$0\" \"$@\" "
				    "< /dev/tty'";
static const char batchrenamecmd[] = "paste -d'\n' %s %s | sed 'N; /^\\(.*\\)\\n\\1$/!p;d' | "
				     "tr '\n' '\\0' | xargs -0 -n2 mv 2>/dev/null";
static const char archive_regex[] = "\\.(bz|bz2|gz|tar|taz|tbz|tbz2|tgz|z|zip)$";
static const char replaceprefixcmd[] = "sed -i 's|^%s\\(.*\\)$|%s\\1|' %s";

/* Event handling */
#ifdef LINUX_INOTIFY
#define NUM_EVENT_SLOTS 8 /* Make room for 8 events */
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
#elif defined(HAIKU_NM)
static bool haiku_nm_active = FALSE;
static haiku_nm_h haiku_hnd;
#endif

/* Function macros */
#define tolastln() move(xlines - 1, 0)
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

#ifdef __GNUC__
#define UNUSED(x) UNUSED_##x __attribute__((__unused__))
#else
#define UNUSED(x) UNUSED_##x
#endif /* __GNUC__ */

/* Forward declarations */
static void redraw(char *path);
static int spawn(char *file, char *arg1, char *arg2, const char *dir, uchar flag);
static int (*nftw_fn)(const char *fpath, const struct stat *sb, int typeflag, struct FTW *ftwbuf);
static int dentfind(const char *fname, int n);
static void move_cursor(int target, int ignore_scrolloff);
static inline bool getutil(char *util);
static size_t mkpath(const char *dir, const char *name, char *out);
static char *xgetenv(const char *name, char *fallback);
static void plugscript(const char *plugin, char *newpath, uchar flags);

/* Functions */

static void sigint_handler(int UNUSED(sig))
{
	g_states |= STATE_INTERRUPTED;
}

static char *xitoa(uint val)
{
	static char ascbuf[32] = {0};
	int i = 30;
	uint rem;

	if (!val)
		return "0";

	while (val && i) {
		rem = val / 10;
		ascbuf[i] = '0' + (val - (rem * 10));
		val = rem;
		--i;
	}

	return &ascbuf[++i];
}

static void clearinfoln(void)
{
	move(xlines - 2, 0);
	addch('\n');
}

#ifdef KEY_RESIZE
/* Clear the old prompt */
static void clearoldprompt(void)
{
	clearinfoln();
	tolastln();
	addch('\n');
}
#endif

/* Messages show up at the bottom */
static void printmsg(const char *msg)
{
	tolastln();
	addstr(msg);
	addch('\n');
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
	if (!cfg.picker && g_selpath)
		unlink(g_selpath);
	free(pselbuf);
	exit(1);
}

/* Print prompt on the last line */
static void printprompt(const char *str)
{
	clearprompt();
	addstr(str);
}

static void printinfoln(const char *str)
{
	clearinfoln();
	mvaddstr(xlines - 2, xcols - strlen(str), str);
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
		if (r == KEY_RESIZE && prompt) {
			clearoldprompt();
			xlines = LINES;
			printprompt(prompt);
		}
	} while (r == KEY_RESIZE);
#else
	r = getch();
#endif
	settimeout();
	return r;
}

static int get_cur_or_sel(void)
{
	if (selbufpos && ndents) {
		int choice = get_input(messages[MSG_CUR_SEL_OPTS]);

		return ((choice == 'c' || choice == 's') ? choice : 0);
	}

	if (selbufpos)
		return 's';

	if (ndents)
		return 'c';

	return 0;
}

static void xdelay(useconds_t delay)
{
	refresh();
	usleep(delay);
}

static char confirm_force(bool selection)
{
	char str[64];
	int r;

	snprintf(str, 64, messages[MSG_FORCE_RM],
		 (selection ? xitoa(nselected) : "current"), (selection ? "(s)" : ""));
	r = get_input(str);

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

	if (n > len)
		n = len;
	else if (len > n)
		/* Save total number of bytes to copy in len */
		len = n;

	/*
	 * To enable -O3 ensure src and dest are 16-byte aligned
	 * More info: https://www.felixcloutier.com/x86/MOVDQA:VMOVDQA32:VMOVDQA64
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

	do
		if (*--ptr == ch)
			return ptr;
	while (s != ptr);

	return NULL;
}

/* Assumes both the paths passed are directories */
static char *common_prefix(const char *path, char *prefix)
{
	const char *x = path, *y = prefix;
	char *sep;

	if (!path || !*path || !prefix)
		return NULL;

	if (!*prefix) {
		xstrlcpy(prefix, path, PATH_MAX);
		return prefix;
	}

	while (*x && *y && (*x == *y))
		++x, ++y;

	/* Strings are same */
	if (!*x && !*y)
		return prefix;

	/* Path is shorter */
	if (!*x && *y == '/') {
		xstrlcpy(prefix, path, y - path);
		return prefix;
	}

	/* Prefix is shorter */
	if (!*y && *x == '/')
		return prefix;

	/* Shorten prefix */
	prefix[y - prefix] = '\0';

	sep = xmemrchr((uchar *)prefix, '/', y - prefix);
	if (sep != prefix)
		*sep = '\0';
	else /* Just '/' */
		prefix[1] = '\0';

	return prefix;
}

/*
 * The library function realpath() resolves symlinks.
 * If there's a symlink in file list we want to show the symlink not what it's points to.
 */
static char *abspath(const char *path, const char *cwd)
{
	if (!path || !cwd)
		return NULL;

	size_t dst_size = 0, src_size = strlen(path), cwd_size = strlen(cwd);
	const char *src, *next;
	char *dst;
	char *resolved_path = malloc(src_size + (*path == '/' ? 0 : cwd_size) + 1);
	if (!resolved_path)
		return NULL;

	/* Turn relative paths into absolute */
	if (path[0] != '/')
		dst_size = xstrlcpy(resolved_path, cwd, cwd_size + 1) - 1;
	else
		resolved_path[0] = '\0';

	src = path;
	dst = resolved_path + dst_size;
	for (next = NULL; next != path + src_size;) {
		next = strchr(src, '/');
		if (!next)
			next = path + src_size;

		if (next - src == 2 && src[0] == '.' && src[1] == '.') {
			if (dst - resolved_path) {
				dst = xmemrchr((uchar *)resolved_path, '/', dst - resolved_path);
				*dst = '\0';
			}
		} else if (next - src == 1 && src[0] == '.') {
			/* NOP */
		} else if (next - src) {
			*(dst++) = '/';
			xstrlcpy(dst, src, next - src + 1);
			dst += next - src;
		}

		src = next + 1;
	}

	if (*resolved_path == '\0') {
		resolved_path[0] = '/';
		resolved_path[1] = '\0';
	}

	return resolved_path;
}

static char *xbasename(char *path)
{
	char *base = xmemrchr((uchar *)path, '/', strlen(path)); // NOLINT

	return base ? base + 1 : path;
}

static int create_tmp_file(void)
{
	xstrlcpy(g_tmpfpath + g_tmpfplen - 1, messages[STR_TMPFILE], TMP_LEN_MAX - g_tmpfplen);

	int fd = mkstemp(g_tmpfpath);

	if (fd == -1) {
		DPRINTF_S(strerror(errno));
	}

	return fd;
}

/* Writes buflen char(s) from buf to a file */
static void writesel(const char *buf, const size_t buflen)
{
	if (cfg.pickraw || !g_selpath)
		return;

	FILE *fp = fopen(g_selpath, "w");

	if (fp) {
		if (fwrite(buf, 1, buflen, fp) != buflen)
			printwarn(NULL);
		fclose(fp);
	} else
		printwarn(NULL);
}

static void appendfpath(const char *path, const size_t len)
{
	if ((selbufpos >= selbuflen) || ((len + 3) > (selbuflen - selbufpos))) {
		selbuflen += PATH_MAX;
		pselbuf = xrealloc(pselbuf, selbuflen);
		if (!pselbuf)
			errexit();
	}

	selbufpos += xstrlcpy(pselbuf + selbufpos, path, len);
}

/* Write selected file paths to fd, linefeed separated */
static size_t seltofile(int fd, uint *pcount)
{
	uint lastpos, count = 0;
	char *pbuf = pselbuf;
	size_t pos = 0, len;
	ssize_t r;

	if (pcount)
		*pcount = 0;

	if (!selbufpos)
		return 0;

	lastpos = selbufpos - 1;

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

/* List selection from selection file (another instance) */
static bool listselfile(void)
{
	struct stat sb;

	if (stat(g_selpath, &sb) == -1)
		return FALSE;

	/* Nothing selected if file size is 0 */
	if (!sb.st_size)
		return FALSE;

	snprintf(g_buf, CMD_LEN_MAX, "tr \'\\0\' \'\\n\' < %s", g_selpath);
	spawn(utils[UTIL_SH_EXEC], g_buf, NULL, NULL, F_CLI | F_CONFIRM);

	return TRUE;
}

/* Reset selection indicators */
static void resetselind(void)
{
	int r = 0;

	for (; r < ndents; ++r)
		if (dents[r].flags & FILE_SELECTED)
			dents[r].flags &= ~FILE_SELECTED;
}

static void startselection(void)
{
	if (!cfg.selmode) {
		cfg.selmode = 1;
		nselected = 0;

		if (selbufpos) {
			resetselind();
			writesel(NULL, 0);
			selbufpos = 0;
		}

		lastappendpos = 0;
	}
}

static void updateselbuf(const char *path, char *newpath)
{
	int i = 0;
	size_t r;

	for (; i < ndents; ++i)
		if (dents[i].flags & FILE_SELECTED) {
			r = mkpath(path, dents[i].name, newpath);
			appendfpath(newpath, r);
		}
}

/* Finish selection procedure before an operation */
static void endselection(void)
{
	int fd;
	ssize_t count;
	char buf[sizeof(replaceprefixcmd) + PATH_MAX + (TMP_LEN_MAX << 1)];

	if (cfg.selmode)
		cfg.selmode = 0;

	if (!g_listpath || !selbufpos)
		return;

	fd = create_tmp_file();
	if (fd == -1) {
		DPRINTF_S("couldn't create tmp file");
		return;
	}

	seltofile(fd, NULL);
	if (close(fd)) {
		DPRINTF_S(strerror(errno));
		printwarn(NULL);
		return;
	}

	snprintf(buf, sizeof(buf), replaceprefixcmd, g_listpath, g_prefixpath, g_tmpfpath);
	spawn(utils[UTIL_SH_EXEC], buf, NULL, NULL, F_CLI);

	fd = open(g_tmpfpath, O_RDONLY);
	if (fd == -1) {
		DPRINTF_S(strerror(errno));
		printwarn(NULL);
		if (unlink(g_tmpfpath)) {
			DPRINTF_S(strerror(errno));
			printwarn(NULL);
		}
		return;
	}

	count = read(fd, pselbuf, selbuflen);
	if (count < 0) {
		DPRINTF_S(strerror(errno));
		printwarn(NULL);
		if (close(fd) || unlink(g_tmpfpath)) {
			DPRINTF_S(strerror(errno));
		}
		return;
	}

	if (close(fd) || unlink(g_tmpfpath)) {
		DPRINTF_S(strerror(errno));
		printwarn(NULL);
		return;
	}

	selbufpos = count;
	pselbuf[--count] = '\0';
	for (--count; count > 0; --count)
		if (pselbuf[count] == '\n' && pselbuf[count+1] == '/')
			pselbuf[count] = '\0';

	writesel(pselbuf, selbufpos - 1);
}

static void clearselection(void)
{
	nselected = 0;
	selbufpos = 0;
	cfg.selmode = 0;
	writesel(NULL, 0);
}

/* Returns: 1 - success, 0 - none selected, -1 - other failure */
static int editselection(void)
{
	int ret = -1;
	int fd, lines = 0;
	ssize_t count;
	struct stat sb;
	time_t mtime;

	if (!selbufpos)
		return listselfile();

	fd = create_tmp_file();
	if (fd == -1) {
		DPRINTF_S("couldn't create tmp file");
		return -1;
	}

	seltofile(fd, NULL);
	if (close(fd)) {
		DPRINTF_S(strerror(errno));
		return -1;
	}

	/* Save the last modification time */
	if (stat(g_tmpfpath, &sb)) {
		DPRINTF_S(strerror(errno));
		unlink(g_tmpfpath);
		return -1;
	}
	mtime = sb.st_mtime;

	spawn((cfg.waitedit ? enveditor : editor), g_tmpfpath, NULL, NULL, F_CLI);

	fd = open(g_tmpfpath, O_RDONLY);
	if (fd == -1) {
		DPRINTF_S(strerror(errno));
		unlink(g_tmpfpath);
		return -1;
	}

	fstat(fd, &sb);

	if (mtime == sb.st_mtime) {
		DPRINTF_S("selection is not modified");
		unlink(g_tmpfpath);
		return 1;
	}

	if (sb.st_size > selbufpos) {
		DPRINTF_S("edited buffer larger than previous");
		unlink(g_tmpfpath);
		goto emptyedit;
	}

	count = read(fd, pselbuf, selbuflen);
	if (count < 0) {
		DPRINTF_S(strerror(errno));
		printwarn(NULL);
		if (close(fd) || unlink(g_tmpfpath)) {
			DPRINTF_S(strerror(errno));
			printwarn(NULL);
		}
		goto emptyedit;
	}

	if (close(fd) || unlink(g_tmpfpath)) {
		DPRINTF_S(strerror(errno));
		printwarn(NULL);
		goto emptyedit;
	}

	if (!count) {
		ret = 1;
		goto emptyedit;
	}

	resetselind();
	selbufpos = count;
	/* The last character should be '\n' */
	pselbuf[--count] = '\0';
	for (--count; count > 0; --count) {
		/* Replace every '\n' that separates two paths */
		if (pselbuf[count] == '\n' && pselbuf[count + 1] == '/') {
			++lines;
			pselbuf[count] = '\0';
		}
	}

	/* Add a line for the last file */
	++lines;

	if (lines > nselected) {
		DPRINTF_S("files added to selection");
		goto emptyedit;
	}

	nselected = lines;
	writesel(pselbuf, selbufpos - 1);

	return 1;

emptyedit:
	resetselind();
	clearselection();
	return ret;
}

static bool selsafe(void)
{
	/* Fail if selection file path not generated */
	if (!g_selpath) {
		printmsg(messages[MSG_SEL_MISSING]);
		return FALSE;
	}

	/* Fail if selection file path isn't accessible */
	if (access(g_selpath, R_OK | W_OK) == -1) {
		errno == ENOENT ? printmsg(messages[MSG_0_SELECTED]) : printwarn(NULL);
		return FALSE;
	}

	return TRUE;
}

/* Initialize curses mode */
static bool initcurses(void *oldmask)
{
	short i;
	char *colors = xgetenv(env_cfg[NNN_COLORS], "4444");

#ifdef NOMOUSE
	(void) oldmask;
#endif

	if (cfg.picker) {
		if (!newterm(NULL, stderr, stdin)) {
			fprintf(stderr, "newterm!\n");
			return FALSE;
		}
	} else if (!initscr()) {
		fprintf(stderr, "initscr!\n");
		DPRINTF_S(getenv("TERM"));
		return FALSE;
	}

	cbreak();
	noecho();
	nonl();
	//intrflush(stdscr, FALSE);
	keypad(stdscr, TRUE);
#ifndef NOMOUSE
#if NCURSES_MOUSE_VERSION <= 1
	mousemask(BUTTON1_PRESSED | BUTTON1_DOUBLE_CLICKED, (mmask_t *)oldmask);
#else
	mousemask(BUTTON1_PRESSED | BUTTON4_PRESSED | BUTTON5_PRESSED, (mmask_t *)oldmask);
#endif
	mouseinterval(0);
#endif
	curs_set(FALSE); /* Hide cursor */
	start_color();
	use_default_colors();

	/* Get and set the context colors */
	for (i = 0; i <  CTX_MAX; ++i) {
		if (*colors) {
			g_ctx[i].color = (*colors < '0' || *colors > '7') ? 4 : *colors - '0';
			++colors;
		} else
			g_ctx[i].color = 4;

		init_pair(i + 1, g_ctx[i].color, -1);

		g_ctx[i].c_fltr[REGEX_MAX - 1] = '\0';
	}

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
	int status;
	pid_t p = fork();

	if (p > 0) {
		/* the parent ignores the interrupt, quit and hangup signals */
		oldsighup = signal(SIGHUP, SIG_IGN);
		oldsigtstp = signal(SIGTSTP, SIG_DFL);
	} else if (p == 0) {
		/* We create a grandchild to detach */
		if (flag & F_NOWAIT) {
			p = fork();

			if (p > 0)
				_exit(0);
			else if (p == 0) {
				signal(SIGHUP, SIG_DFL);
				signal(SIGINT, SIG_DFL);
				signal(SIGQUIT, SIG_DFL);
				signal(SIGTSTP, SIG_DFL);

				setsid();
				return p;
			}

			perror("fork");
			_exit(0);
		}

		/* so they can be used to stop the child */
		signal(SIGHUP, SIG_DFL);
		signal(SIGINT, SIG_DFL);
		signal(SIGQUIT, SIG_DFL);
		signal(SIGTSTP, SIG_DFL);
	}

	/* This is the parent waiting for the child to create grandchild*/
	if (flag & F_NOWAIT)
		waitpid(p, &status, 0);

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
			if (flag & F_CONFIRM) {
				printf("%s", messages[MSG_CONTINUE]);
				while (getchar() != '\n');
			}

			refresh();
		}

		free(cmd);
	}

	return retstatus;
}

static void prompt_run(char *cmd, const char *cur, const char *path)
{
	setenv(envs[ENV_NCUR], cur, 1);
	spawn(shell, "-c", cmd, path, F_CLI | F_CONFIRM);
}

/* Get program name from env var, else return fallback program */
static char *xgetenv(const char * const name, char *fallback)
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

static void opstr(char *buf, char *op)
{
	snprintf(buf, CMD_LEN_MAX, "xargs -0 sh -c '%s \"$0\" \"$@\" . < /dev/tty' < %s",
		 op, g_selpath);
}

static void rmmulstr(char *buf)
{
	if (cfg.trash)
		snprintf(buf, CMD_LEN_MAX, "xargs -0 trash-put < %s", g_selpath);
	else
		snprintf(buf, CMD_LEN_MAX, "xargs -0 sh -c 'rm -%cr \"$0\" \"$@\" < /dev/tty' < %s",
			 confirm_force(TRUE), g_selpath);
}

static void xrm(char *path)
{
	if (cfg.trash)
		spawn("trash-put", path, NULL, NULL, F_NORMAL);
	else {
		char rm_opts[] = "-ir";

		rm_opts[1] = confirm_force(FALSE);
		spawn("rm", rm_opts, path, NULL, F_NORMAL);
	}
}

static uint lines_in_file(int fd, char *buf, size_t buflen)
{
	ssize_t len;
	uint count = 0;

	while ((len = read(fd, buf, buflen)) > 0)
		while (len)
			count += (buf[--len] == '\n');

	/* For all use cases 0 linecount is considered as error */
	return ((len < 0) ? 0 : count);
}

static bool cpmv_rename(int choice, const char *path)
{
	int fd;
	uint count = 0, lines = 0;
	bool ret = FALSE;
	char *cmd = (choice == 'c' ? cp : mv);
	char buf[sizeof(cpmvrenamecmd) + sizeof(cmd) + (PATH_MAX << 1)];

	fd = create_tmp_file();
	if (fd == -1)
		return ret;

	/* selsafe() returned TRUE for this to be called */
	if (!selbufpos) {
		snprintf(buf, sizeof(buf), "tr '\\0' '\\n' < %s > %s", g_selpath, g_tmpfpath);
		spawn(utils[UTIL_SH_EXEC], buf, NULL, NULL, F_CLI);

		count = lines_in_file(fd, buf, sizeof(buf));
		if (!count)
			goto finish;
	} else
		seltofile(fd, &count);

	close(fd);

	snprintf(buf, sizeof(buf), cpmvformatcmd, g_tmpfpath);
	spawn(utils[UTIL_SH_EXEC], buf, NULL, path, F_CLI);

	spawn((cfg.waitedit ? enveditor : editor), g_tmpfpath, NULL, path, F_CLI);

	fd = open(g_tmpfpath, O_RDONLY);
	if (fd == -1)
		goto finish;

	lines = lines_in_file(fd, buf, sizeof(buf));
	DPRINTF_U(count);
	DPRINTF_U(lines);
	if (!lines || (2 * count != lines)) {
		DPRINTF_S("num mismatch");
		goto finish;
	}

	snprintf(buf, sizeof(buf), cpmvrenamecmd, path, g_tmpfpath, cmd);
	spawn(utils[UTIL_SH_EXEC], buf, NULL, path, F_CLI);
	ret = TRUE;

finish:
	if (fd >= 0)
		close(fd);

	return ret;
}

static bool cpmvrm_selection(enum action sel, char *path, int *presel)
{
	int r;

	if (!selsafe()) {
		*presel = MSGWAIT;
		return FALSE;
	}

	switch (sel) {
	case SEL_CP:
		opstr(g_buf, cp);
		break;
	case SEL_MV:
		opstr(g_buf, mv);
		break;
	case SEL_CPMVAS:
		r = get_input(messages[MSG_CP_MV_AS]);
		if (r != 'c' && r != 'm') {
			printwait(messages[MSG_INVALID_KEY], presel);
			return FALSE;
		}

		if (!cpmv_rename(r, path)) {
			printwait(messages[MSG_FAILED], presel);
			return FALSE;
		}
		break;
	default: /* SEL_RM */
		rmmulstr(g_buf);
		break;
	}

	if (sel != SEL_CPMVAS)
		spawn(utils[UTIL_SH_EXEC], g_buf, NULL, path, F_CLI);

	/* Clear selection on move or delete */
	if (sel != SEL_CP)
		clearselection();

	if (cfg.filtermode)
		*presel = FILTER;

	return TRUE;
}

static bool batch_rename(const char *path)
{
	int fd1, fd2, i;
	uint count = 0, lines = 0;
	bool dir = FALSE, ret = FALSE;
	char foriginal[TMP_LEN_MAX] = {0};
	char buf[sizeof(batchrenamecmd) + (PATH_MAX << 1)];

	i = get_cur_or_sel();
	if (!i)
		return ret;

	if (i == 'c') { /* Rename entries in current dir */
		selbufpos = 0;
		dir = TRUE;
	}

	fd1 = create_tmp_file();
	if (fd1 == -1)
		return ret;

	xstrlcpy(foriginal, g_tmpfpath, strlen(g_tmpfpath)+1);

	fd2 = create_tmp_file();
	if (fd2 == -1) {
		unlink(foriginal);
		close(fd1);
		return ret;
	}

	if (dir)
		for (i = 0; i < ndents; ++i)
			appendfpath(dents[i].name, NAME_MAX);

	seltofile(fd1, &count);
	seltofile(fd2, NULL);
	close(fd2);

	if (dir) /* Don't retain dir entries in selection */
		selbufpos = 0;

	spawn((cfg.waitedit ? enveditor : editor), g_tmpfpath, NULL, path, F_CLI);

	/* Reopen file descriptor to get updated contents */
	fd2 = open(g_tmpfpath, O_RDONLY);
	if (fd2 == -1)
		goto finish;

	lines = lines_in_file(fd2, buf, sizeof(buf));
	DPRINTF_U(count);
	DPRINTF_U(lines);
	if (!lines || (count != lines)) {
		DPRINTF_S("cannot delete files");
		goto finish;
	}

	snprintf(buf, sizeof(buf), batchrenamecmd, foriginal, g_tmpfpath);
	spawn(utils[UTIL_SH_EXEC], buf, NULL, path, F_CLI);
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
	if (getutil(utils[UTIL_ATOOL]))
		xstrlcpy(cmd, "atool -a", ARCHIVE_CMD_LEN);
	else if (getutil(utils[UTIL_BSDTAR]))
		xstrlcpy(cmd, "bsdtar -acvf", ARCHIVE_CMD_LEN);
	else if (is_suffix(archive, ".zip"))
		xstrlcpy(cmd, "zip -r", ARCHIVE_CMD_LEN);
	else
		xstrlcpy(cmd, "tar -acvf", ARCHIVE_CMD_LEN);
}

static void archive_selection(const char *cmd, const char *archive, const char *curpath)
{
	/* The 70 comes from the string below */
	char *buf = (char *)malloc((70 + strlen(cmd) + strlen(archive)
				       + strlen(curpath) + strlen(g_selpath)) * sizeof(char));
	if (!buf) {
		DPRINTF_S(strerror(errno));
		printwarn(NULL);
		return;
	}

	snprintf(buf, CMD_LEN_MAX,
#ifdef __linux__
		"sed -ze 's|^%s/||' '%s' | xargs -0 %s %s", curpath, g_selpath, cmd, archive);
#else
		"tr '\\0' '\n' < '%s' | sed -e 's|^%s/||' | tr '\n' '\\0' | xargs -0 %s %s",
		g_selpath, curpath, cmd, archive);
#endif
	spawn(utils[UTIL_SH_EXEC], buf, NULL, curpath, F_CLI);
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
	char *p1, *p2;

	ll v1 = strtoll(s1, &p1, 10);
	ll v2 = strtoll(s2, &p2, 10);

	/* Check if at least 1 string is numeric */
	if (s1 != p1 || s2 != p2) {
		/* Handle both pure numeric */
		if (s1 != p1 && s2 != p2) {
			if (v2 > v1)
				return -1;

			if (v1 > v2)
				return 1;
		}

		/* Only first string non-numeric */
		if (s1 == p1)
			return 1;

		/* Only second string non-numeric */
		if (s2 == p2)
			return -1;
	}

	/* Handle 1. all non-numeric and 2. both same numeric value cases */
#ifndef NOLOCALE
	return strcoll(s1, s2);
#else
	return strcasecmp(s1, s2);
#endif
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
	int state, diff;
	uchar c1, c2;

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

static int (*namecmpfn)(const char * const s1, const char * const s2) = &xstricmp;

/* Return the integer value of a char representing HEX */
static char xchartohex(char c)
{
	if (xisdigit(c))
		return c - '0';

	if (c >= 'a' && c <= 'f')
		return c - 'a' + 10;

	if (c >= 'A' && c <= 'F')
		return c - 'A' + 10;

	return c;
}

static char * (*fnstrstr)(const char *haystack, const char *needle) = &strcasestr;
#ifdef PCRE
static const unsigned char *tables;
static int pcreflags = PCRE_NO_AUTO_CAPTURE | PCRE_EXTENDED | PCRE_CASELESS | PCRE_UTF8;
#else
static int regflags = REG_NOSUB | REG_EXTENDED | REG_ICASE;
#endif

#ifdef PCRE
static int setfilter(pcre **pcrex, const char *filter)
{
	const char *errstr = NULL;
	int erroffset = 0;

	*pcrex = pcre_compile(filter, pcreflags, &errstr, &erroffset, tables);

	return errstr ? -1 : 0;
}
#else
static int setfilter(regex_t *regex, const char *filter)
{
	return regcomp(regex, filter, regflags);
}
#endif

static int visible_re(const fltrexp_t *fltrexp, const char *fname)
{
#ifdef PCRE
	return pcre_exec(fltrexp->pcrex, NULL, fname, strlen(fname), 0, 0, NULL, 0) == 0;
#else
	return regexec(fltrexp->regex, fname, 0, NULL, 0) == 0;
#endif
}

static int visible_str(const fltrexp_t *fltrexp, const char *fname)
{
	return fnstrstr(fname, fltrexp->str) != NULL;
}

static int (*filterfn)(const fltrexp_t *fltr, const char *fname) = &visible_str;

static void clearfilter(void)
{
	char *fltr = g_ctx[cfg.curctx].c_fltr;

	if (fltr[1]) {
		fltr[REGEX_MAX - 1] = fltr[1];
		fltr[1] = '\0';
	}
}

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
		char *extna = xmemrchr((uchar *)pa->name, '.', pa->nlen - 1);
		char *extnb = xmemrchr((uchar *)pb->name, '.', pb->nlen - 1);

		if (extna || extnb) {
			if (!extna)
				return -1;

			if (!extnb)
				return 1;

			int ret = strcasecmp(extna, extnb);

			if (ret)
				return ret;
		}
	}

	return namecmpfn(pa->name, pb->name);
}

static int reventrycmp(const void *va, const void *vb)
{
	if ((((pEntry)vb)->flags & DIR_OR_LINK_TO_DIR)
	    != (((pEntry)va)->flags & DIR_OR_LINK_TO_DIR)) {
		if (((pEntry)vb)->flags & DIR_OR_LINK_TO_DIR)
			return 1;
		return -1;
	}

	return -entrycmp(va, vb);
}

static int (*entrycmpfn)(const void *va, const void *vb) = &entrycmp;

/*
 * Returns SEL_* if key is bound and 0 otherwise.
 * Also modifies the run and env pointers (used on SEL_{RUN,RUNARG}).
 * The next keyboard input can be simulated by presel.
 */
static int nextsel(int presel)
{
	int c = presel;
	uint i;
#ifdef LINUX_INOTIFY
	struct inotify_event *event;
	char inotify_buf[EVENT_BUF_LEN];

	memset((void *)inotify_buf, 0x0, EVENT_BUF_LEN);
#elif defined(BSD_KQUEUE)
	struct kevent event_data[NUM_EVENT_SLOTS];

	memset((void *)event_data, 0x0, sizeof(struct kevent) * NUM_EVENT_SLOTS);
#elif defined(HAIKU_NM)
// TODO: Do some Haiku declarations
#endif

	if (c == 0 || c == MSGWAIT) {
		c = getch();
		//DPRINTF_D(c);
		//DPRINTF_S(keyname(c));

		if (c == ERR && presel == MSGWAIT)
			c = (cfg.filtermode) ? FILTER : CONTROL('L');
		else if (c == FILTER || c == CONTROL('L'))
			/* Clear previous filter when manually starting */
			clearfilter();
	}

	if (c == -1) {
		++idle;

		/*
		 * Do not check for directory changes in du mode.
		 * A redraw forces du calculation.
		 * Check for changes every odd second.
		 */
#ifdef LINUX_INOTIFY
		if (!cfg.selmode && !cfg.blkorder && inotify_wd >= 0 && (idle & 1)) {
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
		if (!cfg.selmode && !cfg.blkorder && event_fd >= 0 && idle & 1
		    && kevent(kq, events_to_monitor, NUM_EVENT_SLOTS,
			      event_data, NUM_EVENT_FDS, &gtimeout) > 0)
			c = CONTROL('L');
#elif defined(HAIKU_NM)
		if (!cfg.selmode && !cfg.blkorder && haiku_nm_active && idle & 1 && haiku_is_update_needed(haiku_hnd))
			c = CONTROL('L');
#endif
	} else
		idle = 0;

	for (i = 0; i < (int)ELEMENTS(bindings); ++i)
		if (c == bindings[i].sym)
			return bindings[i].act;

	return 0;
}

static int getorderstr(char *sort)
{
	int i = 0;

	if (cfg.mtimeorder)
		sort[0] = cfg.mtime ? 'T' : 'A';
	else if (cfg.sizeorder)
		sort[0] = 'S';
	else if (cfg.extnorder)
		sort[0] = 'E';

	if (sort[i])
		++i;

	if (entrycmpfn == &reventrycmp) {
		sort[i] = 'R';
		++i;
	}

	if (namecmpfn == &xstrverscasecmp) {
		sort[i] = 'V';
		++i;
	}

	if (i)
		sort[i] = ' ';

	return i;
}

static void showfilterinfo(void)
{
	int i = 0;
	char info[REGEX_MAX] = "\0\0\0\0";

	i = getorderstr(info);

	snprintf(info + i, REGEX_MAX - i - 1, "  %s [/], %s [:]",
		 (cfg.regex ? "regex" : "str"),
		 ((fnstrstr == &strcasestr) ? "ic" : "noic"));
	printinfoln(info);
}

static void showfilter(char *str)
{
	showfilterinfo();
	printprompt(str);
}

static inline void swap_ent(int id1, int id2)
{
	struct entry _dent, *pdent1 = &dents[id1], *pdent2 =  &dents[id2];

	*(&_dent) = *pdent1;
	*pdent1 = *pdent2;
	*pdent2 = *(&_dent);
}

#ifdef PCRE
static int fill(const char *fltr, pcre *pcrex)
#else
static int fill(const char *fltr, regex_t *re)
#endif
{
	int count = 0;
#ifdef PCRE
	fltrexp_t fltrexp = { .pcrex = pcrex, .str = fltr };
#else
	fltrexp_t fltrexp = { .regex = re, .str = fltr };
#endif

	for (; count < ndents; ++count) {
		if (filterfn(&fltrexp, dents[count].name) == 0) {
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
#ifdef PCRE
	pcre *pcrex = NULL;

	/* Search filter */
	if (cfg.regex && setfilter(&pcrex, fltr))
		return -1;

	ndents = fill(fltr, pcrex);

	if (cfg.regex)
		pcre_free(pcrex);
#else
	regex_t re;

	/* Search filter */
	if (cfg.regex && setfilter(&re, fltr))
		return -1;

	ndents = fill(fltr, &re);

	if (cfg.regex)
		regfree(&re);
#endif

	qsort(dents, ndents, sizeof(*dents), entrycmpfn);

	return ndents;
}

static int filterentries(char *path, char *lastname)
{
	wchar_t *wln = (wchar_t *)alloca(sizeof(wchar_t) * REGEX_MAX);
	char *ln = g_ctx[cfg.curctx].c_fltr;
	wint_t ch[2] = {0};
	int r, total = ndents, len;
	char *pln = g_ctx[cfg.curctx].c_fltr + 1;

	if (ndents && (ln[0] == FILTER || ln[0] == RFILTER) && *pln) {
		if (matches(pln) != -1) {
			move_cursor(dentfind(lastname, ndents), 0);
			redraw(path);
		}

		len = mbstowcs(wln, ln, REGEX_MAX);
	} else {
		ln[0] = wln[0] = cfg.regex ? RFILTER : FILTER;
		ln[1] = wln[1] = '\0';
		len = 1;
	}

	cleartimeout();
	curs_set(TRUE);
	showfilter(ln);

	while ((r = get_wch(ch)) != ERR) {
		//DPRINTF_D(*ch);
		//DPRINTF_S(keyname(*ch));

		switch (*ch) {
#ifdef KEY_RESIZE
		case KEY_RESIZE:
			clearoldprompt();
			redraw(path);
			showfilter(ln);
			continue;
#endif
		case KEY_DC: // fallthrough
		case KEY_BACKSPACE: // fallthrough
		case '\b': // fallthrough
		case 127: /* handle DEL */
			if (len != 1) {
				wln[--len] = '\0';
				wcstombs(ln, wln, REGEX_MAX);
				ndents = total;
			} else
				*ch = CONTROL('L');
			// fallthrough
		case CONTROL('L'):
			if (*ch == CONTROL('L')) {
				if (wln[1]) {
					ln[REGEX_MAX - 1] = ln[1];
					ln[1] = wln[1] = '\0';
					len = 1;
					ndents = total;
				} else if (ln[REGEX_MAX - 1]) { /* Show the previous filter */
					ln[1] = ln[REGEX_MAX - 1];
					ln[REGEX_MAX - 1] = '\0';
					len = mbstowcs(wln, ln, REGEX_MAX);
				}
			}

			/* Go to the top, we don't know if the hovered file will match the filter */
			cur = 0;

			if (matches(pln) != -1)
				redraw(path);

			showfilter(ln);
			continue;
#ifndef NOMOUSE
		case KEY_MOUSE: // fallthrough
#endif
		case 27: /* Exit filter mode on Escape */
			goto end;
		}

		if (r != OK) /* Handle Fn keys in main loop */
			break;

		/* Handle all control chars in main loop */
		if (*ch < ASCII_MAX && keyname(*ch)[0] == '^' && *ch != '^')
			goto end;

		if (len == 1) {
			switch (*ch) {
			case '=': // fallthrough /* Launch app */
			case ']': // fallthorugh /*Prompt key */
			case ';': // fallthrough /* Run plugin key */
			case ',': // fallthrough /* Pin CWD */
			case '?': /* Help and config key, '?' is an invalid regex */
				goto end;
			}

			/* Toggle case-sensitivity */
			if (*ch == CASE) {
				fnstrstr = (fnstrstr == &strcasestr) ? &strstr : &strcasestr;
#ifdef PCRE
				pcreflags ^= PCRE_CASELESS;
#else
				regflags ^= REG_ICASE;
#endif
				showfilter(ln);
				continue;
			}

			/* toggle string or regex filter */
			if (*ch == FILTER) {
				wln[0] = ln[0] = (ln[0] == FILTER) ? RFILTER : FILTER;
				cfg.regex ^= 1;
				filterfn = (filterfn == &visible_str) ? &visible_re : &visible_str;
				showfilter(ln);
				continue;
			}
		}

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

		if (matches(pln) == -1) {
			showfilter(ln);
			continue;
		}

		/* If the only match is a dir, auto-select and cd into it */
		if (ndents == 1 && cfg.filtermode
		    && cfg.autoselect && (dents[0].flags & DIR_OR_LINK_TO_DIR)) {
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
		showfilter(ln);
	}
end:
	clearinfoln();

	/* Save last working filter in-filter */
	if (ln[1])
		ln[REGEX_MAX - 1] = ln[1];

	if (*ch != 27 && *ch != '\t' && *ch != KEY_UP && *ch != KEY_DOWN && *ch != CONTROL('T')) {
		ln[0] = ln[1] = '\0';
		move_cursor(cur, 0);
	}

	/* Save current */
	if (ndents)
		copycurname();

	curs_set(FALSE);
	settimeout();

	/* Return keys for navigation etc. */
	return *ch;
}

/* Show a prompt with input string and return the changes */
static char *xreadline(const char *prefill, const char *prompt)
{
	size_t len, pos;
	int x, r;
	const int WCHAR_T_WIDTH = sizeof(wchar_t);
	wint_t ch[2] = {0};
	wchar_t * const buf = malloc(sizeof(wchar_t) * READLINE_MAX);

	if (!buf)
		errexit();

	cleartimeout();
	printprompt(prompt);

	if (prefill) {
		DPRINTF_S(prefill);
		len = pos = mbstowcs(buf, prefill, READLINE_MAX);
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
		if (r == ERR)
			continue;

		if (r == OK) {
			switch (*ch) {
			case KEY_ENTER: // fallthrough
			case '\n': // fallthrough
			case '\r':
				goto END;
			case CONTROL('D'):
				if (pos < len)
					++pos;
				else if (!(pos || len)) { /* Exit on ^D at empty prompt */
					len = 0;
					goto END;
				} else
					continue;
				// fallthrough
			case 127: // fallthrough
			case '\b': /* rhel25 sends '\b' for backspace */
				if (pos > 0) {
					memmove(buf + pos - 1, buf + pos,
						(len - pos) * WCHAR_T_WIDTH);
					--len, --pos;
				} // fallthrough
			case '\t': /* TAB breaks cursor position, ignore it */
				continue;
			case CONTROL('F'):
				if (pos < len)
					++pos;
				continue;
			case CONTROL('B'):
				if (pos > 0)
					--pos;
				continue;
			case CONTROL('W'):
				printprompt(prompt);
				do {
					if (pos == 0)
						break;
					memmove(buf + pos - 1, buf + pos,
						(len - pos) * WCHAR_T_WIDTH);
					--pos, --len;
				} while (buf[pos - 1] != ' ' && buf[pos - 1] != '/'); // NOLINT
				continue;
			case CONTROL('K'):
				printprompt(prompt);
				len = pos;
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

			if (pos < READLINE_MAX - 1) {
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

END:
	curs_set(FALSE);
	settimeout();
	clearprompt();

	buf[len] = '\0';

	pos = wcstombs(g_buf, buf, READLINE_MAX - 1);
	if (pos >= READLINE_MAX - 1)
		g_buf[READLINE_MAX - 1] = '\0';

	free(buf);
	return g_buf;
}

#ifndef NORL
/*
 * Caller should check the value of presel to confirm if it needs to wait to show warning
 */
static char *getreadline(const char *prompt, char *path, char *curpath, int *presel)
{
	/* Switch to current path for readline(3) */
	if (chdir(path) == -1) {
		printwarn(presel);
		return NULL;
	}

	exitcurses();

	char *input = readline(prompt);

	refresh();

	if (chdir(curpath) == -1)
		printwarn(presel);
	else if (input && input[0]) {
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
static size_t mkpath(const char *dir, const char *name, char *out)
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
 * Returns the number of links created, -1 on error
 */
static int xlink(char *prefix, char *path, char *curfname, char *buf, int *presel, int type)
{
	int count = 0, choice;
	char *psel = pselbuf, *fname;
	size_t pos = 0, len, r;
	int (*link_fn)(const char *, const char *) = NULL;
	char lnpath[PATH_MAX];

	choice = get_cur_or_sel();
	if (!choice)
		return -1;

	if (type == 's') /* symbolic link */
		link_fn = &symlink;
	else /* hard link */
		link_fn = &link;

	if (choice == 'c') {
		r = xstrlcpy(buf, prefix, NAME_MAX + 1); /* Copy prefix */
		xstrlcpy(buf + r - 1, curfname, NAME_MAX - r); /* Suffix target file name */
		mkpath(path, buf, lnpath); /* Generate link path */
		mkpath(path, curfname, buf); /* Generate target file path */

		if (!link_fn(buf, lnpath))
			return 1; /* One link created */

		printwarn(presel);
		return -1;
	}

	while (pos < selbufpos) {
		len = strlen(psel);
		fname = xbasename(psel);

		r = xstrlcpy(buf, prefix, NAME_MAX + 1); /* Copy prefix */
		xstrlcpy(buf + r - 1, fname, NAME_MAX - r); /* Suffix target file name */
		mkpath(path, buf, lnpath); /* Generate link path */

		if (!link_fn(psel, lnpath))
			++count;

		pos += len + 1;
		psel += len + 1;
	}

	return count;
}

static bool parsekvpair(kv *kvarr, char **envcpy, const char *cfgstr, uchar maxitems, size_t maxlen)
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

	for (i = 0; i < maxitems && kvarr[i].key; ++i)
		if (strlen(kvarr[i].val) >= maxlen)
			return FALSE;

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

				if (!buf) {
					buf = (char *)malloc(len + loclen);
					if (!buf) {
						DPRINTF_S(strerror(errno));
						return NULL;
					}
				}

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

static void resetdircolor(int flags)
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
static char *unescape(const char *str, uint maxcols, wchar_t **wstr)
{
	static wchar_t wbuf[NAME_MAX + 1] __attribute__ ((aligned));
	wchar_t *buf = wbuf;
	size_t lencount = 0;

#ifdef NOLOCALE
	memset(wbuf, 0, sizeof(wbuf));
#endif

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

	if (wstr) {
		*wstr = wbuf;
		return NULL;
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

/* Convert a mode field into "ls -l" type perms field. */
static char *get_lsperms(mode_t mode)
{
	static const char * const rwx[] = {"---", "--x", "-w-", "-wx", "r--", "r-x", "rw-", "rwx"};
	static char bits[11] = {'\0'};

	switch (mode & S_IFMT) {
	case S_IFREG:
		bits[0] = '-';
		break;
	case S_IFDIR:
		bits[0] = 'd';
		break;
	case S_IFLNK:
		bits[0] = 'l';
		break;
	case S_IFSOCK:
		bits[0] = 's';
		break;
	case S_IFIFO:
		bits[0] = 'p';
		break;
	case S_IFBLK:
		bits[0] = 'b';
		break;
	case S_IFCHR:
		bits[0] = 'c';
		break;
	default:
		bits[0] = '?';
		break;
	}

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

static void printent(const struct entry *ent, uint namecols, bool sel)
{
	wchar_t *wstr;
	char ind = '\0';

	switch (ent->mode & S_IFMT) {
	case S_IFREG:
		if (ent->mode & 0100)
			ind = '*';
		break;
	case S_IFDIR:
		ind = '/';
		break;
	case S_IFLNK:
		ind = '@';
		break;
	case S_IFSOCK:
		ind = '=';
		break;
	case S_IFIFO:
		ind = '|';
		break;
	case S_IFBLK: // fallthrough
	case S_IFCHR:
		break;
	default:
		ind = '?';
		break;
	}

	if (!ind)
		++namecols;

	unescape(ent->name, namecols, &wstr);

	/* Directories are always shown on top */
	resetdircolor(ent->flags);

	if (sel)
		attron(A_REVERSE);

	addch((ent->flags & FILE_SELECTED) ? '+' : ' ');
	addwstr(wstr);
	if (ind)
		addch(ind);
	addch('\n');

	if (sel)
		attroff(A_REVERSE);
}

static void printent_long(const struct entry *ent, uint namecols, bool sel)
{
	char timebuf[24], permbuf[4], ind1 = '\0', ind2[] = "\0\0";
	const char cp = (ent->flags & FILE_SELECTED) ? '+' : ' ';

	/* Timestamp */
	strftime(timebuf, sizeof(timebuf), "%F %R", localtime(&ent->t));
	timebuf[sizeof(timebuf)-1] = '\0';

	/* Permissions */
	permbuf[0] = '0' + ((ent->mode >> 6) & 7);
	permbuf[1] = '0' + ((ent->mode >> 3) & 7);
	permbuf[2] = '0' + (ent->mode & 7);
	permbuf[3] = '\0';

	/* Add a column if no indicator is needed */
	if (S_ISREG(ent->mode) && !(ent->mode & 0100))
		++namecols;

	/* Trim escape chars from name */
	const char *pname = unescape(ent->name, namecols, NULL);

	/* Directories are always shown on top */
	resetdircolor(ent->flags);

	if (sel)
		attron(A_REVERSE);

	switch (ent->mode & S_IFMT) {
	case S_IFREG:
		if (ent->mode & 0100)
			printw("%c%-16.16s  %s %8.8s* %s*\n", cp, timebuf, permbuf,
			       coolsize(cfg.blkorder ? ent->blocks << blk_shift : ent->size), pname);
		else
			printw("%c%-16.16s  %s %8.8s  %s\n", cp, timebuf, permbuf,
			       coolsize(cfg.blkorder ? ent->blocks << blk_shift : ent->size), pname);
		break;
	case S_IFDIR:
		printw("%c%-16.16s  %s %8.8s  %s/\n", cp, timebuf, permbuf,
		       coolsize(cfg.blkorder ? ent->blocks << blk_shift : ent->size), pname);
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

static void (*printptr)(const struct entry *ent, uint namecols, bool sel) = &printent;

static void savecurctx(settings *curcfg, char *path, char *curname, int r /* next context num */)
{
	settings cfg = *curcfg;
	bool selmode = cfg.selmode ? TRUE : FALSE;

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
		g_ctx[r].c_name[0] = '\0';
		g_ctx[r].c_fltr[0] = g_ctx[r].c_fltr[1] = '\0';
		g_ctx[r].c_cfg = cfg;
		g_ctx[r].c_cfg.runplugin = 0;
	}

	/* Continue selection mode */
	cfg.selmode = selmode;
	cfg.curctx = r;

	*curcfg = cfg;
}

static void save_session(bool last_session, int *presel)
{
	char spath[PATH_MAX];
	int i;
	session_header_t header;
	FILE *fsession;
	char *sname;
	bool status = FALSE;

	memset(&header, 0, sizeof(session_header_t));

	header.ver = SESSIONS_VERSION;

	for (i = 0; i < CTX_MAX; ++i) {
		if (g_ctx[i].c_cfg.ctxactive) {
			if (cfg.curctx == i && ndents)
				/* Update current file name, arrows don't update it */
				xstrlcpy(g_ctx[i].c_name, dents[cur].name, NAME_MAX + 1);
			header.pathln[i] = strnlen(g_ctx[i].c_path, PATH_MAX) + 1;
			header.lastln[i] = strnlen(g_ctx[i].c_last, PATH_MAX) + 1;
			header.nameln[i] = strnlen(g_ctx[i].c_name, NAME_MAX) + 1;
			header.fltrln[i] = strnlen(g_ctx[i].c_fltr, REGEX_MAX) + 1;
		}
	}

	sname = !last_session ? xreadline(NULL, messages[MSG_SSN_NAME]) : "@";
	if (!sname[0])
		return;
	mkpath(sessiondir, sname, spath);

	fsession = fopen(spath, "wb");
	if (!fsession) {
		printwait(messages[MSG_ACCESS], presel);
		return;
	}

	if ((fwrite(&header, sizeof(header), 1, fsession) != 1)
		|| (fwrite(&cfg, sizeof(cfg), 1, fsession) != 1))
		goto END;

	for (i = 0; i < CTX_MAX; ++i)
		if ((fwrite(&g_ctx[i].c_cfg, sizeof(settings), 1, fsession) != 1)
			|| (fwrite(&g_ctx[i].color, sizeof(uint), 1, fsession) != 1)
			|| (header.nameln[i] > 0
			    && fwrite(g_ctx[i].c_name, header.nameln[i], 1, fsession) != 1)
			|| (header.lastln[i] > 0
			    && fwrite(g_ctx[i].c_last, header.lastln[i], 1, fsession) != 1)
			|| (header.fltrln[i] > 0
			    && fwrite(g_ctx[i].c_fltr, header.fltrln[i], 1, fsession) != 1)
			|| (header.pathln[i] > 0
			    && fwrite(g_ctx[i].c_path, header.pathln[i], 1, fsession) != 1))
			goto END;

	status = TRUE;

END:
	fclose(fsession);

	if (!status)
		printwait(messages[MSG_FAILED], presel);
}

static bool load_session(const char *sname, char **path, char **lastdir, char **lastname, bool restore)
{
	char spath[PATH_MAX];
	int i = 0;
	session_header_t header;
	FILE *fsession;
	bool has_loaded_dynamically = !(sname || restore);
	bool status = FALSE;

	if (!restore) {
		sname = sname ? sname : xreadline(NULL, messages[MSG_SSN_NAME]);
		if (!sname[0])
			return FALSE;

		mkpath(sessiondir, sname, spath);
	} else
		mkpath(sessiondir, "@", spath);

	if (has_loaded_dynamically)
		save_session(TRUE, NULL);

	fsession = fopen(spath, "rb");
	if (!fsession) {
		printmsg(messages[MSG_ACCESS]);
		xdelay(XDELAY_INTERVAL_MS);
		return FALSE;
	}

	if ((fread(&header, sizeof(header), 1, fsession) != 1)
		|| (header.ver != SESSIONS_VERSION)
		|| (fread(&cfg, sizeof(cfg), 1, fsession) != 1))
		goto END;

	g_ctx[cfg.curctx].c_name[0] = g_ctx[cfg.curctx].c_last[0]
		= g_ctx[cfg.curctx].c_fltr[0] = g_ctx[cfg.curctx].c_fltr[1] = '\0';

	for (; i < CTX_MAX; ++i)
		if ((fread(&g_ctx[i].c_cfg, sizeof(settings), 1, fsession) != 1)
			|| (fread(&g_ctx[i].color, sizeof(uint), 1, fsession) != 1)
			|| (header.nameln[i] > 0
			    && fread(g_ctx[i].c_name, header.nameln[i], 1, fsession) != 1)
			|| (header.lastln[i] > 0
			    && fread(g_ctx[i].c_last, header.lastln[i], 1, fsession) != 1)
			|| (header.fltrln[i] > 0
			    && fread(g_ctx[i].c_fltr, header.fltrln[i], 1, fsession) != 1)
			|| (header.pathln[i] > 0
			    && fread(g_ctx[i].c_path, header.pathln[i], 1, fsession) != 1))
			goto END;

	*path = g_ctx[cfg.curctx].c_path;
	*lastdir = g_ctx[cfg.curctx].c_last;
	*lastname = g_ctx[cfg.curctx].c_name;
	printptr = cfg.showdetail ? &printent_long : &printent;
	status = TRUE;

END:
	fclose(fsession);

	if (!status) {
		printmsg(messages[MSG_FAILED]);
		xdelay(XDELAY_INTERVAL_MS);
	} else if (restore)
		unlink(spath);

	return status;
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

static void pipetof(char *cmd, FILE *fout)
{
	FILE *fin = popen(cmd, "r");

	if (fin) {
		while (fgets(g_buf, CMD_LEN_MAX - 1, fin))
			fprintf(fout, "%s", g_buf);
		pclose(fin);
	}
}

/*
 * Follows the stat(1) output closely
 */
static bool show_stats(const char *fpath, const struct stat *sb)
{
	int fd;
	FILE *fp;
	char *p, *begin = g_buf;
	size_t r;

	fd = create_tmp_file();
	if (fd == -1)
		return FALSE;

	r = xstrlcpy(g_buf, "stat \"", PATH_MAX);
	r += xstrlcpy(g_buf + r - 1, fpath, PATH_MAX);
	g_buf[r - 2] = '\"';
	g_buf[r - 1] = '\0';
	DPRINTF_S(g_buf);

	fp = fdopen(fd, "w");
	if (!fp) {
		close(fd);
		return FALSE;
	}

	pipetof(g_buf, fp);

	if (S_ISREG(sb->st_mode)) {
		/* Show file(1) output */
		p = get_output(g_buf, CMD_LEN_MAX, "file", "-b", fpath, FALSE);
		if (p) {
			fprintf(fp, "\n\n ");
			while (*p) {
				if (*p == ',') {
					*p = '\0';
					fprintf(fp, " %s\n", begin);
					begin = p + 1;
				}

				++p;
			}
			fprintf(fp, " %s\n  ", begin);

			/* Show the file mime type */
			get_output(g_buf, CMD_LEN_MAX, "file", FILE_MIME_OPTS, fpath, FALSE);
			fprintf(fp, "%s", g_buf);
		}
	}

	fprintf(fp, "\n");
	fclose(fp);
	close(fd);

	spawn(pager, g_tmpfpath, NULL, NULL, F_CLI);
	unlink(g_tmpfpath);
	return TRUE;
}

static bool xchmod(const char *fpath, mode_t mode)
{
	/* (Un)set (S_IXUSR | S_IXGRP | S_IXOTH) */
	(0100 & mode) ? (mode &= ~0111) : (mode |= 0111);

	return (chmod(fpath, mode) == 0);
}

static size_t get_fs_info(const char *path, bool type)
{
	struct statvfs svb;

	if (statvfs(path, &svb) == -1)
		return 0;

	if (type == CAPACITY)
		return svb.f_blocks << ffs((int)(svb.f_frsize >> 1));

	return svb.f_bavail << ffs((int)(svb.f_frsize >> 1));
}

/* List or extract archive */
static void handle_archive(char *fpath, const char *dir, char op)
{
	char arg[] = "-tvf"; /* options for tar/bsdtar to list files */
	char *util;

	if (getutil(utils[UTIL_ATOOL])) {
		util = utils[UTIL_ATOOL];
		arg[1] = op;
		arg[2] = '\0';
	} else if (getutil(utils[UTIL_BSDTAR])) {
		util = utils[UTIL_BSDTAR];
		if (op == 'x')
			arg[1] = op;
	} else if (is_suffix(fpath, ".zip")) {
		util = utils[UTIL_UNZIP];
		arg[1] = (op == 'l') ? 'v' /* verbose listing */ : '\0';
		arg[2] = '\0';
	} else {
		util = utils[UTIL_TAR];
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

static void find_accessible_parent(char *path, char *newpath, char *lastname, int *presel)
{
	char *dir;

	/* Save history */
	xstrlcpy(lastname, xbasename(path), NAME_MAX + 1);

	xstrlcpy(newpath, path, PATH_MAX);
	while (true) {
		dir = visit_parent(path, newpath, presel);
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

	printmsg(messages[MSG_ACCESS]);
	xdelay(XDELAY_INTERVAL_MS);
}

/* Create non-existent parents and a file or dir */
static bool xmktree(char *path, bool dir)
{
	char *p = path;
	char *slash = path;

	if (!p || !*p)
		return FALSE;

	/* Skip the first '/' */
	++p;

	while (*p != '\0') {
		if (*p == '/') {
			slash = p;
			*p = '\0';
		} else {
			++p;
			continue;
		}

		/* Create folder from path to '\0' inserted at p */
		if (mkdir(path, 0777) == -1 && errno != EEXIST) {
#ifdef __HAIKU__
			// XDG_CONFIG_HOME contains a directory
			// that is read-only, but the full path
			// is writeable.
			// Try to continue and see what happens.
			// TODO: Find a more robust solution.
			if (errno == B_READ_ONLY_DEVICE)
				goto next;
#endif
			DPRINTF_S("mkdir1!");
			DPRINTF_S(strerror(errno));
			*slash = '/';
			return FALSE;
		}

#ifdef __HAIKU__
next:
#endif
		/* Restore path */
		*slash = '/';
		++p;
	}

	if (dir) {
		if (mkdir(path, 0777) == -1 && errno != EEXIST) {
			DPRINTF_S("mkdir2!");
			DPRINTF_S(strerror(errno));
			return FALSE;
		}
	} else {
		int fd = open(path, O_CREAT, 0666);

		if (fd == -1 && errno != EEXIST) {
			DPRINTF_S("open!");
			DPRINTF_S(strerror(errno));
			return FALSE;
		}

		close(fd);
	}

	return TRUE;
}

static bool archive_mount(char *name, char *path, char *newpath, int *presel)
{
	char *dir, *cmd = utils[UTIL_ARCHIVEMOUNT];
	size_t len;

	if (!getutil(cmd)) {
		printwait(messages[MSG_UTIL_MISSING], presel);
		return FALSE;
	}

	dir = strdup(name);
	if (!dir)
		return FALSE;

	len = strlen(dir);

	while (len > 1)
		if (dir[--len] == '.') {
			dir[len] = '\0';
			break;
		}

	DPRINTF_S(dir);

	/* Create the mount point */
	mkpath(cfgdir, dir, newpath);
	free(dir);

	if (!xmktree(newpath, TRUE)) {
		printwarn(presel);
		return FALSE;
	}

	/* Mount archive */
	DPRINTF_S(name);
	DPRINTF_S(newpath);
	if (spawn(cmd, name, newpath, path, F_NORMAL)) {
		printwait(messages[MSG_FAILED], presel);
		return FALSE;
	}

	return TRUE;
}

static bool remote_mount(char *newpath, int *presel)
{
	uchar flag = F_CLI;
	int opt;
	char *tmp, *env, *cmd;
	bool r, s;

	r = getutil(utils[UTIL_RCLONE]);
	s = getutil(utils[UTIL_SSHFS]);

	if (!(r || s))
		return FALSE;

	if (r && s)
		opt = get_input(messages[MSG_REMOTE_OPTS]);
	else
		opt = (!s) ? 'r' : 's';

	if (opt == 's') {
		cmd = utils[UTIL_SSHFS];
		env = xgetenv("NNN_SSHFS", cmd);
	} else if (opt == 'r') {
		flag |= F_NOWAIT | F_NOTRACE;
		cmd = utils[UTIL_RCLONE];
		env = xgetenv("NNN_RCLONE", "rclone mount");
	} else {
		printwait(messages[MSG_INVALID_KEY], presel);
		return FALSE;
	}

	if (!getutil(cmd)) {
		printwait(messages[MSG_UTIL_MISSING], presel);
		return FALSE;
	}

	tmp = xreadline(NULL, messages[MSG_HOSTNAME]);
	if (!tmp[0])
		return FALSE;

	/* Create the mount point */
	mkpath(cfgdir, tmp, newpath);
	if (!xmktree(newpath, TRUE)) {
		printwarn(presel);
		return FALSE;
	}

	/* Convert "Host" to "Host:" */
	size_t len = strlen(tmp);

	if (tmp[len - 1] != ':') { /* Append ':' if missing */
		tmp[len] = ':';
		tmp[len + 1] = '\0';
	}

	/* Connect to remote */
	if (opt == 's') {
		if (spawn(env, tmp, newpath, NULL, flag)) {
			printwait(messages[MSG_FAILED], presel);
			return FALSE;
		}
	} else {
		spawn(env, tmp, newpath, NULL, flag);
		printmsg(messages[MSG_RCLONE_DELAY]);
		xdelay(XDELAY_INTERVAL_MS << 2); /* Set 4 times the usual delay */
	}

	return TRUE;
}

/*
 * Unmounts if the directory represented by name is a mount point.
 * Otherwise, asks for hostname
 */
static bool unmount(char *name, char *newpath, int *presel, char *currentpath)
{
#ifdef __APPLE__
	static char cmd[] = "umount";
#else
	static char cmd[] = "fusermount3"; /* Arch Linux utility */
	static bool found = FALSE;
#endif
	char *tmp = name;
	struct stat sb, psb;
	bool child = FALSE;
	bool parent = FALSE;

#ifndef __APPLE__
	/* On Ubuntu it's fusermount */
	if (!found && !getutil(cmd)) {
		cmd[10] = '\0';
		found = TRUE;
	}
#endif

	if (tmp && strcmp(cfgdir, currentpath) == 0) {
		mkpath(cfgdir, tmp, newpath);
		child = lstat(newpath, &sb) != -1;
		parent = lstat(dirname(newpath), &psb) != -1;
		if (!child && !parent) {
			*presel = MSGWAIT;
			return FALSE;
		}
	}

	if (!tmp || !child || !S_ISDIR(sb.st_mode) || (child && parent && sb.st_dev == psb.st_dev)) {
		tmp = xreadline(NULL, messages[MSG_HOSTNAME]);
		if (!tmp[0])
			return FALSE;
	}

	/* Create the mount point */
	mkpath(cfgdir, tmp, newpath);
	if (!xdiraccess(newpath)) {
		*presel = MSGWAIT;
		return FALSE;
	}

#ifdef __APPLE__
	if (spawn(cmd, newpath, NULL, NULL, F_NORMAL)) {
#else
	if (spawn(cmd, "-u", newpath, NULL, F_NORMAL)) {
#endif
		int r = get_input(messages[MSG_LAZY]);

		if (r != 'y' && r != 'Y')
			return FALSE;

#ifdef __APPLE__
		if (spawn(cmd, "-l", newpath, NULL, F_NORMAL)) {
#else
		if (spawn(cmd, "-uz", newpath, NULL, F_NORMAL)) {
#endif
			printwait(messages[MSG_FAILED], presel);
			return FALSE;
		}
	}

	return TRUE;
}

static void lock_terminal(void)
{
	char *tmp = utils[UTIL_LOCKER];

	if (!getutil(tmp))
		tmp = utils[UTIL_CMATRIX];

	spawn(tmp, NULL, NULL, NULL, F_NORMAL);
}

static void printkv(kv *kvarr, FILE *fp, uchar max)
{
	uchar i = 0;

	for (; i < max && kvarr[i].key; ++i)
		fprintf(fp, " %c: %s\n", (char)kvarr[i].key, kvarr[i].val);
}

static void printkeys(kv *kvarr, char *buf, uchar max)
{
	uchar i = 0;

	for (; i < max && kvarr[i].key; ++i) {
		buf[i << 1] = ' ';
		buf[(i << 1) + 1] = kvarr[i].key;
	}

	buf[i << 1] = '\0';
}

/*
 * The help string tokens (each line) start with a HEX value
 * which indicates the number of spaces to print before the
 * particular token. This method was chosen instead of a flat
 * string because the number of bytes in help was increasing
 * the binary size by around a hundred bytes. This would only
 * have increased as we keep adding new options.
 */
static void show_help(const char *path)
{
	int i, fd;
	FILE *fp;
	const char *start, *end;
	const char helpstr[] = {
		"0\n"
		"1NAVIGATION\n"
	       "9Up k  Up%-16cPgUp ^U  Scroll up\n"
	       "9Dn j  Down%-14cPgDn ^D  Scroll down\n"
	       "9Lt h  Parent%-12c~ ` @ -  HOME, /, start, last\n"
	   "5Ret Rt l  Open%-20c'  First file\n"
	       "9g ^A  Top%-18c. F5  Toggle hidden\n"
	       "9G ^E  End%-21c0  Lock terminal\n"
	       "9b ^/  Bookmark key%-12c,  Pin CWD\n"
		"a1-4  Context 1-4%-8c(B)Tab  Cycle context\n"
		  "c/  Filter%-17c^N  Nav-as-you-type toggle\n"
		"aEsc  Exit prompt%-12c^L  Redraw/clear prompt\n"
		  "c?  Help, conf%-14c+  Toggle proceed on open\n"
		  "cq  Quit context%-11c^G  QuitCD\n"
		 "b^Q  Quit%-20cQ  Quit with err\n"
		"1FILES\n"
	       "9o ^O  Open with...%-12cn  Create new/link\n"
	       "9f ^F  File details%-12cd  Detail view toggle\n"
		 "b^R  Rename/dup%-14cr  Batch rename\n"
		  "cz  Archive%-17ce  Edit in EDITOR\n"
	   "5Space ^J  (Un)select%-11cm ^K  Mark range/clear\n"
	       "9p ^P  Copy sel here%-11ca  Select all\n"
	       "9v ^V  Move sel here%-8cw ^W  Copy/move sel as\n"
	       "9x ^X  Delete%-18cE  Edit sel\n"
	          "c*  Toggle exe%-0c\n"
		"1MISC\n"
	       "9; ^S  Select plugin%-11c=  Launch app\n"
	       "9! ^]  Shell%-19c]  Cmd prompt\n"
		  "cc  Connect remote%-10cu  Unmount\n"
	       "9t ^T  Sort toggles%-12cs  Manage session\n"
	};

	fd = create_tmp_file();
	if (fd == -1)
		return;

	fp = fdopen(fd, "w");
	if (!fp) {
		close(fd);
		return;
	}

	start = end = helpstr;
	while (*end) {
		if (*end == '\n') {
			snprintf(g_buf, CMD_LEN_MAX, "%*c%.*s",
				 xchartohex(*start), ' ', (int)(end - start), start + 1);
			fprintf(fp, g_buf, ' ');
			start = end + 1;
		}

		++end;
	}

	fprintf(fp, "\nVOLUME: %s of ", coolsize(get_fs_info(path, FREE)));
	fprintf(fp, "%s free\n\n", coolsize(get_fs_info(path, CAPACITY)));

	if (bookmark[0].val) {
		fprintf(fp, "BOOKMARKS\n");
		printkv(bookmark, fp, BM_MAX);
		fprintf(fp, "\n");
	}

	if (plug[0].val) {
		fprintf(fp, "PLUGIN KEYS\n");
		printkv(plug, fp, PLUGIN_MAX);
		fprintf(fp, "\n");
	}

	for (i = NNN_OPENER; i <= NNN_TRASH; ++i) {
		start = getenv(env_cfg[i]);
		if (start)
			fprintf(fp, "%s: %s\n", env_cfg[i], start);
	}

	if (g_selpath)
		fprintf(fp, "SELECTION FILE: %s\n", g_selpath);

	fprintf(fp, "\nv%s\n%s\n", VERSION, GENERAL_INFO);
	fclose(fp);
	close(fd);

	spawn(pager, g_tmpfpath, NULL, NULL, F_CLI);
	unlink(g_tmpfpath);
}

static bool run_cmd_as_plugin(const char *path, const char *file, char *newpath, char *runfile)
{
	uchar flags = F_CLI | F_CONFIRM;
	size_t len;

	/* Get rid of preceding _ */
	++file;

	if (!*file)
		return FALSE;

	/* Check if GUI flags are to be used */
	if (*file == '|') {
		flags = F_NOTRACE | F_NOWAIT;
		++file;

		if (!*file)
			return FALSE;
	}

	xstrlcpy(newpath, file, PATH_MAX);

	len = strlen(newpath);
	if (len > 1 && newpath[len - 1] == '*') {
		flags &= ~F_CONFIRM; /* Skip user confirmation */
		newpath[len - 1] = '\0'; /* Get rid of trailing no confirmation symbol */
		--len;
	}

	if (is_suffix(newpath, " $nnn")) {
		/* Set `\0` to clear ' $nnn' suffix */
		newpath[len - 5] = '\0';
	} else
		runfile = NULL;

	spawn(newpath, runfile, NULL, path, flags);
	return TRUE;
}

static bool plctrl_init(void)
{
	snprintf(g_buf, CMD_LEN_MAX, "nnn-pipe.%d", getpid());
	/* g_tmpfpath is used to generate tmp file names */
	g_tmpfpath[g_tmpfplen - 1] = '\0';
	mkpath(g_tmpfpath, g_buf, g_pipepath);
	unlink(g_pipepath);
	if (mkfifo(g_pipepath, 0600) != 0)
		return _FAILURE;

	setenv(env_cfg[NNN_PIPE], g_pipepath, TRUE);

	return _SUCCESS;
}

static bool run_selected_plugin(char **path, const char *file, char *newpath, char *runfile, char **lastname, char **lastdir)
{
	int fd;
	size_t len;

	if (*file == '_')
		return run_cmd_as_plugin(*path, file, newpath, runfile);

	if (!(g_states & STATE_PLUGIN_INIT)) {
		plctrl_init();
		g_states |= STATE_PLUGIN_INIT;
	}

	fd = open(g_pipepath, O_RDONLY | O_NONBLOCK);
	if (fd == -1)
		return FALSE;

	/* Generate absolute path to plugin */
	mkpath(plugindir, file, newpath);

	if (runfile && runfile[0]) {
		xstrlcpy(*lastname, runfile, NAME_MAX);
		spawn(newpath, *lastname, *path, *path, F_NORMAL);
	} else
		spawn(newpath, NULL, *path, *path, F_NORMAL);

	len = read(fd, g_buf, PATH_MAX);
	g_buf[len] = '\0';

	close(fd);

	if (len > 1) {
		int ctx = g_buf[0] - '0';

		if (ctx == 0 || ctx == cfg.curctx + 1) {
			xstrlcpy(*lastdir, *path, PATH_MAX);
			xstrlcpy(*path, g_buf + 1, PATH_MAX);
		} else if (ctx >= 1 && ctx <= CTX_MAX) {
			int r = ctx - 1;

			g_ctx[r].c_cfg.ctxactive = 0;
			savecurctx(&cfg, g_buf + 1, dents[cur].name, r);
			*path = g_ctx[r].c_path;
			*lastdir = g_ctx[r].c_last;
			*lastname = g_ctx[r].c_name;
		}
	}

	return TRUE;
}

static void plugscript(const char *plugin, char *newpath, uchar flags)
{
	mkpath(plugindir, plugin, newpath);
	if (!access(newpath, X_OK))
		spawn(newpath, NULL, NULL, NULL, flags);
}

static void launch_app(const char *path, char *newpath)
{
	int r = F_NORMAL;
	char *tmp = newpath;

	mkpath(plugindir, utils[UTIL_LAUNCH], newpath);

	if (!(getutil(utils[UTIL_FZF]) || getutil(utils[UTIL_FZY])) || access(newpath, X_OK) < 0) {
		tmp = xreadline(NULL, messages[MSG_APP_NAME]);
		r = F_NOWAIT | F_NOTRACE | F_MULTI;
	}

	if (tmp && *tmp) // NOLINT
		spawn(tmp, "0", NULL, path, r);
}

static int sum_bsize(const char *UNUSED(fpath), const struct stat *sb, int typeflag, struct FTW *UNUSED(ftwbuf))
{
	if (sb->st_blocks && (typeflag == FTW_F || typeflag == FTW_D))
		ent_blocks += sb->st_blocks;

	++num_files;
	return 0;
}

static int sum_asize(const char *UNUSED(fpath), const struct stat *sb, int typeflag, struct FTW *UNUSED(ftwbuf))
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

static blkcnt_t dirwalk(char *path, struct stat *psb)
{
	static uint open_max;

	/* Increase current open file descriptor limit */
	if (!open_max)
		open_max = max_openfds();

	ent_blocks = 0;
	tolastln();
	addstr(xbasename(path));
	addstr(" [^C aborts]\n");
	refresh();

	if (nftw(path, nftw_fn, open_max, FTW_MOUNT | FTW_PHYS) < 0) {
		DPRINTF_S("nftw failed");
		return cfg.apparentsz ? psb->st_size : psb->st_blocks;
	}

	return ent_blocks;
}

/* Skip self and parent */
static bool selforparent(const char *path)
{
	return path[0] == '.' && (path[1] == '\0' || (path[1] == '.' && path[2] == '\0'));
}

static int dentfill(char *path, struct entry **dents)
{
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
	}

#if _POSIX_C_SOURCE >= 200112L
	posix_fadvise(fd, 0, 0, POSIX_FADV_SEQUENTIAL);
#endif

	dp = readdir(dirp);
	if (!dp)
		goto exit;

#if defined(__sun) || defined(__HAIKU__)
	flags = AT_SYMLINK_NOFOLLOW; /* no d_type */
#else
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
#endif

	do {
		namep = dp->d_name;

		if (selforparent(namep))
			continue;

		if (!cfg.showhidden && namep[0] == '.') {
			if (!cfg.blkorder)
				continue;

			if (fstatat(fd, namep, &sb, AT_SYMLINK_NOFOLLOW) == -1)
				continue;

			if (S_ISDIR(sb.st_mode)) {
				if (sb_path.st_dev == sb.st_dev) {
					mkpath(path, namep, buf);

					dir_blocks += dirwalk(buf, &sb);

					if (g_states & STATE_INTERRUPTED) {
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
			if (flags || (!flags && fstatat(fd, namep, &sb, AT_SYMLINK_NOFOLLOW) == -1)) {
				DPRINTF_U(flags);
				if (!flags) {
					DPRINTF_S(namep);
					DPRINTF_S(strerror(errno));
				}
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
					(dentp + 1)->name = (char *)((size_t)dentp->name + dentp->nlen);
			}
		}

		dentp = *dents + n;

		/* Selection file name */
		dentp->name = (char *)((size_t)pnamebuf + off);
		dentp->nlen = xstrlcpy(dentp->name, namep, NAME_MAX + 1);
		off += dentp->nlen;

		/* Copy other fields */
		dentp->t = cfg.mtime ? sb.st_mtime : sb.st_atime;
#if !(defined(__sun) || defined(__HAIKU__))
		if (!flags && dp->d_type == DT_LNK) {
			 /* Do not add sizes for links */
			dentp->mode = (sb.st_mode & ~S_IFMT) | S_IFLNK;
			dentp->size = g_listpath ? sb.st_size : 0;
		} else {
			dentp->mode = sb.st_mode;
			dentp->size = sb.st_size;
		}
#else
		dentp->mode = sb.st_mode;
		dentp->size = sb.st_size;
#endif
		dentp->flags = 0;

		if (cfg.blkorder) {
			if (S_ISDIR(sb.st_mode)) {
				num_saved = num_files + 1;
				mkpath(path, namep, buf);

				/* Need to show the disk usage of this dir */
				dentp->blocks = dirwalk(buf, &sb);

				if (sb_path.st_dev == sb.st_dev) // NOLINT
					dir_blocks += dentp->blocks;
				else
					num_files = num_saved;

				if (g_states & STATE_INTERRUPTED) {
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
#if !(defined(__sun) || defined(__HAIKU__)) /* no d_type */
		} else if (dp->d_type == DT_DIR || (dp->d_type == DT_LNK && S_ISDIR(sb.st_mode))) {
			dentp->flags |= DIR_OR_LINK_TO_DIR;
#endif
		}

		++n;
	} while ((dp = readdir(dirp)));

exit:
	/* Should never be null */
	if (closedir(dirp) == -1)
		errexit();

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

	qsort(dents, ndents, sizeof(*dents), entrycmpfn);

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

	// Force full redraw
	last_curscroll = -1;
}

static void move_cursor(int target, int ignore_scrolloff)
{
	int delta, scrolloff, onscreen = xlines - 4;

	last_curscroll = curscroll;
	target = MAX(0, MIN(ndents - 1, target));
	delta = target - cur;
	last = cur;
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
		if (((cur < (curscroll + scrolloff)) && delta < 0)
		    || ((cur > (curscroll + onscreen - scrolloff - 1)) && delta > 0))
			curscroll += delta;
	}
	curscroll = MIN(curscroll, MIN(cur, ndents - onscreen));
	curscroll = MAX(curscroll, MAX(cur - (onscreen - 1), 0));
}

static void handle_screen_move(enum action sel)
{
	int onscreen;

	switch (sel) {
	case SEL_NEXT:
		if (ndents && (cfg.rollover || (cur != ndents - 1)))
			move_cursor((cur + 1) % ndents, 0);
		break;
	case SEL_PREV:
		if (ndents && (cfg.rollover || cur))
			move_cursor((cur + ndents - 1) % ndents, 0);
		break;
	case SEL_PGDN:
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
	default: /* case SEL_FIRST */
	{
		int r = 0;

		for (; r < ndents; ++r) {
			if (!(dents[r].flags & DIR_OR_LINK_TO_DIR)) {
				move_cursor((r) % ndents, 0);
				break;
			}
		}
		break;
	}
	}
}

static int handle_context_switch(enum action sel, char *newpath)
{
	int r = -1, input;

	switch (sel) {
	case SEL_CYCLE: // fallthrough
	case SEL_CYCLER:
		/* visit next and previous contexts */
		r = cfg.curctx;
		if (sel == SEL_CYCLE)
			do
				r = (r + 1) & ~CTX_MAX;
			while (!g_ctx[r].c_cfg.ctxactive);
		else
			do
				r = (r + (CTX_MAX - 1)) & (CTX_MAX - 1);
			while (!g_ctx[r].c_cfg.ctxactive);
		// fallthrough
	default: /* SEL_CTXN */
		if (sel >= SEL_CTX1) /* CYCLE keys are lesser in value */
			r = sel - SEL_CTX1; /* Save the next context id */

		if (cfg.curctx == r) {
			if (sel != SEL_CYCLE)
				return -1;

			(r == CTX_MAX - 1) ? (r = 0) : ++r;
			snprintf(newpath, PATH_MAX, messages[MSG_CREATE_CTX], r + 1);
			input = get_input(newpath);
			if (input != 'y' && input != 'Y')
				return -1;
		}

		if (cfg.selmode)
			lastappendpos = selbufpos;
	}

	return r;
}

static bool set_sort_flags(void)
{
	int r = get_input(messages[MSG_ORDER]);

	if ((r == 'a' || r == 'd' || r == 'e' || r == 's' || r == 't') && (entrycmpfn == &reventrycmp))
		entrycmpfn = &entrycmp;

	switch (r) {
	case 'a': /* Apparent du */
		cfg.apparentsz ^= 1;
		if (cfg.apparentsz) {
			nftw_fn = &sum_asize;
			cfg.blkorder = 1;
			blk_shift = 0;
		} else
			cfg.blkorder = 0;
		// fallthrough
	case 'd': /* Disk usage */
		if (r == 'd') {
			if (!cfg.apparentsz)
				cfg.blkorder ^= 1;
			nftw_fn = &sum_bsize;
			cfg.apparentsz = 0;
			blk_shift = ffs(S_BLKSIZE) - 1;
		}

		if (cfg.blkorder) {
			cfg.showdetail = 1;
			printptr = &printent_long;
		}
		cfg.mtimeorder = 0;
		cfg.sizeorder = 0;
		cfg.extnorder = 0;
		clearfilter(); /* Reload directory */
		endselection(); /* We are going to reload dir */
		break;
	case 'e': /* File extension */
		cfg.extnorder ^= 1;
		cfg.sizeorder = 0;
		cfg.mtimeorder = 0;
		cfg.apparentsz = 0;
		cfg.blkorder = 0;
		break;
	case 'r': /* Reverse sort */
		entrycmpfn = (entrycmpfn == &entrycmp) ? &reventrycmp : &entrycmp;
		break;
	case 's': /* File size */
		cfg.sizeorder ^= 1;
		cfg.mtimeorder = 0;
		cfg.apparentsz = 0;
		cfg.blkorder = 0;
		cfg.extnorder = 0;
		break;
	case 't': /* Modification or access time */
		cfg.mtimeorder ^= 1;
		cfg.sizeorder = 0;
		cfg.apparentsz = 0;
		cfg.blkorder = 0;
		cfg.extnorder = 0;
		break;
	case 'v': /* Version */
		namecmpfn = (namecmpfn == &xstrverscasecmp) ? &xstricmp : &xstrverscasecmp;
		break;
	default:
		return FALSE;
	}

	return TRUE;
}

static void statusbar(char *path)
{
	int i = 0, extnlen = 0;
	char *ptr;
	char buf[24];
	pEntry pent = &dents[cur];

	if (!ndents) {
		printmsg("0/0");
		return;
	}

	/* Get the file extension for regular files */
	if (S_ISREG(pent->mode)) {
		i = (int)(pent->nlen - 1);
		ptr = xmemrchr((uchar *)pent->name, '.', i);
		if (ptr) {
			extnlen = ptr - pent->name;
			extnlen = i - extnlen;
		}
		if (!ptr || extnlen > 5 || extnlen < 2)
			ptr = "\b";
	} else
		ptr = "\b";

	if (cfg.blkorder) { /* du mode */
		xstrlcpy(buf, coolsize(dir_blocks << blk_shift), 12);

		mvprintw(xlines - 1, 0, "%d/%d [%s:%s] %cu:%s free:%s files:%lu %lldB %s",
			 cur + 1, ndents, (cfg.selmode ? "s" : ""),
			 ((g_states & STATE_RANGESEL) ? "*" : (nselected ? xitoa(nselected) : "")),
			 (cfg.apparentsz ? 'a' : 'd'), buf, coolsize(get_fs_info(path, FREE)),
			 num_files, (ll)pent->blocks << blk_shift, ptr);
	} else { /* light or detail mode */
		char sort[] = "\0\0\0\0";

		getorderstr(sort);

		/* Timestamp */
		strftime(buf, sizeof(buf), "%F %R", localtime(&pent->t));
		buf[sizeof(buf)-1] = '\0';

		mvprintw(xlines - 1, 0, "%d/%d [%s:%s] %s%s %s %s %s",
			 cur + 1, ndents, (cfg.selmode ? "s" : ""),
			 ((g_states & STATE_RANGESEL) ? "*" : (nselected ? xitoa(nselected) : "")),
			 sort, buf, get_lsperms(pent->mode), coolsize(pent->size), ptr);
	}

	addch('\n');
}

static int adjust_cols(int ncols)
{
	/* Calculate the number of cols available to print entry name */
	if (cfg.showdetail) {
		/* Fallback to light mode if less than 35 columns */
		if (ncols < 36) {
			cfg.showdetail ^= 1;
			printptr = &printent;
			ncols -= 3; /* Preceding space, indicator, newline */
		} else
			ncols -= 35;
	} else
		ncols -= 3; /* Preceding space, indicator, newline */

	return ncols;
}

static void draw_line(char *path, int ncols)
{
	bool dir = FALSE;

	ncols = adjust_cols(ncols);

	if (dents[last].flags & DIR_OR_LINK_TO_DIR) {
		attron(COLOR_PAIR(cfg.curctx + 1) | A_BOLD);
		dir = TRUE;
	}
	move(2 + last - curscroll, 0);
	printptr(&dents[last], ncols, false);

	if (dents[cur].flags & DIR_OR_LINK_TO_DIR) {
		if (!dir)  {/* First file is not a directory */
			attron(COLOR_PAIR(cfg.curctx + 1) | A_BOLD);
			dir = TRUE;
		}
	} else if (dir) { /* Second file is not a directory */
		attroff(COLOR_PAIR(cfg.curctx + 1) | A_BOLD);
		dir = FALSE;
	}

	move(2 + cur - curscroll, 0);
	printptr(&dents[cur], ncols, true);

	/* Must reset e.g. no files in dir */
	if (dir)
		attroff(COLOR_PAIR(cfg.curctx + 1) | A_BOLD);

	statusbar(path);
}

static void redraw(char *path)
{
	xlines = LINES;
	xcols = COLS;

	int ncols = (xcols <= PATH_MAX) ? xcols : PATH_MAX;
	int onscreen = xlines - 4;
	int i;
	char *ptr = path;

	// Fast redraw
	if (g_states & STATE_MOVE_OP) {
		g_states &= ~STATE_MOVE_OP;

		if (ndents && (last_curscroll == curscroll))
			return draw_line(path, ncols);
	}

	/* Clear screen */
	erase();

	/* Enforce scroll/cursor invariants */
	move_cursor(cur, 1);

	/* Fail redraw if < than 10 columns, context info prints 10 chars */
	if (ncols < MIN_DISPLAY_COLS) {
		printmsg(messages[MSG_FEW_COLUMNS]);
		return;
	}

	//DPRINTF_D(cur);
	DPRINTF_S(path);

	addch('[');
	for (i = 0; i < CTX_MAX; ++i) {
		if (!g_ctx[i].c_cfg.ctxactive) {
			addch(i + '1');
		} else {
			int attrs = (cfg.curctx != i)
				     ? (COLOR_PAIR(i + 1) | A_BOLD | A_UNDERLINE) /* Active */
				     : (COLOR_PAIR(i + 1) | A_BOLD | A_REVERSE); /* Current */
			attron(attrs);
			addch(i + '1');
			attroff(attrs);
		}
		addch(' ');
	}
	addstr("\b] "); /* 10 chars printed for contexts - "[1 2 3 4] " */

	attron(A_UNDERLINE);

	/* Print path */
	i = (int)strlen(path);
	if ((i + MIN_DISPLAY_COLS) <= ncols)
		addnstr(path, ncols - MIN_DISPLAY_COLS);
	else {
		char *base = xbasename(path);

		if ((base - ptr) <= 1)
			addnstr(path, ncols - MIN_DISPLAY_COLS);
		else {
			i = 0;
			--base;
			while (ptr < base) {
				if (*ptr == '/') {
					i += 2; /* 2 characters added */
					if (ncols < i + MIN_DISPLAY_COLS) {
						base = NULL; /* Can't print more characters */
						break;
					}

					addch(*ptr);
					addch(*(++ptr));
				}
				++ptr;
			}

			addnstr(base, ncols - (MIN_DISPLAY_COLS + i));
		}
	}

	/* Go to first entry */
	move(2, 0);

	attroff(A_UNDERLINE);

	ncols = adjust_cols(ncols);

	attron(COLOR_PAIR(cfg.curctx + 1) | A_BOLD);
	cfg.dircolor = 1;

	/* Print listing */
	for (i = curscroll; i < ndents && i < curscroll + onscreen; ++i)
		printptr(&dents[i], ncols, i == cur);

	/* Must reset e.g. no files in dir */
	if (cfg.dircolor) {
		attroff(COLOR_PAIR(cfg.curctx + 1) | A_BOLD);
		cfg.dircolor = 0;
	}

	statusbar(path);
}

static bool browse(char *ipath, const char *session)
{
	char newpath[PATH_MAX] __attribute__ ((aligned));
	char rundir[PATH_MAX] __attribute__ ((aligned));
	char runfile[NAME_MAX + 1] __attribute__ ((aligned));
	char *path, *lastdir, *lastname, *dir, *tmp, *mark = NULL;
	enum action sel;
	struct stat sb;
	int r = -1, fd, presel, selstartid = 0, selendid = 0;
	const uchar opener_flags = (cfg.cliopener ? F_CLI : (F_NOTRACE | F_NOWAIT));
	bool dir_changed = FALSE;

#ifndef NOMOUSE
	MEVENT event;
	struct timespec mousetimings[2] = {{.tv_sec = 0, .tv_nsec = 0}, {.tv_sec = 0, .tv_nsec = 0} };
	bool currentmouse = 1;
#endif

#ifndef DIR_LIMITED_SELECTION
	ino_t inode = 0;
#endif

	atexit(dentfree);

	/* setup first context */
	if (!session || !load_session(session, &path, &lastdir, &lastname, FALSE)) {
		xstrlcpy(g_ctx[0].c_path, ipath, PATH_MAX); /* current directory */
		path = g_ctx[0].c_path;
		g_ctx[0].c_last[0] = g_ctx[0].c_name[0] = '\0';
		lastdir = g_ctx[0].c_last; /* last visited directory */
		lastname = g_ctx[0].c_name; /* last visited filename */
		g_ctx[0].c_fltr[0] = g_ctx[0].c_fltr[1] = '\0';
		g_ctx[0].c_cfg = cfg; /* current configuration */
	}

	newpath[0] = rundir[0] = runfile[0] = '\0';

	presel = cfg.filtermode ? FILTER : 0;

	dents = xrealloc(dents, total_dents * sizeof(struct entry));
	if (!dents)
		errexit();

	/* Allocate buffer to hold names */
	pnamebuf = (char *)xrealloc(pnamebuf, NAMEBUF_INCR);
	if (!pnamebuf)
		errexit();

begin:
	/* Can fail when permissions change while browsing.
	 * It's assumed that path IS a directory when we are here.
	 */
	if (access(path, R_OK) == -1) {
		DPRINTF_S("directory inaccessible");
		find_accessible_parent(path, newpath, lastname, &presel);
		setdirwatch();
	}

	if (cfg.selmode && lastdir[0])
		lastappendpos = selbufpos;

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
#elif defined(HAIKU_NM)
	if ((presel == FILTER || dir_changed) && haiku_hnd != NULL) {
		haiku_stop_watch(haiku_hnd);
		haiku_nm_active = FALSE;
		dir_changed = FALSE;
	}
#endif

	populate(path, lastname);
	if (g_states & STATE_INTERRUPTED) {
		g_states &= ~STATE_INTERRUPTED;
		cfg.apparentsz = 0;
		cfg.blkorder = 0;
		blk_shift = BLK_SHIFT_512;
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
#elif defined(HAIKU_NM)
	haiku_nm_active = haiku_watch_dir(haiku_hnd, path) == _SUCCESS;
#endif

	while (1) {
		redraw(path);

		/* Display a one-time message */
		if (g_listpath && (g_states & STATE_MSG)) {
			g_states &= ~STATE_MSG;
			printwait(messages[MSG_IGNORED], &presel);
		}

nochange:
		/* Exit if parent has exited */
		if (getppid() == 1) {
			free(mark);
			_exit(0);
		}

		/* If CWD is deleted or moved or perms changed, find an accessible parent */
		if (access(path, F_OK))
			goto begin;

		/* If STDIN is no longer a tty (closed) we should exit */
		if (!isatty(STDIN_FILENO) && !cfg.picker) {
			free(mark);
			return _FAILURE;
		}

		sel = nextsel(presel);
		if (presel)
			presel = 0;

		switch (sel) {
#ifndef NOMOUSE
		case SEL_CLICK:
			if (getmouse(&event) != OK)
				goto nochange;

			/* Handle clicking on a context at the top */
			if (event.bstate == BUTTON1_PRESSED && event.y == 0) {
				/* Get context from: "[1 2 3 4]..." */
				r = event.x >> 1;

				/* If clicked after contexts, go to parent */
				if (r >= CTX_MAX)
					sel = SEL_BACK;
				else if (r >= 0 && r != cfg.curctx) {
					if (cfg.selmode)
						lastappendpos = selbufpos;

					savecurctx(&cfg, path, dents[cur].name, r);

					/* Reset the pointers */
					path = g_ctx[r].c_path;
					lastdir = g_ctx[r].c_last;
					lastname = g_ctx[r].c_name;

					setdirwatch();
					goto begin;
				}
			}
#endif
			// fallthrough
		case SEL_BACK:
#ifndef NOMOUSE
			if (sel == SEL_BACK) {
#endif
				dir = visit_parent(path, newpath, &presel);
				if (!dir)
					goto nochange;

				/* Save last working directory */
				xstrlcpy(lastdir, path, PATH_MAX);

				/* Save history */
				xstrlcpy(lastname, xbasename(path), NAME_MAX + 1);
				clearfilter();

				xstrlcpy(path, dir, PATH_MAX);

				setdirwatch();
				goto begin;
#ifndef NOMOUSE
			}
#endif

#ifndef NOMOUSE
#if NCURSES_MOUSE_VERSION > 1
			/* Scroll up */
			if (event.bstate == BUTTON4_PRESSED && ndents && (cfg.rollover || cur)) {
				move_cursor((cur + ndents - 1) % ndents, 0);
				break;
			}

			/* Scroll down */
			if (event.bstate == BUTTON5_PRESSED && ndents
			    && (cfg.rollover || (cur != ndents - 1))) {
				move_cursor((cur + 1) % ndents, 0);
				break;
			}
#endif

			/* Toggle filter mode on left click on last 2 lines */
			if (event.y >= xlines - 2 && event.bstate == BUTTON1_PRESSED) {
				clearfilter();
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
			if (event.y >= 2 && event.y <= ndents + 1 && event.bstate == BUTTON1_PRESSED) {
				r = curscroll + (event.y - 2);
				move_cursor(r, 1);
				currentmouse ^= 1;
				clock_gettime(
#if defined(CLOCK_MONOTONIC_RAW)
				    CLOCK_MONOTONIC_RAW,
#elif defined(CLOCK_MONOTONIC)
				    CLOCK_MONOTONIC,
#else
				    CLOCK_REALTIME,
#endif
				    &mousetimings[currentmouse]);

				/*Single click just selects, double click also opens */
				if (((_ABSSUB(mousetimings[0].tv_sec, mousetimings[1].tv_sec) << 30)
				  + (mousetimings[0].tv_nsec - mousetimings[1].tv_nsec))
					> DOUBLECLICK_INTERVAL_NS)
					break;
				mousetimings[currentmouse].tv_sec = 0;
			} else {
				if (cfg.filtermode)
					presel = FILTER;
				goto nochange; // fallthrough
			}
#endif
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
				clearfilter();
				setdirwatch();
				goto begin;
			case S_IFREG:
			{
				/* If opened as vim plugin and Enter/^M pressed, pick */
				if (cfg.picker && sel == SEL_GOIN) {
					appendfpath(newpath, mkpath(path, dents[cur].name, newpath));
					writesel(pselbuf, selbufpos - 1);
					free(mark);
					return _SUCCESS;
				}

				/* If open file is disabled on right arrow or `l`, return */
				if (cfg.nonavopen && sel == SEL_NAV_IN)
					goto nochange;

				/* Handle plugin selection mode */
				if (cfg.runplugin) {
					cfg.runplugin = 0;
					/* Must be in plugin dir and same context to select plugin */
					if ((cfg.runctx == cfg.curctx) && !strcmp(path, plugindir)) {
						endselection();
						/* Copy path so we can return back to earlier dir */
						xstrlcpy(path, rundir, PATH_MAX);
						rundir[0] = '\0';

						if (!run_selected_plugin(&path, dents[cur].name,
									 newpath, runfile, &lastname,
									 &lastdir)) {
							DPRINTF_S("plugin failed!");
						}

						if (runfile[0])
							runfile[0] = '\0';
						clearfilter();

						setdirwatch();
						goto begin;
					}
				}

				if (cfg.useeditor && (!sb.st_size ||
#ifdef FILE_MIME_OPTS
				    (get_output(g_buf, CMD_LEN_MAX, "file", FILE_MIME_OPTS, newpath, FALSE)
				    && !strncmp(g_buf, "text/", 5)))) {
#else
				    /* no mime option; guess from description instead */
				    (get_output(g_buf, CMD_LEN_MAX, "file", "-b", newpath, FALSE)
				    && strstr(g_buf, "text")))) {
#endif
					spawn(editor, newpath, NULL, path, F_CLI);
					continue;
				}

				if (!sb.st_size) {
					printwait(messages[MSG_EMPTY_FILE], &presel);
					goto nochange;
				}

#ifdef PCRE
				if (!pcre_exec(archive_pcre, NULL, dents[cur].name,
					       strlen(dents[cur].name), 0, 0, NULL, 0)) {
#else
				if (!regexec(&archive_re, dents[cur].name, 0, NULL, 0)) {
#endif
					r = get_input(messages[MSG_ARCHIVE_OPTS]);
					if (r == 'l' || r == 'x') {
						mkpath(path, dents[cur].name, newpath);
						handle_archive(newpath, path, r);
						copycurname();
						goto begin;
					}

					if (r == 'm') {
						if (archive_mount(dents[cur].name,
								  path, newpath, &presel)) {
							lastname[0] = '\0';

							/* Save last working directory */
							xstrlcpy(lastdir, path, PATH_MAX);

							/* Switch to mount point */
							xstrlcpy(path, newpath, PATH_MAX);

							setdirwatch();
							goto begin;
						} else {
							printwait(messages[MSG_FAILED], &presel);
							goto nochange;
						}
					}

					if (r != 'd') {
						printwait(messages[MSG_INVALID_KEY], &presel);
						goto nochange;
					}
				}

				/* Invoke desktop opener as last resort */
				spawn(opener, newpath, NULL, NULL, opener_flags);

				/* Move cursor to the next entry if not the last entry */
				if ((g_states & STATE_AUTONEXT) && cur != ndents - 1)
					move_cursor((cur + 1) % ndents, 0);

				continue;
			}
			default:
				printwait(messages[MSG_UNSUPPORTED], &presel);
				goto nochange;
			}
		case SEL_NEXT: // fallthrough
		case SEL_PREV: // fallthrough
		case SEL_PGDN: // fallthrough
		case SEL_CTRL_D: // fallthrough
		case SEL_PGUP: // fallthrough
		case SEL_CTRL_U: // fallthrough
		case SEL_HOME: // fallthrough
		case SEL_END: // fallthrough
		case SEL_FIRST:
			g_states |= STATE_MOVE_OP;
			handle_screen_move(sel);
			break;
		case SEL_CDHOME: // fallthrough
		case SEL_CDBEGIN: // fallthrough
		case SEL_CDLAST: // fallthrough
		case SEL_CDROOT:
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
			default: /* SEL_CDROOT */
				dir = "/";
				break;
			}

			if (!dir || !*dir) {
				printwait(messages[MSG_NOT_SET], &presel);
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
			clearfilter();
			DPRINTF_S(path);
			setdirwatch();
			goto begin;
		case SEL_BOOKMARK:
			r = xstrlcpy(g_buf, messages[MSG_BOOKMARK_KEYS], CMD_LEN_MAX);
			if (mark) { /* There is a pinned directory */
				g_buf[--r] = ' ';
				g_buf[++r] = ',';
				g_buf[++r] = '\0';
				++r;
			}
			printkeys(bookmark, g_buf + r - 1, BM_MAX);
			printprompt(g_buf);
			fd = get_input(NULL);

			r = FALSE;
			if (fd == ',') /* Visit pinned directory */
				mark ? xstrlcpy(newpath, mark, PATH_MAX) : (r = MSG_NOT_SET);
			else if (!get_kv_val(bookmark, newpath, fd, BM_MAX, TRUE))
				r = MSG_INVALID_KEY;

			if (!r && !xdiraccess(newpath))
				r = MSG_ACCESS;

			if (r) {
				printwait(messages[r], &presel);
				goto nochange;
			}

			if (strcmp(path, newpath) == 0)
				break;

			lastname[0] =  '\0';
			clearfilter();

			/* Save last working directory */
			xstrlcpy(lastdir, path, PATH_MAX);

			/* Save the newly opted dir in path */
			xstrlcpy(path, newpath, PATH_MAX);
			DPRINTF_S(path);

			setdirwatch();
			goto begin;
		case SEL_CYCLE: // fallthrough
		case SEL_CYCLER: // fallthrough
		case SEL_CTX1: // fallthrough
		case SEL_CTX2: // fallthrough
		case SEL_CTX3: // fallthrough
		case SEL_CTX4:
			r = handle_context_switch(sel, newpath);
			if (r < 0)
				continue;
			savecurctx(&cfg, path, dents[cur].name, r);

			/* Reset the pointers */
			path = g_ctx[r].c_path;
			lastdir = g_ctx[r].c_last;
			lastname = g_ctx[r].c_name;

			setdirwatch();
			goto begin;
		case SEL_PIN:
			free(mark);
			mark = strdup(path);
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
#elif defined(HAIKU_NM)
			if (haiku_nm_active) {
				haiku_stop_watch(haiku_hnd);
				haiku_nm_active = FALSE;
			}
#endif
			presel = filterentries(path, lastname);

			if (presel == 27) {
				presel = 0;
				break;
			}
			goto nochange;
		case SEL_MFLTR: // fallthrough
		case SEL_HIDDEN: // fallthrough
		case SEL_DETAIL: // fallthrough
		case SEL_SORT:
			switch (sel) {
			case SEL_MFLTR:
				cfg.filtermode ^= 1;
				if (cfg.filtermode) {
					presel = FILTER;
					clearfilter();
					goto nochange;
				}

				/* Start watching the directory */
				dir_changed = TRUE;
				break;
			case SEL_HIDDEN:
				cfg.showhidden ^= 1;
				if (ndents)
					copycurname();
				if (cfg.filtermode)
					presel = FILTER;
				clearfilter();
				goto begin;
			case SEL_DETAIL:
				cfg.showdetail ^= 1;
				cfg.showdetail ? (printptr = &printent_long) : (printptr = &printent);
				cfg.blkorder = 0;
				continue;
			default: /* SEL_SORT */
				if (!set_sort_flags()) {
					if (cfg.filtermode)
						presel = FILTER;
					printwait(messages[MSG_INVALID_KEY], &presel);
					goto nochange;
				}
			}

			if (cfg.filtermode)
				presel = FILTER;

			/* Save current */
			if (ndents)
				copycurname();

			/* If there's no filter, reload the directory */
			if (!g_ctx[cfg.curctx].c_fltr[1])
				goto begin;

			presel = FILTER; /* If there's a filter, apply it */
			break;
		case SEL_STATS: // fallthrough
		case SEL_CHMODX:
			if (ndents) {
				tmp = (g_listpath && xstrcmp(path, g_listpath) == 0) ? g_prefixpath : path;
				mkpath(tmp, dents[cur].name, newpath);

				if (lstat(newpath, &sb) == -1
				    || (sel == SEL_STATS && !show_stats(newpath, &sb))
				    || (sel == SEL_CHMODX && !xchmod(newpath, sb.st_mode))) {
					printwarn(&presel);
					goto nochange;
				}

				if (sel == SEL_CHMODX)
					dents[cur].mode ^= 0111;
			}
			break;
		case SEL_REDRAW: // fallthrough
		case SEL_RENAMEMUL: // fallthrough
		case SEL_HELP: // fallthrough
		case SEL_EDIT: // fallthrough
		case SEL_LOCK:
		{
			bool refresh = FALSE;

			if (ndents)
				mkpath(path, dents[cur].name, newpath);
			else if (sel == SEL_EDIT) /* Avoid trying to edit a non-existing file */
				goto nochange;

			switch (sel) {
			case SEL_REDRAW:
				refresh = TRUE;
				break;
			case SEL_RENAMEMUL:
				endselection();

				if (!batch_rename(path)) {
					printwait(messages[MSG_FAILED], &presel);
					goto nochange;
				}
				refresh = TRUE;
				break;
			case SEL_HELP:
				show_help(path);
				if (cfg.filtermode)
					presel = FILTER;
				continue;
			case SEL_EDIT:
				spawn(editor, dents[cur].name, NULL, path, F_CLI);
				continue;
			default: /* SEL_LOCK */
				lock_terminal();
				break;
			}

			/* In case of successful operation, reload contents */

			/* Continue in navigate-as-you-type mode, if enabled */
			if (cfg.filtermode && !refresh)
				break;

			/* Save current */
			if (ndents)
				copycurname();
			/* Repopulate as directory content may have changed */
			goto begin;
		}
		case SEL_SEL:
			if (!ndents)
				goto nochange;

			startselection();
			if (g_states & STATE_RANGESEL)
				g_states &= ~STATE_RANGESEL;

			/* Toggle selection status */
			dents[cur].flags ^= FILE_SELECTED;

			if (dents[cur].flags & FILE_SELECTED) {
				++nselected;
				appendfpath(newpath, mkpath(path, dents[cur].name, newpath));
				writesel(pselbuf, selbufpos - 1); /* Truncate NULL from end */
			} else {
				selbufpos = lastappendpos;
				if (--nselected) {
					updateselbuf(path, newpath);
					writesel(pselbuf, selbufpos - 1); /* Truncate NULL from end */
				} else
					writesel(NULL, 0);
			}

			if (cfg.x11)
				plugscript(utils[UTIL_CBCP], newpath, F_NOWAIT | F_NOTRACE);

			if (!nselected)
				unlink(g_selpath);

			/* move cursor to the next entry if this is not the last entry */
			if (!cfg.picker && cur != ndents - 1)
				move_cursor((cur + 1) % ndents, 0);
			break;
		case SEL_SELMUL:
			if (!ndents)
				goto nochange;

			startselection();
			g_states ^= STATE_RANGESEL;

			if (stat(path, &sb) == -1) {
				printwarn(&presel);
				goto nochange;
			}

			if (g_states & STATE_RANGESEL) { /* Range selection started */
#ifndef DIR_LIMITED_SELECTION
				inode = sb.st_ino;
#endif
				selstartid = cur;
				continue;
			}

#ifndef DIR_LIMITED_SELECTION
			if (inode != sb.st_ino) {
				printwait(messages[MSG_DIR_CHANGED], &presel);
				goto nochange;
			}
#endif
			if (cur < selstartid) {
				selendid = selstartid;
				selstartid = cur;
			} else
				selendid = cur;

			/* Clear selection on repeat on same file */
			if (selstartid == selendid) {
				resetselind();
				clearselection();
				break;
			} // fallthrough
		case SEL_SELALL:
			if (sel == SEL_SELALL) {
				if (!ndents)
					goto nochange;

				startselection();
				if (g_states & STATE_RANGESEL)
					g_states &= ~STATE_RANGESEL;

				selstartid = 0;
				selendid = ndents - 1;
			}

			/* Remember current selection buffer position */
			for (r = selstartid; r <= selendid; ++r)
				if (!(dents[r].flags & FILE_SELECTED)) {
					/* Write the path to selection file to avoid flush */
					appendfpath(newpath, mkpath(path, dents[r].name, newpath));

					dents[r].flags |= FILE_SELECTED;
					++nselected;
				}

			writesel(pselbuf, selbufpos - 1); /* Truncate NULL from end */
			if (cfg.x11)
				plugscript(utils[UTIL_CBCP], newpath, F_NOWAIT | F_NOTRACE);
			continue;
		case SEL_SELEDIT:
			r = editselection();
			if (r <= 0) {
				r = !r ? MSG_0_SELECTED : MSG_FAILED;
				printwait(messages[r], &presel);
			} else {
				if (cfg.x11)
					plugscript(utils[UTIL_CBCP], newpath, F_NOWAIT | F_NOTRACE);
				cfg.filtermode ?  presel = FILTER : statusbar(path);
			}
			goto nochange;
		case SEL_CP: // fallthrough
		case SEL_MV: // fallthrough
		case SEL_CPMVAS: // fallthrough
		case SEL_RM:
		{
			if (sel == SEL_RM) {
				r = get_cur_or_sel();
				if (!r) {
					statusbar(path);
					goto nochange;
				}

				if (r == 'c') {
					tmp = (g_listpath && xstrcmp(path, g_listpath) == 0) ? g_prefixpath : path;
					mkpath(tmp, dents[cur].name, newpath);
					xrm(newpath);

					if (cur && access(newpath, F_OK) == -1) {
						move_cursor(cur - 1, 0);
						copycurname();
					} else
						lastname[0] = '\0';

					if (cfg.filtermode)
						presel = FILTER;
					goto begin;
				}
			}

			endselection();

			if (!cpmvrm_selection(sel, path, &presel))
				goto nochange;

			clearfilter();

			/* Show notification on operation complete */
			if (cfg.x11)
				plugscript(utils[UTIL_NTFY], newpath, F_NOWAIT | F_NOTRACE);

			if (ndents)
				copycurname();
			goto begin;
		}
		case SEL_ARCHIVE: // fallthrough
		case SEL_OPENWITH: // fallthrough
		case SEL_NEW: // fallthrough
		case SEL_RENAME:
		{
			int dup = 'n';

			if (!ndents && (sel == SEL_OPENWITH || sel == SEL_RENAME))
				break;

			if (sel != SEL_OPENWITH)
				endselection();

			switch (sel) {
			case SEL_ARCHIVE:
				r = get_cur_or_sel();
				if (!r) {
					statusbar(path);
					goto nochange;
				}

				if (r == 's') {
					if (!selsafe()) {
						presel = MSGWAIT;
						goto nochange;
					}

					tmp = NULL;
				} else
					tmp = dents[cur].name;

				tmp = xreadline(tmp, messages[MSG_ARCHIVE_NAME]);
				break;
			case SEL_OPENWITH:
#ifdef NORL
				tmp = xreadline(NULL, messages[MSG_OPEN_WITH]);
#else
				presel = 0;
				tmp = getreadline(messages[MSG_OPEN_WITH], path, ipath, &presel);
				if (presel == MSGWAIT)
					goto nochange;
#endif
				break;
			case SEL_NEW:
				r = get_input(messages[MSG_NEW_OPTS]);
				if (r == 'f' || r == 'd')
					tmp = xreadline(NULL, messages[MSG_REL_PATH]);
				else if (r == 's' || r == 'h')
					tmp = xreadline(NULL, messages[MSG_LINK_PREFIX]);
				else
					tmp = NULL;
				break;
			default: /* SEL_RENAME */
				tmp = xreadline(dents[cur].name, "");
				break;
			}

			if (!tmp || !*tmp)
				break;

			/* Allow only relative, same dir paths */
			if (tmp[0] == '/'
			    || ((r != 'f' && r != 'd') && (xstrcmp(xbasename(tmp), tmp) != 0))) {
				printwait(messages[MSG_NO_TRAVERSAL], &presel);
				goto nochange;
			}

			switch (sel) {
			case SEL_ARCHIVE:
				if (r == 'c' && strcmp(tmp, dents[cur].name) == 0)
					goto nochange;

				mkpath(path, tmp, newpath);
				if (access(newpath, F_OK) == 0) {
					fd = get_input(messages[MSG_OVERWRITE]);
					if (fd != 'y' && fd != 'Y') {
						statusbar(path);
						goto nochange;
					}
				}
				get_archive_cmd(newpath, tmp);
				(r == 's') ? archive_selection(newpath, tmp, path)
					   : spawn(newpath, tmp, dents[cur].name,
						    path, F_NORMAL | F_MULTI);
				// fallthrough
			case SEL_OPENWITH:
				if (sel == SEL_OPENWITH) {
					/* Confirm if app is CLI or GUI */
					r = get_input(messages[MSG_CLI_MODE]);
					r = (r == 'c' ? F_CLI :
					     (r == 'g' ? F_NOWAIT | F_NOTRACE | F_MULTI : 0));
					if (!r) {
						cfg.filtermode ? presel = FILTER : statusbar(path);
						goto nochange;
					}
					mkpath(path, dents[cur].name, newpath);
					spawn(tmp, newpath, NULL, path, r);
				}

				if (cfg.filtermode)
					presel = FILTER;
				copycurname();
				goto begin;
			case SEL_RENAME:
				/* Skip renaming to same name */
				if (strcmp(tmp, dents[cur].name) == 0) {
					tmp = xreadline(dents[cur].name, messages[MSG_COPY_NAME]);
					if (strcmp(tmp, dents[cur].name) == 0)
						goto nochange;

					dup = 'd';
				}
				break;
			default: /* SEL_NEW */
				break;
			}

			/* Open the descriptor to currently open directory */
#ifdef O_DIRECTORY
			fd = open(path, O_RDONLY | O_DIRECTORY);
#else
			fd = open(path, O_RDONLY);
#endif
			if (fd == -1) {
				printwarn(&presel);
				goto nochange;
			}

			/* Check if another file with same name exists */
			if (faccessat(fd, tmp, F_OK, AT_SYMLINK_NOFOLLOW) != -1) {
				if (sel == SEL_RENAME) {
					/* Overwrite file with same name? */
					r = get_input(messages[MSG_OVERWRITE]);
					if (r != 'y' && r != 'Y') {
						close(fd);
						break;
					}
				} else {
					/* Do nothing in case of NEW */
					close(fd);
					printwait(messages[MSG_EXISTS], &presel);
					goto nochange;
				}
			}

			if (sel == SEL_RENAME) {
				/* Rename the file */
				if (dup == 'd')
					spawn("cp -rp", dents[cur].name, tmp, path, F_SILENT);
				else if (renameat(fd, dents[cur].name, fd, tmp) != 0) {
					close(fd);
					printwarn(&presel);
					goto nochange;
				}
				close(fd);
				xstrlcpy(lastname, tmp, NAME_MAX + 1);
			} else { /* SEL_NEW */
				close(fd); /* Use fd as tmp var */
				presel = 0;

				/* Check if it's a dir or file */
				if (r == 'f') {
					mkpath(path, tmp, newpath);
					fd = xmktree(newpath, FALSE);
				} else if (r == 'd') {
					mkpath(path, tmp, newpath);
					fd = xmktree(newpath, TRUE);
				} else if (r == 's' || r == 'h') {

					if (tmp[0] == '@' && tmp[1] == '\0')
						tmp[0] = '\0';
					fd = xlink(tmp, path, (ndents ? dents[cur].name : NULL),
						  newpath, &presel, r);
				}

				if (!fd)
					printwait(messages[MSG_FAILED], &presel);

				if (fd <= 0)
					goto nochange;

				if (r == 'f' || r == 'd')
					xstrlcpy(lastname, tmp, NAME_MAX + 1);
				else if (ndents) {
					if (cfg.filtermode)
						presel = FILTER;
					copycurname();
				}
			}

			goto begin;
		}
		case SEL_PLUGIN:
			/* Check if directory is accessible */
			if (!xdiraccess(plugindir)) {
				printwarn(&presel);
				goto nochange;
			}

			r = xstrlcpy(g_buf, messages[MSG_PLUGIN_KEYS], CMD_LEN_MAX);
			printkeys(plug, g_buf + r - 1, PLUGIN_MAX);
			printprompt(g_buf);
			r = get_input(NULL);
			if (r != '\r') {
				endselection();
				tmp = get_kv_val(plug, NULL, r, PLUGIN_MAX, FALSE);
				if (!tmp) {
					printwait(messages[MSG_INVALID_KEY], &presel);
					goto nochange;
				}

				if (tmp[0] == '-' && tmp[1]) {
					++tmp;
					r = FALSE; /* Do not refresh dir after completion */
				} else
					r = TRUE;

				if (!run_selected_plugin(&path, tmp, newpath,
							 (ndents ? dents[cur].name : NULL),
							 &lastname, &lastdir)) {
					printwait(messages[MSG_FAILED], &presel);
					goto nochange;
				}

				if (ndents)
					copycurname();

				if (!r) {
					cfg.filtermode ? presel = FILTER : statusbar(path);
					goto nochange;
				}
			} else { /* 'Return/Enter' enters the plugin directory */
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

					/* Otherwise, initiate choosing plugin again */
					cfg.runplugin = 1;
				}

				xstrlcpy(rundir, path, PATH_MAX);
				xstrlcpy(path, plugindir, PATH_MAX);
				if (ndents)
					xstrlcpy(runfile, dents[cur].name, NAME_MAX);
				cfg.runctx = cfg.curctx;
				lastname[0] = '\0';
			}
			setdirwatch();
			clearfilter();
			goto begin;
		case SEL_SHELL: // fallthrough
		case SEL_LAUNCH: // fallthrough
		case SEL_RUNCMD:
			endselection();

			switch (sel) {
			case SEL_SHELL:
				/* Set nnn nesting level */
				tmp = getenv(env_cfg[NNNLVL]);
				setenv(env_cfg[NNNLVL], xitoa((tmp ? atoi(tmp) : 0) + 1), 1);

				setenv(envs[ENV_NCUR], (ndents ? dents[cur].name : ""), 1);
				spawn(shell, NULL, NULL, path, F_CLI);
				break;
			case SEL_LAUNCH:
				launch_app(path, newpath);

				if (cfg.filtermode)
					presel = FILTER;
				goto nochange;
			default: /* SEL_RUNCMD */
#ifndef NORL
				if (cfg.picker) {
#endif
					tmp = xreadline(NULL, ">>> ");
#ifndef NORL
				} else {
					presel = 0;
					tmp = getreadline(">>> ", path, ipath, &presel);
					if (presel == MSGWAIT)
						goto nochange;
				}
#endif
				if (tmp && *tmp) // NOLINT
					prompt_run(tmp, (ndents ? dents[cur].name : ""), path);
				else
					goto nochange;
			}

			/* Continue in navigate-as-you-type mode, if enabled */
			if (cfg.filtermode)
				presel = FILTER;

			/* Save current */
			if (ndents)
				copycurname();
			/* Repopulate as directory content may have changed */
			goto begin;
		case SEL_REMOTE:
			if (sel == SEL_REMOTE && !remote_mount(newpath, &presel))
				goto nochange;

			lastname[0] = '\0';

			/* Save last working directory */
			xstrlcpy(lastdir, path, PATH_MAX);

			/* Switch to mount point */
			xstrlcpy(path, newpath, PATH_MAX);

			setdirwatch();
			goto begin;
		case SEL_UMOUNT:
			tmp = ndents ? dents[cur].name : NULL;
			unmount(tmp, newpath, &presel, path);
			goto nochange;
		case SEL_SESSIONS:
			r = get_input(messages[MSG_SSN_OPTS]);

			if (r == 's')
				save_session(FALSE, &presel);
			else if (r == 'l' || r == 'r') {
				if (load_session(NULL, &path, &lastdir, &lastname, r == 'r')) {
					setdirwatch();
					goto begin;
				}
			}

			statusbar(path);
			goto nochange;
		case SEL_AUTONEXT:
			g_states ^= STATE_AUTONEXT;
			goto nochange;
		case SEL_QUITCTX: // fallthrough
		case SEL_QUITCD: // fallthrough
		case SEL_QUIT:
		case SEL_QUITFAIL:
			if (sel == SEL_QUITCTX) {
				fd = cfg.curctx; /* fd used as tmp var */
				for (r = (fd + 1) & ~CTX_MAX;
				     (r != fd) && !g_ctx[r].c_cfg.ctxactive;
				     r = ((r + 1) & ~CTX_MAX)) {
				};

				if (r != fd) {
					bool selmode = cfg.selmode ? TRUE : FALSE;

					g_ctx[fd].c_cfg.ctxactive = 0;

					/* Switch to next active context */
					path = g_ctx[r].c_path;
					lastdir = g_ctx[r].c_last;
					lastname = g_ctx[r].c_name;

					/* Switch light/detail mode */
					if (cfg.showdetail != g_ctx[r].c_cfg.showdetail)
						/* Set the reverse */
						printptr = cfg.showdetail ?
								&printent : &printent_long;

					cfg = g_ctx[r].c_cfg;

					/* Continue selection mode */
					cfg.selmode = selmode;
					cfg.curctx = r;
					setdirwatch();
					goto begin;
				}
			} else if (!cfg.forcequit) {
				for (r = 0; r < CTX_MAX; ++r)
					if (r != cfg.curctx && g_ctx[r].c_cfg.ctxactive) {
						r = get_input(messages[MSG_QUIT_ALL]);
						break;
					}

				if (!(r == CTX_MAX || r == 'y' || r == 'Y'))
					break; // fallthrough
			}

			/* CD on Quit */
			/* In vim picker mode, clear selection and exit */
			/* Picker mode: reset buffer or clear file */
			if (sel == SEL_QUITCD || getenv("NNN_TMPFILE"))
				cfg.picker ? selbufpos = 0 : write_lastdir(path);
			free(mark);
			return sel == SEL_QUITFAIL ? _FAILURE : _SUCCESS;
		default:
			if (xlines != LINES || xcols != COLS)
				setdirwatch(); /* Terminal resized */
			else if (idletimeout && idle == idletimeout)
				lock_terminal(); /* Locker */
			else
				goto nochange;

			idle = 0;
			if (ndents)
				copycurname();
			goto begin;
		} /* switch (sel) */
	}
}

static char *make_tmp_tree(char **paths, ssize_t entries, const char *prefix)
{
	/* tmpdir holds the full path */
	/* tmp holds the path without the tmp dir prefix */
	int err, ignore = 0;
	struct stat sb;
	char *slash, *tmp;
	ssize_t i, len = strlen(prefix);
	char *tmpdir = malloc(sizeof(char) * (PATH_MAX + TMP_LEN_MAX));

	if (!tmpdir) {
		DPRINTF_S(strerror(errno));
		return NULL;
	}

	tmp = tmpdir + g_tmpfplen - 1;
	xstrlcpy(tmpdir, g_tmpfpath, g_tmpfplen);
	xstrlcpy(tmp, "/nnnXXXXXX", 11);

	/* Points right after the base tmp dir */
	tmp += 10;

	if (!mkdtemp(tmpdir)) {
		free(tmpdir);

		DPRINTF_S(strerror(errno));
		return NULL;
	}

	g_listpath = tmpdir;

	for (i = 0; i < entries; ++i) {
		if (!paths[i])
			continue;

		err = stat(paths[i], &sb);
		if (err && errno == ENOENT) {
			ignore = 1;
			continue;
		}

		/* Don't copy the common prefix */
		xstrlcpy(tmp, paths[i] + len, strlen(paths[i]) - len + 1);

		/* Get the dir containing the path */
		slash = xmemrchr((uchar *)tmp, '/', strlen(paths[i]) - len);
		if (slash)
			*slash = '\0';

		xmktree(tmpdir, TRUE);

		if (slash)
			*slash = '/';

		if (symlink(paths[i], tmpdir)) {
			DPRINTF_S(paths[i]);
			DPRINTF_S(strerror(errno));
		}
	}

	if (ignore)
		g_states |= STATE_MSG;

	/* Get the dir in which to start */
	*tmp = '\0';
	return tmpdir;
}

static char *load_input()
{
	/* 512 KiB chunk size */
	ssize_t i, chunk_count = 1, chunk = 512 * 1024, entries = 0;
	char *input = malloc(sizeof(char) * chunk), *tmpdir = NULL;
	char cwd[PATH_MAX], *next, *tmp;
	size_t offsets[LIST_FILES_MAX];
	char **paths = NULL;
	ssize_t input_read, total_read = 0, off = 0;

	if (!input) {
		DPRINTF_S(strerror(errno));
		return NULL;
	}

	if (!getcwd(cwd, PATH_MAX)) {
		free(input);
		return NULL;
	}

	while (chunk_count < 512) {
		input_read = read(STDIN_FILENO, input + total_read, chunk);
		if (input_read < 0) {
			DPRINTF_S(strerror(errno));
			goto malloc_1;
		}

		if (input_read == 0)
			break;

		total_read += input_read;
		++chunk_count;

		while (off < total_read) {
			next = memchr(input + off, '\0', total_read - off) + 1;
			if (next == (void *)1)
				break;

			if (next - input == off + 1) {
				off = next - input;
				continue;
			}

			if (entries == LIST_FILES_MAX)
				goto malloc_1;

			offsets[entries++] = off;
			off = next - input;
		}

		if (chunk_count == 512)
			goto malloc_1;

		/* We don't need to allocate another chunk */
		if (chunk_count == (total_read - input_read) / chunk)
			continue;

		chunk_count = total_read / chunk;
		if (total_read % chunk)
			++chunk_count;


		if (!(input = xrealloc(input, (chunk_count + 1) * chunk)))
			return NULL;
	}

	if (off != total_read) {
		if (entries == LIST_FILES_MAX)
			goto malloc_1;

		offsets[entries++] = off;
	}

	DPRINTF_D(entries);
	DPRINTF_D(total_read);
	DPRINTF_D(chunk_count);

	if (!entries)
		goto malloc_1;

	input[total_read] = '\0';

	paths = malloc(entries * sizeof(char *));
	if (!paths)
		goto malloc_1;

	for (i = 0; i < entries; ++i)
		paths[i] = input + offsets[i];

	g_prefixpath = malloc(sizeof(char) * PATH_MAX);
	if (!g_prefixpath)
		goto malloc_1;
	g_prefixpath[0] = '\0';

	DPRINTF_S(paths[0]);

	for (i = 0; i < entries; ++i) {
		if (paths[i][0] == '\n' || selforparent(paths[i])) {
			paths[i] = NULL;
			continue;
		}

		if (!(paths[i] = abspath(paths[i], cwd))) {
			entries = i; // free from the previous entry
			goto malloc_2;

		}

		DPRINTF_S(paths[i]);

		xstrlcpy(g_buf, paths[i], PATH_MAX);
		if (!common_prefix(dirname(g_buf), g_prefixpath)) {
			entries = i + 1; // free from the current entry
			goto malloc_2;
		}

		DPRINTF_S(g_prefixpath);
	}

	DPRINTF_S(g_prefixpath);

	if (g_prefixpath[0]) {
		if (entries == 1) {
			tmp = xmemrchr((uchar *)g_prefixpath, '/', strlen(g_prefixpath));
			if (!tmp)
				goto malloc_2;

			*(tmp != g_prefixpath ? tmp : tmp + 1) = '\0';
		}

		tmpdir = make_tmp_tree(paths, entries, g_prefixpath);
	}

malloc_2:
	for (i = entries - 1; i >= 0; --i)
		free(paths[i]);
malloc_1:
	free(input);
	free(paths);
	return tmpdir;
}

static void check_key_collision(void)
{
	int key;
	ulong i = 0;
	bool bitmap[KEY_MAX] = {FALSE};

	for (; i < sizeof(bindings) / sizeof(struct key); ++i) {
		key = bindings[i].sym;

		if (bitmap[key])
			fprintf(stdout, "key collision! [%s]\n", keyname(key));
		else
			bitmap[key] = TRUE;
	}
}

static void usage(void)
{
	fprintf(stdout,
		"%s: nnn [OPTIONS] [PATH]\n\n"
		"The missing terminal file manager for X.\n\n"
		"positional args:\n"
		"  PATH   start dir [default: .]\n\n"
		"optional args:\n"
		" -a      use access time\n"
		" -A      no dir auto-select\n"
		" -b key  open bookmark key\n"
		" -c      cli-only opener (overrides -e)\n"
		" -d      detail mode\n"
		" -e      text in $VISUAL ($EDITOR/vi)\n"
		" -E      use EDITOR for undetached edits\n"
		" -g      regex filters [default: string]\n"
		" -H      show hidden files\n"
		" -K      detect key collision\n"
		" -n      nav-as-you-type mode\n"
		" -o      open files only on Enter\n"
		" -p file selection file [stdout if '-']\n"
		" -Q      no quit confirmation\n"
		" -r      use advcpmv patched cp, mv\n"
		" -R      no rollover at edges\n"
		" -s name load session by name\n"
		" -S      du mode\n"
		" -t secs timeout to lock\n"
		" -v      version sort\n"
		" -V      show version\n"
		" -x      notis, sel to system clipboard\n"
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

		len = strlen(xdgcfg) + 1 + 13; /* add length of "/nnn/sessions" */
		xdg = TRUE;
	}

	if (!xdg)
		len = strlen(home) + 1 + 21; /* add length of "/.config/nnn/sessions" */

	cfgdir = (char *)malloc(len);
	plugindir = (char *)malloc(len);
	sessiondir = (char *)malloc(len);
	if (!cfgdir || !plugindir || !sessiondir) {
		xerror();
		return FALSE;
	}

	if (xdg) {
		xstrlcpy(cfgdir, xdgcfg, len);
		r = len - 13; /* subtract length of "/nnn/sessions" */
	} else {
		r = xstrlcpy(cfgdir, home, len);

		/* Create ~/.config */
		xstrlcpy(cfgdir + r - 1, "/.config", len - r);
		DPRINTF_S(cfgdir);
		r += 8; /* length of "/.config" */
	}

	/* Create ~/.config/nnn */
	xstrlcpy(cfgdir + r - 1, "/nnn", len - r);
	DPRINTF_S(cfgdir);

	/* Create ~/.config/nnn/plugins */
	xstrlcpy(cfgdir + r + 4 - 1, "/plugins", 9); /* subtract length of "/nnn" (4) */
	DPRINTF_S(cfgdir);

	xstrlcpy(plugindir, cfgdir, len);
	DPRINTF_S(plugindir);

	if (access(plugindir, F_OK) && !xmktree(plugindir, TRUE)) {
		xerror();
		return FALSE;
	}

	/* Create ~/.config/nnn/sessions */
	xstrlcpy(cfgdir + r + 4 - 1, "/sessions", 10); /* subtract length of "/nnn" (4) */
	DPRINTF_S(cfgdir);

	xstrlcpy(sessiondir, cfgdir, len);
	DPRINTF_S(sessiondir);

	if (access(sessiondir, F_OK) && !xmktree(sessiondir, TRUE)) {
		xerror();
		return FALSE;
	}

	/* Reset to config path */
	cfgdir[r + 3] = '\0';
	DPRINTF_S(cfgdir);

	/* Set selection file path */
	if (!cfg.picker) {
		/* Length of "/.config/nnn/.selection" */
		g_selpath = (char *)malloc(len + 3);
		if (!g_selpath) {
			xerror();
			return FALSE;
		}

		r = xstrlcpy(g_selpath, cfgdir, len + 3);
		xstrlcpy(g_selpath + r - 1, "/.selection", 12);
		DPRINTF_S(g_selpath);
	}

	return TRUE;
}

static bool set_tmp_path(void)
{
        char *tmp = "/tmp";
        char *path = xdiraccess(tmp) ? tmp : getenv("TMPDIR");

        if (!path) {
                fprintf(stderr, "set TMPDIR\n");
                return FALSE;
        }

        g_tmpfplen = (uchar)xstrlcpy(g_tmpfpath, path, TMP_LEN_MAX);
        return TRUE;
}

static void cleanup(void)
{
	free(g_selpath);
	free(plugindir);
	free(sessiondir);
	free(cfgdir);
	free(initpath);
	free(bmstr);
	free(pluginstr);
	free(g_prefixpath);

	unlink(g_pipepath);

#ifdef DBGMODE
	disabledbg();
#endif
}

int main(int argc, char *argv[])
{
	char *arg = NULL;
	char *session = NULL;
	int opt;
#ifndef NOMOUSE
	mmask_t mask;
#endif
	const char* const env_opts = xgetenv(env_cfg[NNN_OPTS], NULL);
	int env_opts_id = env_opts ? (int)strlen(env_opts) : -1;

	while ((opt = (env_opts_id > 0
		       ? env_opts[--env_opts_id]
		       : getopt(argc, argv, "aAb:cdeEgHKnop:QrRs:St:vVxh"))) != -1) {
		switch (opt) {
		case 'a':
			cfg.mtime = 0;
			break;
		case 'A':
			cfg.autoselect = 0;
			break;
		case 'b':
			arg = optarg;
			break;
		case 'c':
			cfg.cliopener = 1;
			break;
		case 'S':
			cfg.blkorder = 1;
			nftw_fn = sum_bsize;
			blk_shift = ffs(S_BLKSIZE) - 1; // fallthrough
		case 'd':
			cfg.showdetail = 1;
			printptr = &printent_long;
			break;
		case 'e':
			cfg.useeditor = 1;
			break;
		case 'E':
			cfg.waitedit = 1;
			break;
		case 'g':
			cfg.regex = 1;
			filterfn = &visible_re;
			break;
		case 'H':
			cfg.showhidden = 1;
			break;
		case 'K':
			check_key_collision();
			return _SUCCESS;
		case 'n':
			cfg.filtermode = 1;
			break;
		case 'o':
			cfg.nonavopen = 1;
			break;
		case 'p':
			if (env_opts_id >= 0)
				break;

			cfg.picker = 1;
			if (optarg[0] == '-' && optarg[1] == '\0')
				cfg.pickraw = 1;
			else {
				int fd = open(optarg, O_WRONLY | O_CREAT, 0600);

				if (fd == -1) {
					xerror();
					return _FAILURE;
				}

				close(fd);
				g_selpath = realpath(optarg, NULL);
				unlink(g_selpath);
			}
			break;
		case 'Q':
			cfg.forcequit = 1;
			break;
		case 'r':
#ifdef __linux__
			cp[2] = cp[5] = mv[2] = mv[5] = 'g'; /* cp -iRp -> cpg -giRp */
			cp[4] = mv[4] = '-';
#endif
			break;
		case 'R':
			cfg.rollover = 0;
			break;
		case 's':
			if (env_opts_id < 0)
				session = optarg;
			break;
		case 't':
			if (env_opts_id < 0)
				idletimeout = atoi(optarg);
			break;
		case 'v':
			namecmpfn = &xstrverscasecmp;
			break;
		case 'V':
			fprintf(stdout, "%s\n", VERSION);
			return _SUCCESS;
		case 'x':
			cfg.x11 = 1;
			break;
		case 'h':
			usage();
			return _SUCCESS;
		default:
			usage();
			return _FAILURE;
		}
	}

#ifdef DBGMODE
	enabledbg();
	DPRINTF_S(VERSION);
#endif

	/* Prefix for temporary files */
	if (!set_tmp_path())
		return _FAILURE;

	atexit(cleanup);

	if (!cfg.picker) {
		/* Confirm we are in a terminal */
		if (!isatty(STDOUT_FILENO))
			exit(1);

		/* Now we are in path list mode */
		if (!isatty(STDIN_FILENO)) {
			/* This is the same as g_listpath */
			initpath = load_input();
			if (!initpath)
				exit(1);

			/* We return to tty */
			dup2(STDOUT_FILENO, STDIN_FILENO);
		}
	}

	home = getenv("HOME");
	if (!home) {
		fprintf(stderr, "set HOME\n");
		return _FAILURE;
	}
	DPRINTF_S(home);

	if (!setup_config())
		return _FAILURE;

	/* Get custom opener, if set */
	opener = xgetenv(env_cfg[NNN_OPENER], utils[UTIL_OPENER]);
	DPRINTF_S(opener);

	/* Parse bookmarks string */
	if (!parsekvpair(bookmark, &bmstr, env_cfg[NNN_BMS], BM_MAX, PATH_MAX)) {
		fprintf(stderr, "%s\n", env_cfg[NNN_BMS]);
		return _FAILURE;
	}

	/* Parse plugins string */
	if (!parsekvpair(plug, &pluginstr, env_cfg[NNN_PLUG], PLUGIN_MAX, PATH_MAX)) {
		fprintf(stderr, "%s\n", env_cfg[NNN_PLUG]);
		return _FAILURE;
	}

	if (!initpath) {
		if (arg) { /* Open a bookmark directly */
			if (!arg[1]) /* Bookmarks keys are single char */
				initpath = get_kv_val(bookmark, NULL, *arg, BM_MAX, TRUE);

			if (!initpath) {
				fprintf(stderr, "%s\n", messages[MSG_INVALID_KEY]);
				return _FAILURE;
			}
		} else if (argc == optind) {
			/* Start in the current directory */
			initpath = getcwd(NULL, PATH_MAX);
			if (!initpath)
				initpath = "/";
		} else {
			arg = argv[optind];
			DPRINTF_S(arg);
			if (strlen(arg) > 7 && !strncmp(arg, "file://", 7))
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
				spawn(opener, arg, NULL, NULL, cfg.cliopener ? F_CLI : F_NOTRACE | F_NOWAIT);
				return _SUCCESS;
			}
		}
	}

	/* Set archive handling (enveditor used as tmp var) */
	enveditor = getenv(env_cfg[NNN_ARCHIVE]);
#ifdef PCRE
	if (setfilter(&archive_pcre, (enveditor ? enveditor : archive_regex))) {
#else
	if (setfilter(&archive_re, (enveditor ? enveditor : archive_regex))) {
#endif
		fprintf(stderr, "%s\n", messages[MSG_INVALID_REG]);
		return _FAILURE;
	}

	/* An all-CLI opener overrides option -e) */
	if (cfg.cliopener)
		cfg.useeditor = 0;

	/* Get VISUAL/EDITOR */
	enveditor = xgetenv(envs[ENV_EDITOR], utils[UTIL_VI]);
	editor = xgetenv(envs[ENV_VISUAL], enveditor);
	DPRINTF_S(getenv(envs[ENV_VISUAL]));
	DPRINTF_S(getenv(envs[ENV_EDITOR]));
	DPRINTF_S(editor);

	/* Get PAGER */
	pager = xgetenv(envs[ENV_PAGER], utils[UTIL_LESS]);
	DPRINTF_S(pager);

	/* Get SHELL */
	shell = xgetenv(envs[ENV_SHELL], utils[UTIL_SH]);
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
#elif defined(HAIKU_NM)
	haiku_hnd = haiku_init_nm();
	if (!haiku_hnd) {
		xerror();
		return _FAILURE;
	}
#endif

	/* Configure trash preference */
	if (xgetenv_set(env_cfg[NNN_TRASH]))
		cfg.trash = 1;

	/* Ignore/handle certain signals */
	struct sigaction act = {.sa_handler = sigint_handler};

	if (sigaction(SIGINT, &act, NULL) < 0) {
		xerror();
		return _FAILURE;
	}
	signal(SIGQUIT, SIG_IGN);

#ifndef NOLOCALE
	/* Set locale */
	setlocale(LC_ALL, "");
#ifdef PCRE
	tables = pcre_maketables();
#endif
#endif

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
	mkpath(cfgdir, ".history", g_buf);
	read_history(g_buf);
#endif

#ifndef NOMOUSE
	if (!initcurses(&mask))
#else
	if (!initcurses(NULL))
#endif
		return _FAILURE;

	opt = browse(initpath, session);

#ifndef NOMOUSE
	mousemask(mask, NULL);
#endif

	if (g_listpath)
		spawn("rm -rf", initpath, NULL, NULL, F_SILENT);

	exitcurses();

#ifndef NORL
	mkpath(cfgdir, ".history", g_buf);
	write_history(g_buf);
#endif

	if (cfg.pickraw) {
		if (selbufpos && (seltofile(1, NULL) != (size_t)(selbufpos)))
			xerror();
	} else if (cfg.picker) {
		if (selbufpos)
			writesel(pselbuf, selbufpos - 1);
	} else if (g_selpath)
		unlink(g_selpath);

	/* Free the regex */
#ifdef PCRE
	pcre_free(archive_pcre);
#else
	regfree(&archive_re);
#endif

	/* Free the selection buffer */
	free(pselbuf);

#ifdef LINUX_INOTIFY
	/* Shutdown inotify */
	if (inotify_wd >= 0)
		inotify_rm_watch(inotify_fd, inotify_wd);
	close(inotify_fd);
#elif defined(BSD_KQUEUE)
	if (event_fd >= 0)
		close(event_fd);
	close(kq);
#elif defined(HAIKU_NM)
	haiku_close_nm(haiku_hnd);
#endif

	return opt;
}
