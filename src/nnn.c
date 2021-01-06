/*
 * BSD 2-Clause License
 *
 * Copyright (C) 2014-2016, Lazaros Koromilas <lostd@2f30.org>
 * Copyright (C) 2014-2016, Dimitris Papastamos <sin@2f30.org>
 * Copyright (C) 2016-2021, Arun Prakash Jana <engineerarun@gmail.com>
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

#if defined(__linux__) || defined(MINGW) || defined(__MINGW32__) || defined(__MINGW64__) || defined(__CYGWIN__)
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#if defined(__arm__) || defined(__i386__)
#define _FILE_OFFSET_BITS 64 /* Support large files on 32-bit */
#endif
#if defined(__linux__)
#include <sys/inotify.h>
#define LINUX_INOTIFY
#endif
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
#include <string.h>
#include <strings.h>
#include <time.h>
#include <unistd.h>
#ifndef __USE_XOPEN_EXTENDED
#define __USE_XOPEN_EXTENDED 1
#endif
#include <ftw.h>
#include <wchar.h>
#include <pwd.h>
#include <grp.h>

#if !defined(alloca) && defined(__GNUC__)
/*
 * GCC doesn't expand alloca() to __builtin_alloca() in standards mode
 * (-std=...) and not all standard libraries do or supply it, e.g.
 * NetBSD/arm64 so explicitly use the builtin.
 */
#define alloca(size) __builtin_alloca(size)
#endif

#include "nnn.h"
#include "dbg.h"

#if defined(ICONS) || defined(NERD)
#include "icons.h"
#define ICONS_ENABLED
#endif

#ifdef TOURBIN_QSORT
#include "qsort.h"
#endif

/* Macro definitions */
#define VERSION "3.5"
#define GENERAL_INFO "BSD 2-Clause\nhttps://github.com/jarun/nnn"

#ifndef NOSSN
#define SESSIONS_VERSION 1
#endif

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
#define SELECT ' '
#define REGEX_MAX 48
#define ENTRY_INCR 64 /* Number of dir 'entry' structures to allocate per shot */
#define NAMEBUF_INCR 0x800 /* 64 dir entries at once, avg. 32 chars per filename = 64*32B = 2KB */
#define DESCRIPTOR_LEN 32
#define _ALIGNMENT 0x10 /* 16-byte alignment */
#define _ALIGNMENT_MASK 0xF
#define TMP_LEN_MAX 64
#define DOT_FILTER_LEN 7
#define ASCII_MAX 128
#define EXEC_ARGS_MAX 8
#define LIST_FILES_MAX (1 << 16)
#define SCROLLOFF 3

#ifndef CTX8
#define CTX_MAX 4
#else
#define CTX_MAX 8
#endif

#ifdef __APPLE__
#define SED "gsed"
#else
#define SED "sed"
#endif

#define MIN_DISPLAY_COLS ((CTX_MAX * 2) + 2) /* Two chars for [ and ] */
#define ARCHIVE_CMD_LEN 16
#define BLK_SHIFT_512 9

/* Detect hardlinks in du */
#define HASH_BITS (0xFFFFFF)
#define HASH_OCTETS (HASH_BITS >> 6) /* 2^6 = 64 */

/* Entry flags */
#define DIR_OR_LINK_TO_DIR 0x01
#define HARD_LINK 0x02
#define SYM_ORPHAN 0x04
#define FILE_MISSING 0x08
#define FILE_SELECTED 0x10

/* Macros to define process spawn behaviour as flags */
#define F_NONE    0x00  /* no flag set */
#define F_MULTI   0x01  /* first arg can be combination of args; to be used with F_NORMAL */
#define F_NOWAIT  0x02  /* don't wait for child process (e.g. file manager) */
#define F_NOTRACE 0x04  /* suppress stdout and strerr (no traces) */
#define F_NORMAL  0x08  /* spawn child process in non-curses regular CLI mode */
#define F_CONFIRM 0x10  /* run command - show results before exit (must have F_NORMAL) */
#define F_CHKRTN  0x20  /* wait for user prompt if cmd returns failure status */
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
typedef unsigned int uint_t;
typedef unsigned char uchar_t;
typedef unsigned short ushort_t;
typedef unsigned long long ulong_t;

/* STRUCTURES */

/* Directory entry */
typedef struct entry {
	char *name;
	time_t t;
	off_t size;
	blkcnt_t blocks; /* number of 512B blocks allocated */
	mode_t mode;
#ifndef NOUG
	uid_t uid;
	gid_t gid;
#endif
	ushort_t nlen; /* Length of file name */
	uchar_t flags; /* Flags specific to the file */
} *pEntry;

/* Key-value pairs from env */
typedef struct {
	int key;
	int off;
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
	uint_t filtermode : 1;  /* Set to enter filter mode */
	uint_t timeorder  : 1;  /* Set to sort by time */
	uint_t sizeorder  : 1;  /* Set to sort by file size */
	uint_t apparentsz : 1;  /* Set to sort by apparent size (disk usage) */
	uint_t blkorder   : 1;  /* Set to sort by blocks used (disk usage) */
	uint_t extnorder  : 1;  /* Order by extension */
	uint_t showhidden : 1;  /* Set to show hidden files */
	uint_t reserved0  : 1;
	uint_t showdetail : 1;  /* Clear to show lesser file info */
	uint_t ctxactive  : 1;  /* Context active or not */
	uint_t reverse    : 1;  /* Reverse sort */
	uint_t version    : 1;  /* Version sort */
	uint_t reserved1  : 1;
	/* The following settings are global */
	uint_t curctx     : 3;  /* Current context number */
	uint_t prefersel  : 1;  /* Prefer selection over current, if exists */
	uint_t reserved2  : 1;
	uint_t nonavopen  : 1;  /* Open file on right arrow or `l` */
	uint_t autoselect : 1;  /* Auto-select dir in type-to-nav mode */
	uint_t cursormode : 1;  /* Move hardware cursor with selection */
	uint_t useeditor  : 1;  /* Use VISUAL to open text files */
	uint_t reserved3  : 3;
	uint_t regex      : 1;  /* Use regex filters */
	uint_t x11        : 1;  /* Copy to system clipboard and show notis */
	uint_t timetype   : 2;  /* Time sort type (0: access, 1: change, 2: modification) */
	uint_t cliopener  : 1;  /* All-CLI app opener */
	uint_t waitedit   : 1;  /* For ops that can't be detached, used EDITOR */
	uint_t rollover   : 1;  /* Roll over at edges */
} settings;

/* Non-persistent program-internal states */
typedef struct {
	uint_t pluginit   : 1;  /* Plugin framework initialized */
	uint_t interrupt  : 1;  /* Program received an interrupt */
	uint_t rangesel   : 1;  /* Range selection on */
	uint_t move       : 1;  /* Move operation */
	uint_t autonext   : 1;  /* Auto-proceed on open */
	uint_t fortune    : 1;  /* Show fortune messages in help */
	uint_t trash      : 2;  /* Use trash to delete files 1: trash-cli, 2: gio trash */
	uint_t forcequit  : 1;  /* Do not prompt on quit */
	uint_t autofifo   : 1;  /* Auto-create NNN_FIFO */
	uint_t initfile   : 1;  /* Positional arg is a file */
	uint_t dircolor   : 1;  /* Current status of dir color */
	uint_t picker     : 1;  /* Write selection to user-specified file */
	uint_t pickraw    : 1;  /* Write selection to stdout before exit */
	uint_t runplugin  : 1;  /* Choose plugin mode */
	uint_t runctx     : 2;  /* The context in which plugin is to be run */
	uint_t selmode    : 1;  /* Set when selecting files */
	uint_t oldcolor   : 1;  /* Use older colorscheme */
	uint_t stayonsel	: 1;  /* Disable auto-proceed on select */
	uint_t dirctx     : 1;  /* Show dirs in context color */
	uint_t uidgid     : 1;  /* Show owner and group info */
	uint_t reserved   : 10; /* Adjust when adding/removing a field */
} runstate;

/* Contexts or workspaces */
typedef struct {
	char c_path[PATH_MAX]; /* Current dir */
	char c_last[PATH_MAX]; /* Last visited dir */
	char c_name[NAME_MAX + 1]; /* Current file name */
	char c_fltr[REGEX_MAX]; /* Current filter */
	settings c_cfg; /* Current configuration */
	uint_t color; /* Color code for directories */
} context;

#ifndef NOSSN
typedef struct {
	size_t ver;
	size_t pathln[CTX_MAX];
	size_t lastln[CTX_MAX];
	size_t nameln[CTX_MAX];
	size_t fltrln[CTX_MAX];
} session_header_t;
#endif

/* GLOBALS */

/* Configuration, contexts */
static settings cfg = {
	0, /* filtermode */
	0, /* timeorder */
	0, /* sizeorder */
	0, /* apparentsz */
	0, /* blkorder */
	0, /* extnorder */
	0, /* showhidden */
	0, /* reserved0 */
	0, /* showdetail */
	1, /* ctxactive */
	0, /* reverse */
	0, /* version */
	0, /* reserved1 */
	0, /* curctx */
	0, /* prefersel */
	0, /* reserved2 */
	0, /* nonavopen */
	1, /* autoselect */
	0, /* cursormode */
	0, /* useeditor */
	0, /* reserved3 */
	0, /* regex */
	0, /* x11 */
	2, /* timetype (T_MOD) */
	0, /* cliopener */
	0, /* waitedit */
	1, /* rollover */
};

static context g_ctx[CTX_MAX] __attribute__ ((aligned));

static int ndents, cur, last, curscroll, last_curscroll, total_dents = ENTRY_INCR, scroll_lines = 1;
static int nselected;
#ifndef NOFIFO
static int fifofd = -1;
#endif
static uint_t idletimeout, selbufpos, lastappendpos, selbuflen;
static ushort_t xlines, xcols;
static ushort_t idle;
static uchar_t maxbm, maxplug;
static char *bmstr;
static char *pluginstr;
static char *opener;
static char *editor;
static char *enveditor;
static char *pager;
static char *shell;
static char *home;
static char *initpath;
static char *cfgpath;
static char *selpath;
static char *listpath;
static char *listroot;
static char *plgpath;
static char *pnamebuf, *pselbuf;
static char *mark;
#ifndef NOFIFO
static char *fifopath;
#endif
static unsigned long long *ihashbmp;
static struct entry *pdents;
static blkcnt_t ent_blocks;
static blkcnt_t dir_blocks;
static ulong_t num_files;
static kv *bookmark;
static kv *plug;
static uchar_t tmpfplen;
static uchar_t blk_shift = BLK_SHIFT_512;
#ifndef NOMOUSE
static int middle_click_key;
#endif
#ifdef PCRE
static pcre *archive_pcre;
#else
static regex_t archive_re;
#endif

/* Retain old signal handlers */
static struct sigaction oldsighup;
static struct sigaction oldsigtstp;

/* For use in functions which are isolated and don't return the buffer */
static char g_buf[CMD_LEN_MAX] __attribute__ ((aligned));

/* Buffer to store tmp file path to show selection, file stats and help */
static char g_tmpfpath[TMP_LEN_MAX] __attribute__ ((aligned));

/* Buffer to store plugins control pipe location */
static char g_pipepath[TMP_LEN_MAX] __attribute__ ((aligned));

/* Non-persistent runtime states */
static runstate g_state;

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
#define UTIL_LAUNCH 6
#define UTIL_SH_EXEC 7
#define UTIL_BASH 8
#define UTIL_SSHFS 9
#define UTIL_RCLONE 10
#define UTIL_VI 11
#define UTIL_LESS 12
#define UTIL_SH 13
#define UTIL_FZF 14
#define UTIL_NTFY 15
#define UTIL_CBCP 16
#define UTIL_NMV 17

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
	"launch",
	"sh -c",
	"bash",
	"sshfs",
	"rclone",
	"vi",
	"less",
	"sh",
	"fzf",
	".ntfy",
	".cbcp",
	".nmv",
};

/* Common strings */
#define MSG_INVALID_KEY 0
#define MSG_0_ENTRIES 1
#define STR_TMPFILE 2
#define MSG_0_SELECTED 3
#define MSG_CANCEL 4
#define MSG_FAILED 5
#define MSG_SSN_NAME 6
#define MSG_CP_MV_AS 7
#define MSG_CUR_SEL_OPTS 8
#define MSG_FORCE_RM 9
#define MSG_LIMIT 10
#define MSG_NEW_OPTS 11
#define MSG_CLI_MODE 12
#define MSG_OVERWRITE 13
#define MSG_SSN_OPTS 14
#define MSG_QUIT_ALL 15
#define MSG_HOSTNAME 16
#define MSG_ARCHIVE_NAME 17
#define MSG_OPEN_WITH 18
#define MSG_NEW_PATH 19
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
#define MSG_FIRST 39
#define MSG_RM_TMP 40
#define MSG_NOCHNAGE 41
#ifndef DIR_LIMITED_SELECTION
#define MSG_DIR_CHANGED 42 /* Must be the last entry */
#endif

static const char * const messages[] = {
	"invalid key",
	"0 entries",
	"/.nnnXXXXXX",
	"0 selected",
	"cancelled",
	"failed!",
	"session name: ",
	"'c'p / 'm'v as?",
	"'c'urrent / 's'el?",
	"rm -rf %s file%s? [Esc cancels]",
	"limit exceeded",
	"'f'ile / 'd'ir / 's'ym / 'h'ard?",
	"'c'li / 'g'ui?",
	"overwrite?",
	"'s'ave / 'l'oad / 'r'estore?",
	"Quit all contexts?",
	"remote name ('-' for hovered): ",
	"archive [path/]name: ",
	"open with: ",
	"[path/]name: ",
	"link prefix [@ for none]: ",
	"copy [path/]name: ",
	"\n'Enter' to continue",
	"open failed",
	"dir inaccessible",
	"empty: edit/open with",
	"unknown",
	"not set",
	"entry exists",
	"too few columns!",
	"'s'shfs / 'r'clone?",
	"refresh if slow",
	"app name: ",
	"'d'efault / e'x'tract / 'l'ist / 'm'ount?",
	"plugin keys:",
	"bookmark keys:",
	"invalid regex",
	"'a'u / 'd'u / 'e'xtn / 'r'ev / 's'ize / 't'ime / 'v'er / 'c'lr / '^T' (cycle)?",
	"unmount failed! try lazy?",
	"first file (\')/char?",
	"remove tmp file?",
	"unchanged",
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
#define NNN_MCLICK 7
#define NNN_SEL 8
#define NNN_ARCHIVE 9 /* strings end here */
#define NNN_TRASH 10 /* flags begin here */

static const char * const env_cfg[] = {
	"NNN_OPTS",
	"NNN_BMS",
	"NNN_PLUG",
	"NNN_OPENER",
	"NNN_COLORS",
	"NNNLVL",
	"NNN_PIPE",
	"NNN_MCLICK",
	"NNN_SEL",
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

/* Time type used */
#define T_ACCESS 0
#define T_CHANGE 1
#define T_MOD 2

#ifdef __linux__
static char cp[] = "cp   -iRp";
static char mv[] = "mv   -i";
#else
static char cp[] = "cp -iRp";
static char mv[] = "mv -i";
#endif

/* Archive commands */
const char *archive_cmd[] = {"atool -a", "bsdtar -acvf", "zip -r", "tar -acvf"};

/* Tokens used for path creation */
#define TOK_SSN 0
#define TOK_MNT 1
#define TOK_PLG 2

static const char * const toks[] = {
	"sessions",
	"mounts",
	"plugins", /* must be the last entry */
};

/* Patterns */
#define P_CPMVFMT 0
#define P_CPMVRNM 1
#define P_ARCHIVE 2
#define P_REPLACE 3

static const char * const patterns[] = {
	SED" -i 's|^\\(\\(.*/\\)\\(.*\\)$\\)|#\\1\\n\\3|' %s",
	SED" 's|^\\([^#/][^/]\\?.*\\)$|%s/\\1|;s|^#\\(/.*\\)$|\\1|' "
		"%s | tr '\\n' '\\0' | xargs -0 -n2 sh -c '%s \"$0\" \"$@\" < /dev/tty'",
	"\\.(bz|bz2|gz|tar|taz|tbz|tbz2|tgz|z|zip)$",
	SED" -i 's|^%s\\(.*\\)$|%s\\1|' %s",
};

/* Colors */
#define C_BLK (CTX_MAX + 1) /* Block device: DarkSeaGreen1 */
#define C_CHR (C_BLK + 1) /* Character device: Yellow1 */
#define C_DIR (C_CHR + 1) /* Directory: DeepSkyBlue1 */
#define C_EXE (C_DIR + 1) /* Executable file: Green1 */
#define C_FIL (C_EXE + 1) /* Regular file: Normal */
#define C_HRD (C_FIL + 1) /* Hard link: Plum4 */
#define C_LNK (C_HRD + 1) /* Symbolic link: Cyan1 */
#define C_MIS (C_LNK + 1) /* Missing file OR file details: Grey62 */
#define C_ORP (C_MIS + 1) /* Orphaned symlink: DeepPink1 */
#define C_PIP (C_ORP + 1) /* Named pipe (FIFO): Orange1 */
#define C_SOC (C_PIP + 1) /* Socket: MediumOrchid1 */
#define C_UND (C_SOC + 1) /* Unknown OR 0B regular/exe file: Red1 */

#ifdef ICONS_ENABLED
/* 0-9, A-Z, OTHER = 36. */
static ushort_t icon_positions[37];
#endif

static char gcolors[] = "c1e2272e006033f7c6d6abc4";
static uint_t fcolors[C_UND + 1] = {0};

/* Event handling */
#ifdef LINUX_INOTIFY
#define NUM_EVENT_SLOTS 32 /* Make room for 32 events */
#define EVENT_SIZE (sizeof(struct inotify_event))
#define EVENT_BUF_LEN (EVENT_SIZE * NUM_EVENT_SLOTS)
static int inotify_fd, inotify_wd = -1;
static uint_t INOTIFY_MASK = /* IN_ATTRIB | */ IN_CREATE | IN_DELETE | IN_DELETE_SELF
			   | IN_MODIFY | IN_MOVE_SELF | IN_MOVED_FROM | IN_MOVED_TO;
#elif defined(BSD_KQUEUE)
#define NUM_EVENT_SLOTS 1
#define NUM_EVENT_FDS 1
static int kq, event_fd = -1;
static struct kevent events_to_monitor[NUM_EVENT_FDS];
static uint_t KQUEUE_FFLAGS = NOTE_DELETE | NOTE_EXTEND | NOTE_LINK
			    | NOTE_RENAME | NOTE_REVOKE | NOTE_WRITE;
static struct timespec gtimeout;
#elif defined(HAIKU_NM)
static bool haiku_nm_active = FALSE;
static haiku_nm_h haiku_hnd;
#endif

/* Function macros */
#define tolastln() move(xlines - 1, 0)
#define tocursor() move(cur + 2 - curscroll, 0)
#define exitcurses() endwin()
#define printwarn(presel) printwait(strerror(errno), presel)
#define istopdir(path) ((path)[1] == '\0' && (path)[0] == '/')
#define copycurname() xstrsncpy(lastname, pdents[cur].name, NAME_MAX + 1)
#define settimeout() timeout(1000)
#define cleartimeout() timeout(-1)
#define errexit() printerr(__LINE__)
#define setdirwatch() (cfg.filtermode ? (presel = FILTER) : (watch = TRUE))
#define filterset() (g_ctx[cfg.curctx].c_fltr[1])
/* We don't care about the return value from strcmp() */
#define xstrcmp(a, b)  (*(a) != *(b) ? -1 : strcmp((a), (b)))
/* A faster version of xisdigit */
#define xisdigit(c) ((unsigned int) (c) - '0' <= 9)
#define xerror() perror(xitoa(__LINE__))

#ifdef TOURBIN_QSORT
#define ENTLESS(i, j) (entrycmpfn(pdents + (i), pdents + (j)) < 0)
#define ENTSWAP(i, j) (swap_ent((i), (j)))
#define ENTSORT(pdents, ndents, entrycmpfn) QSORT((ndents), ENTLESS, ENTSWAP)
#else
#define ENTSORT(pdents, ndents, entrycmpfn) qsort((pdents), (ndents), sizeof(*(pdents)), (entrycmpfn))
#endif

#ifdef __GNUC__
#define UNUSED(x) UNUSED_##x __attribute__((__unused__))
#else
#define UNUSED(x) UNUSED_##x
#endif /* __GNUC__ */

/* Forward declarations */
static void redraw(char *path);
static int spawn(char *file, char *arg1, char *arg2, uchar_t flag);
static int (*nftw_fn)(const char *fpath, const struct stat *sb, int typeflag, struct FTW *ftwbuf);
static void move_cursor(int target, int ignore_scrolloff);
static char *load_input(int fd, const char *path);
static int set_sort_flags(int r);
#ifndef NOFIFO
static void notify_fifo(bool force);
#endif

/* Functions */

static void sigint_handler(int UNUSED(sig))
{
	g_state.interrupt = 1;
}

static void clean_exit_sighandler(int UNUSED(sig))
{
	exitcurses();
	/* This triggers cleanup() thanks to atexit() */
	exit(EXIT_SUCCESS);
}

static char *xitoa(uint_t val)
{
	static char ascbuf[32] = {0};
	int i = 30;
	uint_t rem;

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

/* Return the integer value of a char representing HEX */
static uchar_t xchartohex(uchar_t c)
{
	if (xisdigit(c))
		return c - '0';

	if (c >= 'a' && c <= 'f')
		return c - 'a' + 10;

	if (c >= 'A' && c <= 'F')
		return c - 'A' + 10;

	return c;
}

/*
 * Source: https://elixir.bootlin.com/linux/latest/source/arch/alpha/include/asm/bitops.h
 */
static bool test_set_bit(uint_t nr)
{
	nr &= HASH_BITS;

	unsigned long long *m = ((unsigned long long *)ihashbmp) + (nr >> 6);

	if (*m & (1 << (nr & 63)))
		return FALSE;

	*m |= 1 << (nr & 63);

	return TRUE;
}

/* Increase the limit on open file descriptors, if possible */
static rlim_t max_openfds(void)
{
	struct rlimit rl;
	rlim_t limit = getrlimit(RLIMIT_NOFILE, &rl);

	if (!limit) {
		limit = rl.rlim_cur;
		rl.rlim_cur = rl.rlim_max;

		/* Return ~75% of max possible */
		if (setrlimit(RLIMIT_NOFILE, &rl) == 0) {
			limit = rl.rlim_max - (rl.rlim_max >> 2);
			/*
			 * 20K is arbitrary. If the limit is set to max possible
			 * value, the memory usage increases to more than double.
			 */
			if (limit > 20480)
				limit = 20480;
		}
	} else
		limit = 32;

	return limit;
}

/*
 * Wrapper to realloc()
 * Frees current memory if realloc() fails and returns NULL.
 *
 * As per the docs, the *alloc() family is supposed to be memory aligned:
 * Ubuntu: https://manpages.ubuntu.com/manpages/xenial/man3/malloc.3.html
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
static size_t xstrsncpy(char *restrict dst, const char *restrict src, size_t n)
{
	char *end = memccpy(dst, src, '\0', n);

	if (!end) {
		dst[n - 1] = '\0'; // NOLINT
		end = dst + n; /* If we return n here, binary size increases due to auto-inlining */
	}

	return end - dst;
}

static inline size_t xstrlen(const char *restrict s)
{
#if !defined(__GLIBC__)
	return strlen(s); // NOLINT
#else
	return (char *)rawmemchr(s, '\0') - s; // NOLINT
#endif
}

static char *xstrdup(const char *restrict s)
{
	size_t len = xstrlen(s) + 1;
	char *ptr = malloc(len);

	if (ptr)
		xstrsncpy(ptr, s, len);
	return ptr;
}

static bool is_suffix(const char *restrict str, const char *restrict suffix)
{
	if (!str || !suffix)
		return FALSE;

	size_t lenstr = xstrlen(str);
	size_t lensuffix = xstrlen(suffix);

	if (lensuffix > lenstr)
		return FALSE;

	return (xstrcmp(str + (lenstr - lensuffix), suffix) == 0);
}

static bool is_prefix(const char *restrict str, const char *restrict prefix, size_t len)
{
	return !strncmp(str, prefix, len);
}

/*
 * The poor man's implementation of memrchr(3).
 * We are only looking for '/' in this program.
 * And we are NOT expecting a '/' at the end.
 * Ideally 0 < n <= xstrlen(s).
 */
static void *xmemrchr(uchar_t *restrict s, uchar_t ch, size_t n)
{
#if defined(__GLIBC__) || defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__)
	return memrchr(s, ch, n);
#else

	if (!s || !n)
		return NULL;

	uchar_t *ptr = s + n;

	do {
		if (*--ptr == ch)
			return ptr;
	} while (s != ptr);

	return NULL;
#endif
}

/* A very simplified implementation, changes path */
static char *xdirname(char *path)
{
	char *base = xmemrchr((uchar_t *)path, '/', xstrlen(path));

	if (base == path)
		path[1] = '\0';
	else
		*base = '\0';

	return path;
}

static char *xbasename(char *path)
{
	char *base = xmemrchr((uchar_t *)path, '/', xstrlen(path)); // NOLINT

	return base ? base + 1 : path;
}

static char *xextension(const char *fname, size_t len)
{
	return xmemrchr((uchar_t *)fname, '.', len);
}

static inline bool getutil(char *util)
{
	return spawn("which", util, NULL, F_NORMAL | F_NOTRACE) == 0;
}

/*
 * Updates out with "dir/name or "/name"
 * Returns the number of bytes copied including the terminating NULL byte
 *
 * Note: dir and out must be PATH_MAX in length to avoid macOS fault
 */
static size_t mkpath(const char *dir, const char *name, char *out)
{
	size_t len;

	/* Handle absolute path */
	if (name[0] == '/') // NOLINT
		return xstrsncpy(out, name, PATH_MAX);

	/* Handle root case */
	if (istopdir(dir))
		len = 1;
	else
		len = xstrsncpy(out, dir, PATH_MAX);

	out[len - 1] = '/'; // NOLINT
	return (xstrsncpy(out + len, name, PATH_MAX - len) + len);
}

/* Assumes both the paths passed are directories */
static char *common_prefix(const char *path, char *prefix)
{
	const char *x = path, *y = prefix;
	char *sep;

	if (!path || !*path || !prefix)
		return NULL;

	if (!*prefix) {
		xstrsncpy(prefix, path, PATH_MAX);
		return prefix;
	}

	while (*x && *y && (*x == *y))
		++x, ++y;

	/* Strings are same */
	if (!*x && !*y)
		return prefix;

	/* Path is shorter */
	if (!*x && *y == '/') {
		xstrsncpy(prefix, path, y - path);
		return prefix;
	}

	/* Prefix is shorter */
	if (!*y && *x == '/')
		return prefix;

	/* Shorten prefix */
	prefix[y - prefix] = '\0';

	sep = xmemrchr((uchar_t *)prefix, '/', y - prefix);
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

	size_t dst_size = 0, src_size = xstrlen(path), cwd_size = xstrlen(cwd);
	size_t len = src_size;
	const char *src;
	char *dst;
	/*
	 * We need to add 2 chars at the end as relative paths may start with:
	 * ./ (find .)
	 * no separator (fd .): this needs an additional char for '/'
	 */
	char *resolved_path = malloc(src_size + (*path == '/' ? 0 : cwd_size) + 2);

	if (!resolved_path)
		return NULL;

	/* Turn relative paths into absolute */
	if (path[0] != '/')
		dst_size = xstrsncpy(resolved_path, cwd, cwd_size + 1) - 1;
	else
		resolved_path[0] = '\0';

	src = path;
	dst = resolved_path + dst_size;
	for (const char *next = NULL; next != path + src_size;) {
		next = memchr(src, '/', len);
		if (!next)
			next = path + src_size;

		if (next - src == 2 && src[0] == '.' && src[1] == '.') {
			if (dst - resolved_path) {
				dst = xmemrchr((uchar_t *)resolved_path, '/', dst - resolved_path);
				*dst = '\0';
			}
		} else if (next - src == 1 && src[0] == '.') {
			/* NOP */
		} else if (next - src) {
			*(dst++) = '/';
			xstrsncpy(dst, src, next - src + 1);
			dst += next - src;
		}

		src = next + 1;
		len = src_size - (src - path);
	}

	if (*resolved_path == '\0') {
		resolved_path[0] = '/';
		resolved_path[1] = '\0';
	}

	return resolved_path;
}

static int create_tmp_file(void)
{
	xstrsncpy(g_tmpfpath + tmpfplen - 1, messages[STR_TMPFILE], TMP_LEN_MAX - tmpfplen);

	int fd = mkstemp(g_tmpfpath);

	if (fd == -1) {
		DPRINTF_S(strerror(errno));
	}

	return fd;
}

static void clearinfoln(void)
{
	move(xlines - 2, 0);
	clrtoeol();
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
static inline void printmsg_nc(const char *msg)
{
	tolastln();
	addstr(msg);
	addch('\n');
}

static void printmsg(const char *msg)
{
	attron(COLOR_PAIR(cfg.curctx + 1));
	printmsg_nc(msg);
	attroff(COLOR_PAIR(cfg.curctx + 1));
}

static void printwait(const char *msg, int *presel)
{
	printmsg(msg);
	if (presel) {
		*presel = MSGWAIT;
		if (ndents)
			xstrsncpy(g_ctx[cfg.curctx].c_name, pdents[cur].name, NAME_MAX + 1);
	}
}

/* Kill curses and display error before exiting */
static void printerr(int linenum)
{
	exitcurses();
	perror(xitoa(linenum));
	if (!g_state.picker && selpath)
		unlink(selpath);
	free(pselbuf);
	exit(1);
}

static inline bool xconfirm(int c)
{
	return (c == 'y' || c == 'Y');
}

static int get_input(const char *prompt)
{
	if (prompt)
		printmsg(prompt);
	cleartimeout();

	int r = getch();

#ifdef KEY_RESIZE
	while (r == KEY_RESIZE) {
		if (prompt) {
			clearoldprompt();
			xlines = LINES;
			printmsg(prompt);
		}

		r = getch();
	}
#endif
	settimeout();
	return r;
}

static int get_cur_or_sel(void)
{
	if (selbufpos && ndents) {
		if (cfg.prefersel)
			return 's';

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

	snprintf(str, 64, messages[MSG_FORCE_RM],
		 (selection ? xitoa(nselected) : "current"), (selection ? "(s)" : ""));

	int r = get_input(str);

	if (r == ESC)
		return '\0'; /* cancel */
	if (r == 'y' || r == 'Y')
		return 'f'; /* forceful */
	return 'i'; /* interactive */
}

/* Writes buflen char(s) from buf to a file */
static void writesel(const char *buf, const size_t buflen)
{
	if (g_state.pickraw || !selpath)
		return;

	FILE *fp = fopen(selpath, "w");

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

	selbufpos += xstrsncpy(pselbuf + selbufpos, path, len);
}

/* Write selected file paths to fd, linefeed separated */
static size_t seltofile(int fd, uint_t *pcount)
{
	uint_t lastpos, count = 0;
	char *pbuf = pselbuf;
	size_t pos = 0;
	ssize_t len, prefixlen = 0, initlen = 0;

	if (pcount)
		*pcount = 0;

	if (!selbufpos)
		return 0;

	lastpos = selbufpos - 1;

	if (listpath) {
		prefixlen = (ssize_t)xstrlen(listroot);
		initlen = (ssize_t)xstrlen(listpath);
	}

	while (pos <= lastpos) {
		DPRINTF_S(pbuf);
		len = (ssize_t)xstrlen(pbuf);

		if (!listpath || !is_prefix(pbuf, listpath, initlen)) {
			if (write(fd, pbuf, len) != len)
				return pos;
		} else {
			if (write(fd, listroot, prefixlen) != prefixlen)
				return pos;
			if (write(fd, pbuf + initlen, len - initlen) != (len - initlen))
				return pos;
		}

		pos += len;
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

	if (stat(selpath, &sb) == -1)
		return FALSE;

	/* Nothing selected if file size is 0 */
	if (!sb.st_size)
		return FALSE;

	snprintf(g_buf, CMD_LEN_MAX, "tr \'\\0\' \'\\n\' < %s", selpath);
	spawn(utils[UTIL_SH_EXEC], g_buf, NULL, F_CLI | F_CONFIRM);

	return TRUE;
}

/* Reset selection indicators */
static void resetselind(void)
{
	for (int r = 0; r < ndents; ++r)
		if (pdents[r].flags & FILE_SELECTED)
			pdents[r].flags &= ~FILE_SELECTED;
}

static void startselection(void)
{
	if (!g_state.selmode) {
		g_state.selmode = 1;
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
	size_t r;

	for (int i = 0; i < ndents; ++i)
		if (pdents[i].flags & FILE_SELECTED) {
			r = mkpath(path, pdents[i].name, newpath);
			appendfpath(newpath, r);
		}
}

/* Finish selection procedure before an operation */
static void endselection(void)
{
	int fd;
	ssize_t count;
	char buf[sizeof(patterns[P_REPLACE]) + PATH_MAX + (TMP_LEN_MAX << 1)];

	if (g_state.selmode)
		g_state.selmode = 0;

	if (!listpath || !selbufpos)
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

	snprintf(buf, sizeof(buf), patterns[P_REPLACE], listpath, listroot, g_tmpfpath);
	spawn(utils[UTIL_SH_EXEC], buf, NULL, F_CLI);

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
	g_state.selmode = 0;
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

	spawn((cfg.waitedit ? enveditor : editor), g_tmpfpath, NULL, F_CLI);

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
	if (!selpath) {
		printmsg(messages[MSG_SEL_MISSING]);
		return FALSE;
	}

	/* Fail if selection file path isn't accessible */
	if (access(selpath, R_OK | W_OK) == -1) {
		errno == ENOENT ? printmsg(messages[MSG_0_SELECTED]) : printwarn(NULL);
		return FALSE;
	}

	return TRUE;
}

static void export_file_list(void)
{
	if (!ndents)
		return;

	struct entry *pdent = pdents;
	int fd = create_tmp_file();

	if (fd == -1) {
		DPRINTF_S(strerror(errno));
		return;
	}

	for (int r = 0; r < ndents; ++pdent, ++r) {
		if (write(fd, pdent->name, pdent->nlen - 1) != (pdent->nlen - 1))
			break;

		if ((r != ndents - 1) && (write(fd, "\n", 1) != 1))
			break;
	}

	if (close(fd)) {
		DPRINTF_S(strerror(errno));
	}

	spawn(editor, g_tmpfpath, NULL, F_CLI);

	if (xconfirm(get_input(messages[MSG_RM_TMP])))
		unlink(g_tmpfpath);
}

static bool init_fcolors(void)
{
	char *f_colors = getenv("NNN_FCOLORS");

	if (!f_colors || !*f_colors)
		f_colors = gcolors;

	for (uchar_t id = C_BLK; *f_colors && id <= C_UND; ++id) {
		fcolors[id] = xchartohex(*f_colors) << 4;
		if (*++f_colors) {
			fcolors[id] += xchartohex(*f_colors);
			if (fcolors[id])
				init_pair(id, fcolors[id], -1);
		} else
			return FALSE;
		++f_colors;
	}

	return TRUE;
}

/* Initialize curses mode */
static bool initcurses(void *oldmask)
{
#ifdef NOMOUSE
	(void) oldmask;
#endif

	if (g_state.picker) {
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
	mousemask(BUTTON1_PRESSED | BUTTON1_DOUBLE_CLICKED | BUTTON2_PRESSED | BUTTON3_PRESSED,
			(mmask_t *)oldmask);
#else
	mousemask(BUTTON1_PRESSED | BUTTON2_PRESSED | BUTTON3_PRESSED | BUTTON4_PRESSED | BUTTON5_PRESSED,
			(mmask_t *)oldmask);
#endif
	mouseinterval(0);
#endif
	curs_set(FALSE); /* Hide cursor */

	char *colors = getenv(env_cfg[NNN_COLORS]);

	if (colors || !getenv("NO_COLOR")) {
		uint_t *pcode;
		bool ext = FALSE;

		start_color();
		use_default_colors();

		/* Initialize file colors */
		if (COLORS >= 256) {
			if (!(g_state.oldcolor || init_fcolors())) {
				exitcurses();
				fprintf(stderr, "NNN_FCOLORS!\n");
				return FALSE;
			}
		} else
			g_state.oldcolor = 1;

		DPRINTF_D(COLORS);
		DPRINTF_D(COLOR_PAIRS);

		if (colors && *colors == '#') {
			char *sep = strchr(colors, ';');

			if (!g_state.oldcolor && COLORS >= 256) {
				++colors;
				ext = TRUE;

				/*
				 * If fallback colors are specified, set the separator
				 * to NULL so we don't interpret separator and fallback
				 * if fewer than CTX_MAX xterm 256 colors are specified.
				 */
				if (sep)
					*sep = '\0';
			} else {
				colors = sep; /* Detect if 8 colors fallback is appended */
				if (colors)
					++colors;
			}
		}

		/* Get and set the context colors */
		for (uchar_t i = 0; i <  CTX_MAX; ++i) {
			pcode = &g_ctx[i].color;

			if (colors && *colors) {
				if (ext) {
					*pcode = xchartohex(*colors) << 4;
					if (*++colors)
						fcolors[i + 1] = *pcode += xchartohex(*colors);
					else { /* Each color code must be 2 hex symbols */
						exitcurses();
						fprintf(stderr, "NNN_COLORS!\n");
						return FALSE;
					}
				} else
					*pcode = (*colors < '0' || *colors > '7') ? 4 : *colors - '0';
				++colors;
			} else
				*pcode = 4;

			init_pair(i + 1, *pcode, -1);
		}
	}

#ifdef ICONS_ENABLED
	if (!g_state.oldcolor) {
		uchar_t icolors[256] = {0};
		char c;

		memset(icon_positions, 0x7f, sizeof(icon_positions));

		for (uint_t i = 0; i < sizeof(icons_ext)/sizeof(struct icon_pair); ++i) {
			c = TOUPPER(icons_ext[i].match[0]);
			if (c >= 'A' && c <= 'Z') {
				if (icon_positions[c - 'A' + 10] == 0x7f7f)
					icon_positions[c - 'A' + 10] = i;
			} else if (c >= '0' && c <= '9') {
				if (icon_positions[c - '0'] == 0x7f7f)
					icon_positions[c - '0'] = i;
			} else if (icon_positions[36] == 0x7f7f)
				icon_positions[36] = i;

			if (icons_ext[i].color && !icolors[icons_ext[i].color]) {
				init_pair(C_UND + 1 + icons_ext[i].color, icons_ext[i].color, -1);
				icolors[icons_ext[i].color] = 1;
			}
		}
	}
#endif

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

static pid_t xfork(uchar_t flag)
{
	int status;
	pid_t p = fork();
	struct sigaction dfl_act = {.sa_handler = SIG_DFL};

	if (p > 0) {
		/* the parent ignores the interrupt, quit and hangup signals */
		sigaction(SIGHUP, &(struct sigaction){.sa_handler = SIG_IGN}, &oldsighup);
		sigaction(SIGTSTP, &dfl_act, &oldsigtstp);
	} else if (p == 0) {
		/* We create a grandchild to detach */
		if (flag & F_NOWAIT) {
			p = fork();

			if (p > 0)
				_exit(EXIT_SUCCESS);
			else if (p == 0) {
				sigaction(SIGHUP, &dfl_act, NULL);
				sigaction(SIGINT, &dfl_act, NULL);
				sigaction(SIGQUIT, &dfl_act, NULL);
				sigaction(SIGTSTP, &dfl_act, NULL);

				setsid();
				return p;
			}

			perror("fork");
			_exit(EXIT_FAILURE);
		}

		/* so they can be used to stop the child */
		sigaction(SIGHUP, &dfl_act, NULL);
		sigaction(SIGINT, &dfl_act, NULL);
		sigaction(SIGQUIT, &dfl_act, NULL);
		sigaction(SIGTSTP, &dfl_act, NULL);
	}

	/* This is the parent waiting for the child to create grandchild*/
	if (flag & F_NOWAIT)
		waitpid(p, &status, 0);

	if (p == -1)
		perror("fork");
	return p;
}

static int join(pid_t p, uchar_t flag)
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
	sigaction(SIGHUP, &oldsighup, NULL);
	sigaction(SIGTSTP, &oldsigtstp, NULL);

	return status;
}

/*
 * Spawns a child process. Behaviour can be controlled using flag.
 * Limited to 2 arguments to a program, flag works on bit set.
 */
static int spawn(char *file, char *arg1, char *arg2, uchar_t flag)
{
	pid_t pid;
	int status = 0, retstatus = 0xFFFF;
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
		size_t len = xstrlen(file) + 1;

		cmd = (char *)malloc(len);
		if (!cmd) {
			DPRINTF_S("malloc()!");
			return retstatus;
		}

		xstrsncpy(cmd, file, len);
		status = parseargs(cmd, argv);
		if (status == -1 || status > (EXEC_ARGS_MAX - 3)) { /* arg1, arg2 and last NULL */
			free(cmd);
			DPRINTF_S("NULL or too many args");
			return retstatus;
		}
	} else
		argv[status++] = file;

	argv[status] = arg1;
	argv[++status] = arg2;

	if (flag & F_NORMAL)
		exitcurses();

	pid = xfork(flag);
	if (pid == 0) {
		/* Suppress stdout and stderr */
		if (flag & F_NOTRACE) {
			int fd = open("/dev/null", O_WRONLY, 0200);

			dup2(fd, 1);
			dup2(fd, 2);
			close(fd);
		}

		execvp(*argv, argv);
		_exit(EXIT_SUCCESS);
	} else {
		retstatus = join(pid, flag);

		DPRINTF_D(pid);

		if ((flag & F_CONFIRM) || ((flag & F_CHKRTN) && retstatus)) {
			printf("%s", messages[MSG_CONTINUE]);
#ifndef NORL
			fflush(stdout);
#endif
			while (getchar() != '\n') {};
		}

		if (flag & F_NORMAL)
			refresh();

		free(cmd);
	}

	return retstatus;
}

/* Get program name from env var, else return fallback program */
static char *xgetenv(const char * const name, char *fallback)
{
	char *value = getenv(name);

	return value && value[0] ? value : fallback;
}

/* Checks if an env variable is set to 1 */
static inline uint_t xgetenv_val(const char *name)
{
	char *str = getenv(name);

	if (str && str[0])
		return atoi(str);

	return 0;
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
		 op, selpath);
}

static bool rmmulstr(char *buf)
{
	if (!g_state.trash) {
		char r = confirm_force(TRUE);

		if (!r)
			return FALSE;

		snprintf(buf, CMD_LEN_MAX, "xargs -0 sh -c 'rm -%cr \"$0\" \"$@\" < /dev/tty' < %s",
			 r, selpath);
	} else if (g_state.trash == 1)
		snprintf(buf, CMD_LEN_MAX, "xargs -0 trash-put < %s", selpath);
	else
		snprintf(buf, CMD_LEN_MAX, "xargs -0 gio trash < %s", selpath);

	return TRUE;
}

/* Returns TRUE if file is removed, else FALSE */
static bool xrm(char *fpath)
{
	if (!g_state.trash) {
		char rm_opts[] = "-ir";

		rm_opts[1] = confirm_force(FALSE);
		if (!rm_opts[1])
			return FALSE;

		spawn("rm", rm_opts, fpath, F_NORMAL | F_CHKRTN);
	} else if (g_state.trash == 1)
		spawn("trash-put", fpath, NULL, F_NORMAL);
	else
		spawn("gio trash", fpath, NULL, F_NORMAL | F_MULTI);

	return (access(fpath, F_OK) == -1); /* File is removed */
}

static uint_t lines_in_file(int fd, char *buf, size_t buflen)
{
	ssize_t len;
	uint_t count = 0;

	while ((len = read(fd, buf, buflen)) > 0)
		while (len)
			count += (buf[--len] == '\n');

	/* For all use cases 0 linecount is considered as error */
	return ((len < 0) ? 0 : count);
}

static bool cpmv_rename(int choice, const char *path)
{
	int fd;
	uint_t count = 0, lines = 0;
	bool ret = FALSE;
	char *cmd = (choice == 'c' ? cp : mv);
	char buf[sizeof(patterns[P_CPMVRNM]) + sizeof(cmd) + (PATH_MAX << 1)];

	fd = create_tmp_file();
	if (fd == -1)
		return ret;

	/* selsafe() returned TRUE for this to be called */
	if (!selbufpos) {
		snprintf(buf, sizeof(buf), "tr '\\0' '\\n' < %s > %s", selpath, g_tmpfpath);
		spawn(utils[UTIL_SH_EXEC], buf, NULL, F_CLI);

		count = lines_in_file(fd, buf, sizeof(buf));
		if (!count)
			goto finish;
	} else
		seltofile(fd, &count);

	close(fd);

	snprintf(buf, sizeof(buf), patterns[P_CPMVFMT], g_tmpfpath);
	spawn(utils[UTIL_SH_EXEC], buf, NULL, F_CLI);

	spawn((cfg.waitedit ? enveditor : editor), g_tmpfpath, NULL, F_CLI);

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

	snprintf(buf, sizeof(buf), patterns[P_CPMVRNM], path, g_tmpfpath, cmd);
	if (!spawn(utils[UTIL_SH_EXEC], buf, NULL, F_CLI | F_CHKRTN))
		ret = TRUE;
finish:
	if (fd >= 0)
		close(fd);

	return ret;
}

static bool cpmvrm_selection(enum action sel, char *path)
{
	int r;

	if (!selsafe())
		return FALSE;

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
			printmsg(messages[MSG_INVALID_KEY]);
			return FALSE;
		}

		if (!cpmv_rename(r, path)) {
			printmsg(messages[MSG_FAILED]);
			return FALSE;
		}
		break;
	default: /* SEL_RM */
		if (!rmmulstr(g_buf)) {
			printmsg(messages[MSG_CANCEL]);
			return FALSE;
		}
	}

	if (sel != SEL_CPMVAS && spawn(utils[UTIL_SH_EXEC], g_buf, NULL, F_CLI | F_CHKRTN)) {
		printmsg(messages[MSG_FAILED]);
		return FALSE;
	}

	/* Clear selection */
	clearselection();

	return TRUE;
}

#ifndef NOBATCH
static bool batch_rename(void)
{
	int fd1, fd2;
	uint_t count = 0, lines = 0;
	bool dir = FALSE, ret = FALSE;
	char foriginal[TMP_LEN_MAX] = {0};
	static const char batchrenamecmd[] = "paste -d'\n' %s %s | "SED" 'N; /^\\(.*\\)\\n\\1$/!p;d' | "
					     "tr '\n' '\\0' | xargs -0 -n2 mv 2>/dev/null";
	char buf[sizeof(batchrenamecmd) + (PATH_MAX << 1)];
	int i = get_cur_or_sel();

	if (!i)
		return ret;

	if (i == 'c') { /* Rename entries in current dir */
		selbufpos = 0;
		dir = TRUE;
	}

	fd1 = create_tmp_file();
	if (fd1 == -1)
		return ret;

	xstrsncpy(foriginal, g_tmpfpath, xstrlen(g_tmpfpath) + 1);

	fd2 = create_tmp_file();
	if (fd2 == -1) {
		unlink(foriginal);
		close(fd1);
		return ret;
	}

	if (dir)
		for (i = 0; i < ndents; ++i)
			appendfpath(pdents[i].name, NAME_MAX);

	seltofile(fd1, &count);
	seltofile(fd2, NULL);
	close(fd2);

	if (dir) /* Don't retain dir entries in selection */
		selbufpos = 0;

	spawn((cfg.waitedit ? enveditor : editor), g_tmpfpath, NULL, F_CLI);

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
	spawn(utils[UTIL_SH_EXEC], buf, NULL, F_CLI);
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
#endif

static void get_archive_cmd(char *cmd, const char *archive)
{
	uchar_t i = 3;

	if (getutil(utils[UTIL_ATOOL]))
		i = 0;
	else if (getutil(utils[UTIL_BSDTAR]))
		i = 1;
	else if (is_suffix(archive, ".zip"))
		i = 2;
	// else tar

	xstrsncpy(cmd, archive_cmd[i], ARCHIVE_CMD_LEN);
}

static void archive_selection(const char *cmd, const char *archive, const char *curpath)
{
	/* The 70 comes from the string below */
	char *buf = (char *)malloc((70 + xstrlen(cmd) + xstrlen(archive)
				       + xstrlen(curpath) + xstrlen(selpath)) * sizeof(char));
	if (!buf) {
		DPRINTF_S(strerror(errno));
		printwarn(NULL);
		return;
	}

	snprintf(buf, CMD_LEN_MAX,
#ifdef __linux__
		SED" -ze 's|^%s/||' '%s' | xargs -0 %s %s", curpath, selpath, cmd, archive
#else
		"tr '\\0' '\n' < '%s' | "SED" -e 's|^%s/||' | tr '\n' '\\0' | xargs -0 %s %s",
		selpath, curpath, cmd, archive
#endif
		);
	spawn(utils[UTIL_SH_EXEC], buf, NULL, F_CLI | F_CONFIRM);
	free(buf);
}

static bool write_lastdir(const char *curpath)
{
	bool ret = TRUE;
	size_t len = xstrlen(cfgpath);

	xstrsncpy(cfgpath + len, "/.lastd", 8);
	DPRINTF_S(cfgpath);

	FILE *fp = fopen(cfgpath, "w");

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

	long long v1 = strtoll(s1, &p1, 10);
	long long v2 = strtoll(s2, &p2, 10);

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
	const uchar_t *p1 = (const uchar_t *)s1;
	const uchar_t *p2 = (const uchar_t *)s2;
	int state, diff;
	uchar_t c1, c2;

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

	state = result_type[state * 3 + (((c2 == '0') + (xisdigit(c2) != 0)))]; // NOLINT

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
	return pcre_exec(fltrexp->pcrex, NULL, fname, xstrlen(fname), 0, 0, NULL, 0) == 0;
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
	if (cfg.timeorder) {
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
		char *extna = xextension(pa->name, pa->nlen - 1);
		char *extnb = xextension(pb->name, pb->nlen - 1);

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

/* In case of an error, resets *wch to Esc */
static int handle_alt_key(wint_t *wch)
{
	timeout(0);

	int r = get_wch(wch);

	if (r == ERR)
		*wch = ESC;
	cleartimeout();

	return r;
}

/*
 * Returns SEL_* if key is bound and 0 otherwise.
 * Also modifies the run and env pointers (used on SEL_{RUN,RUNARG}).
 * The next keyboard input can be simulated by presel.
 */
static int nextsel(int presel)
{
#ifdef BENCH
	return SEL_QUIT;
#endif
	int c = presel;
	uint_t i;
	bool escaped = FALSE;

	if (c == 0 || c == MSGWAIT) {
try_quit:
		c = getch();
		//DPRINTF_D(c);
		//DPRINTF_S(keyname(c));

		/* Handle Alt+key */
		if (c == ESC) {
			timeout(0);
			c = getch();
			if (c != ERR) {
				if (c == ESC)
					c = CONTROL('L');
				else {
					ungetch(c);
					c = ';';
				}
				settimeout();
			} else if (escaped) {
				settimeout();
				c = CONTROL('Q');
			} else {
#ifndef NOFIFO
				/* Send hovered path to NNN_FIFO */
				notify_fifo(TRUE);
#endif
				escaped = TRUE;
				settimeout();
				goto try_quit;
			}
		}

		if (c == ERR && presel == MSGWAIT)
			c = (cfg.filtermode || filterset()) ? FILTER : CONTROL('L');
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
		if (!g_state.selmode && !cfg.blkorder && inotify_wd >= 0 && (idle & 1)) {
			struct inotify_event *event;
			char inotify_buf[EVENT_BUF_LEN];

			memset((void *)inotify_buf, 0x0, EVENT_BUF_LEN);
			i = read(inotify_fd, inotify_buf, EVENT_BUF_LEN);
			if (i > 0) {
				for (char *ptr = inotify_buf;
				     ptr + ((struct inotify_event *)ptr)->len < inotify_buf + i;
				     ptr += sizeof(struct inotify_event) + event->len) {
					event = (struct inotify_event *)ptr;
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
		if (!g_state.selmode && !cfg.blkorder && event_fd >= 0 && idle & 1) {
			struct kevent event_data[NUM_EVENT_SLOTS];

			memset((void *)event_data, 0x0, sizeof(struct kevent) * NUM_EVENT_SLOTS);
			if (kevent(kq, events_to_monitor, NUM_EVENT_SLOTS, event_data, NUM_EVENT_FDS, &gtimeout) > 0)
				c = CONTROL('L');
		}
#elif defined(HAIKU_NM)
		if (!g_state.selmode && !cfg.blkorder && haiku_nm_active && idle & 1 && haiku_is_update_needed(haiku_hnd))
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

	if (cfg.showhidden)
		sort[i++] = 'H';

	if (cfg.timeorder)
		sort[i++] = (cfg.timetype == T_MOD) ? 'M' : ((cfg.timetype == T_ACCESS) ? 'A' : 'C');
	else if (cfg.sizeorder)
		sort[i++] = 'S';
	else if (cfg.extnorder)
		sort[i++] = 'E';

	if (entrycmpfn == &reventrycmp)
		sort[i++] = 'R';

	if (namecmpfn == &xstrverscasecmp)
		sort[i++] = 'V';

	if (i)
		sort[i] = ' ';

	return i;
}

static void showfilterinfo(void)
{
	int i = 0;
	char info[REGEX_MAX] = "\0\0\0\0\0";

	i = getorderstr(info);

	snprintf(info + i, REGEX_MAX - i - 1, "  %s [/], %s [:]",
		 (cfg.regex ? "regex" : "str"),
		 ((fnstrstr == &strcasestr) ? "ic" : "noic"));

	clearinfoln();
	mvaddstr(xlines - 2, xcols - xstrlen(info), info);
}

static void showfilter(char *str)
{
	attron(COLOR_PAIR(cfg.curctx + 1));
	showfilterinfo();
	printmsg(str);
	// printmsg calls attroff()
}

static inline void swap_ent(int id1, int id2)
{
	struct entry _dent, *pdent1 = &pdents[id1], *pdent2 =  &pdents[id2];

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
#ifdef PCRE
	fltrexp_t fltrexp = { .pcrex = pcrex, .str = fltr };
#else
	fltrexp_t fltrexp = { .regex = re, .str = fltr };
#endif

	for (int count = 0; count < ndents; ++count) {
		if (filterfn(&fltrexp, pdents[count].name) == 0) {
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

	ENTSORT(pdents, ndents, entrycmpfn);

	return ndents;
}

/*
 * Return the position of the matching entry or 0 otherwise
 * Note there's no NULL check for fname
 */
static int dentfind(const char *fname, int n)
{
	for (int i = 0; i < n; ++i)
		if (xstrcmp(fname, pdents[i].name) == 0)
			return i;

	return 0;
}

static int filterentries(char *path, char *lastname)
{
	wchar_t *wln = (wchar_t *)alloca(sizeof(wchar_t) * REGEX_MAX);
	char *ln = g_ctx[cfg.curctx].c_fltr;
	wint_t ch[2] = {0};
	int r, total = ndents, len;
	char *pln = g_ctx[cfg.curctx].c_fltr + 1;

	DPRINTF_S(__func__);

	if (ndents && (ln[0] == FILTER || ln[0] == RFILTER) && *pln) {
		if (matches(pln) != -1) {
			move_cursor(dentfind(lastname, ndents), 0);
			redraw(path);
		}

		if (!cfg.filtermode)
			return 0;

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
		case DEL: /* handle DEL */
			if (len != 1) {
				wln[--len] = '\0';
				wcstombs(ln, wln, REGEX_MAX);
				ndents = total;
			} else
				continue;
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
				} else
					goto end;
			}

			/* Go to the top, we don't know if the hovered file will match the filter */
			cur = 0;

			if (matches(pln) != -1)
				redraw(path);

			showfilter(ln);
			continue;
#ifndef NOMOUSE
		case KEY_MOUSE:
			goto end;
#endif
		case ESC: /* Exit filter mode on Escape and Alt+key */
			if (handle_alt_key(ch) != ERR) {
				if (*ch == ESC) { /* Handle Alt + Esc */
					if (wln[1]) {
						ln[REGEX_MAX - 1] = ln[1];
						ln[1] = wln[1] = '\0';
						ndents = total;
						*ch = CONTROL('L');
					}
				} else {
					unget_wch(*ch);
					*ch = ';';
				}
			}
			goto end;
		}

		if (r != OK) /* Handle Fn keys in main loop */
			break;

		/* Handle all control chars in main loop */
		if (*ch < ASCII_MAX && keyname(*ch)[0] == '^' && *ch != '^') {
			if (keyname(*ch)[1] == '@')
				*ch = 'm';
			goto end;
		}

		if (len == 1) {
			if (*ch == '?') /* Help and config key, '?' is an invalid regex */
				goto end;

			if (cfg.filtermode) {
				switch (*ch) {
				case '\'': // fallthrough /* Go to first non-dir file */
				case '+': // fallthrough /* Toggle auto-advance */
				case ',': // fallthrough /* Mark CWD */
				case '-': // fallthrough /* Visit last visited dir */
				case '.': // fallthrough /* Show hidden files */
				case ';': // fallthrough /* Run plugin key */
				case '=': // fallthrough /* Launch app */
				case '>': // fallthrough /* Export file list */
				case '@': // fallthrough /* Visit start dir */
				case ']': // fallthorugh /* Prompt key */
				case '`': // fallthrough /* Visit / */
				case '~': /* Go HOME */
					goto end;
				}
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
				ln[0] = (ln[0] == FILTER) ? RFILTER : FILTER;
				wln[0] = (uchar_t)ln[0];
				cfg.regex ^= 1;
				filterfn = cfg.regex ? &visible_re : &visible_str;
				showfilter(ln);
				continue;
			}

			/* Reset cur in case it's a repeat search */
			cur = 0;
		} else if (len == REGEX_MAX - 1)
			continue;

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
		    && cfg.autoselect && (pdents[0].flags & DIR_OR_LINK_TO_DIR)) {
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
	printmsg(prompt);

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
		attron(COLOR_PAIR(cfg.curctx + 1));
		mvaddnwstr(xlines - 1, x, buf, len + 1);
		move(xlines - 1, x + wcswidth(buf, pos));
		attroff(COLOR_PAIR(cfg.curctx + 1));

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
			case DEL: // fallthrough
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
				printmsg(prompt);
				do {
					if (pos == 0)
						break;
					memmove(buf + pos - 1, buf + pos,
						(len - pos) * WCHAR_T_WIDTH);
					--pos, --len;
				} while (buf[pos - 1] != ' ' && buf[pos - 1] != '/'); // NOLINT
				continue;
			case CONTROL('K'):
				printmsg(prompt);
				len = pos;
				continue;
			case CONTROL('L'):
				printmsg(prompt);
				len = pos = 0;
				continue;
			case CONTROL('A'):
				pos = 0;
				continue;
			case CONTROL('E'):
				pos = len;
				continue;
			case CONTROL('U'):
				printmsg(prompt);
				memmove(buf, buf + pos, (len - pos) * WCHAR_T_WIDTH);
				len -= pos;
				pos = 0;
				continue;
			case ESC: /* Exit prompt on Escape, but just filter out Alt+key */
				if (handle_alt_key(ch) != ERR)
					continue;

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
				printmsg(prompt);
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
	printmsg("");

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
static char *getreadline(const char *prompt)
{
	exitcurses();

	char *input = readline(prompt);

	refresh();

	if (input && input[0]) {
		add_history(input);
		xstrsncpy(g_buf, input, CMD_LEN_MAX);
		free(input);
		return g_buf;
	}

	free(input);
	return NULL;
}
#endif

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
		r = xstrsncpy(buf, prefix, NAME_MAX + 1); /* Copy prefix */
		xstrsncpy(buf + r - 1, curfname, NAME_MAX - r); /* Suffix target file name */
		mkpath(path, buf, lnpath); /* Generate link path */
		mkpath(path, curfname, buf); /* Generate target file path */

		if (!link_fn(buf, lnpath))
			return 1; /* One link created */

		printwarn(presel);
		return -1;
	}

	while (pos < selbufpos) {
		len = xstrlen(psel);
		fname = xbasename(psel);

		r = xstrsncpy(buf, prefix, NAME_MAX + 1); /* Copy prefix */
		xstrsncpy(buf + r - 1, fname, NAME_MAX - r); /* Suffix target file name */
		mkpath(path, buf, lnpath); /* Generate link path */

		if (!link_fn(psel, lnpath))
			++count;

		pos += len + 1;
		psel += len + 1;
	}

	clearselection();
	return count;
}

static bool parsekvpair(kv **arr, char **envcpy, const uchar_t id, uchar_t *items)
{
	bool new = TRUE;
	const uchar_t INCR = 8;
	uint_t i = 0;
	kv *kvarr = NULL;
	char *ptr = getenv(env_cfg[id]);

	if (!ptr || !*ptr)
		return TRUE;

	*envcpy = xstrdup(ptr);
	if (!*envcpy) {
		xerror();
		return FALSE;
	}

	ptr = *envcpy;

	while (*ptr && i < 100) {
		if (new) {
			if (!(i & (INCR - 1))) {
				kvarr = xrealloc(kvarr, sizeof(kv) * (i + INCR));
				*arr = kvarr;
				if (!kvarr) {
					xerror();
					return FALSE;
				}
				memset(kvarr + i, 0, sizeof(kv) * INCR);
			}
			kvarr[i].key = (uchar_t)*ptr;
			if (*++ptr != ':' || *++ptr == '\0' || *ptr == ';')
				return FALSE;
			kvarr[i].off = ptr - *envcpy;
			++i;

			new = FALSE;
		}

		if (*ptr == ';') {
			*ptr = '\0';
			new = TRUE;
		}

		++ptr;
	}

	*items = i;
	return (i != 0);
}

/*
 * Get the value corresponding to a key
 *
 * NULL is returned in case of no match, path resolution failure etc.
 * buf would be modified, so check return value before access
 */
static char *get_kv_val(kv *kvarr, char *buf, int key, uchar_t max, uchar_t id)
{
	char *val;

	if (!kvarr)
		return NULL;

	for (int r = 0; kvarr[r].key && r < max; ++r) {
		if (kvarr[r].key == key) {
			/* Do not allocate new memory for plugin */
			if (id == NNN_PLUG)
				return pluginstr + kvarr[r].off;

			val = bmstr + kvarr[r].off;

			if (val[0] == '~') {
				ssize_t len = xstrlen(home);
				ssize_t loclen = xstrlen(val);

				xstrsncpy(g_buf, home, len + 1);
				xstrsncpy(g_buf + len, val + 1, loclen);
			}

			return realpath(((val[0] == '~') ? g_buf : val), buf);
		}
	}

	DPRINTF_S("Invalid key");
	return NULL;
}

static void resetdircolor(int flags)
{
	if (g_state.dircolor && !(flags & DIR_OR_LINK_TO_DIR)) {
		attroff(COLOR_PAIR(cfg.curctx + 1) | A_BOLD);
		g_state.dircolor = 0;
	}
}

/*
 * Replace escape characters in a string with '?'
 * Adjust string length to maxcols if > 0;
 * Max supported str length: NAME_MAX;
 */
#ifndef NOLOCALE
static wchar_t *unescape(const char *str, uint_t maxcols)
{
	wchar_t * const wbuf = (wchar_t *)g_buf;
	wchar_t *buf = wbuf;
	size_t lencount = 0;

	/* Convert multi-byte to wide char */
	size_t len = mbstowcs(wbuf, str, NAME_MAX);

	len = wcswidth(wbuf, len);

	/* Reduce number of wide chars to max columns */
	if (len > maxcols) {
		while (*buf && lencount <= maxcols) {
			if (*buf <= '\x1f' || *buf == '\x7f')
				*buf = '\?';

			++buf;
			++lencount;
		}

		lencount = maxcols + 1;

		/* Reduce wide chars one by one till it fits */
		do
			len = wcswidth(wbuf, --lencount);
		while (len > maxcols);

		wbuf[lencount] = L'\0';
	} else {
		do { /* We do not expect a NULL string */
			if (*buf <= '\x1f' || *buf == '\x7f')
				*buf = '\?';
		} while (*++buf);
	}

	return wbuf;
}
#else
static char *unescape(const char *str, uint_t maxcols)
{
	ssize_t len = (ssize_t)xstrsncpy(g_buf, str, maxcols);

	--len;
	while (--len >= 0)
		if (g_buf[len] <= '\x1f' || g_buf[len] == '\x7f')
			g_buf[len] = '\?';

	return g_buf;
}
#endif

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
		ret = xstrsncpy(size_buf, xitoa(size), 12);
		size_buf[ret - 1] = '.';

		char *frac = xitoa(rem);
		size_t toprint = i > 3 ? 3 : i;
		size_t len = xstrlen(frac);

		if (len < toprint) {
			size_buf[ret] = size_buf[ret + 1] = size_buf[ret + 2] = '0';
			xstrsncpy(size_buf + ret + (toprint - len), frac, len + 1);
		} else
			xstrsncpy(size_buf + ret, frac, toprint + 1);

		ret += toprint;
	} else {
		ret = xstrsncpy(size_buf, size ? xitoa(size) : "0", 12);
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

	xstrsncpy(&bits[1], rwx[(mode >> 6) & 7], 4);
	xstrsncpy(&bits[4], rwx[(mode >> 3) & 7], 4);
	xstrsncpy(&bits[7], rwx[(mode & 7)], 4);

	if (mode & S_ISUID)
		bits[3] = (mode & 0100) ? 's' : 'S';  /* user executable */
	if (mode & S_ISGID)
		bits[6] = (mode & 0010) ? 's' : 'l';  /* group executable */
	if (mode & S_ISVTX)
		bits[9] = (mode & 0001) ? 't' : 'T';  /* others executable */

	return bits;
}

#ifdef ICONS_ENABLED
static const struct icon_pair *get_icon(const struct entry *ent)
{
	ushort_t i = 0;

	for (; i < sizeof(icons_name)/sizeof(struct icon_pair); ++i)
		if (strcasecmp(ent->name, icons_name[i].match) == 0)
			return &icons_name[i];

	if (ent->flags & DIR_OR_LINK_TO_DIR)
		return &dir_icon;

	char *tmp = xextension(ent->name, ent->nlen);

	if (!tmp) {
		if (ent->mode & 0100)
			return &exec_icon;

		return &file_icon;
	}

	/* Skip the . */
	++tmp;

	if (*tmp >= '0' && *tmp <= '9')
		i = *tmp - '0'; /* NUMBER 0-9 */
	else if (TOUPPER(*tmp) >= 'A' && TOUPPER(*tmp) <= 'Z')
		i = TOUPPER(*tmp) - 'A' + 10; /* LETTER A-Z */
	else
		i = 36; /* OTHER */

	for (ushort_t j = icon_positions[i]; j < sizeof(icons_ext)/sizeof(struct icon_pair) &&
			icons_ext[j].match[0] == icons_ext[icon_positions[i]].match[0]; ++j)
		if (strcasecmp(tmp, icons_ext[j].match) == 0)
			return &icons_ext[j];

	/* If there's no match and the file is executable, icon that */
	if (ent->mode & 0100)
		return &exec_icon;

	return &file_icon;
}

static void print_icon(const struct entry *ent, const int attrs)
{
	const struct icon_pair *picon = get_icon(ent);

	addstr(ICON_PADDING_LEFT);
	if (picon->color)
		attron(COLOR_PAIR(C_UND + 1 + picon->color));
	else if (attrs)
		attron(attrs);
	addstr(picon->icon);
	if (picon->color)
		attroff(COLOR_PAIR(C_UND + 1 + picon->color));
	else if (attrs)
		attroff(attrs);
	addstr(ICON_PADDING_RIGHT);
}
#endif

static void print_time(const time_t *timep)
{
	struct tm *t = localtime(timep);

	printw("%d-%02d-%02d %02d:%02d",
	       t->tm_year + 1900, t->tm_mon + 1, t->tm_mday, t->tm_hour, t->tm_min);
}

static void printent(const struct entry *ent, uint_t namecols, bool sel)
{
	uchar_t pair = 0;
	char ind = '\0';
	int attrs = 0;

	switch (ent->mode & S_IFMT) {
	case S_IFREG:
		if (ent->mode & 0100) {
			pair = C_EXE;
			ind = '*';
		}

		if (!ent->size)
			pair = C_UND;
		else if (ent->flags & HARD_LINK)
			pair = C_HRD;
		else if (!pair)
			pair = C_FIL;
		break;
	case S_IFDIR:
		pair = C_DIR;
		if (!g_state.oldcolor) {
			attrs |= A_BOLD;
			if (g_state.dirctx)
				pair = cfg.curctx + 1;
		}
		ind = '/';
		break;
	case S_IFLNK:
		if (ent->flags & DIR_OR_LINK_TO_DIR) {
			if (!g_state.oldcolor)
				attrs |= A_BOLD;
			ind = '/';
		} else
			ind = '@';

		if (g_state.oldcolor)
			attrs |= A_DIM;
		else
			pair = (ent->flags & SYM_ORPHAN) ? C_ORP : C_LNK;
		break;
	case S_IFSOCK:
		pair = C_SOC;
		ind = '=';
		break;
	case S_IFIFO:
		pair = C_PIP;
		ind = '|';
		break;
	case S_IFBLK:
		pair = C_BLK;
		break;
	case S_IFCHR:
		pair = C_CHR;
		break;
	default:
		pair = C_UND;
		ind = '?';
		break;
	}

	if (!g_state.oldcolor) {
		if (ent->flags & FILE_MISSING)
			pair = C_MIS;

		if (pair && fcolors[pair])
			attrs |= COLOR_PAIR(pair);
	}

	if (!ind)
		++namecols;

	/* Directories are always shown on top */
	resetdircolor(ent->flags);

	addch((ent->flags & FILE_SELECTED) ? '+' : ' ');

#ifdef ICONS_ENABLED
	if (!g_state.oldcolor)
		print_icon(ent, attrs);
#endif

	if (sel)
		attrs |= A_REVERSE;
	if (attrs)
		attron(attrs);

#ifndef NOLOCALE
	addwstr(unescape(ent->name, namecols));
#else
	addstr(unescape(ent->name, MIN(namecols, ent->nlen) + 1));
#endif

	if (attrs)
		attroff(attrs);

	if (ind)
		addch(ind);
	addch('\n');
}

static void printent_long(const struct entry *ent, uint_t namecols, bool sel)
{
	bool ln = FALSE;
	char ind1 = '\0', ind2 = '\0';
	uchar_t pair = 0;
	int attrs = sel ? (A_REVERSE | (g_state.oldcolor ? A_DIM : COLOR_PAIR(C_MIS)))
			: (g_state.oldcolor ? A_DIM : COLOR_PAIR(C_MIS));
	uint_t len;
	char *size;
	char selgap[] = "  ";

	if (ent->flags & FILE_SELECTED)
		selgap[1] = '+';

	/* Directories are always shown on top */
	resetdircolor(ent->flags);

	addch(' ');

	if (attrs)
		attron(attrs);

	/* Timestamp */
	print_time(&ent->t);

	addstr("  ");

	/* Permissions */
	addch('0' + ((ent->mode >> 6) & 7));
	addch('0' + ((ent->mode >> 3) & 7));
	addch('0' + (ent->mode & 7));

	switch (ent->mode & S_IFMT) {
	case S_IFDIR:
		pair = C_DIR;
		if (!g_state.oldcolor) {
			attrs |= A_BOLD;
			if (g_state.dirctx)
				pair = cfg.curctx + 1;
		}
		ind2 = '/'; // fallthrough
	case S_IFREG:
		if (!ind2) {
			if (ent->mode & 0100) {
				pair = C_EXE;
				ind2 = '*';
			}

			if (ent->flags & HARD_LINK) {
				pair = C_HRD;
				ln = TRUE;
			}

			if (!ent->size)
				pair = C_UND;
			else if (!pair)
				pair = C_FIL;

			if (!ind2) /* Add a column if end indicator is not needed */
				++namecols;
		}

		size = coolsize(cfg.blkorder ? ent->blocks << blk_shift : ent->size);
		len = 10 - (uint_t)xstrlen(size);
		while (--len)
			addch(' ');
		addstr(size);
		break;
	case S_IFLNK:
		ln = TRUE;
		pair = (ent->flags & SYM_ORPHAN) ? C_ORP : C_LNK;
		ind1 = '@';
		ind2 = (ent->flags & DIR_OR_LINK_TO_DIR) ? '/' : '@';
		if (ind2 == '/' && !g_state.oldcolor)
			attrs |= A_BOLD; // fallthrough
	case S_IFSOCK:
		if (!ind1) {
			pair = C_SOC;
			ind1 = ind2 = '=';
		} // fallthrough
	case S_IFIFO:
		if (!ind1) {
			pair = C_PIP;
			ind1 = ind2 = '|';
		} // fallthrough
	case S_IFBLK:
		if (!ind1) {
			pair = C_BLK;
			ind1 = 'b';
		} // fallthrough
	case S_IFCHR:
		if (!ind1) {
			pair = C_CHR;
			ind1 = 'c';
		} // fallthrough
	default:
		if (!ind1) {
			pair = C_UND;
			ind1 = ind2 = '?';
		}
		addstr("        ");
		addch(ind1);
		break;
	}

	if (g_state.oldcolor) {
		if (!sel)
			attroff(A_DIM);
		addstr(selgap);
		if (!ln) {
			attroff(A_DIM);
			attrs ^= A_DIM;
		}
	} else {
		if (!sel)
			attroff(COLOR_PAIR(C_MIS));
#ifndef ICONS_ENABLED
		addstr(selgap);
#endif
		if (ent->flags & FILE_MISSING)
			pair = C_MIS;
		else {
			attroff(COLOR_PAIR(C_MIS));
			attrs ^= (COLOR_PAIR(C_MIS));
		}

		if (pair && fcolors[pair])
			attrs |= COLOR_PAIR(pair);
#ifdef ICONS_ENABLED
		attroff(attrs);
		addstr(selgap);
		if (sel)
			attrs &= ~A_REVERSE;
		print_icon(ent, attrs);
		if (sel)
			attrs |= A_REVERSE;
#endif
		attron(attrs);
	}

#ifndef NOLOCALE
	addwstr(unescape(ent->name, namecols));
#else
	addstr(unescape(ent->name, MIN(namecols, ent->nlen) + 1));
#endif

	if (attrs)
		attroff(attrs);
	if (ind2)
		addch(ind2);
	addch('\n');
}

static void (*printptr)(const struct entry *ent, uint_t namecols, bool sel) = &printent;

static void savecurctx(settings *curcfg, char *path, char *curname, int r /* next context num */)
{
	settings tmpcfg = *curcfg;
	context *ctxr = &g_ctx[r];

	/* Save current context */
	if (ndents)
		xstrsncpy(g_ctx[tmpcfg.curctx].c_name, curname, NAME_MAX + 1);
	else
		g_ctx[tmpcfg.curctx].c_name[0] = '\0';

	g_ctx[tmpcfg.curctx].c_cfg = tmpcfg;

	if (ctxr->c_cfg.ctxactive) { /* Switch to saved context */
		/* Switch light/detail mode */
		if (tmpcfg.showdetail != ctxr->c_cfg.showdetail)
			/* set the reverse */
			printptr = tmpcfg.showdetail ? &printent : &printent_long;

		tmpcfg = ctxr->c_cfg;
	} else { /* Setup a new context from current context */
		ctxr->c_cfg.ctxactive = 1;
		xstrsncpy(ctxr->c_path, path, PATH_MAX);
		ctxr->c_last[0] = ctxr->c_name[0] = ctxr->c_fltr[0] = ctxr->c_fltr[1] = '\0';
		ctxr->c_cfg = tmpcfg;
	}

	tmpcfg.curctx = r;
	*curcfg = tmpcfg;
}

#ifndef NOSSN
static void save_session(bool last_session, int *presel)
{
	int i;
	session_header_t header;
	FILE *fsession;
	char *sname;
	bool status = FALSE;
	char ssnpath[PATH_MAX];
	char spath[PATH_MAX];

	memset(&header, 0, sizeof(session_header_t));

	header.ver = SESSIONS_VERSION;

	for (i = 0; i < CTX_MAX; ++i) {
		if (g_ctx[i].c_cfg.ctxactive) {
			if (cfg.curctx == i && ndents)
				/* Update current file name, arrows don't update it */
				xstrsncpy(g_ctx[i].c_name, pdents[cur].name, NAME_MAX + 1);
			header.pathln[i] = strnlen(g_ctx[i].c_path, PATH_MAX) + 1;
			header.lastln[i] = strnlen(g_ctx[i].c_last, PATH_MAX) + 1;
			header.nameln[i] = strnlen(g_ctx[i].c_name, NAME_MAX) + 1;
			header.fltrln[i] = strnlen(g_ctx[i].c_fltr, REGEX_MAX) + 1;
		}
	}

	sname = !last_session ? xreadline(NULL, messages[MSG_SSN_NAME]) : "@";
	if (!sname[0])
		return;

	mkpath(cfgpath, toks[TOK_SSN], ssnpath);
	mkpath(ssnpath, sname, spath);

	fsession = fopen(spath, "wb");
	if (!fsession) {
		printwait(messages[MSG_SEL_MISSING], presel);
		return;
	}

	if ((fwrite(&header, sizeof(header), 1, fsession) != 1)
		|| (fwrite(&cfg, sizeof(cfg), 1, fsession) != 1))
		goto END;

	for (i = 0; i < CTX_MAX; ++i)
		if ((fwrite(&g_ctx[i].c_cfg, sizeof(settings), 1, fsession) != 1)
			|| (fwrite(&g_ctx[i].color, sizeof(uint_t), 1, fsession) != 1)
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
	int i = 0;
	session_header_t header;
	FILE *fsession;
	bool has_loaded_dynamically = !(sname || restore);
	bool status = FALSE;
	char ssnpath[PATH_MAX];
	char spath[PATH_MAX];

	mkpath(cfgpath, toks[TOK_SSN], ssnpath);

	if (!restore) {
		sname = sname ? sname : xreadline(NULL, messages[MSG_SSN_NAME]);
		if (!sname[0])
			return FALSE;

		mkpath(ssnpath, sname, spath);

		/* If user is explicitly loading the "last session", skip auto-save */
		if ((sname[0] == '@') && !sname[1])
			has_loaded_dynamically = FALSE;
	} else
		mkpath(ssnpath, "@", spath);

	if (has_loaded_dynamically)
		save_session(TRUE, NULL);

	fsession = fopen(spath, "rb");
	if (!fsession) {
		printmsg(messages[MSG_SEL_MISSING]);
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
			|| (fread(&g_ctx[i].color, sizeof(uint_t), 1, fsession) != 1)
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
	set_sort_flags('\0'); /* Set correct sort options */
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
#endif

static uchar_t get_free_ctx(void)
{
	uchar_t r = cfg.curctx;

	do
		r = (r + 1) & ~CTX_MAX;
	while (g_ctx[r].c_cfg.ctxactive && (r != cfg.curctx));

	return r;
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
		_exit(EXIT_SUCCESS);
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
		spawn(pager, NULL, NULL, F_CLI);
		_exit(EXIT_SUCCESS);
	}

	/* In parent */
	waitpid(pid, &tmp, 0);
	close(pipefd[0]);

	return NULL;
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

	r = xstrsncpy(g_buf, "stat \"", PATH_MAX);
	r += xstrsncpy(g_buf + r - 1, fpath, PATH_MAX);
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

#ifdef FILE_MIME_OPTS
			/* Show the file mime type */
			get_output(g_buf, CMD_LEN_MAX, "file", FILE_MIME_OPTS, fpath, FALSE);
			fprintf(fp, "%s", g_buf);
#endif
		}
	}

	fprintf(fp, "\n");
	fclose(fp);
	close(fd);

	spawn(pager, g_tmpfpath, NULL, F_CLI);
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
static void handle_archive(char *fpath, char op)
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

	if (op == 'x') /* extract */
		spawn(util, arg, fpath, F_NORMAL);
	else /* list */
		get_output(NULL, 0, util, arg, fpath, TRUE);
}

static char *visit_parent(char *path, char *newpath, int *presel)
{
	char *dir;

	/* There is no going back */
	if (istopdir(path)) {
		/* Continue in type-to-nav mode, if enabled */
		if (cfg.filtermode && presel)
			*presel = FILTER;
		return NULL;
	}

	/* Use a copy as xdirname() may change the string passed */
	if (newpath)
		xstrsncpy(newpath, path, PATH_MAX);
	else
		newpath = path;

	dir = xdirname(newpath);
	if (chdir(dir) == -1) {
		printwarn(presel);
		return NULL;
	}

	return dir;
}

static void valid_parent(char *path, char *lastname)
{
	/* Save history */
	xstrsncpy(lastname, xbasename(path), NAME_MAX + 1);

	while (!istopdir(path))
		if (visit_parent(path, NULL, NULL))
			break;

	printwarn(NULL);
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

static bool archive_mount(char *newpath)
{
	char *str = "install archivemount";
	char *dir, *cmd = str + 8; /* Start of "archivemount" */
	char *name = pdents[cur].name;
	size_t len = pdents[cur].nlen;
	char mntpath[PATH_MAX];

	if (!getutil(cmd)) {
		printmsg(str);
		return FALSE;
	}

	dir = xstrdup(name);
	if (!dir) {
		printmsg(messages[MSG_FAILED]);
		return FALSE;
	}

	while (len > 1)
		if (dir[--len] == '.') {
			dir[len] = '\0';
			break;
		}

	DPRINTF_S(dir);

	/* Create the mount point */
	mkpath(cfgpath, toks[TOK_MNT], mntpath);
	mkpath(mntpath, dir, newpath);
	free(dir);

	if (!xmktree(newpath, TRUE)) {
		printwarn(NULL);
		return FALSE;
	}

	/* Mount archive */
	DPRINTF_S(name);
	DPRINTF_S(newpath);
	if (spawn(cmd, name, newpath, F_NORMAL)) {
		printmsg(messages[MSG_FAILED]);
		return FALSE;
	}

	return TRUE;
}

static bool remote_mount(char *newpath)
{
	uchar_t flag = F_CLI;
	int opt;
	char *tmp, *env;
	bool r = getutil(utils[UTIL_RCLONE]), s = getutil(utils[UTIL_SSHFS]);
	char mntpath[PATH_MAX];

	if (!(r || s)) {
		printmsg("install sshfs/rclone");
		return FALSE;
	}

	if (r && s)
		opt = get_input(messages[MSG_REMOTE_OPTS]);
	else
		opt = (!s) ? 'r' : 's';

	if (opt == 's')
		env = xgetenv("NNN_SSHFS", utils[UTIL_SSHFS]);
	else if (opt == 'r') {
		flag |= F_NOWAIT | F_NOTRACE;
		env = xgetenv("NNN_RCLONE", "rclone mount");
	} else {
		printmsg(messages[MSG_INVALID_KEY]);
		return FALSE;
	}

	tmp = xreadline(NULL, "host[:dir] > ");
	if (!tmp[0]) {
		printmsg(messages[MSG_CANCEL]);
		return FALSE;
	}

	char *div = strchr(tmp, ':');

	if (div)
		*div = '\0';

	/* Create the mount point */
	mkpath(cfgpath, toks[TOK_MNT], mntpath);
	mkpath(mntpath, tmp, newpath);
	if (!xmktree(newpath, TRUE)) {
		printwarn(NULL);
		return FALSE;
	}

	if (!div) { /* Convert "host" to "host:" */
		size_t len = xstrlen(tmp);

		tmp[len] = ':';
		tmp[len + 1] = '\0';
	} else
		*div = ':';

	/* Connect to remote */
	if (opt == 's') {
		if (spawn(env, tmp, newpath, flag)) {
			printmsg(messages[MSG_FAILED]);
			return FALSE;
		}
	} else {
		spawn(env, tmp, newpath, flag);
		printmsg(messages[MSG_RCLONE_DELAY]);
		xdelay(XDELAY_INTERVAL_MS << 2); /* Set 4 times the usual delay */
	}

	return TRUE;
}

/*
 * Unmounts if the directory represented by name is a mount point.
 * Otherwise, asks for hostname
 * Returns TRUE if directory needs to be refreshed *.
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
	bool hovered = TRUE;
	char mntpath[PATH_MAX];

#ifndef __APPLE__
	/* On Ubuntu it's fusermount */
	if (!found && !getutil(cmd)) {
		cmd[10] = '\0';
		found = TRUE;
	}
#endif

	mkpath(cfgpath, toks[TOK_MNT], mntpath);

	if (tmp && strcmp(mntpath, currentpath) == 0) {
		mkpath(mntpath, tmp, newpath);
		child = lstat(newpath, &sb) != -1;
		parent = lstat(xdirname(newpath), &psb) != -1;
		if (!child && !parent) {
			*presel = MSGWAIT;
			return FALSE;
		}
	}

	if (!tmp || !child || !S_ISDIR(sb.st_mode) || (child && parent && sb.st_dev == psb.st_dev)) {
		tmp = xreadline(NULL, messages[MSG_HOSTNAME]);
		if (!tmp[0])
			return FALSE;
		hovered = FALSE;
	}

	/* Create the mount point */
	mkpath(mntpath, tmp, newpath);
	if (!xdiraccess(newpath)) {
		*presel = MSGWAIT;
		return FALSE;
	}

#ifdef __APPLE__
	if (spawn(cmd, newpath, NULL, F_NORMAL)) {
#else
	if (spawn(cmd, "-u", newpath, F_NORMAL)) {
#endif
		if (!xconfirm(get_input(messages[MSG_LAZY])))
			return FALSE;

#ifdef __APPLE__
		if (spawn(cmd, "-l", newpath, F_NORMAL)) {
#else
		if (spawn(cmd, "-uz", newpath, F_NORMAL)) {
#endif
			printwait(messages[MSG_FAILED], presel);
			return FALSE;
		}
	}

	if (rmdir(newpath) == -1) {
		printwarn(presel);
		return FALSE;
	}

	return hovered;
}

static void lock_terminal(void)
{
	spawn(xgetenv("NNN_LOCKER", utils[UTIL_LOCKER]), NULL, NULL, F_CLI);
}

static void printkv(kv *kvarr, FILE *fp, uchar_t max, uchar_t id)
{
	char *val = (id == NNN_BMS) ? bmstr : pluginstr;

	for (uchar_t i = 0; i < max && kvarr[i].key; ++i)
		fprintf(fp, " %c: %s\n", (char)kvarr[i].key, val + kvarr[i].off);
}

static void printkeys(kv *kvarr, char *buf, uchar_t max)
{
	uchar_t i = 0;

	for (; i < max && kvarr[i].key; ++i) {
		buf[i << 1] = ' ';
		buf[(i << 1) + 1] = kvarr[i].key;
	}

	buf[i << 1] = '\0';
}

static size_t handle_bookmark(const char *bmark, char *newpath)
{
	int fd;
	size_t r = xstrsncpy(g_buf, messages[MSG_BOOKMARK_KEYS], CMD_LEN_MAX);

	if (bmark) { /* There is a marked directory */
		g_buf[--r] = ' ';
		g_buf[++r] = ',';
		g_buf[++r] = '\0';
		++r;
	}
	printkeys(bookmark, g_buf + r - 1, maxbm);
	printmsg(g_buf);

	r = FALSE;
	fd = get_input(NULL);
	if (fd == ',') /* Visit marked directory */
		bmark ? xstrsncpy(newpath, bmark, PATH_MAX) : (r = MSG_NOT_SET);
	else if (!get_kv_val(bookmark, newpath, fd, maxbm, NNN_BMS))
		r = MSG_INVALID_KEY;

	if (!r && chdir(newpath) == -1)
		r = MSG_ACCESS;

	return r;
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
	int fd;
	FILE *fp;
	const char *start, *end;
	const char helpstr[] = {
		"0\n"
		"1NAVIGATION\n"
	       "9Up k  Up%-16cPgUp ^U  Scroll up\n"
	       "9Dn j  Down%-14cPgDn ^D  Scroll down\n"
	       "9Lt h  Parent%-12c~ ` @ -  HOME, /, start, last\n"
	   "5Ret Rt l  Open%-20c'  First file/match\n"
	       "9g ^A  Top%-21c.  Toggle hidden\n"
	       "9G ^E  End%-21c+  Toggle auto-advance\n"
	       "9b ^/  Bookmark key%-12c,  Mark CWD\n"
		"a1-4  Context 1-4%-7c(Sh)Tab  Cycle context\n"
		"aEsc  Send to FIFO%-11c^L  Redraw\n"
		  "cQ  Pick/err, quit%-9c^G  QuitCD\n"
	          "cq  Quit context%-6c2Esc ^Q  Quit\n"
		  "c?  Help, conf\n"
		"1FILTER & PROMPT\n"
		  "c/  Filter%-12cAlt+Esc  Clear filter & redraw\n"
		"aEsc  Exit prompt%-12c^L  Clear prompt/last filter\n"
		 "b^N  Toggle type-to-nav%-0c\n"
		"1FILES\n"
	       "9o ^O  Open with...%-12cn  Create new/link\n"
	       "9f ^F  File details%-12cd  Detail mode toggle\n"
		 "b^R  Rename/dup%-14cr  Batch rename\n"
		  "cz  Archive%-17ce  Edit file\n"
		  "c*  Toggle exe%-14c>  Export list\n"
	   "5Space ^J  (Un)select%-7cm ^Space  Mark range/clear sel\n"
	          "ca  Select all%-14cA  Invert sel\n"
	       "9p ^P  Copy sel here%-8cw ^W  Cp/mv sel as\n"
	       "9v ^V  Move sel here%-11cE  Edit sel\n"
	       "9x ^X  Delete\n"
		"1MISC\n"
	      "8Alt ;  Select plugin%-11c=  Launch app\n"
	       "9! ^]  Shell%-19c]  Cmd prompt\n"
		  "cc  Connect remote%-10cu  Unmount remote/archive\n"
	       "9t ^T  Sort toggles%-12cs  Manage session\n"
		  "cT  Set time type%-11c0  Lock\n"
	};

	fd = create_tmp_file();
	if (fd == -1)
		return;

	fp = fdopen(fd, "w");
	if (!fp) {
		close(fd);
		return;
	}

	if (g_state.fortune && getutil("fortune"))
#ifndef __HAIKU__
		pipetof("fortune -s", fp);
#else
		pipetof("fortune", fp);
#endif

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

	if (bookmark) {
		fprintf(fp, "BOOKMARKS\n");
		printkv(bookmark, fp, maxbm, NNN_BMS);
		fprintf(fp, "\n");
	}

	if (plug) {
		fprintf(fp, "PLUGIN KEYS\n");
		printkv(plug, fp, maxplug, NNN_PLUG);
		fprintf(fp, "\n");
	}

	for (uchar_t i = NNN_OPENER; i <= NNN_TRASH; ++i) {
		start = getenv(env_cfg[i]);
		if (start)
			fprintf(fp, "%s: %s\n", env_cfg[i], start);
	}

	if (selpath)
		fprintf(fp, "SELECTION FILE: %s\n", selpath);

	fprintf(fp, "\nv%s\n%s\n", VERSION, GENERAL_INFO);
	fclose(fp);
	close(fd);

	spawn(pager, g_tmpfpath, NULL, F_CLI);
	unlink(g_tmpfpath);
}

static bool run_cmd_as_plugin(const char *file, char *runfile, uchar_t flags)
{
	size_t len;

	xstrsncpy(g_buf, file, PATH_MAX);

	len = xstrlen(g_buf);
	if (len > 1 && g_buf[len - 1] == '*') {
		flags &= ~F_CONFIRM; /* Skip user confirmation */
		g_buf[len - 1] = '\0'; /* Get rid of trailing no confirmation symbol */
		--len;
	}

	if (is_suffix(g_buf, " $nnn"))
		g_buf[len - 5] = '\0'; /* Set `\0` to clear ' $nnn' suffix */
	else
		runfile = NULL;

	spawn(g_buf, runfile, NULL, flags);
	return TRUE;
}

static bool plctrl_init(void)
{
	size_t len;

	/* g_tmpfpath is used to generate tmp file names */
	g_tmpfpath[tmpfplen - 1] = '\0';
	len = xstrsncpy(g_pipepath, g_tmpfpath, TMP_LEN_MAX);
	g_pipepath[len - 1] = '/';
	len = xstrsncpy(g_pipepath + len, "nnn-pipe.", TMP_LEN_MAX - len) + len;
	xstrsncpy(g_pipepath + len - 1, xitoa(getpid()), TMP_LEN_MAX - len);
	setenv(env_cfg[NNN_PIPE], g_pipepath, TRUE);

	return EXIT_SUCCESS;
}

static void rmlistpath(void)
{
	if (listpath) {
		DPRINTF_S(__func__);
		DPRINTF_S(listpath);
		spawn("rm -rf", listpath, NULL, F_NOTRACE | F_MULTI);
		/* Do not free if program was started in list mode */
		if (listpath != initpath)
			free(listpath);
		listpath = NULL;
	}
}

static ssize_t read_nointr(int fd, void *buf, size_t count)
{
	ssize_t len;

	do
		len = read(fd, buf, count);
	while (len == -1 && errno == EINTR);

	return len;
}

static void readpipe(int fd, char **path, char **lastname, char **lastdir)
{
	int r;
	char ctx, *nextpath = NULL;
	ssize_t len = read_nointr(fd, g_buf, 1);

	if (len != 1)
		return;

	if (g_buf[0] == '+')
		ctx = (char)(get_free_ctx() + 1);
	else if (g_buf[0] < '0')
		return;
	else {
		ctx = g_buf[0] - '0';
		if (ctx > CTX_MAX)
			return;
	}

	len = read_nointr(fd, g_buf, 1);
	if (len != 1)
		return;

	char op = g_buf[0];

	if (op == 'c') {
		len = read_nointr(fd, g_buf, PATH_MAX);
		if (len <= 0)
			return;

		/* Terminate the path read */
		g_buf[len] = '\0';
		nextpath = g_buf;
	} else if (op == 'l') {
		/* Remove last list mode path, if any */
		rmlistpath();

		nextpath = load_input(fd, *path);
	}

	if (nextpath) {
		if (ctx == 0 || ctx == cfg.curctx + 1) {
			xstrsncpy(*lastdir, *path, PATH_MAX);
			xstrsncpy(*path, nextpath, PATH_MAX);
			DPRINTF_S(*path);
		} else {
			r = ctx - 1;

			g_ctx[r].c_cfg.ctxactive = 0;
			savecurctx(&cfg, nextpath, pdents[cur].name, r);
			*path = g_ctx[r].c_path;
			*lastdir = g_ctx[r].c_last;
			*lastname = g_ctx[r].c_name;
		}
	}
}

static bool run_selected_plugin(char **path, const char *file, char *runfile, char **lastname, char **lastdir)
{
	bool cmd_as_plugin = FALSE;
	uchar_t flags = 0;

	if (!g_state.pluginit) {
		plctrl_init();
		g_state.pluginit = 1;
	}

	if (*file == '_') {
		flags = F_MULTI | F_CONFIRM;

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

			run_cmd_as_plugin(file, runfile, flags);
			return TRUE;
		}

		cmd_as_plugin = TRUE;
	}

	if (mkfifo(g_pipepath, 0600) != 0)
		return EXIT_FAILURE;

	exitcurses();

	if (fork() == 0) { // In child
		int wfd = open(g_pipepath, O_WRONLY | O_CLOEXEC);

		if (wfd == -1)
			_exit(EXIT_FAILURE);

		if (!cmd_as_plugin) {
			/* Generate absolute path to plugin */
			mkpath(plgpath, file, g_buf);

			if (runfile && runfile[0]) {
				xstrsncpy(*lastname, runfile, NAME_MAX);
				spawn(g_buf, *lastname, *path, 0);
			} else
				spawn(g_buf, NULL, *path, 0);
		} else
			run_cmd_as_plugin(file, runfile, flags);

		close(wfd);
		_exit(EXIT_SUCCESS);
	}

	int rfd;

	do
		rfd = open(g_pipepath, O_RDONLY);
	while (rfd == -1 && errno == EINTR);

	readpipe(rfd, path, lastname, lastdir);
	close(rfd);

	refresh();

	unlink(g_pipepath);

	return TRUE;
}

static bool plugscript(const char *plugin, uchar_t flags)
{
	mkpath(plgpath, plugin, g_buf);
	if (!access(g_buf, X_OK)) {
		spawn(g_buf, NULL, NULL, flags);
		return TRUE;
	}

	return FALSE;
}

static bool launch_app(char *newpath)
{
	int r = F_NORMAL;
	char *tmp = newpath;

	mkpath(plgpath, utils[UTIL_LAUNCH], newpath);

	if (!getutil(utils[UTIL_FZF]) || access(newpath, X_OK) < 0) {
		tmp = xreadline(NULL, messages[MSG_APP_NAME]);
		r = F_NOWAIT | F_NOTRACE | F_MULTI;
	}

	if (tmp && *tmp) // NOLINT
		spawn(tmp, (r == F_NORMAL) ? "0" : NULL, NULL, r);

	return FALSE;
}

/* Returns TRUE if at least  command was run */
static bool prompt_run(const char *current)
{
	bool ret = FALSE;
	char *tmp;

	setenv(envs[ENV_NCUR], current, 1);

	while (1) {
#ifndef NORL
		if (g_state.picker) {
#endif
			tmp = xreadline(NULL, ">>> ");
#ifndef NORL
		} else
			tmp = getreadline("\n>>> ");
#endif
		if (tmp && *tmp) { // NOLINT
			ret = TRUE;
			spawn(shell, "-c", tmp, F_CLI | F_CONFIRM);
		} else
			break;
	}

	return ret;
}

static bool handle_cmd(enum action sel, const char *current, char *newpath)
{
	endselection();

	if (sel == SEL_RUNCMD)
		return prompt_run(current);

	if (sel == SEL_LAUNCH)
		return launch_app(newpath);

	/* Set nnn nesting level */
	char *tmp = getenv(env_cfg[NNNLVL]);
	int r = tmp ? atoi(tmp) : 0;

	setenv(env_cfg[NNNLVL], xitoa(r + 1), 1);
	setenv(envs[ENV_NCUR], current, 1);
	spawn(shell, NULL, NULL, F_CLI);
	setenv(env_cfg[NNNLVL], xitoa(r), 1);
	return TRUE;
}

static int sum_bsize(const char *UNUSED(fpath), const struct stat *sb, int typeflag, struct FTW *UNUSED(ftwbuf))
{
	if (sb->st_blocks
	    && ((typeflag == FTW_F && (sb->st_nlink <= 1 || test_set_bit((uint_t)sb->st_ino)))
	    || typeflag == FTW_D))
		ent_blocks += sb->st_blocks;

	++num_files;
	return 0;
}

static int sum_asize(const char *UNUSED(fpath), const struct stat *sb, int typeflag, struct FTW *UNUSED(ftwbuf))
{
	if (sb->st_size
	    && ((typeflag == FTW_F && (sb->st_nlink <= 1 || test_set_bit((uint_t)sb->st_ino)))
	    || typeflag == FTW_D))
		ent_blocks += sb->st_size;

	++num_files;
	return 0;
}

static void dentfree(void)
{
	free(pnamebuf);
	free(pdents);
	free(mark);
}

static blkcnt_t dirwalk(char *path, struct stat *psb)
{
	static uint_t open_max;

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

static int dentfill(char *path, struct entry **ppdents)
{
	uchar_t entflags = 0;
	int n = 0, flags = 0;
	ulong_t num_saved;
	struct dirent *dp;
	char *namep, *pnb, *buf = NULL;
	struct entry *dentp;
	size_t off = 0, namebuflen = NAMEBUF_INCR;
	struct stat sb_path, sb;
	DIR *dirp = opendir(path);

	DPRINTF_S(__func__);

	if (!dirp)
		return 0;

	int fd = dirfd(dirp);

	if (cfg.blkorder) {
		num_files = 0;
		dir_blocks = 0;
		buf = (char *)alloca(xstrlen(path) + NAME_MAX + 2);
		if (!buf)
			return 0;

		if (fstatat(fd, path, &sb_path, 0) == -1)
			goto exit;

		if (!ihashbmp) {
			ihashbmp = calloc(1, HASH_OCTETS << 3);
			if (!ihashbmp)
				goto exit;
		} else
			memset(ihashbmp, 0, HASH_OCTETS << 3);

		attron(COLOR_PAIR(cfg.curctx + 1));
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
				if (sb_path.st_dev == sb.st_dev) { // NOLINT
					mkpath(path, namep, buf);

					dir_blocks += dirwalk(buf, &sb);

					if (g_state.interrupt)
						goto exit;
				}
			} else {
				/* Do not recount hard links */
				if (sb.st_nlink <= 1 || test_set_bit((uint_t)sb.st_ino))
					dir_blocks += (cfg.apparentsz ? sb.st_size : sb.st_blocks);
				++num_files;
			}

			continue;
		}

		if (fstatat(fd, namep, &sb, flags) == -1) {
			if (flags || (fstatat(fd, namep, &sb, AT_SYMLINK_NOFOLLOW) == -1)) {
				/* Missing file */
				DPRINTF_U(flags);
				if (!flags) {
					DPRINTF_S(namep);
					DPRINTF_S(strerror(errno));
				}

				entflags = FILE_MISSING;
				memset(&sb, 0, sizeof(struct stat));
			} else /* Orphaned symlink */
				entflags = SYM_ORPHAN;
		}

		if (n == total_dents) {
			total_dents += ENTRY_INCR;
			*ppdents = xrealloc(*ppdents, total_dents * sizeof(**ppdents));
			if (!*ppdents) {
				free(pnamebuf);
				closedir(dirp);
				errexit();
			}
			DPRINTF_P(*ppdents);
		}

		/* If not enough bytes left to copy a file name of length NAME_MAX, re-allocate */
		if (namebuflen - off < NAME_MAX + 1) {
			namebuflen += NAMEBUF_INCR;

			pnb = pnamebuf;
			pnamebuf = (char *)xrealloc(pnamebuf, namebuflen);
			if (!pnamebuf) {
				free(*ppdents);
				closedir(dirp);
				errexit();
			}
			DPRINTF_P(pnamebuf);

			/* realloc() may result in memory move, we must re-adjust if that happens */
			if (pnb != pnamebuf) {
				dentp = *ppdents;
				dentp->name = pnamebuf;

				for (int count = 1; count < n; ++dentp, ++count)
					/* Current filename starts at last filename start + length */
					(dentp + 1)->name = (char *)((size_t)dentp->name + dentp->nlen);
			}
		}

		dentp = *ppdents + n;

		/* Selection file name */
		dentp->name = (char *)((size_t)pnamebuf + off);
		dentp->nlen = xstrsncpy(dentp->name, namep, NAME_MAX + 1);
		off += dentp->nlen;

		/* Copy other fields */
		dentp->t = ((cfg.timetype == T_MOD)
				? sb.st_mtime
				: ((cfg.timetype == T_ACCESS) ? sb.st_atime : sb.st_ctime));
#if !(defined(__sun) || defined(__HAIKU__))
		if (!flags && dp->d_type == DT_LNK) {
			 /* Do not add sizes for links */
			dentp->mode = (sb.st_mode & ~S_IFMT) | S_IFLNK;
			dentp->size = listpath ? sb.st_size : 0;
		} else {
			dentp->mode = sb.st_mode;
			dentp->size = sb.st_size;
		}
#else
		dentp->mode = sb.st_mode;
		dentp->size = sb.st_size;
#endif

#ifndef NOUG
		dentp->uid = sb.st_uid;
		dentp->gid = sb.st_gid;
#endif

		dentp->flags = S_ISDIR(sb.st_mode) ? 0 : ((sb.st_nlink > 1) ? HARD_LINK : 0);
		if (entflags) {
			dentp->flags |= entflags;
			entflags = 0;
		}

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

				if (g_state.interrupt)
					goto exit;
			} else {
				dentp->blocks = (cfg.apparentsz ? sb.st_size : sb.st_blocks);
				/* Do not recount hard links */
				if (sb.st_nlink <= 1 || test_set_bit((uint_t)sb.st_ino))
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
		} else if (dp->d_type == DT_DIR || ((dp->d_type == DT_LNK || dp->d_type == DT_UNKNOWN) && S_ISDIR(sb.st_mode))) {
			dentp->flags |= DIR_OR_LINK_TO_DIR;
#endif
		}

		++n;
	} while ((dp = readdir(dirp)));

exit:
	if (cfg.blkorder)
		attroff(COLOR_PAIR(cfg.curctx + 1));

	/* Should never be null */
	if (closedir(dirp) == -1)
		errexit();

	return n;
}

static void populate(char *path, char *lastname)
{
#ifdef DBGMODE
	struct timespec ts1, ts2;

	clock_gettime(CLOCK_REALTIME, &ts1); /* Use CLOCK_MONOTONIC on FreeBSD */
#endif

	ndents = dentfill(path, &pdents);
	if (!ndents)
		return;

	ENTSORT(pdents, ndents, entrycmpfn);

#ifdef DBGMODE
	clock_gettime(CLOCK_REALTIME, &ts2);
	DPRINTF_U(ts2.tv_nsec - ts1.tv_nsec);
#endif

	/* Find cur from history */
	/* No NULL check for lastname, always points to an array */
	move_cursor(*lastname ? dentfind(lastname, ndents) : 0, 0);

	// Force full redraw
	last_curscroll = -1;
}

#ifndef NOFIFO
static void notify_fifo(bool force)
{
	if (!fifopath)
		return;

	if (fifofd == -1) {
		fifofd = open(fifopath, O_WRONLY|O_NONBLOCK|O_CLOEXEC);
		if (fifofd == -1) {
			if (errno != ENXIO)
				/* Unexpected error, the FIFO file might have been removed */
				/* We give up FIFO notification */
				fifopath = NULL;
			return;
		}
	}

	static struct entry lastentry;

	if (!force && !memcmp(&lastentry, &pdents[cur], sizeof(struct entry)))
		return;

	lastentry = pdents[cur];

	char path[PATH_MAX];
	size_t len = mkpath(g_ctx[cfg.curctx].c_path, ndents ? pdents[cur].name : "", path);

	path[len - 1] = '\n';

	ssize_t ret = write(fifofd, path, len);

	if (ret != (ssize_t)len && !(ret == -1 && (errno == EAGAIN || errno == EPIPE))) {
		DPRINTF_S(strerror(errno));
	}
}
#endif

static void move_cursor(int target, int ignore_scrolloff)
{
	int onscreen = xlines - 4; /* Leave top 2 and bottom 2 lines */

	target = MAX(0, MIN(ndents - 1, target));
	last_curscroll = curscroll;
	last = cur;
	cur = target;

	if (!ignore_scrolloff) {
		int delta = target - last;
		int scrolloff = MIN(SCROLLOFF, onscreen >> 1);

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

#ifndef NOFIFO
	notify_fifo(FALSE);
#endif
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
		int c = get_input(messages[MSG_FIRST]);

		if (!c)
			break;

		c = TOUPPER(c);

		int r = (c == TOUPPER(*pdents[cur].name)) ? (cur + 1) : 0;

		for (; r < ndents; ++r) {
			if (((c == '\'') && !(pdents[r].flags & DIR_OR_LINK_TO_DIR))
			    || (c == TOUPPER(*pdents[r].name))) {
				move_cursor((r) % ndents, 0);
				break;
			}
		}
		break;
	}
	}
}

static void copynextname(char *lastname)
{
	if (cur) {
		cur += (cur != (ndents - 1)) ? 1 : -1;
		copycurname();
	} else
		lastname[0] = '\0';
}

static int handle_context_switch(enum action sel)
{
	int r = -1;

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
			if (sel == SEL_CYCLE)
				(r == CTX_MAX - 1) ? (r = 0) : ++r;
			else if (sel == SEL_CYCLER)
				(r == 0) ? (r = CTX_MAX - 1) : --r;
			else
				return -1;
		}

		if (g_state.selmode)
			lastappendpos = selbufpos;
	}

	return r;
}

static int set_sort_flags(int r)
{
	bool session = !r;

	/* Set the correct input in case of a session load */
	if (session) {
		if (cfg.apparentsz) {
			cfg.apparentsz = 0;
			r = 'a';
		} else if (cfg.blkorder) {
			cfg.blkorder = 0;
			r = 'd';
		}

		if (cfg.version)
			namecmpfn = &xstrverscasecmp;

		if (cfg.reverse)
			entrycmpfn = &reventrycmp;
	} else if (r == CONTROL('T')) {
		/* Cycling order: clear -> size -> time -> clear */
		if (cfg.timeorder)
			r = 's';
		else if (cfg.sizeorder)
			r = 'c';
		else
			r = 't';
	}

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
		cfg.timeorder = 0;
		cfg.sizeorder = 0;
		cfg.extnorder = 0;
		if (!session) {
			cfg.reverse = 0;
			entrycmpfn = &entrycmp;
		}
		endselection(); /* We are going to reload dir */
		break;
	case 'c':
		cfg.timeorder = 0;
		cfg.sizeorder = 0;
		cfg.apparentsz = 0;
		cfg.blkorder = 0;
		cfg.extnorder = 0;
		cfg.reverse = 0;
		cfg.version = 0;
		entrycmpfn = &entrycmp;
		namecmpfn = &xstricmp;
		break;
	case 'e': /* File extension */
		cfg.extnorder ^= 1;
		cfg.sizeorder = 0;
		cfg.timeorder = 0;
		cfg.apparentsz = 0;
		cfg.blkorder = 0;
		cfg.reverse = 0;
		entrycmpfn = &entrycmp;
		break;
	case 'r': /* Reverse sort */
		cfg.reverse ^= 1;
		entrycmpfn = cfg.reverse ? &reventrycmp : &entrycmp;
		break;
	case 's': /* File size */
		cfg.sizeorder ^= 1;
		cfg.timeorder = 0;
		cfg.apparentsz = 0;
		cfg.blkorder = 0;
		cfg.extnorder = 0;
		cfg.reverse = 0;
		entrycmpfn = &entrycmp;
		break;
	case 't': /* Time */
		cfg.timeorder ^= 1;
		cfg.sizeorder = 0;
		cfg.apparentsz = 0;
		cfg.blkorder = 0;
		cfg.extnorder = 0;
		cfg.reverse = 0;
		entrycmpfn = &entrycmp;
		break;
	case 'v': /* Version */
		cfg.version ^= 1;
		namecmpfn = cfg.version ? &xstrverscasecmp : &xstricmp;
		cfg.timeorder = 0;
		cfg.sizeorder = 0;
		cfg.apparentsz = 0;
		cfg.blkorder = 0;
		cfg.extnorder = 0;
		break;
	default:
		return 0;
	}

	return r;
}

static bool set_time_type(int *presel)
{
	bool ret = FALSE;
	char buf[] = "'a'ccess / 'c'hange / 'm'od [ ]";

	buf[sizeof(buf) - 3] = cfg.timetype == T_MOD ? 'm' : (cfg.timetype == T_ACCESS ? 'a' : 'c');

	int r = get_input(buf);

	if (r == 'a' || r == 'c' || r == 'm') {
		r = (r == 'm') ? T_MOD : ((r == 'a') ? T_ACCESS : T_CHANGE);
		if (cfg.timetype != r) {
			cfg.timetype = r;

			if (cfg.filtermode || g_ctx[cfg.curctx].c_fltr[1])
				*presel = FILTER;

			ret = TRUE;
		} else
			r = MSG_NOCHNAGE;
	} else
		r = MSG_INVALID_KEY;

	if (!ret)
		printwait(messages[r], presel);

	return ret;
}

static void statusbar(char *path)
{
	int i = 0, extnlen = 0;
	char *ptr;
	pEntry pent = &pdents[cur];

	if (!ndents) {
		printmsg("0/0");
		return;
	}

	/* Get the file extension for regular files */
	if (S_ISREG(pent->mode)) {
		i = (int)(pent->nlen - 1);
		ptr = xextension(pent->name, i);
		if (ptr)
			extnlen = i - (ptr - pent->name);
		if (!ptr || extnlen > 5 || extnlen < 2)
			ptr = "\b";
	} else
		ptr = "\b";

	tolastln();
	attron(COLOR_PAIR(cfg.curctx + 1));

	printw("%d/%d ", cur + 1, ndents);

	if (g_state.selmode) {
		attron(A_REVERSE);
		addch(' ');
		if (g_state.rangesel)
			addch('*');
		else if (nselected)
			addstr(xitoa(nselected));
		else
			addch('+');
		addch(' ');
		attroff(A_REVERSE);
		addch(' ');
	}

	if (cfg.blkorder) { /* du mode */
		char buf[24];

		xstrsncpy(buf, coolsize(dir_blocks << blk_shift), 12);

		printw("%cu:%s free:%s files:%lu %lldB %s\n",
		       (cfg.apparentsz ? 'a' : 'd'), buf, coolsize(get_fs_info(path, FREE)),
		       num_files, (long long)pent->blocks << blk_shift, ptr);
	} else { /* light or detail mode */
		char sort[] = "\0\0\0\0\0";

		if (getorderstr(sort))
			printw("%s", sort);

		/* Timestamp */
		print_time(&pent->t);

		addch(' ');
		addstr(get_lsperms(pent->mode));
		addch(' ');
#ifndef NOUG
		if (g_state.uidgid) {
			struct passwd *pw = getpwuid(pent->uid);
			struct group  *gr = getgrgid(pent->gid);

			if (pw)
				addstr(pw->pw_name);
			else
				addch('-');
			addch(' ');

			if (gr)
				addstr(gr->gr_name);
			else
				addch('-');
			addch(' ');
		}
#endif
		addstr(coolsize(pent->size));
		addch(' ');
		addstr(ptr);
		addch('\n');
	}

	attroff(COLOR_PAIR(cfg.curctx + 1));

	if (cfg.cursormode)
		tocursor();
}

static int adjust_cols(int ncols)
{
	/* Calculate the number of cols available to print entry name */
	if (cfg.showdetail) {
		/* Fallback to light mode if less than 35 columns */
		if (ncols < 36) {
			cfg.showdetail ^= 1;
			printptr = &printent;
		} else {
			/* 3 more accounted for below */
			ncols -= 32;
		}
	}

/* 3 = Preceding space, indicator, newline */
#ifdef ICONS_ENABLED
	ncols -= 3 + xstrlen(ICON_PADDING_LEFT) + xstrlen(ICON_PADDING_RIGHT) + 1;
#else
	ncols -= 3;
#endif

	return ncols;
}

static void draw_line(char *path, int ncols)
{
	bool dir = FALSE;

	ncols = adjust_cols(ncols);

	if (g_state.oldcolor && (pdents[last].flags & DIR_OR_LINK_TO_DIR)) {
		attron(COLOR_PAIR(cfg.curctx + 1) | A_BOLD);
		dir = TRUE;
	}

	move(2 + last - curscroll, 0);
	printptr(&pdents[last], ncols, false);

	if (g_state.oldcolor && (pdents[cur].flags & DIR_OR_LINK_TO_DIR)) {
		if (!dir)  {/* First file is not a directory */
			attron(COLOR_PAIR(cfg.curctx + 1) | A_BOLD);
			dir = TRUE;
		}
	} else if (dir) { /* Second file is not a directory */
		attroff(COLOR_PAIR(cfg.curctx + 1) | A_BOLD);
		dir = FALSE;
	}

	move(2 + cur - curscroll, 0);
	printptr(&pdents[cur], ncols, true);

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
	if (g_state.move) {
		g_state.move = 0;

		if (ndents && (last_curscroll == curscroll))
			return draw_line(path, ncols);
	}

	DPRINTF_S(__func__);

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
		if (!g_ctx[i].c_cfg.ctxactive)
			addch(i + '1');
		else
			addch((i + '1') | (COLOR_PAIR(i + 1) | A_BOLD
				/* active: underline, current: reverse */
				| ((cfg.curctx != i) ? A_UNDERLINE : A_REVERSE)));

		if (i != CTX_MAX - 1)
			addch(' ');
	}
	addstr("] "); /* 10 chars printed for contexts - "[1 2 3 4] " */

	attron(A_UNDERLINE | COLOR_PAIR(cfg.curctx + 1));

	/* Print path */
	i = (int)xstrlen(path);
	if ((i + MIN_DISPLAY_COLS) <= ncols)
		addnstr(path, ncols - MIN_DISPLAY_COLS);
	else {
		char *base = xmemrchr((uchar_t *)path, '/', i);

		i = 0;

		if (base != ptr) {
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
		}

		addnstr(base, ncols - (MIN_DISPLAY_COLS + i));
	}

	attroff(A_UNDERLINE | COLOR_PAIR(cfg.curctx + 1));

	ncols = adjust_cols(ncols);

	/* Go to first entry */
	if (curscroll > 0) {
		move(1, 0);
		addch('^');
	}

	move(2, 0);

	if (g_state.oldcolor) {
		attron(COLOR_PAIR(cfg.curctx + 1) | A_BOLD);
		g_state.dircolor = 1;
	}

	/* Print listing */
	for (i = curscroll; i < ndents && i < curscroll + onscreen; ++i)
		printptr(&pdents[i], ncols, i == cur);

	/* Must reset e.g. no files in dir */
	if (g_state.dircolor) {
		attroff(COLOR_PAIR(cfg.curctx + 1) | A_BOLD);
		g_state.dircolor = 0;
	}

	/* Go to first entry */
	if (i < ndents) {
		move(xlines - 2, 0);
		addch('v');
	}

	statusbar(path);
}

static bool cdprep(char *lastdir, char *lastname, char *path, char *newpath)
{
	if (lastname)
		lastname[0] =  '\0';

	/* Save last working directory */
	xstrsncpy(lastdir, path, PATH_MAX);

	/* Save the newly opted dir in path */
	xstrsncpy(path, newpath, PATH_MAX);
	DPRINTF_S(path);

	clearfilter();
	return cfg.filtermode;
}

static bool browse(char *ipath, const char *session, int pkey)
{
	char newpath[PATH_MAX] __attribute__ ((aligned));
	char rundir[PATH_MAX] __attribute__ ((aligned));
	char runfile[NAME_MAX + 1] __attribute__ ((aligned));
	char *path, *lastdir, *lastname, *dir, *tmp;
	pEntry pent;
	enum action sel;
	struct stat sb;
	int r = -1, presel, selstartid = 0, selendid = 0;
	const uchar_t opener_flags = (cfg.cliopener ? F_CLI : (F_NOTRACE | F_NOWAIT));
	bool watch = FALSE;

#ifndef NOMOUSE
	MEVENT event;
	struct timespec mousetimings[2] = {{.tv_sec = 0, .tv_nsec = 0}, {.tv_sec = 0, .tv_nsec = 0} };
	int mousedent[2] = {-1, -1};
	bool currentmouse = 1, rightclicksel = 0;
#endif

#ifndef DIR_LIMITED_SELECTION
	ino_t inode = 0;
#endif

	atexit(dentfree);

	xlines = LINES;
	xcols = COLS;

#ifndef NOSSN
	/* setup first context */
	if (!session || !load_session(session, &path, &lastdir, &lastname, FALSE)) {
#else
		(void)session;
#endif
		g_ctx[0].c_last[0] = '\0';
		lastdir = g_ctx[0].c_last; /* last visited directory */

		if (g_state.initfile) {
			xstrsncpy(g_ctx[0].c_name, xbasename(ipath), sizeof(g_ctx[0].c_name));
			xdirname(ipath);
		} else
			g_ctx[0].c_name[0] = '\0';

		lastname = g_ctx[0].c_name; /* last visited filename */

		xstrsncpy(g_ctx[0].c_path, ipath, PATH_MAX);
		path = g_ctx[0].c_path; /* current directory */

		g_ctx[0].c_fltr[0] = g_ctx[0].c_fltr[1] = '\0';
		g_ctx[0].c_cfg = cfg; /* current configuration */
#ifndef NOSSN
	}
#endif

	newpath[0] = rundir[0] = runfile[0] = '\0';

	presel = pkey ? ';' : (cfg.filtermode ? FILTER : 0);

	pdents = xrealloc(pdents, total_dents * sizeof(struct entry));
	if (!pdents)
		errexit();

	/* Allocate buffer to hold names */
	pnamebuf = (char *)xrealloc(pnamebuf, NAMEBUF_INCR);
	if (!pnamebuf)
		errexit();

begin:
	/* Can fail when permissions change while browsing.
	 * It's assumed that path IS a directory when we are here.
	 */
	if (chdir(path) == -1) {
		DPRINTF_S("directory inaccessible");
		valid_parent(path, lastname);
		setdirwatch();
	}

	if (g_state.selmode && lastdir[0])
		lastappendpos = selbufpos;

#ifdef LINUX_INOTIFY
	if ((presel == FILTER || watch) && inotify_wd >= 0) {
		inotify_rm_watch(inotify_fd, inotify_wd);
		inotify_wd = -1;
		watch = FALSE;
	}
#elif defined(BSD_KQUEUE)
	if ((presel == FILTER || watch) && event_fd >= 0) {
		close(event_fd);
		event_fd = -1;
		watch = FALSE;
	}
#elif defined(HAIKU_NM)
	if ((presel == FILTER || watch) && haiku_hnd != NULL) {
		haiku_stop_watch(haiku_hnd);
		haiku_nm_active = FALSE;
		watch = FALSE;
	}
#endif

	populate(path, lastname);
	if (g_state.interrupt) {
		g_state.interrupt = 0;
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
	haiku_nm_active = haiku_watch_dir(haiku_hnd, path) == EXIT_SUCCESS;
#endif

	while (1) {
		/* Do not do a double redraw in filterentries */
		if ((presel != FILTER) || !filterset())
			redraw(path);

nochange:
		/* Exit if parent has exited */
		if (getppid() == 1)
			_exit(EXIT_FAILURE);

		/* If CWD is deleted or moved or perms changed, find an accessible parent */
		if (chdir(path) == -1)
			goto begin;

		/* If STDIN is no longer a tty (closed) we should exit */
		if (!isatty(STDIN_FILENO) && !g_state.picker)
			return EXIT_FAILURE;

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
					if (g_state.selmode)
						lastappendpos = selbufpos;

					savecurctx(&cfg, path, pdents[cur].name, r);

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

				/* Save history */
				xstrsncpy(lastname, xbasename(path), NAME_MAX + 1);

				cdprep(lastdir, NULL, path, dir) ? (presel = FILTER) : (watch = TRUE);
				goto begin;
#ifndef NOMOUSE
			}
#endif

#ifndef NOMOUSE
			/* Middle click action */
			if (event.bstate == BUTTON2_PRESSED) {
				presel = middle_click_key;
				goto nochange;
			}
#if NCURSES_MOUSE_VERSION > 1
			/* Scroll up */
			if (event.bstate == BUTTON4_PRESSED && ndents && (cfg.rollover || cur)) {
				if (!cfg.rollover && cur < scroll_lines)
					move_cursor(0, 0);
				else
					move_cursor((cur + ndents - scroll_lines) % ndents, 0);
				break;
			}

			/* Scroll down */
			if (event.bstate == BUTTON5_PRESSED && ndents
			    && (cfg.rollover || (cur != ndents - 1))) {
				if (!cfg.rollover && cur >= ndents - scroll_lines)
					move_cursor(ndents-1, 0);
				else
					move_cursor((cur + scroll_lines) % ndents, 0);
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
				watch = TRUE;

				if (ndents)
					copycurname();
				goto begin;
			}

			/* Handle clicking on a file */
			if (event.y >= 2 && event.y <= ndents + 1 &&
					(event.bstate == BUTTON1_PRESSED ||
					 event.bstate == BUTTON3_PRESSED)) {
				r = curscroll + (event.y - 2);
				if (r != cur)
					move_cursor(r, 1);
#ifndef NOFIFO
				else if (event.bstate == BUTTON1_PRESSED)
					notify_fifo(TRUE);
#endif
				/* Handle right click selection */
				if (event.bstate == BUTTON3_PRESSED) {
					rightclicksel = 1;
					presel = SELECT;
					goto nochange;
				}

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
				mousedent[currentmouse] = cur;

				/* Single click just selects, double click falls through to SEL_OPEN */
				if ((mousedent[0] != mousedent[1]) ||
				  (((_ABSSUB(mousetimings[0].tv_sec, mousetimings[1].tv_sec) << 30)
				  + (_ABSSUB(mousetimings[0].tv_nsec, mousetimings[1].tv_nsec)))
					> DOUBLECLICK_INTERVAL_NS))
					break;
				mousetimings[currentmouse].tv_sec = 0;
				mousedent[currentmouse] = -1;
			} else {
				if (cfg.filtermode || filterset())
					presel = FILTER;
				if (ndents)
					copycurname();
				goto nochange;
			}
#endif
			// fallthrough
		case SEL_NAV_IN: // fallthrough
		case SEL_OPEN:
			/* Cannot descend in empty directories */
			if (!ndents)
				goto begin;

			pent = &pdents[cur];
			mkpath(path, pent->name, newpath);
			DPRINTF_S(newpath);

			/* Visit directory */
			if (pent->flags & DIR_OR_LINK_TO_DIR) {
				if (chdir(newpath) == -1) {
					printwarn(&presel);
					goto nochange;
				}

				cdprep(lastdir, lastname, path, newpath) ? (presel = FILTER) : (watch = TRUE);
				goto begin;
			}

			/* Cannot use stale data in entry, file may be missing by now */
			if (stat(newpath, &sb) == -1) {
				printwarn(&presel);
				goto nochange;
			}
			DPRINTF_U(sb.st_mode);

			/* Do not open non-regular files */
			if (!S_ISREG(sb.st_mode)) {
				printwait(messages[MSG_UNSUPPORTED], &presel);
				goto nochange;
			}

			/* If opened as vim plugin and Enter/^M pressed, pick */
			if (g_state.picker && sel == SEL_OPEN) {
				appendfpath(newpath, mkpath(path, pent->name, newpath));
				writesel(pselbuf, selbufpos - 1);
				return EXIT_SUCCESS;
			}

			if (sel == SEL_NAV_IN) {
				/* If in listing dir, go to target on `l` or Right on symlink */
				if (listpath && S_ISLNK(pent->mode)
				    && is_prefix(path, listpath, xstrlen(listpath))) {
					if (!realpath(pent->name, newpath)) {
						printwarn(&presel);
						goto nochange;
					}

					xdirname(newpath);

					if (chdir(newpath) == -1) {
						printwarn(&presel);
						goto nochange;
					}

					/* Mark current directory */
					free(mark);
					mark = xstrdup(path);

					cdprep(lastdir, NULL, path, newpath)
					       ? (presel = FILTER) : (watch = TRUE);
					xstrsncpy(lastname, pent->name, NAME_MAX + 1);
					goto begin;
				}

				/* Open file disabled on right arrow or `l` */
				if (cfg.nonavopen)
					goto nochange;
			}

			/* Handle plugin selection mode */
			if (g_state.runplugin) {
				g_state.runplugin = 0;
				/* Must be in plugin dir and same context to select plugin */
				if ((g_state.runctx == cfg.curctx) && !strcmp(path, plgpath)) {
					endselection();
					/* Copy path so we can return back to earlier dir */
					xstrsncpy(path, rundir, PATH_MAX);
					rundir[0] = '\0';

					if (chdir(path) == -1
					    || !run_selected_plugin(&path, pent->name,
								    runfile, &lastname, &lastdir)) {
						DPRINTF_S("plugin failed!");
					}

					if (runfile[0])
						runfile[0] = '\0';
					clearfilter();
					setdirwatch();
					goto begin;
				}
			}

			if (!sb.st_size) {
				printwait(messages[MSG_EMPTY_FILE], &presel);
				goto nochange;
			}

			if (cfg.useeditor
#ifdef FILE_MIME
			    && get_output(g_buf, CMD_LEN_MAX, "file", FILE_MIME_OPTS, newpath, FALSE)
			    && is_prefix(g_buf, "text/", 5)
#else
			    /* no mime option; guess from description instead */
			    && get_output(g_buf, CMD_LEN_MAX, "file", "-b", newpath, FALSE)
			    && strstr(g_buf, "text")
#endif
			) {
				spawn(editor, newpath, NULL, F_CLI);
				continue;
			}

			/* Get the extension for regext match */
			tmp = xextension(pent->name, pent->nlen - 1);
#ifdef PCRE
			if (tmp && !pcre_exec(archive_pcre, NULL, tmp,
					      pent->nlen - (tmp - pent->name) - 1, 0, 0, NULL, 0)) {
#else
			if (tmp && !regexec(&archive_re, tmp, 0, NULL, 0)) {
#endif
				r = get_input(messages[MSG_ARCHIVE_OPTS]);
				if (r == 'l' || r == 'x') {
					mkpath(path, pent->name, newpath);
					handle_archive(newpath, r);
					if (r == 'l') {
						statusbar(path);
						goto nochange;
					}
					copycurname();
					clearfilter();
					goto begin;
				}

				if (r == 'm') {
					if (!archive_mount(newpath)) {
						presel = MSGWAIT;
						goto nochange;
					}

					/* Mark current directory */
					free(mark);
					mark = xstrdup(path);

					cdprep(lastdir, lastname, path, newpath)
						? (presel = FILTER) : (watch = TRUE);
					goto begin;
				}

				if (r != 'd') {
					printwait(messages[MSG_INVALID_KEY], &presel);
					goto nochange;
				}
			}

			/* Invoke desktop opener as last resort */
			spawn(opener, newpath, NULL, opener_flags);

			/* Move cursor to the next entry if not the last entry */
			if (g_state.autonext && cur != ndents - 1)
				move_cursor((cur + 1) % ndents, 0);
			continue;
		case SEL_NEXT: // fallthrough
		case SEL_PREV: // fallthrough
		case SEL_PGDN: // fallthrough
		case SEL_CTRL_D: // fallthrough
		case SEL_PGUP: // fallthrough
		case SEL_CTRL_U: // fallthrough
		case SEL_HOME: // fallthrough
		case SEL_END: // fallthrough
		case SEL_FIRST:
			if (ndents) {
				g_state.move = 1;
				handle_screen_move(sel);
			}
			break;
		case SEL_CDHOME: // fallthrough
		case SEL_CDBEGIN: // fallthrough
		case SEL_CDLAST: // fallthrough
		case SEL_CDROOT:
			dir = (sel == SEL_CDHOME) ? home
				: ((sel == SEL_CDBEGIN) ? ipath
				: ((sel == SEL_CDLAST) ? lastdir
				: "/" /* SEL_CDROOT */));

			if (!dir || !*dir) {
				printwait(messages[MSG_NOT_SET], &presel);
				goto nochange;
			}

			if (strcmp(path, dir) == 0) {
				if (cfg.filtermode)
					presel = FILTER;
				goto nochange;
			}

			if (chdir(dir) == -1) {
				presel = MSGWAIT;
				goto nochange;
			}

			/* SEL_CDLAST: dir pointing to lastdir */
			xstrsncpy(newpath, dir, PATH_MAX); // fallthrough
		case SEL_BOOKMARK:
			if (sel == SEL_BOOKMARK) {
				r = (int)handle_bookmark(mark, newpath);
				if (r) {
					printwait(messages[r], &presel);
					goto nochange;
				}

				if (strcmp(path, newpath) == 0)
					break;
			} // fallthrough
		case SEL_REMOTE:
			if (sel == SEL_REMOTE && !remote_mount(newpath)) {
				presel = MSGWAIT;
				goto nochange;
			}

			/* Mark current directory */
			free(mark);
			mark = xstrdup(path);

			/* In list mode, retain the last file name to highlight it, if possible */
			cdprep(lastdir, listpath && sel == SEL_CDLAST ? NULL : lastname, path, newpath)
			       ? (presel = FILTER) : (watch = TRUE);
			goto begin;
		case SEL_CYCLE: // fallthrough
		case SEL_CYCLER: // fallthrough
		case SEL_CTX1: // fallthrough
		case SEL_CTX2: // fallthrough
		case SEL_CTX3: // fallthrough
		case SEL_CTX4:
#ifdef CTX8
		case SEL_CTX5:
		case SEL_CTX6:
		case SEL_CTX7:
		case SEL_CTX8:
#endif
			r = handle_context_switch(sel);
			if (r < 0)
				continue;
			savecurctx(&cfg, path, pdents[cur].name, r);

			/* Reset the pointers */
			path = g_ctx[r].c_path;
			lastdir = g_ctx[r].c_last;
			lastname = g_ctx[r].c_name;
			tmp = g_ctx[r].c_fltr;

			if (cfg.filtermode || ((tmp[0] == FILTER || tmp[0] == RFILTER) && tmp[1]))
				presel = FILTER;
			else
				watch = TRUE;

			goto begin;
		case SEL_MARK:
			free(mark);
			mark = xstrdup(path);
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

			if (presel == ESC) {
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

				watch = TRUE; // fallthrough
			case SEL_HIDDEN:
				if (sel == SEL_HIDDEN) {
					cfg.showhidden ^= 1;
					if (cfg.filtermode)
						presel = FILTER;
					clearfilter();
				}
				if (ndents)
					copycurname();
				goto begin;
			case SEL_DETAIL:
				cfg.showdetail ^= 1;
				cfg.showdetail ? (printptr = &printent_long) : (printptr = &printent);
				cfg.blkorder = 0;
				continue;
			default: /* SEL_SORT */
				r = set_sort_flags(get_input(messages[MSG_ORDER]));
				if (!r) {
					printwait(messages[MSG_INVALID_KEY], &presel);
					goto nochange;
				}
			}

			if (cfg.filtermode || filterset())
				presel = FILTER;

			if (ndents) {
				copycurname();

				if (r == 'd' || r == 'a')
					goto begin;

				ENTSORT(pdents, ndents, entrycmpfn);
				move_cursor(ndents ? dentfind(lastname, ndents) : 0, 0);
			}
			continue;
		case SEL_STATS: // fallthrough
		case SEL_CHMODX:
			if (ndents) {
				tmp = (listpath && xstrcmp(path, listpath) == 0) ? listroot : path;
				mkpath(tmp, pdents[cur].name, newpath);

				if (lstat(newpath, &sb) == -1
				    || (sel == SEL_STATS && !show_stats(newpath, &sb))
				    || (sel == SEL_CHMODX && !xchmod(newpath, sb.st_mode))) {
					printwarn(&presel);
					goto nochange;
				}

				if (sel == SEL_CHMODX)
					pdents[cur].mode ^= 0111;
			}
			break;
		case SEL_REDRAW: // fallthrough
		case SEL_RENAMEMUL: // fallthrough
		case SEL_HELP: // fallthrough
		case SEL_AUTONEXT: // fallthrough
		case SEL_EDIT: // fallthrough
		case SEL_LOCK:
		{
			bool refresh = FALSE;

			if (ndents)
				mkpath(path, pdents[cur].name, newpath);
			else if (sel == SEL_EDIT) /* Avoid trying to edit a non-existing file */
				goto nochange;

			switch (sel) {
			case SEL_REDRAW:
				refresh = TRUE;
				break;
			case SEL_RENAMEMUL:
				endselection();

				if (!(getutil(utils[UTIL_BASH])
				      && plugscript(utils[UTIL_NMV], F_CLI))
#ifndef NOBATCH
				    && !batch_rename()
#endif
				) {
					printwait(messages[MSG_FAILED], &presel);
					goto nochange;
				}
				clearselection();
				refresh = TRUE;
				break;
			case SEL_HELP:
				show_help(path); // fallthrough
			case SEL_AUTONEXT:
				if (sel == SEL_AUTONEXT)
					g_state.autonext ^= 1;
				if (cfg.filtermode)
					presel = FILTER;
				if (ndents)
					copycurname();
				goto nochange;
			case SEL_EDIT:
				spawn(editor, newpath, NULL, F_CLI);
				continue;
			default: /* SEL_LOCK */
				lock_terminal();
				break;
			}

			/* In case of successful operation, reload contents */

			/* Continue in type-to-nav mode, if enabled */
			if ((cfg.filtermode || filterset()) && !refresh) {
				presel = FILTER;
				goto nochange;
			}

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
			if (g_state.rangesel)
				g_state.rangesel = 0;

			/* Toggle selection status */
			pdents[cur].flags ^= FILE_SELECTED;

			if (pdents[cur].flags & FILE_SELECTED) {
				++nselected;
				appendfpath(newpath, mkpath(path, pdents[cur].name, newpath));
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
				plugscript(utils[UTIL_CBCP], F_NOWAIT | F_NOTRACE);

			if (!nselected)
				unlink(selpath);
#ifndef NOMOUSE
			if (rightclicksel)
				rightclicksel = 0;
			else
#endif
				/* move cursor to the next entry if this is not the last entry */
				if (!g_state.stayonsel && !g_state.picker && cur != ndents - 1)
					move_cursor((cur + 1) % ndents, 0);
			break;
		case SEL_SELMUL:
			if (!ndents)
				goto nochange;

			startselection();
			g_state.rangesel ^= 1;

			if (stat(path, &sb) == -1) {
				printwarn(&presel);
				goto nochange;
			}

			if (g_state.rangesel) { /* Range selection started */
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
		case SEL_SELALL: // fallthrough
		case SEL_SELINV:
			if (sel == SEL_SELALL || sel == SEL_SELINV) {
				if (!ndents)
					goto nochange;

				startselection();
				if (g_state.rangesel)
					g_state.rangesel = 0;

				selstartid = 0;
				selendid = ndents - 1;
			}

			if (sel == SEL_SELINV) {
				/* Toggle selection status */
				for (r = selstartid; r <= selendid; ++r) {
					pdents[r].flags ^= FILE_SELECTED;
					pdents[r].flags & FILE_SELECTED ? ++nselected : --nselected;
				}

				selbufpos = lastappendpos;
				if (nselected) {
					updateselbuf(path, newpath);
					writesel(pselbuf, selbufpos - 1); /* Truncate NULL from end */
				} else
					writesel(NULL, 0);
			} else {
				/* Remember current selection buffer position */
				for (r = selstartid; r <= selendid; ++r) {
					if (!(pdents[r].flags & FILE_SELECTED)) {
						/* Write the path to selection file to avoid flush */
						appendfpath(newpath, mkpath(path, pdents[r].name, newpath));

						pdents[r].flags |= FILE_SELECTED;
						++nselected;
					}
				}

				writesel(pselbuf, selbufpos - 1); /* Truncate NULL from end */
			}

			if (cfg.x11)
				plugscript(utils[UTIL_CBCP], F_NOWAIT | F_NOTRACE);
			continue;
		case SEL_SELEDIT:
			r = editselection();
			if (r <= 0) {
				r = !r ? MSG_0_SELECTED : MSG_FAILED;
				printwait(messages[r], &presel);
			} else {
				if (cfg.x11)
					plugscript(utils[UTIL_CBCP], F_NOWAIT | F_NOTRACE);
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
					tmp = (listpath && xstrcmp(path, listpath) == 0)
					      ? listroot : path;
					mkpath(tmp, pdents[cur].name, newpath);
					if (!xrm(newpath))
						continue;

					copynextname(lastname);
					clearselection();

					if (cfg.filtermode || filterset())
						presel = FILTER;
					goto begin;
				}
			}

			if (nselected == 1 && (sel == SEL_CP || sel == SEL_MV))
				mkpath(path, xbasename(pselbuf), newpath);
			else
				newpath[0] = '\0';

			endselection();

			if (!cpmvrm_selection(sel, path)) {
				presel = MSGWAIT;
				goto nochange;
			}

			if (cfg.filtermode)
				presel = FILTER;
			clearfilter();

			/* Show notification on operation complete */
			if (cfg.x11)
				plugscript(utils[UTIL_NTFY], F_NOWAIT | F_NOTRACE);

			if (newpath[0] && !access(newpath, F_OK))
				xstrsncpy(lastname, xbasename(newpath), NAME_MAX+1);
			else if (ndents)
				copycurname();
			goto begin;
		}
		case SEL_ARCHIVE: // fallthrough
		case SEL_OPENWITH: // fallthrough
		case SEL_NEW: // fallthrough
		case SEL_RENAME:
		{
			int fd, ret = 'n';

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
					tmp = pdents[cur].name;

				tmp = xreadline(tmp, messages[MSG_ARCHIVE_NAME]);
				break;
			case SEL_OPENWITH:
#ifdef NORL
				tmp = xreadline(NULL, messages[MSG_OPEN_WITH]);
#else
				tmp = getreadline(messages[MSG_OPEN_WITH]);
#endif
				break;
			case SEL_NEW:
				r = get_input(messages[MSG_NEW_OPTS]);
				if (r == 'f' || r == 'd')
					tmp = xreadline(NULL, messages[MSG_NEW_PATH]);
				else if (r == 's' || r == 'h')
					tmp = xreadline(NULL, messages[MSG_LINK_PREFIX]);
				else
					tmp = NULL;
				break;
			default: /* SEL_RENAME */
				tmp = xreadline(pdents[cur].name, "");
				break;
			}

			if (!tmp || !*tmp)
				break;

			switch (sel) {
			case SEL_ARCHIVE:
				if (r == 'c' && strcmp(tmp, pdents[cur].name) == 0)
					goto nochange;

				mkpath(path, tmp, newpath);
				if (access(newpath, F_OK) == 0) {
					if (!xconfirm(get_input(messages[MSG_OVERWRITE]))) {
						statusbar(path);
						goto nochange;
					}
				}
				get_archive_cmd(newpath, tmp);
				(r == 's') ? archive_selection(newpath, tmp, path)
					   : spawn(newpath, tmp, pdents[cur].name, F_CLI | F_CONFIRM);

				mkpath(path, tmp, newpath);
				if (access(newpath, F_OK) == 0) { /* File created */
					xstrsncpy(lastname, tmp, NAME_MAX + 1);
					clearfilter(); /* Archive name may not match */
					clearselection(); /* Archive operation complete */
					goto begin;
				}
				continue;
			case SEL_OPENWITH:
				/* Confirm if app is CLI or GUI */
				r = get_input(messages[MSG_CLI_MODE]);
				r = (r == 'c' ? F_CLI :
				     (r == 'g' ? F_NOWAIT | F_NOTRACE | F_MULTI : 0));
				if (r) {
					mkpath(path, pdents[cur].name, newpath);
					spawn(tmp, newpath, NULL, r);
				}

				cfg.filtermode ?  presel = FILTER : statusbar(path);
				copycurname();
				goto nochange;
			case SEL_RENAME:
				/* Skip renaming to same name */
				if (strcmp(tmp, pdents[cur].name) == 0) {
					tmp = xreadline(pdents[cur].name, messages[MSG_COPY_NAME]);
					if (!tmp || !tmp[0] || !strcmp(tmp, pdents[cur].name)) {
						cfg.filtermode ?  presel = FILTER : statusbar(path);
						copycurname();
						goto nochange;
					}
					ret = 'd';
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
					if (!xconfirm(get_input(messages[MSG_OVERWRITE]))) {
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
				if (ret == 'd')
					spawn("cp -rp", pdents[cur].name, tmp, F_SILENT);
				else if (renameat(fd, pdents[cur].name, fd, tmp) != 0) {
					close(fd);
					printwarn(&presel);
					goto nochange;
				}
				close(fd);
				xstrsncpy(lastname, tmp, NAME_MAX + 1);
			} else { /* SEL_NEW */
				close(fd);
				presel = 0;

				/* Check if it's a dir or file */
				if (r == 'f' || r == 'd') {
					mkpath(path, tmp, newpath);
					ret = xmktree(newpath, r == 'f' ? FALSE : TRUE);
				} else if (r == 's' || r == 'h') {
					if (tmp[0] == '@' && tmp[1] == '\0')
						tmp[0] = '\0';
					ret = xlink(tmp, path, (ndents ? pdents[cur].name : NULL),
						  newpath, &presel, r);
				}

				if (!ret)
					printwait(messages[MSG_FAILED], &presel);

				if (ret <= 0)
					goto nochange;

				if (r == 'f' || r == 'd')
					xstrsncpy(lastname, tmp, NAME_MAX + 1);
				else if (ndents) {
					if (cfg.filtermode)
						presel = FILTER;
					copycurname();
				}
				clearfilter();
			}

			goto begin;
		}
		case SEL_PLUGIN:
			/* Check if directory is accessible */
			if (!xdiraccess(plgpath)) {
				printwarn(&presel);
				goto nochange;
			}

			if (!pkey) {
				r = xstrsncpy(g_buf, messages[MSG_PLUGIN_KEYS], CMD_LEN_MAX);
				printkeys(plug, g_buf + r - 1, maxplug);
				printmsg(g_buf);
				r = get_input(NULL);
			} else {
				r = pkey;
				pkey = '\0';
			}

			if (r != '\r') {
				endselection();
				tmp = get_kv_val(plug, NULL, r, maxplug, NNN_PLUG);
				if (!tmp) {
					printwait(messages[MSG_INVALID_KEY], &presel);
					goto nochange;
				}

				if (tmp[0] == '-' && tmp[1]) {
					++tmp;
					r = FALSE; /* Do not refresh dir after completion */
				} else
					r = TRUE;

				if (!run_selected_plugin(&path, tmp, (ndents ? pdents[cur].name : NULL),
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
				g_state.runplugin ^= 1;
				if (!g_state.runplugin && rundir[0]) {
					/*
					 * If toggled, and still in the plugin dir,
					 * switch to original directory
					 */
					if (strcmp(path, plgpath) == 0) {
						xstrsncpy(path, rundir, PATH_MAX);
						xstrsncpy(lastname, runfile, NAME_MAX + 1);
						rundir[0] = runfile[0] = '\0';
						setdirwatch();
						goto begin;
					}

					/* Otherwise, initiate choosing plugin again */
					g_state.runplugin = 1;
				}

				xstrsncpy(rundir, path, PATH_MAX);
				xstrsncpy(path, plgpath, PATH_MAX);
				if (ndents)
					xstrsncpy(runfile, pdents[cur].name, NAME_MAX);
				g_state.runctx = cfg.curctx;
				lastname[0] = '\0';
			}
			setdirwatch();
			clearfilter();
			goto begin;
		case SEL_SHELL: // fallthrough
		case SEL_LAUNCH: // fallthrough
		case SEL_RUNCMD:
			r = handle_cmd(sel, (ndents ? pdents[cur].name : ""), newpath);

			/* Continue in type-to-nav mode, if enabled */
			if (cfg.filtermode)
				presel = FILTER;

			/* Save current */
			if (ndents)
				copycurname();

			if (!r)
				goto nochange;

			/* Repopulate as directory content may have changed */
			goto begin;
		case SEL_UMOUNT:
			if (!unmount((ndents ? pdents[cur].name : NULL), newpath, &presel, path))
				goto nochange;

			/* Dir removed, go to next entry */
			copynextname(lastname);
			goto begin;
#ifndef NOSSN
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
#endif
		case SEL_EXPORT:
			export_file_list();
			cfg.filtermode ?  presel = FILTER : statusbar(path);
			goto nochange;
		case SEL_TIMETYPE:
			if (!set_time_type(&presel))
				goto nochange;
			goto begin;
		case SEL_QUITCTX: // fallthrough
		case SEL_QUITCD: // fallthrough
		case SEL_QUIT:
		case SEL_QUITERR:
			if (sel == SEL_QUITCTX) {
				int ctx = cfg.curctx;

				for (r = (ctx + 1) & ~CTX_MAX;
				     (r != ctx) && !g_ctx[r].c_cfg.ctxactive;
				     r = ((r + 1) & ~CTX_MAX)) {
				};

				if (r != ctx) {
					g_ctx[ctx].c_cfg.ctxactive = 0;

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

					cfg.curctx = r;
					setdirwatch();
					goto begin;
				}
			} else if (!g_state.forcequit) {
				for (r = 0; r < CTX_MAX; ++r)
					if (r != cfg.curctx && g_ctx[r].c_cfg.ctxactive) {
						r = get_input(messages[MSG_QUIT_ALL]);
						break;
					}

				if (!(r == CTX_MAX || xconfirm(r)))
					break; // fallthrough
			}

#ifndef NOSSN
			if (session && *session == '@' && !session[1])
				save_session(TRUE, NULL);
#endif

			/* CD on Quit */
			if (sel == SEL_QUITCD || getenv("NNN_TMPFILE")) {
				write_lastdir(path);
				if (g_state.picker)
					selbufpos = 0;
			}

			if (sel != SEL_QUITERR)
				return EXIT_SUCCESS;

			if (selbufpos && !g_state.picker) {
				g_state.pickraw = 1;
				return EXIT_SUCCESS;
			}

			return EXIT_FAILURE;
		default:
			if (xlines != LINES || xcols != COLS)
				continue;

			if (idletimeout && idle == idletimeout) {
				lock_terminal(); /* Locker */
				idle = 0;
			}

			if (ndents)
				copycurname();

			goto nochange;
		} /* switch (sel) */
	}
}

static char *make_tmp_tree(char **paths, ssize_t entries, const char *prefix)
{
	/* tmpdir holds the full path */
	/* tmp holds the path without the tmp dir prefix */
	int err;
	struct stat sb;
	char *slash, *tmp;
	ssize_t len = xstrlen(prefix);
	char *tmpdir = malloc(PATH_MAX);

	if (!tmpdir) {
		DPRINTF_S(strerror(errno));
		return NULL;
	}

	tmp = tmpdir + tmpfplen - 1;
	xstrsncpy(tmpdir, g_tmpfpath, tmpfplen);
	xstrsncpy(tmp, "/nnnXXXXXX", 11);

	/* Points right after the base tmp dir */
	tmp += 10;

	/* handle the case where files are directly under / */
	if (!prefix[1] && (prefix[0] == '/'))
		len = 0;

	if (!mkdtemp(tmpdir)) {
		free(tmpdir);

		DPRINTF_S(strerror(errno));
		return NULL;
	}

	listpath = tmpdir;

	for (ssize_t i = 0; i < entries; ++i) {
		if (!paths[i])
			continue;

		err = stat(paths[i], &sb);
		if (err && errno == ENOENT)
			continue;

		/* Don't copy the common prefix */
		xstrsncpy(tmp, paths[i] + len, xstrlen(paths[i]) - len + 1);

		/* Get the dir containing the path */
		slash = xmemrchr((uchar_t *)tmp, '/', xstrlen(paths[i]) - len);
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

	/* Get the dir in which to start */
	*tmp = '\0';
	return tmpdir;
}

static char *load_input(int fd, const char *path)
{
	ssize_t i, chunk_count = 1, chunk = 512 * 1024 /* 512 KiB chunk size */, entries = 0;
	char *input = malloc(sizeof(char) * chunk), *tmpdir = NULL;
	char cwd[PATH_MAX], *next;
	size_t offsets[LIST_FILES_MAX];
	char **paths = NULL;
	ssize_t input_read, total_read = 0, off = 0;
	int msgnum = 0;

	if (!input) {
		DPRINTF_S(strerror(errno));
		return NULL;
	}

	if (!path) {
		if (!getcwd(cwd, PATH_MAX)) {
			free(input);
			return NULL;
		}
	} else
		xstrsncpy(cwd, path, PATH_MAX);

	while (chunk_count < 512) {
		input_read = read(fd, input + total_read, chunk);
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

			if (entries == LIST_FILES_MAX) {
				msgnum = MSG_LIMIT;
				goto malloc_1;
			}

			offsets[entries++] = off;
			off = next - input;
		}

		if (chunk_count == 512) {
			msgnum = MSG_LIMIT;
			goto malloc_1;
		}

		/* We don't need to allocate another chunk */
		if (chunk_count == (total_read - input_read) / chunk)
			continue;

		chunk_count = total_read / chunk;
		if (total_read % chunk)
			++chunk_count;

		input = xrealloc(input, (chunk_count + 1) * chunk);
		if (!input)
			return NULL;
	}

	if (off != total_read) {
		if (entries == LIST_FILES_MAX) {
			msgnum = MSG_LIMIT;
			goto malloc_1;
		}

		offsets[entries++] = off;
	}

	DPRINTF_D(entries);
	DPRINTF_D(total_read);
	DPRINTF_D(chunk_count);

	if (!entries) {
		msgnum = MSG_0_ENTRIES;
		goto malloc_1;
	}

	input[total_read] = '\0';

	paths = malloc(entries * sizeof(char *));
	if (!paths)
		goto malloc_1;

	for (i = 0; i < entries; ++i)
		paths[i] = input + offsets[i];

	listroot = malloc(sizeof(char) * PATH_MAX);
	if (!listroot)
		goto malloc_1;
	listroot[0] = '\0';

	DPRINTF_S(paths[0]);

	for (i = 0; i < entries; ++i) {
		if (paths[i][0] == '\n' || selforparent(paths[i])) {
			paths[i] = NULL;
			continue;
		}

		paths[i] = abspath(paths[i], cwd);
		if (!paths[i]) {
			entries = i; // free from the previous entry
			goto malloc_2;

		}

		DPRINTF_S(paths[i]);

		xstrsncpy(g_buf, paths[i], PATH_MAX);
		if (!common_prefix(xdirname(g_buf), listroot)) {
			entries = i + 1; // free from the current entry
			goto malloc_2;
		}

		DPRINTF_S(listroot);
	}

	DPRINTF_S(listroot);

	if (listroot[0])
		tmpdir = make_tmp_tree(paths, entries, listroot);

malloc_2:
	for (i = entries - 1; i >= 0; --i)
		free(paths[i]);
malloc_1:
	if (msgnum) {
		if (home) { /* We are past init stage */
			printmsg(messages[msgnum]);
			xdelay(XDELAY_INTERVAL_MS);
		} else
			fprintf(stderr, "%s\n", messages[msgnum]);
	}
	free(input);
	free(paths);
	return tmpdir;
}

static void check_key_collision(void)
{
	int key;
	bool bitmap[KEY_MAX] = {FALSE};

	for (ulong_t i = 0; i < sizeof(bindings) / sizeof(struct key); ++i) {
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
		"The unorthodox terminal file manager.\n\n"
		"positional args:\n"
		"  PATH   start dir/file [default: .]\n\n"
		"optional args:\n"
#ifndef NOFIFO
		" -a      auto NNN_FIFO\n"
#endif
		" -A      no dir auto-select\n"
		" -b key  open bookmark key (trumps -s/S)\n"
		" -c      cli-only NNN_OPENER (trumps -e)\n"
		" -C      earlier colorscheme\n"
		" -d      detail mode\n"
		" -D      dirs in context color\n"
		" -e      text in $VISUAL/$EDITOR/vi\n"
		" -E      use EDITOR for undetached edits\n"
#ifndef NORL
		" -f      use readline history file\n"
#endif
		" -F      show fortune\n"
		" -g      regex filters [default: string]\n"
		" -H      show hidden files\n"
		" -J      no auto-proceed on select\n"
		" -K      detect key collision\n"
		" -l val  set scroll lines\n"
		" -n      type-to-nav mode\n"
		" -o      open files only on Enter\n"
		" -p file selection file [stdout if '-']\n"
		" -P key  run plugin key\n"
		" -Q      no quit confirmation\n"
		" -r      use advcpmv patched cp, mv\n"
		" -R      no rollover at edges\n"
#ifndef NOSSN
		" -s name load session by name\n"
		" -S      persistent session\n"
#endif
		" -t secs timeout to lock\n"
		" -T key  sort order [a/d/e/r/s/t/v]\n"
		" -u      use selection (no prompt)\n"
#ifndef NOUG
		" -U      show user and group\n"
#endif
		" -V      show version\n"
		" -w      place HW cursor on hovered\n"
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
			r = xstrsncpy(g_buf, home, PATH_MAX);
			xstrsncpy(g_buf + r - 1, xdgcfg + 1, PATH_MAX);
			xdgcfg = g_buf;
			DPRINTF_S(xdgcfg);
		}

		if (!xdiraccess(xdgcfg)) {
			xerror();
			return FALSE;
		}

		len = xstrlen(xdgcfg) + 1 + 13; /* add length of "/nnn/sessions" */
		xdg = TRUE;
	}

	if (!xdg)
		len = xstrlen(home) + 1 + 21; /* add length of "/.config/nnn/sessions" */

	cfgpath = (char *)malloc(len);
	plgpath = (char *)malloc(len);
	if (!cfgpath || !plgpath) {
		xerror();
		return FALSE;
	}

	if (xdg) {
		xstrsncpy(cfgpath, xdgcfg, len);
		r = len - 13; /* subtract length of "/nnn/sessions" */
	} else {
		r = xstrsncpy(cfgpath, home, len);

		/* Create ~/.config */
		xstrsncpy(cfgpath + r - 1, "/.config", len - r);
		DPRINTF_S(cfgpath);
		r += 8; /* length of "/.config" */
	}

	/* Create ~/.config/nnn */
	xstrsncpy(cfgpath + r - 1, "/nnn", len - r);
	DPRINTF_S(cfgpath);

	/* Create sessions, mounts and plugins directories */
	for (r = 0; r < ELEMENTS(toks); ++r) {
		mkpath(cfgpath, toks[r], plgpath);
		if (!xmktree(plgpath, TRUE)) {
			DPRINTF_S(toks[r]);
			xerror();
			return FALSE;
		}
	}

	/* Set selection file path */
	if (!g_state.picker) {
		char *env_sel = xgetenv(env_cfg[NNN_SEL], NULL);

		selpath = env_sel ? xstrdup(env_sel)
				  : (char *)malloc(len + 3); /* Length of "/.config/nnn/.selection" */

		if (!selpath) {
			xerror();
			return FALSE;
		}

		if (!env_sel) {
			r = xstrsncpy(selpath, cfgpath, len + 3);
			xstrsncpy(selpath + r - 1, "/.selection", 12);
			DPRINTF_S(selpath);
		}
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

	tmpfplen = (uchar_t)xstrsncpy(g_tmpfpath, path, TMP_LEN_MAX);
	DPRINTF_S(g_tmpfpath);
	DPRINTF_U(tmpfplen);

	return TRUE;
}

static void cleanup(void)
{
	free(selpath);
	free(plgpath);
	free(cfgpath);
	free(initpath);
	free(bmstr);
	free(pluginstr);
	free(listroot);
	free(ihashbmp);
	free(bookmark);
	free(plug);
#ifndef NOFIFO
	if (g_state.autofifo)
		unlink(fifopath);
#endif
	if (g_state.pluginit)
		unlink(g_pipepath);
#ifdef DBGMODE
	disabledbg();
#endif
}

int main(int argc, char *argv[])
{
	char *arg = NULL;
	char *session = NULL;
	int fd, opt, sort = 0, pkey = '\0'; /* Plugin key */
#ifndef NOMOUSE
	mmask_t mask;
	char *middle_click_env = xgetenv(env_cfg[NNN_MCLICK], "\0");

	if (middle_click_env[0] == '^' && middle_click_env[1])
		middle_click_key = CONTROL(middle_click_env[1]);
	else
		middle_click_key = (uchar_t)middle_click_env[0];
#endif

	const char * const env_opts = xgetenv(env_cfg[NNN_OPTS], NULL);
	int env_opts_id = env_opts ? (int)xstrlen(env_opts) : -1;
#ifndef NORL
	bool rlhist = FALSE;
#endif

	while ((opt = (env_opts_id > 0
		       ? env_opts[--env_opts_id]
		       : getopt(argc, argv, "aAb:cCdDeEfFgHJKl:nop:P:QrRs:St:T:uUVwxh"))) != -1) {
		switch (opt) {
#ifndef NOFIFO
		case 'a':
			g_state.autofifo = 1;
			break;
#endif
		case 'A':
			cfg.autoselect = 0;
			break;
		case 'b':
			if (env_opts_id < 0)
				arg = optarg;
			break;
		case 'c':
			cfg.cliopener = 1;
			break;
		case 'C':
			g_state.oldcolor = 1;
			break;
		case 'd':
			cfg.showdetail = 1;
			printptr = &printent_long;
			break;
		case 'D':
			g_state.dirctx = 1;
			break;
		case 'e':
			cfg.useeditor = 1;
			break;
		case 'E':
			cfg.waitedit = 1;
			break;
		case 'f':
#ifndef NORL
			rlhist = TRUE;
#endif
			break;
		case 'F':
			g_state.fortune = 1;
			break;
		case 'g':
			cfg.regex = 1;
			filterfn = &visible_re;
			break;
		case 'H':
			cfg.showhidden = 1;
			break;
		case 'J':
			g_state.stayonsel = 1;
			break;
		case 'K':
			check_key_collision();
			return EXIT_SUCCESS;
		case 'l':
			if (env_opts_id < 0)
				scroll_lines = atoi(optarg);
			break;
		case 'n':
			cfg.filtermode = 1;
			break;
		case 'o':
			cfg.nonavopen = 1;
			break;
		case 'p':
			if (env_opts_id >= 0)
				break;

			g_state.picker = 1;
			if (optarg[0] == '-' && optarg[1] == '\0')
				g_state.pickraw = 1;
			else {
				fd = open(optarg, O_WRONLY | O_CREAT, 0600);
				if (fd == -1) {
					xerror();
					return EXIT_FAILURE;
				}

				close(fd);
				selpath = realpath(optarg, NULL);
				unlink(selpath);
			}
			break;
		case 'P':
			if (env_opts_id < 0 && !optarg[1])
				pkey = (uchar_t)optarg[0];
			break;
		case 'Q':
			g_state.forcequit = 1;
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
#ifndef NOSSN
		case 's':
			if (env_opts_id < 0)
				session = optarg;
			break;
		case 'S':
			session = "@";
			break;
#endif
		case 't':
			if (env_opts_id < 0)
				idletimeout = atoi(optarg);
			break;
		case 'T':
			if (env_opts_id < 0)
				sort = (uchar_t)optarg[0];
			break;
		case 'u':
			cfg.prefersel = 1;
			break;
		case 'U':
			g_state.uidgid = 1;
			break;
		case 'V':
			fprintf(stdout, "%s\n", VERSION);
			return EXIT_SUCCESS;
		case 'w':
			cfg.cursormode = 1;
			break;
		case 'x':
			cfg.x11 = 1;
			break;
		case 'h':
			usage();
			return EXIT_SUCCESS;
		default:
			usage();
			return EXIT_FAILURE;
		}
		if (env_opts_id == 0)
			env_opts_id = -1;
	}

#ifdef DBGMODE
	enabledbg();
	DPRINTF_S(VERSION);
#endif

	/* Prefix for temporary files */
	if (!set_tmp_path())
		return EXIT_FAILURE;

	atexit(cleanup);

	/* Check if we are in path list mode */
	if (!isatty(STDIN_FILENO)) {
		/* This is the same as listpath */
		initpath = load_input(STDIN_FILENO, NULL);
		if (!initpath)
			return EXIT_FAILURE;

		/* We return to tty */
		dup2(STDOUT_FILENO, STDIN_FILENO);

		if (session)
			session = NULL;
	}

	home = getenv("HOME");
	if (!home) {
		fprintf(stderr, "set HOME\n");
		return EXIT_FAILURE;
	}
	DPRINTF_S(home);

	if (!setup_config())
		return EXIT_FAILURE;

	/* Get custom opener, if set */
	opener = xgetenv(env_cfg[NNN_OPENER], utils[UTIL_OPENER]);
	DPRINTF_S(opener);

	/* Parse bookmarks string */
	if (!parsekvpair(&bookmark, &bmstr, NNN_BMS, &maxbm)) {
		fprintf(stderr, "%s\n", env_cfg[NNN_BMS]);
		return EXIT_FAILURE;
	}

	/* Parse plugins string */
	if (!parsekvpair(&plug, &pluginstr, NNN_PLUG, &maxplug)) {
		fprintf(stderr, "%s\n", env_cfg[NNN_PLUG]);
		return EXIT_FAILURE;
	}

	if (!initpath) {
		if (arg) { /* Open a bookmark directly */
			if (!arg[1]) /* Bookmarks keys are single char */
				initpath = get_kv_val(bookmark, NULL, *arg, maxbm, NNN_BMS);

			if (!initpath) {
				fprintf(stderr, "%s\n", messages[MSG_INVALID_KEY]);
				return EXIT_FAILURE;
			}

			if (session)
				session = NULL;
		} else if (argc == optind) {
			/* Start in the current directory */
			initpath = getcwd(NULL, 0);
			if (!initpath)
				initpath = "/";
		} else {
			arg = argv[optind];
			DPRINTF_S(arg);
			if (xstrlen(arg) > 7 && is_prefix(arg, "file://", 7))
				arg = arg + 7;
			initpath = realpath(arg, NULL);
			DPRINTF_S(initpath);
			if (!initpath) {
				xerror();
				return EXIT_FAILURE;
			}

			/*
			 * If nnn is set as the file manager, applications may try to open
			 * files by invoking nnn. In that case pass the file path to the
			 * desktop opener and exit.
			 */
			struct stat sb;

			if (stat(initpath, &sb) == -1) {
				xerror();
				return EXIT_FAILURE;
			}

			if (!S_ISDIR(sb.st_mode))
				g_state.initfile = 1;

			if (session)
				session = NULL;
		}
	}

	/* Set archive handling (enveditor used as tmp var) */
	enveditor = getenv(env_cfg[NNN_ARCHIVE]);
#ifdef PCRE
	if (setfilter(&archive_pcre, (enveditor ? enveditor : patterns[P_ARCHIVE]))) {
#else
	if (setfilter(&archive_re, (enveditor ? enveditor : patterns[P_ARCHIVE]))) {
#endif
		fprintf(stderr, "%s\n", messages[MSG_INVALID_REG]);
		return EXIT_FAILURE;
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

#ifndef NOFIFO
	/* Create fifo */
	if (g_state.autofifo) {
		g_tmpfpath[tmpfplen - 1] = '\0';

		size_t r = mkpath(g_tmpfpath, "nnn-fifo.", g_buf);

		xstrsncpy(g_buf + r - 1, xitoa(getpid()), PATH_MAX - r);
		setenv("NNN_FIFO", g_buf, TRUE);
	}

	fifopath = xgetenv("NNN_FIFO", NULL);
	if (fifopath) {
		if (mkfifo(fifopath, 0600) != 0 && !(errno == EEXIST && access(fifopath, W_OK) == 0)) {
			xerror();
			return EXIT_FAILURE;
		}

		sigaction(SIGPIPE, &(struct sigaction){.sa_handler = SIG_IGN}, NULL);
	}
#endif

#ifdef LINUX_INOTIFY
	/* Initialize inotify */
	inotify_fd = inotify_init1(IN_NONBLOCK);
	if (inotify_fd < 0) {
		xerror();
		return EXIT_FAILURE;
	}
#elif defined(BSD_KQUEUE)
	kq = kqueue();
	if (kq < 0) {
		xerror();
		return EXIT_FAILURE;
	}
#elif defined(HAIKU_NM)
	haiku_hnd = haiku_init_nm();
	if (!haiku_hnd) {
		xerror();
		return EXIT_FAILURE;
	}
#endif

	/* Configure trash preference */
	opt = xgetenv_val(env_cfg[NNN_TRASH]);
	if (opt && opt <= 2)
		g_state.trash = opt;

	/* Ignore/handle certain signals */
	struct sigaction act = {.sa_handler = sigint_handler};

	if (sigaction(SIGINT, &act, NULL) < 0) {
		xerror();
		return EXIT_FAILURE;
	}

	act.sa_handler = clean_exit_sighandler;

	if (sigaction(SIGTERM, &act, NULL) < 0 || sigaction(SIGHUP, &act, NULL) < 0) {
		xerror();
		return EXIT_FAILURE;
	}

	act.sa_handler = SIG_IGN;

	if (sigaction(SIGQUIT, &act, NULL) < 0) {
		xerror();
		return EXIT_FAILURE;
	}

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
	if (rlhist) {
		mkpath(cfgpath, ".history", g_buf);
		read_history(g_buf);
	}
#endif

#ifndef NOMOUSE
	if (!initcurses(&mask))
#else
	if (!initcurses(NULL))
#endif
		return EXIT_FAILURE;

	if (sort)
		set_sort_flags(sort);

	opt = browse(initpath, session, pkey);

#ifndef NOMOUSE
	mousemask(mask, NULL);
#endif

	exitcurses();

#ifndef NORL
	if (rlhist) {
		mkpath(cfgpath, ".history", g_buf);
		write_history(g_buf);
	}
#endif

	if (g_state.pickraw || g_state.picker) {
		if (selbufpos) {
			fd = g_state.pickraw ? STDOUT_FILENO : open(selpath, O_WRONLY | O_CREAT, 0600);
			if ((fd == -1) || (seltofile(fd, NULL) != (size_t)(selbufpos)))
				xerror();

			if (fd > 1)
				close(fd);
		}
	} else if (selpath)
		unlink(selpath);

	/* Remove tmp dir in list mode */
	rmlistpath();

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

#ifndef NOFIFO
	notify_fifo(FALSE);
	if (fifofd != -1)
		close(fifofd);
#endif

	return opt;
}
