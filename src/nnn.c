/*
 * BSD 2-Clause License
 *
 * Copyright (C) 2014-2016, Lazaros Koromilas <lostd@2f30.org>
 * Copyright (C) 2014-2016, Dimitris Papastamos <sin@2f30.org>
 * Copyright (C) 2016-2024, Arun Prakash Jana <engineerarun@gmail.com>
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

#define _FILE_OFFSET_BITS 64 /* Support large files on 32-bit glibc */

#if defined(__linux__) || defined(MINGW) || defined(__MINGW32__) \
	|| defined(__MINGW64__) || defined(__CYGWIN__)
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
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
#elif defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__) \
	|| defined(__APPLE__) || defined(__sun)
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
#include <fts.h>
#include <libgen.h>
#include <limits.h>
#ifndef NOLC
#include <locale.h>
#endif
#include <pthread.h>
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
#include <stddef.h>
#include <stdalign.h>
#ifndef __USE_XOPEN_EXTENDED
#define __USE_XOPEN_EXTENDED 1
#endif
#include <ftw.h>
#include <pwd.h>
#include <grp.h>

#ifdef MACOS_BELOW_1012
#include "../misc/macos-legacy/mach_gettime.h"
#endif

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

#if defined(ICONS_IN_TERM) || defined(NERD) || defined(EMOJI)
#define ICONS_ENABLED
#include ICONS_INCLUDE
#include "icons-hash.c"
#include "icons.h"
#endif

#if defined(ICONS_ENABLED) && defined(__APPLE__)
/*
 * For some reason, wcswidth returns 2 for certain icons on macOS
 * leading to duplicated first characters in filenames when navigating.
 * https://github.com/jarun/nnn/issues/1692
 * There might be a better way to fix it without requiring a refresh.
 */
#define macos_icons_hack() do { clrtoeol(); refresh(); } while(0)
#else
#define macos_icons_hack()
#endif

#ifdef TOURBIN_QSORT
#include "qsort.h"
#endif

/* Macro definitions */
#define VERSION      "5.0"
#define GENERAL_INFO "BSD 2-Clause\nhttps://github.com/jarun/nnn"

#ifndef NOSSN
#define SESSIONS_VERSION 1
#endif

#ifndef S_BLKSIZE
#define S_BLKSIZE 512 /* S_BLKSIZE is missing on Android NDK (Termux) */
#endif

/*
 * NAME_MAX and PATH_MAX may not exist, e.g. with dirent.c_name being a
 * flexible array on Illumos. Use somewhat accommodating fallback values.
 */
#ifndef NAME_MAX
#define NAME_MAX 255
#endif

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

#define _ABSSUB(N, M)   (((N) <= (M)) ? ((M) - (N)) : ((N) - (M)))
#define ELEMENTS(x)     (sizeof(x) / sizeof(*(x)))
#undef MIN
#define MIN(x, y)       ((x) < (y) ? (x) : (y))
#undef MAX
#define MAX(x, y)       ((x) > (y) ? (x) : (y))
#define ISODD(x)        ((x) & 1)
#define ISBLANK(x)      ((x) == ' ' || (x) == '\t')
#define TOUPPER(ch)     (((ch) >= 'a' && (ch) <= 'z') ? ((ch) - 'a' + 'A') : (ch))
#define TOLOWER(ch)     (((ch) >= 'A' && (ch) <= 'Z') ? ((ch) - 'A' + 'a') : (ch))
#define ISUPPER_(ch)    ((ch) >= 'A' && (ch) <= 'Z')
#define ISLOWER_(ch)    ((ch) >= 'a' && (ch) <= 'z')
#define CMD_LEN_MAX     (PATH_MAX + ((NAME_MAX + 1) << 1))
#define ALIGN_UP(x, A)  ((((x) + (A) - 1) / (A)) * (A))
#define READLINE_MAX    256
#define FILTER          '/'
#define RFILTER         '\\'
#define CASE            ':'
#define MSGWAIT         '$'
#define SELECT          ' '
#define PROMPT          ">>> "
#undef NEWLINE
#define NEWLINE         "\n"
#define REGEX_MAX       48
#define ENTRY_INCR      64 /* Number of dir 'entry' structures to allocate per shot */
#define NAMEBUF_INCR    0x800 /* 64 dir entries at once, avg. 32 chars per file name = 64*32B = 2KB */
#define DESCRIPTOR_LEN  32
#define _ALIGNMENT      0x10 /* 16-byte alignment */
#define _ALIGNMENT_MASK 0xF
#define TMP_LEN_MAX     64
#define DOT_FILTER_LEN  7
#define ASCII_MAX       128
#define EXEC_ARGS_MAX   10
#define LIST_FILES_MAX  (1 << 14) /* Support listing 16K files */
#define LIST_INPUT_MAX  ((size_t)LIST_FILES_MAX * PATH_MAX)
#define SCROLLOFF       3
#define COLOR_256       256
#define CREATE_NEW_KEY  (-1)

/* Time intervals */
#define DBLCLK_INTERVAL_NS (400000000)
#define XDELAY_INTERVAL_MS (350000) /* 350 ms delay */

#ifndef CTX8
#define CTX_MAX 4
#else
#define CTX_MAX 8
#endif

#ifndef SED
/* BSDs or Solaris or SunOS */
#if defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__) || defined(__APPLE__) || defined(sun) || defined(__sun)
#define SED "gsed"
#else
#define SED "sed"
#endif
#endif

/* Large selection threshold */
#ifndef LARGESEL
#define LARGESEL 1000
#endif

#define MIN_DISPLAY_COL (CTX_MAX * 2)
#define ARCHIVE_CMD_LEN 16
#define BLK_SHIFT_512   9

/* Detect hardlinks in du */
#define HASH_BITS   (0xFFFFFF)
#define HASH_OCTETS (HASH_BITS >> 6) /* 2^6 = 64 */

/* Entry flags */
#define DIR_OR_DIRLNK 0x01
#define HARD_LINK     0x02
#define SYM_ORPHAN    0x04
#define FILE_MISSING  0x08
#define FILE_SELECTED 0x10
#define FILE_SCANNED  0x20
#define FILE_YOUNG    0x40

/* Macros to define process spawn behaviour as flags */
#define F_NONE    0x00  /* no flag set */
#define F_MULTI   0x01  /* first arg can be combination of args; to be used with F_NORMAL */
#define F_NOWAIT  0x02  /* don't wait for child process (e.g. file manager) */
#define F_NOTRACE 0x04  /* suppress stdout and stderr (no traces) */
#define F_NORMAL  0x08  /* spawn child process in non-curses regular CLI mode */
#define F_CONFIRM 0x10  /* run command - show results before exit (must have F_NORMAL) */
#define F_CHKRTN  0x20  /* wait for user prompt if cmd returns failure status */
#define F_NOSTDIN 0x40  /* suppress stdin */
#define F_PAGE    0x80  /* page output in run-cmd-as-plugin mode */
#define F_TTY     0x100 /* Force stdout to go to tty if redirected to a non-tty */
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
#define VFS_AVAIL 0
#define VFS_USED  1
#define VFS_SIZE  2

/* TYPE DEFINITIONS */
typedef unsigned int uint_t;
typedef unsigned char uchar_t;
typedef unsigned short ushort_t;
typedef unsigned long long ullong_t;

/* STRUCTURES */

/* Directory entry */
typedef struct entry {
	char *name;  /* 8 bytes */
	time_t sec;  /* 8 bytes */
	uint_t nsec; /* 4 bytes (enough to store nanosec) */
	mode_t mode; /* 4 bytes */
	off_t size;  /* 8 bytes */
	struct {
		ullong_t blocks : 40; /* 5 bytes (enough for 512 TiB in 512B blocks allocated) */
		ullong_t nlen   : 16; /* 2 bytes (length of file name) */
		ullong_t flags  : 8;  /* 1 byte (flags specific to the file) */
	};
#ifndef NOUG
	uid_t uid; /* 4 bytes */
	gid_t gid; /* 4 bytes */
#endif
} *pEntry;

/* Selection marker */
typedef struct {
	char *startpos;
	size_t len;
} selmark;

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
	uint_t fileinfo   : 1;  /* Show file information on hover */
	uint_t nonavopen  : 1;  /* Open file on right arrow or `l` */
	uint_t autoenter  : 1;  /* auto-enter dir in type-to-nav mode */
	uint_t reserved2  : 1;
	uint_t useeditor  : 1;  /* Use VISUAL to open text files */
	uint_t reserved3  : 3;
	uint_t regex      : 1;  /* Use regex filters */
	uint_t x11        : 1;  /* Copy to system clipboard, show notis, xterm title */
	uint_t timetype   : 2;  /* Time sort type (0: access, 1: change, 2: modification) */
	uint_t cliopener  : 1;  /* All-CLI app opener */
	uint_t waitedit   : 1;  /* For ops that can't be detached, used EDITOR */
	uint_t rollover   : 1;  /* Roll over at edges */
} settings;

/* Non-persistent program-internal states (alphabeical order) */
typedef struct {
	uint_t autofifo   : 1;  /* Auto-create NNN_FIFO */
	uint_t autonext   : 1;  /* Auto-advance on file open */
	uint_t dircolor   : 1;  /* Current status of dir color */
	uint_t dirctx     : 1;  /* Show dirs in context color */
	uint_t duinit     : 1;  /* Initialize disk usage */
	uint_t fifomode   : 1;  /* FIFO notify mode: 0: preview, 1: explorer */
	uint_t forcequit  : 1;  /* Do not prompt on quit */
	uint_t initfile   : 1;  /* Positional arg is a file */
	uint_t interrupt  : 1;  /* Program received an interrupt */
	uint_t move       : 1;  /* Move operation */
	uint_t oldcolor   : 1;  /* Use older colorscheme */
	uint_t picked     : 1;  /* Plugin has picked files */
	uint_t picker     : 1;  /* Write selection to user-specified file */
	uint_t pluginit   : 1;  /* Plugin framework initialized */
	uint_t prstssn    : 1;  /* Persistent session */
	uint_t rangesel   : 1;  /* Range selection on */
	uint_t runctx     : 3;  /* The context in which plugin is to be run */
	uint_t runplugin  : 1;  /* Choose plugin mode */
	uint_t selbm      : 1;  /* Select a bookmark from bookmarks directory */
	uint_t selmode    : 1;  /* Set when selecting files */
	uint_t stayonsel  : 1;  /* Disable auto-advance on selection */
	uint_t trash      : 2;  /* Trash method 0: rm -rf, 1: trash-cli, 2: gio trash */
	uint_t uidgid     : 1;  /* Show owner and group info */
	uint_t usebsdtar  : 1;  /* Use bsdtar as default archive utility */
	uint_t xprompt    : 1;  /* Use native prompt instead of readline prompt */
	uint_t showlines  : 1;  /* Show line numbers */
	uint_t reserved   : 3;  /* Adjust when adding/removing a field */
} runstate;

/* Contexts or workspaces */
typedef struct {
	char c_path[PATH_MAX];     /* Current dir */
	char c_last[PATH_MAX];     /* Last visited dir */
	char c_name[NAME_MAX + 1]; /* Current file name */
	char c_fltr[REGEX_MAX];    /* Current filter */
	settings c_cfg;            /* Current configuration */
	uint_t color;              /* Color code for directories */
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
	.ctxactive = 1,
	.autoenter = 1,
	.timetype = 2, /* T_MOD */
	.rollover = 1,
};

alignas(max_align_t) static context g_ctx[CTX_MAX];

static int ndents, cur, last, curscroll, last_curscroll, total_dents = ENTRY_INCR, scroll_lines = 1;
static int nselected;
#ifndef NOFIFO
static int fifofd = -1;
#endif
static time_t gtimesecs;
static uint_t idletimeout, selbufpos, selbuflen;
static ushort_t xlines, xcols;
static ushort_t idle;
static uchar_t maxbm, maxplug, maxorder;
static uchar_t cfgsort[CTX_MAX + 1];
static char *bmstr;
static char *pluginstr;
static char *orderstr;
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
static char *pnamebuf, *pselbuf, *findselpos;
static char *mark;
#ifndef NOX11
static char hostname[_POSIX_HOST_NAME_MAX + 1];
#endif
#ifndef NOFIFO
static char *fifopath;
#endif
static char *lastcmd;
static ullong_t *ihashbmp;
static struct entry *pdents;
static blkcnt_t dir_blocks;
static kv *bookmark;
static kv *plug;
static kv *order;
static uchar_t tmpfplen, homelen;
static uchar_t blk_shift = BLK_SHIFT_512;
#ifndef NOMOUSE
static int middle_click_key;
#endif
#ifdef PCRE
static pcre *archive_pcre;
#else
static regex_t archive_re;
#endif

/* pthread related */
#define NUM_DU_THREADS (4) /* Can use sysconf(_SC_NPROCESSORS_ONLN) */
#define DU_TEST (((node->fts_info & FTS_F) && \
		(sb->st_nlink <= 1 || test_set_bit((uint_t)sb->st_ino))) || node->fts_info & FTS_DP)

static int threadbmp = -1; /* Has 1 in the bit position for idle threads */
static volatile int active_threads;
static pthread_mutex_t running_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t hardlink_mutex = PTHREAD_MUTEX_INITIALIZER;
static ullong_t *core_files;
static blkcnt_t *core_blocks;
static ullong_t num_files;

typedef struct {
	char path[PATH_MAX];
	int entnum;
	ushort_t core;
	bool mntpoint;
} thread_data;

static thread_data *core_data;

/* Retain old signal handlers */
static struct sigaction oldsighup;
static struct sigaction oldsigtstp;
static struct sigaction oldsigwinch;

/* For use in functions which are isolated and don't return the buffer */
alignas(max_align_t) static char g_buf[CMD_LEN_MAX];

/* For use as a scratch buffer in selection manipulation */
alignas(max_align_t) static char g_sel[PATH_MAX];

/* Buffer to store tmp file path to show selection, file stats and help */
alignas(max_align_t) static char g_tmpfpath[TMP_LEN_MAX];

/* Buffer to store plugins control pipe location */
alignas(max_align_t) static char g_pipepath[TMP_LEN_MAX];

/* Non-persistent runtime states */
static runstate g_state;

/* Options to identify file MIME */
#if defined(__APPLE__)
#define FILE_MIME_OPTS "-bIL"
#elif !defined(__sun) /* no MIME option for 'file' */
#define FILE_MIME_OPTS "-biL"
#endif

/* Macros for utilities */
#define UTIL_OPENER    0
#define UTIL_ATOOL     1
#define UTIL_BSDTAR    2
#define UTIL_UNZIP     3
#define UTIL_TAR       4
#define UTIL_LOCKER    5
#define UTIL_LAUNCH    6
#define UTIL_SH_EXEC   7
#define UTIL_BASH      8
#define UTIL_SSHFS     9
#define UTIL_RCLONE    10
#define UTIL_VI        11
#define UTIL_LESS      12
#define UTIL_SH        13
#define UTIL_FZF       14
#define UTIL_NTFY      15
#define UTIL_CBCP      16
#define UTIL_NMV       17
#define UTIL_TRASH_CLI 18
#define UTIL_GIO_TRASH 19
#define UTIL_RM_RF     20
#define UTIL_ARCHMNT   21

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
	"trash-put",
	"gio trash",
	"rm -rf --",
	"archivemount",
};

/* Common strings */
#define MSG_ZERO         0 /* Unused */
#define MSG_0_ENTRIES    1
#define STR_TMPFILE      2
#define MSG_0_SELECTED   3
#define MSG_CANCEL       4
#define MSG_FAILED       5
#define MSG_SSN_NAME     6
#define MSG_CP_MV_AS     7
#define MSG_CUR_SEL_OPTS 8
#define MSG_FILE_LIMIT   9
#define MSG_SIZE_LIMIT   10
#define MSG_NEW_OPTS     11
#define MSG_CLI_MODE     12
#define MSG_OVERWRITE    13
#define MSG_SSN_OPTS     14
#define MSG_QUIT_ALL     15
#define MSG_HOSTNAME     16
#define MSG_ARCHIVE_NAME 17
#define MSG_OPEN_WITH    18
#define MSG_NEW_PATH     19
#define MSG_LINK_PREFIX  20
#define MSG_COPY_NAME    21
#define MSG_ENTER        22
#define MSG_SEL_MISSING  23
#define MSG_ACCESS       24
#define MSG_EMPTY_FILE   25
#define MSG_UNSUPPORTED  26
#define MSG_NOT_SET      27
#define MSG_EXISTS       28
#define MSG_FEW_COLUMNS  29
#define MSG_REMOTE_OPTS  30
#define MSG_RCLONE_DELAY 31
#define MSG_APP_NAME     32
#define MSG_ARCHIVE_OPTS 33
#define MSG_KEYS         34
#define MSG_INVALID_REG  35
#define MSG_ORDER        36
#define MSG_LAZY         37
#define MSG_FIRST        38
#define MSG_RM_TMP       39
#define MSG_INVALID_KEY  40
#define MSG_NOCHANGE     41
#define MSG_DIR_CHANGED  42
#define MSG_BM_NAME      43

static const char * const messages[] = {
	"",
	"0 entries",
	"/.nnnXXXXXX",
	"0 selected",
	"cancelled",
	"failed!",
	"session name: ",
	"'c'p/'m'v as?",
	"'c'urrent/'s'el?",
	"file limit exceeded",
	"size limit exceeded",
	"['f'ile]/'d'ir/'s'ym/'h'ard?",
	"['g'ui]/'c'li?",
	"overwrite?",
	"'s'ave/'l'oad/'r'estore?",
	"Quit all contexts?",
	"remote name (- for hovered): ",
	"archive [path/]name: ",
	"open with: ",
	"[path/]name: ",
	"link prefix [@ for none]: ",
	"copy [path/]name: ",
	"\n'Enter' to continue",
	"open failed",
	"dir inaccessible",
	"empty! edit/open with",
	"?",
	"not set",
	"entry exists",
	"too few cols!",
	"'s'shfs/'r'clone?",
	"refresh if slow",
	"app: ",
	"['l's]/'o'pen/e'x'tract/'m'nt?",
	"keys:",
	"invalid regex",
	"'a'u/'d'u/'e'xt/'r'ev/'s'z/'t'm/'v'er/'c'lr/'^T'?",
	"unmount failed! try lazy?",
	"first file (\')/char?",
	"remove tmp file?",
	"invalid key",
	"unchanged",
	"dir changed, range sel off",
	"name: ",
};

/* Supported configuration environment variables */
#define NNN_OPTS    0
#define NNN_BMS     1
#define NNN_PLUG    2
#define NNN_OPENER  3
#define NNN_COLORS  4
#define NNN_FCOLORS 5
#define NNNLVL      6
#define NNN_PIPE    7
#define NNN_MCLICK  8
#define NNN_SEL     9
#define NNN_ARCHIVE 10
#define NNN_ORDER   11
#define NNN_HELP    12 /* strings end here */
#define NNN_TRASH   13 /* flags begin here */

static const char * const env_cfg[] = {
	"NNN_OPTS",
	"NNN_BMS",
	"NNN_PLUG",
	"NNN_OPENER",
	"NNN_COLORS",
	"NNN_FCOLORS",
	"NNNLVL",
	"NNN_PIPE",
	"NNN_MCLICK",
	"NNN_SEL",
	"NNN_ARCHIVE",
	"NNN_ORDER",
	"NNN_HELP",
	"NNN_TRASH",
};

/* Required environment variables */
#define ENV_SHELL  0
#define ENV_VISUAL 1
#define ENV_EDITOR 2
#define ENV_PAGER  3
#define ENV_NCUR   4

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
#define T_MOD    2

#define PROGRESS_CP   "cpg -giRp --"
#define PROGRESS_MV   "mvg -gi --"
static char cp[sizeof PROGRESS_CP] = "cp -iRp --";
static char mv[sizeof PROGRESS_MV] = "mv -i --";

/* Archive commands */
static char * const archive_cmd[] = {"atool -a", "bsdtar -acvf", "zip -r", "tar -acvf"};

/* Tokens used for path creation */
#define TOK_BM  0
#define TOK_SSN 1
#define TOK_MNT 2
#define TOK_PLG 3

static const char * const toks[] = {
	"bookmarks",
	"sessions",
	"mounts",
	"plugins", /* must be the last entry */
};

/* Patterns */
#define P_CPMVFMT 0
#define P_CPMVRNM 1
#define P_ARCHIVE 2
#define P_REPLACE 3
#define P_ARCHIVE_CMD 4

static const char * const patterns[] = {
	SED" -i 's|^\\(\\(.*/\\)\\(.*\\)$\\)|#\\1\\n\\3|' %s",
	SED" 's|^\\([^#/][^/]\\?.*\\)$|%s/\\1|;s|^#\\(/.*\\)$|\\1|' "
		"%s | tr '\\n' '\\0' | xargs -0 -n2 sh -c '%s \"$0\" \"$@\" < /dev/tty'",
	"\\.(bz|bz2|gz|tar|taz|tbz|tbz2|tgz|z|zip)$", /* Basic formats that don't need external tools */
	SED" -i 's|^%s\\(.*\\)$|%s\\1|' %s",
	"xargs -0 %s %s < '%s'",
};

/* Colors */
#define C_BLK (CTX_MAX + 1) /* Block device: DarkSeaGreen1 */
#define C_CHR (C_BLK + 1)   /* Character device: Yellow1 */
#define C_DIR (C_CHR + 1)   /* Directory: DeepSkyBlue1 */
#define C_EXE (C_DIR + 1)   /* Executable file: Green1 */
#define C_FIL (C_EXE + 1)   /* Regular file: Normal */
#define C_HRD (C_FIL + 1)   /* Hard link: Plum4 */
#define C_LNK (C_HRD + 1)   /* Symbolic link: Cyan1 */
#define C_MIS (C_LNK + 1)   /* Missing file OR file details: Grey62 */
#define C_ORP (C_MIS + 1)   /* Orphaned symlink: DeepPink1 */
#define C_PIP (C_ORP + 1)   /* Named pipe (FIFO): Orange1 */
#define C_SOC (C_PIP + 1)   /* Socket: MediumOrchid1 */
#define C_UND (C_SOC + 1)   /* Unknown OR 0B regular/exe file: Red1 */

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
#define copycurname() xstrsncpy(lastname, ndents ? pdents[cur].name : "\0", NAME_MAX + 1)
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

/* Forward declarations */
static void redraw(char *path);
static int spawn(char *file, char *arg1, char *arg2, char *arg3, ushort_t flag);
static void move_cursor(int target, int ignore_scrolloff);
static char *load_input(int fd, const char *path);
static int set_sort_flags(int r);
static void statusbar(char *path);
static bool get_output(char *file, char *arg1, char *arg2, int fdout, bool page);
#ifndef NOFIFO
static void notify_fifo(bool force);
#endif

/* Functions */

static void sigint_handler(int sig)
{
	(void) sig;
	g_state.interrupt = 1;
}

static void clean_exit_sighandler(int sig)
{
	(void) sig;
	exitcurses();
	/* This triggers cleanup() thanks to atexit() */
	exit(EXIT_SUCCESS);
}

static char *xitoa(uint_t val)
{
	static char dst[32] = {'\0'};
	static const char digits[201] =
		"0001020304050607080910111213141516171819"
		"2021222324252627282930313233343536373839"
		"4041424344454647484950515253545556575859"
		"6061626364656667686970717273747576777879"
		"8081828384858687888990919293949596979899";
	uint_t next = 30, quo, i;

	while (val >= 100) {
		quo = val / 100;
		i = (val - (quo * 100)) * 2;
		val = quo;
		dst[next] = digits[i + 1];
		dst[--next] = digits[i];
		--next;
	}

	/* Handle last 1-2 digits */
	if (val < 10)
		dst[next] = '0' + val;
	else {
		i = val * 2;
		dst[next] = digits[i + 1];
		dst[--next] = digits[i];
	}

	return &dst[next];
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

	pthread_mutex_lock(&hardlink_mutex);
	ullong_t *m = ihashbmp + (nr >> 6);

	if (*m & (1 << (nr & 63))) {
		pthread_mutex_unlock(&hardlink_mutex);
		return FALSE;
	}

	*m |= 1 << (nr & 63);
	pthread_mutex_unlock(&hardlink_mutex);

	return TRUE;
}

#ifndef __APPLE__
/* Increase the limit on open file descriptors, if possible */
static void max_openfds(void)
{
	struct rlimit rl;

	if (!getrlimit(RLIMIT_NOFILE, &rl))
		if (rl.rlim_cur < rl.rlim_max) {
			rl.rlim_cur = rl.rlim_max;
			setrlimit(RLIMIT_NOFILE, &rl);
		}
}
#endif

/*
 * Wrapper to realloc()
 * Frees current memory if realloc() fails and returns NULL.
 *
 * The *alloc() family returns aligned address: https://man7.org/linux/man-pages/man3/malloc.3.html
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
	return ptr ? memcpy(ptr, s, len) : NULL;
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

static inline bool is_prefix(const char *restrict str, const char *restrict prefix, size_t len)
{
	return !strncmp(str, prefix, len);
}

static inline bool is_bad_len_or_dir(const char *restrict path)
{
	size_t len = xstrlen(path);

	return ((len >= PATH_MAX) || (path[len - 1] == '/'));
}

static char *get_cwd_entry(const char *restrict cwdpath, char *entrypath, size_t *tokenlen)
{
	size_t len = xstrlen(cwdpath);
	char *end;

	if (!is_prefix(entrypath, cwdpath, len))
		return NULL;

	entrypath += len + 1; /* Add 1 for trailing / */
	end = strchr(entrypath, '/');
	if (end)
		*tokenlen = end - entrypath;
	else
		*tokenlen = xstrlen(entrypath);
	DPRINTF_U(*tokenlen);

	return entrypath;
}

/*
 * The poor man's implementation of memrchr(3).
 * We are only looking for '/' and '.' in this program.
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

static inline char *xextension(const char *fname, size_t len)
{
	return xmemrchr((uchar_t *)fname, '.', len);
}

#ifndef NOUG
/*
 * One-shot cache for getpwuid/getgrgid. Returns the cached name if the
 * provided uid is the same as the previous uid. Returns xitoa(guid) if
 * the guid is not found in the password database.
 */
static char *getpwname(uid_t uid)
{
	static uint_t uidcache = UINT_MAX;
	static char *namecache;

	if (uidcache != uid) {
		struct passwd *pw = getpwuid(uid);

		uidcache = uid;
		namecache = pw ? pw->pw_name : NULL;
	}

	return namecache ? namecache : xitoa(uid);
}

static char *getgrname(gid_t gid)
{
	static uint_t gidcache = UINT_MAX;
	static char *grpcache;

	if (gidcache != gid) {
		struct group *gr = getgrgid(gid);

		gidcache = gid;
		grpcache = gr ? gr->gr_name : NULL;
	}

	return grpcache ? grpcache : xitoa(gid);
}
#endif

static inline bool getutil(char *util)
{
	return spawn("which", util, NULL, NULL, F_NORMAL | F_NOTRACE) == 0;
}

static inline bool tilde_is_home(const char *s)
{
	return s[0] == '~' && (s[1] == '\0' || s[1] == '/');
}

static inline bool tilde_is_home_strict(const char *s)
{
	return s[0] == '~' && s[1] == '/';
}

/*
 * Updates out with "dir/name or "/name"
 * Returns the number of bytes copied including the terminating NULL byte
 *
 * Note: dir and out must be PATH_MAX in length to avoid macOS fault
 */
static size_t mkpath(const char *dir, const char *name, char *out)
{
	size_t len = 0;

	/* same rational for being strict as abspath() */
	if (tilde_is_home_strict(name)) { //NOLINT
		len = xstrsncpy(out, home, PATH_MAX);
		--len;
		++name;
	} else if (name[0] != '/') { // NOLINT
		/* Handle root case */
		len = istopdir(dir) ? 1 : xstrsncpy(out, dir, PATH_MAX);
		out[len - 1] = '/'; // NOLINT
	}
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
 * Resolves ./../~ in filepath
 */
static char *abspath(const char *filepath, char *cwd, char *buf)
{
	const char *path = filepath;
	bool allocated = FALSE;

	if (!path)
		return NULL;

	/* when dealing with tilde, we need to be strict.
	 * otherwise a file named "~" can end up expanding to
	 * $HOME and causing disaster */
	if (tilde_is_home_strict(path)) {
		cwd = home;
		path += 2; /* advance 2 bytes past the "~/" */
	} else if ((path[0] != '/') && !cwd) {
		cwd = getcwd(NULL, 0);
		if (!cwd)
			return NULL;
		allocated = TRUE;
	}

	size_t dst_size = 0, src_size = xstrlen(path), cwd_size = cwd ? xstrlen(cwd) : 0;
	size_t len = src_size;
	const char *src;
	char *dst;
	/*
	 * We need to add 2 chars at the end as relative paths may start with:
	 * ./ (find .)
	 * no separator (fd .): this needs an additional char for '/'
	 */
	char *resolved_path = buf ? buf : malloc(src_size + cwd_size + 2);

	if (!resolved_path) {
		if (allocated)
			free(cwd);
		return NULL;
	}

	/* Turn relative paths into absolute */
	if (path[0] != '/') {
		if (!cwd) {
			if (!buf)
				free(resolved_path);
			return NULL;
		}
		dst_size = xstrsncpy(resolved_path, cwd, cwd_size + 1) - 1;
		if (allocated)
			free(cwd);
	} else
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

	if (xstrlen(resolved_path) >= PATH_MAX) {
		if (!buf)
			free(resolved_path);
		else
			buf[0] = '\0';
		return NULL;
	}

	return resolved_path;
}

/* finds abspath of link pointed by filepath, taking cwd into account */
static char *bmtarget(const char *filepath, char *cwd, char *buf)
{
	char target[PATH_MAX + 1];
	ssize_t n = readlink(filepath, target, PATH_MAX);
	if (n != -1) {
		target[n] = '\0';
		return abspath(target, cwd, buf);
	}
	return NULL;
}

/* wraps the argument in single quotes so it can be safely fed to shell */
static bool shell_escape(char *output, size_t outlen, const char *s)
{
	size_t n = xstrlen(s), w = 0;

	if (s == output) {
		DPRINTF_S("s == output");
		return FALSE;
	}

	output[w++] = '\''; /* begin single quote */
	for (size_t r = 0; r < n; ++r) {
		/* potentially too big: 4 for the single quote case, 2 from
		 * outside the loop */
		if (w + 6 >= outlen)
			return FALSE;

		switch (s[r]) {
		/* the only thing that has special meaning inside single
		 * quotes are single quotes themselves. */
		case '\'':
			output[w++] = '\''; /* end single quote */
			output[w++] = '\\'; /* put \' so it's treated as literal single quote */
			output[w++] = '\'';
			output[w++] = '\''; /* start single quoting again */
			break;
		default:
			output[w++] = s[r];
			break;
		}
	}
	output[w++] = '\''; /* end single quote */
	output[w++] = '\0'; /* nul terminator */
	return TRUE;
}

static bool set_tilde_in_path(char *path)
{
	if (is_prefix(path, home, homelen)) {
		home[homelen] = path[homelen - 1];
		path[homelen - 1] = '~';
		return TRUE;
	}

	return FALSE;
}

static void reset_tilde_in_path(char *path)
{
	path[homelen - 1] = home[homelen];
	home[homelen] = '\0';
}

#ifndef NOX11
static void xterm_cfg(char *path)
{
	if (cfg.x11 && !g_state.picker) {
		/* Signal CWD change to terminal */
		printf("\033]7;file://%s%s\033\\", hostname, path);

		/* Set terminal window title */
		bool r = set_tilde_in_path(path);

		printf("\033]2;%s\007", r ? &path[homelen - 1] : path);
		fflush(stdout);

		if (r)
			reset_tilde_in_path(path);
	}
}
#endif

static bool convert_tilde(const char *path, char *buf)
{
	if (tilde_is_home(path)) {
		ssize_t len = xstrlen(home);
		ssize_t loclen = xstrlen(path);

		xstrsncpy(buf, home, len + 1);
		xstrsncpy(buf + len, path + 1, loclen);
		return true;
	}
	return false;
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

static void msg(const char *message)
{
	fprintf(stderr, "%s\n", message);
}

#ifdef KEY_RESIZE
static void handle_key_resize(void)
{
	endwin();
	refresh();
}

/* Clear the old prompt */
static void clearoldprompt(void)
{
	// clear info line
	move(xlines - 2, 0);
	clrtoeol();

	tolastln();
	clrtoeol();
	handle_key_resize();
}
#endif

/* Messages show up at the bottom */
static inline void printmsg_nc(const char *msg)
{
	tolastln();
	addstr(msg);
	clrtoeol();
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
	wint_t ch[1];

	if (prompt)
		printmsg(prompt);
	cleartimeout();

	get_wch(ch);

#ifdef KEY_RESIZE
	while (*ch == KEY_RESIZE) {
		if (prompt) {
			clearoldprompt();
			xlines = LINES;
			printmsg(prompt);
		}

		get_wch(ch);
	}
#endif
	settimeout();
	return (int)*ch;
}

static bool isselfileempty(void)
{
	struct stat sb;

	return (stat(selpath, &sb) == -1) || (!sb.st_size);
}

static int get_cur_or_sel(void)
{
	bool sel = (selbufpos || !isselfileempty());

	/* Check both local buffer and selection file for external selection */
	if (sel && ndents) {
		/* If selection is preferred and we have a local selection, return selection.
		 * Always show the prompt in case of an external selection.
		 */
		if (cfg.prefersel && selbufpos)
			return 's';

		int choice = get_input(messages[MSG_CUR_SEL_OPTS]);

		return ((choice == 'c' || choice == 's') ? choice : 0);
	}

	if (sel)
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

static char confirm_force(bool selection, bool use_trash)
{
	char str[300];

	/* Note: ideally we should use utils[UTIL_RM_RF] instead of the "rm -rf" string */
	int r = snprintf(str, 20, "%s", use_trash ? utils[UTIL_GIO_TRASH] + 4 : "rm -rf");

	if (selection)
		snprintf(str + r, 280, " %d files?", nselected);
	else
		snprintf(str + r, 280, " '%s'?", pdents[cur].name);

	r = get_input(str);

	if (r == ESC)
		return '\0'; /* cancel */
	if (r == 'y' || r == 'Y')
		return 'f'; /* forceful for rm */
	if (r == 'n' || r == 'N')
		return '\0'; /* cancel */
	return (use_trash ? '\0' : 'i'); /* interactive for rm */
}

/* Writes buflen char(s) from buf to a file */
static void writesel(const char *buf, const size_t buflen)
{
	if (!selpath)
		return;

	int fd = open(selpath, O_CREAT | O_WRONLY | O_TRUNC, S_IWUSR | S_IRUSR);

	if (fd != -1) {
		if (write(fd, buf, buflen) != (ssize_t)buflen)
			printwarn(NULL);
		close(fd);
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

static void selbufrealloc(const size_t alloclen)
{
	if ((selbufpos + alloclen) > selbuflen) {
		selbuflen = ALIGN_UP(selbufpos + alloclen, PATH_MAX);
		pselbuf = xrealloc(pselbuf, selbuflen);
		if (!pselbuf)
			errexit();
	}
}

/* Write selected file paths to fd, linefeed separated */
static size_t seltofile(int fd, uint_t *pcount, const char *separator)
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
			if (write(fd, separator, 1) != 1)
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
	if (isselfileempty())
		return FALSE;

	snprintf(g_buf, CMD_LEN_MAX, "tr \'\\0\' \'\\n\' < %s", selpath);
	spawn(utils[UTIL_SH_EXEC], g_buf, NULL, NULL, F_CLI | F_CONFIRM);

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
	}
}

static void clearselection(void)
{
	nselected = 0;
	selbufpos = 0;
	g_state.selmode = 0;
	writesel(NULL, 0);
}

static char *findinsel(char *startpos, int len)
{
	if (!selbufpos)
		return FALSE;

	if (!startpos)
		startpos = pselbuf;

	char *found = startpos;
	size_t buflen = selbufpos - (startpos - pselbuf);

	while (1) {
		/* memmem(3): not specified in POSIX.1, but present on a number of other systems. */
		found = memmem(found, buflen - (found - startpos), g_sel, len);
		if (!found)
			return NULL;
		if (found == startpos || *(found - 1) == '\0')
			return found;
		found += len; /* We found g_sel as a substring of a path, move forward */
		if (found >= startpos + buflen)
			return NULL;
	}
}

static int markcmp(const void *va, const void *vb)
{
	const selmark *ma = (selmark*)va;
	const selmark *mb = (selmark*)vb;

	return ma->startpos - mb->startpos;
}

/* scanselforpath() must be called before calling this */
static inline void findmarkentry(size_t len, struct entry *dentp)
{
	if (!(dentp->flags & FILE_SCANNED)) {
		if (findinsel(findselpos, len + xstrsncpy(g_sel + len, dentp->name, dentp->nlen)))
			dentp->flags |= FILE_SELECTED;
		dentp->flags |= FILE_SCANNED;
	}
}

/*
 * scanselforpath() must be called before calling this
 * pathlen = length of path + 1 (+1 for trailing slash)
 */
static void invertselbuf(const int pathlen)
{
	size_t len, endpos, shrinklen = 0, alloclen = 0;
	char * const pbuf = g_sel + pathlen;
	char *found;
	int i, nmarked = 0, prev = 0;
	struct entry *dentp;
	bool scan = FALSE;
	selmark *marked = malloc(nselected * sizeof(selmark));

	if (!marked) {
		printwarn(NULL);
		return;
	}

	/* First pass: inversion */
	for (i = 0; i < ndents; ++i) {
		dentp = &pdents[i];

		if (dentp->flags & FILE_SCANNED) {
			if (dentp->flags & FILE_SELECTED) {
				dentp->flags ^= FILE_SELECTED; /* Clear selection status */
				scan = TRUE;
			} else {
				dentp->flags |= FILE_SELECTED;
				alloclen += pathlen + dentp->nlen;
			}
		} else {
			dentp->flags |= FILE_SCANNED;
			scan = TRUE;
		}

		if (scan) {
			len = pathlen + xstrsncpy(pbuf, dentp->name, NAME_MAX);
			found = findinsel(findselpos, len);
			if (found) {
				if (findselpos == found)
					findselpos += len;

				if (nmarked && (found
				    == (marked[nmarked - 1].startpos + marked[nmarked - 1].len)))
					marked[nmarked - 1].len += len;
				else {
					marked[nmarked].startpos = found;
					marked[nmarked].len = len;
					++nmarked;
				}

				--nselected;
				shrinklen += len; /* buffer size adjustment */
			} else {
				dentp->flags |= FILE_SELECTED;
				alloclen += pathlen + dentp->nlen;
			}
			scan = FALSE;
		}
	}

	/*
	 * Files marked for deselection could be found in arbitrary order.
	 * Sort by appearance in selection buffer.
	 * With entries sorted we can merge adjacent ones allowing us to
	 * move them in a single go.
	 */
	qsort(marked, nmarked, sizeof(selmark), &markcmp);

	/* Some files might be adjacent. Merge them into a single entry */
	for (i = 1; i < nmarked; ++i) {
		if (marked[i].startpos == marked[prev].startpos + marked[prev].len)
			marked[prev].len += marked[i].len;
		else {
			++prev;
			marked[prev].startpos = marked[i].startpos;
			marked[prev].len = marked[i].len;
		}
	}

	/*
	 * Number of entries is increased by encountering a non-adjacent entry
	 * After we finish the loop we should increment it once more.
	 */

	if (nmarked) /* Make sure there is something to deselect */
		nmarked = prev + 1;

	/* Using merged entries remove unselected chunks from selection buffer */
	for (i = 0; i < nmarked; ++i) {
		/*
		 * found: points to where the current block starts
		 *        variable is recycled from previous for readability
		 * endpos: points to where the the next block starts
		 *         area between the end of current block (found + len)
		 *         and endpos is selected entries. This is what we are
		 *         moving back.
		 */
		found = marked[i].startpos;
		endpos = (i + 1 == nmarked ? selbufpos : marked[i + 1].startpos - pselbuf);
		len = marked[i].len;

		/* Move back only selected entries. No selected memory is moved twice */
		memmove(found, found + len, endpos - (found + len - pselbuf));
	}

	free(marked);

	/* Buffer size adjustment */
	selbufpos -= shrinklen;

	selbufrealloc(alloclen);

	/* Second pass: append newly selected to buffer */
	for (i = 0; i < ndents; ++i) {
		if (pdents[i].flags & FILE_SELECTED) {
			len = pathlen + xstrsncpy(pbuf, pdents[i].name, NAME_MAX);
			appendfpath(g_sel, len);
			++nselected;
		}
	}

	nselected ? writesel(pselbuf, selbufpos - 1) : clearselection();
}

/*
 * scanselforpath() must be called before calling this
 * pathlen = length of path + 1 (+1 for trailing slash)
 */
static void addtoselbuf(const int pathlen, int startid, int endid)
{
	int i;
	size_t len, alloclen = 0;
	struct entry *dentp;
	char *found;
	char * const pbuf = g_sel + pathlen;

	/* Remember current selection buffer position */
	for (i = startid; i <= endid; ++i) {
		dentp = &pdents[i];

		if (findselpos) {
			len = pathlen + xstrsncpy(pbuf, dentp->name, NAME_MAX);
			found = findinsel(findselpos, len);
			if (found) {
				dentp->flags |= (FILE_SCANNED | FILE_SELECTED);
				if (found == findselpos) {
					findselpos += len;
					if (findselpos == (pselbuf + selbufpos))
						findselpos = NULL;
				}
			} else
				alloclen += pathlen + dentp->nlen;
		} else
			alloclen += pathlen + dentp->nlen;
	}

	selbufrealloc(alloclen);

	for (i = startid; i <= endid; ++i) {
		if (!(pdents[i].flags & FILE_SELECTED)) {
			len = pathlen + xstrsncpy(pbuf, pdents[i].name, NAME_MAX);
			appendfpath(g_sel, len);
			++nselected;
			pdents[i].flags |= (FILE_SCANNED | FILE_SELECTED);
		}
	}

	writesel(pselbuf, selbufpos - 1); /* Truncate NULL from end */
}

/* Removes g_sel from selbuf */
static void rmfromselbuf(size_t len)
{
	char *found = findinsel(findselpos, len);
	if (!found)
		return;

	memmove(found, found + len, selbufpos - (found + len - pselbuf));
	selbufpos -= len;

	nselected ? writesel(pselbuf, selbufpos - 1) : clearselection();
}

static int scanselforpath(const char *path, bool getsize)
{
	if (!path[1]) { /* path should always be at least two bytes (including NULL) */
		g_sel[0] = '/';
		findselpos = pselbuf;
		return 1; /* Length of '/' is 1 */
	}

	size_t off = xstrsncpy(g_sel, path, PATH_MAX);

	g_sel[off - 1] = '/';
	/*
	 * We set findselpos only here. Directories can be listed in arbitrary order.
	 * This is the best best we can do for remembering position.
	 */
	findselpos = findinsel(NULL, off);

	if (getsize)
		return off;
	return (findselpos ? off : 0);
}

/* Finish selection procedure before an operation */
static void endselection(bool endselmode)
{
	int fd;
	ssize_t count;
	char buf[sizeof(patterns[P_REPLACE]) + PATH_MAX + (TMP_LEN_MAX << 1)];

	if (endselmode && g_state.selmode)
		g_state.selmode = 0;

	/* The code below is only for listing mode */
	if (!listpath || !selbufpos)
		return;

	fd = create_tmp_file();
	if (fd == -1) {
		DPRINTF_S("couldn't create tmp file");
		return;
	}

	seltofile(fd, NULL, NEWLINE);
	if (close(fd)) {
		DPRINTF_S(strerror(errno));
		printwarn(NULL);
		return;
	}

	snprintf(buf, sizeof(buf), patterns[P_REPLACE], listpath, listroot, g_tmpfpath);
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

/* Returns: 1 - success, 0 - none selected, -1 - other failure */
static int editselection(void)
{
	int ret = -1;
	int fd, lines = 0;
	ssize_t count;
	struct stat sb;
	time_t mtime;

	if (!selbufpos) /* External selection is only editable at source */
		return listselfile();

	fd = create_tmp_file();
	if (fd == -1) {
		DPRINTF_S("couldn't create tmp file");
		return -1;
	}

	seltofile(fd, NULL, NEWLINE);
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

	spawn(editor, g_tmpfpath, NULL, NULL, F_CLI);

	if (xconfirm(get_input(messages[MSG_RM_TMP])))
		unlink(g_tmpfpath);
}

static bool init_fcolors(void)
{
	char *f_colors = getenv(env_cfg[NNN_FCOLORS]);

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
			msg("newterm!");
			return FALSE;
		}
	} else if (!initscr()) {
		msg("initscr!");
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
	mousemask(BUTTON1_PRESSED | BUTTON2_PRESSED | BUTTON3_PRESSED | BUTTON4_PRESSED
		  | BUTTON5_PRESSED, (mmask_t *)oldmask);
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
		if (COLORS >= COLOR_256) {
			if (!(g_state.oldcolor || init_fcolors())) {
				exitcurses();
				msg(env_cfg[NNN_FCOLORS]);
				return FALSE;
			}
		} else
			g_state.oldcolor = 1;

		DPRINTF_D(COLORS);
		DPRINTF_D(COLOR_PAIRS);

		if (colors && *colors == '#') {
			char *sep = strchr(colors, ';');

			if (!g_state.oldcolor && COLORS >= COLOR_256) {
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
		for (uchar_t i = 0; i < CTX_MAX; ++i) {
			pcode = &g_ctx[i].color;

			if (colors && *colors) {
				if (ext) {
					*pcode = xchartohex(*colors) << 4;
					if (*++colors)
						fcolors[i + 1] = *pcode += xchartohex(*colors);
					else { /* Each color code must be 2 hex symbols */
						exitcurses();
						msg(env_cfg[NNN_COLORS]);
						return FALSE;
					}
				} else {
					*pcode = (*colors < '0' || *colors > '7') ? 4 : *colors - '0';
					fcolors[i + 1] = *pcode;
				}
				++colors;
			} else
				*pcode = 4;

			init_pair(i + 1, *pcode, -1);
		}
	}
#ifdef ICONS_ENABLED
	if (!g_state.oldcolor) {
		for (uint_t i = 0; i < ELEMENTS(init_colors); ++i)
			init_pair(C_UND + 1 + init_colors[i], init_colors[i], -1);
	}
#endif

	settimeout(); /* One second */
	set_escdelay(25);
	return TRUE;
}

/* No NULL check here as spawn() guards against it */
static char *parseargs(char *cmd, char **argv, int *pindex)
{
	int count = 0;
	size_t len = xstrlen(cmd) + 1;
	char *line = (char *)malloc(len);

	if (!line) {
		DPRINTF_S("malloc()!");
		return NULL;
	}

	xstrsncpy(line, cmd, len);
	argv[count++] = line;
	cmd = line;

	while (*line) { // NOLINT
		if (ISBLANK(*line)) {
			*line++ = '\0';

			if (!*line) // NOLINT
				break;

			argv[count++] = line;
			if (count == EXEC_ARGS_MAX) {
				count = -1;
				break;
			}
		}

		++line;
	}

	if (count == -1 || count > (EXEC_ARGS_MAX - 4)) { /* 3 args and last NULL */
		free(cmd);
		cmd = NULL;
		DPRINTF_S("NULL or too many args");
	}

	*pindex = count;
	return cmd;
}

static void enable_signals(void)
{
	struct sigaction dfl_act = {.sa_handler = SIG_DFL};

	sigaction(SIGHUP, &dfl_act, NULL);
	sigaction(SIGINT, &dfl_act, NULL);
	sigaction(SIGQUIT, &dfl_act, NULL);
	sigaction(SIGTSTP, &dfl_act, NULL);
	sigaction(SIGWINCH, &dfl_act, NULL);
}

static pid_t xfork(uchar_t flag)
{
	pid_t p = fork();

	if (p > 0) {
		/* the parent ignores the interrupt, quit and hangup signals */
		sigaction(SIGHUP, &(struct sigaction){.sa_handler = SIG_IGN}, &oldsighup);
		sigaction(SIGTSTP, &(struct sigaction){.sa_handler = SIG_DFL}, &oldsigtstp);
		sigaction(SIGWINCH, &(struct sigaction){.sa_handler = SIG_IGN}, &oldsigwinch);
	} else if (p == 0) {
		/* We create a grandchild to detach */
		if (flag & F_NOWAIT) {
			p = fork();

			if (p > 0)
				_exit(EXIT_SUCCESS);
			else if (p == 0) {
				enable_signals();
				setsid();
				return p;
			}

			perror("fork");
			_exit(EXIT_FAILURE);
		}

		/* So they can be used to stop the child */
		enable_signals();
	}

	/* This is the parent waiting for the child to create grandchild */
	if (flag & F_NOWAIT)
		waitpid(p, NULL, 0);

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
	sigaction(SIGWINCH, &oldsigwinch, NULL);

	return status;
}

/*
 * Spawns a child process. Behaviour can be controlled using flag.
 * Limited to 3 arguments to a program, flag works on bit set.
 */
static int spawn(char *file, char *arg1, char *arg2, char *arg3, ushort_t flag)
{
	pid_t pid;
	int status = 0, retstatus = 0xFFFF;
	char *argv[EXEC_ARGS_MAX] = {0};
	char *cmd = NULL;

	if (!file || !*file)
		return retstatus;

	/* Swap args if the first arg is NULL and the other 2 aren't */
	if (!arg1 && arg2) {
		arg1 = arg2;
		if (arg3) {
			arg2 = arg3;
			arg3 = NULL;
		} else
			arg2 = NULL;
	}

	if (flag & F_MULTI) {
		cmd = parseargs(file, argv, &status);
		if (!cmd)
			return -1;
	} else
		argv[status++] = file;

	argv[status] = arg1;
	argv[++status] = arg2;
	argv[++status] = arg3;

	if (flag & F_NORMAL)
		exitcurses();

	pid = xfork(flag);
	if (pid == 0) {
		/* Suppress stdout and stderr */
		if (flag & F_NOTRACE) {
			int fd = open("/dev/null", O_WRONLY, 0200);

			if (flag & F_NOSTDIN)
				dup2(fd, STDIN_FILENO);
			dup2(fd, STDOUT_FILENO);
			dup2(fd, STDERR_FILENO);
			close(fd);
		} else if (flag & F_TTY) {
			/* If stdout has been redirected to a non-tty, force output to tty */
			if (!isatty(STDOUT_FILENO)) {
				int fd = open(ctermid(NULL), O_WRONLY, 0200);
				dup2(fd, STDOUT_FILENO);
				close(fd);
			}
		}

		execvp(*argv, argv);
		_exit(EXIT_SUCCESS);
	} else {
		retstatus = join(pid, flag);
		DPRINTF_D(pid);

		if ((flag & F_CONFIRM) || ((flag & F_CHKRTN) && retstatus)) {
			status = write(STDOUT_FILENO, messages[MSG_ENTER], xstrlen(messages[MSG_ENTER]));
			(void)status;
			while ((read(STDIN_FILENO, &status, 1) > 0) && (status != '\n'));
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

/* Check if a dir exists, IS a dir, and is readable */
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

static bool plugscript(const char *plugin, uchar_t flags)
{
	mkpath(plgpath, plugin, g_buf);
	if (!access(g_buf, X_OK)) {
		spawn(g_buf, NULL, NULL, NULL, flags);
		return TRUE;
	}

	return FALSE;
}

static void opstr(char *buf, char *op)
{
	snprintf(buf, CMD_LEN_MAX, "xargs -0 sh -c '%s \"$0\" \"$@\" . < /dev/tty' < %s", op, selpath);
}

static bool rmmulstr(char *buf, bool use_trash)
{
	char r = confirm_force(TRUE, use_trash);
	if (!r)
		return FALSE;

	if (!use_trash)
		snprintf(buf, CMD_LEN_MAX, "xargs -0 sh -c 'rm -%cvr -- \"$0\" \"$@\" < /dev/tty' < %s",
			 r, selpath);
	else
		snprintf(buf, CMD_LEN_MAX, "xargs -0 %s < %s",
			 utils[(g_state.trash == 1) ? UTIL_TRASH_CLI : UTIL_GIO_TRASH], selpath);

	return TRUE;
}

/* Returns TRUE if file is removed, else FALSE */
static bool xrm(char * const fpath, bool use_trash)
{
	char r = confirm_force(FALSE, use_trash);
	if (!r)
		return FALSE;

	if (!use_trash) {
		char rm_opts[5] = "-vr\0";

		rm_opts[3] = r;
		spawn("rm", rm_opts, "--", fpath, F_NORMAL | F_CHKRTN);
	} else
		spawn(utils[(g_state.trash == 1) ? UTIL_TRASH_CLI : UTIL_GIO_TRASH],
		      fpath, NULL, NULL, F_NORMAL | F_MULTI);

	return (access(fpath, F_OK) == -1); /* File is removed */
}

static void xrmfromsel(char *path, char *fpath)
{
#ifndef NOX11
	bool selected = TRUE;
#endif

	if ((pdents[cur].flags & DIR_OR_DIRLNK) && scanselforpath(fpath, FALSE))
		clearselection();
	else if (pdents[cur].flags & FILE_SELECTED) {
		--nselected;
		rmfromselbuf(mkpath(path, pdents[cur].name, g_sel));
	}
#ifndef NOX11
	else
		selected = FALSE;

	if (selected && cfg.x11)
		plugscript(utils[UTIL_CBCP], F_NOWAIT | F_NOTRACE);
#endif
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
	char buf[sizeof(patterns[P_CPMVRNM]) + (MAX(sizeof(cp), sizeof(mv))) + (PATH_MAX << 1)];

	fd = create_tmp_file();
	if (fd == -1)
		return ret;

	/* selsafe() returned TRUE for this to be called */
	if (!selbufpos) {
		snprintf(buf, sizeof(buf), "tr '\\0' '\\n' < %s > %s", selpath, g_tmpfpath);
		spawn(utils[UTIL_SH_EXEC], buf, NULL, NULL, F_CLI);

		count = lines_in_file(fd, buf, sizeof(buf));
		if (!count)
			goto finish;
	} else
		seltofile(fd, &count, NEWLINE);

	close(fd);

	snprintf(buf, sizeof(buf), patterns[P_CPMVFMT], g_tmpfpath);
	spawn(utils[UTIL_SH_EXEC], buf, NULL, NULL, F_CLI);

	spawn((cfg.waitedit ? enveditor : editor), g_tmpfpath, NULL, NULL, F_CLI);

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
	if (!spawn(utils[UTIL_SH_EXEC], buf, NULL, NULL, F_CLI | F_CHKRTN))
		ret = TRUE;
finish:
	if (fd >= 0)
		close(fd);

	return ret;
}

static bool cpmvrm_selection(enum action sel, char *path)
{
	int r;

	if (isselfileempty()) {
		if (nselected)
			clearselection();
		printmsg(messages[MSG_0_SELECTED]);
		return FALSE;
	}

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
	default: /* SEL_TRASH, SEL_RM_ONLY */
		if (!rmmulstr(g_buf, g_state.trash && sel == SEL_TRASH)) {
			printmsg(messages[MSG_CANCEL]);
			return FALSE;
		}
	}

	if (sel != SEL_CPMVAS && spawn(utils[UTIL_SH_EXEC], g_buf, NULL, NULL, F_CLI | F_CHKRTN)) {
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
					     "tr '\n' '\\0' | xargs -0 -n2 sh -c 'mv -i -- \"$0\" \"$@\" <"
					     " /dev/tty'";
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

	seltofile(fd1, &count, NEWLINE);
	seltofile(fd2, NULL, NEWLINE);
	close(fd2);

	if (dir) /* Don't retain dir entries in selection */
		selbufpos = 0;

	spawn((cfg.waitedit ? enveditor : editor), g_tmpfpath, NULL, NULL, F_CLI);

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
	spawn(utils[UTIL_SH_EXEC], buf, NULL, NULL, F_CLI);
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

static char *get_archive_cmd(const char *archive)
{
	uchar_t i = 3;

	if (!g_state.usebsdtar && getutil(utils[UTIL_ATOOL]))
		i = 0;
	else if (getutil(utils[UTIL_BSDTAR]))
		i = 1;
	else if (is_suffix(archive, ".zip"))
		i = 2;
	// else tar

	return archive_cmd[i];
}

static void archive_selection(const char *cmd, const char *archive)
{
	char *buf = malloc((xstrlen(patterns[P_ARCHIVE_CMD]) + xstrlen(cmd) + xstrlen(archive)
	                   + xstrlen(selpath)) * sizeof(char));
	if (!buf) {
		DPRINTF_S(strerror(errno));
		printwarn(NULL);
		return;
	}

	snprintf(buf, CMD_LEN_MAX, patterns[P_ARCHIVE_CMD], cmd, archive, selpath);
	spawn(utils[UTIL_SH_EXEC], buf, NULL, NULL, F_CLI | F_CONFIRM);
	free(buf);
}

static void write_lastdir(const char *curpath, const char *outfile)
{
	bool tilde = false;
	if (!outfile)
		xstrsncpy(cfgpath + xstrlen(cfgpath), "/.lastd", 8);
	else
		tilde = convert_tilde(outfile, g_buf);

	int fd = open(outfile
			? (tilde ? g_buf : outfile)
			: cfgpath, O_CREAT | O_WRONLY | O_TRUNC, S_IWUSR | S_IRUSR);

	if (fd != -1 && shell_escape(g_buf, sizeof(g_buf), curpath)) {
		if (write(fd, "cd ", 3) == 3) {
			if (write(fd, g_buf, strlen(g_buf)) != (ssize_t)strlen(g_buf)) {
				DPRINTF_S("write failed!");
			}
		}
		close(fd);
	}
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
#ifndef NOLC
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

	alignas(max_align_t) static const int8_t result_type[] = {
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
	char * const fltr = g_ctx[cfg.curctx].c_fltr;

	if (fltr[1]) {
		fltr[REGEX_MAX - 1] = fltr[1];
		fltr[1] = '\0';
	}
}

static int entrycmp(const void *va, const void *vb)
{
	const struct entry *pa = (pEntry)va;
	const struct entry *pb = (pEntry)vb;

	if ((pb->flags & DIR_OR_DIRLNK) != (pa->flags & DIR_OR_DIRLNK)) {
		if (pb->flags & DIR_OR_DIRLNK)
			return 1;
		return -1;
	}

	/* Sort based on specified order */
	if (cfg.timeorder) {
		if (pb->sec > pa->sec)
			return 1;
		if (pb->sec < pa->sec)
			return -1;
		/* If sec matches, comare nsec */
		if (pb->nsec > pa->nsec)
			return 1;
		if (pb->nsec < pa->nsec)
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
	} else if (cfg.extnorder && !(pb->flags & DIR_OR_DIRLNK)) {
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
	if ((((pEntry)vb)->flags & DIR_OR_DIRLNK)
	    != (((pEntry)va)->flags & DIR_OR_DIRLNK)) {
		if (((pEntry)vb)->flags & DIR_OR_DIRLNK)
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

static inline int handle_event(void)
{
	if (nselected && isselfileempty())
		clearselection();
	return CONTROL('L');
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
	wint_t c = presel;
	int i = 0;
	bool escaped = FALSE;

	if (c == 0 || c == MSGWAIT) {
try_quit:
		i = get_wch(&c);
		//DPRINTF_D(c);
		//DPRINTF_S(keyname(c));

#ifdef KEY_RESIZE
		if (c == KEY_RESIZE)
			handle_key_resize();
#endif

		/* Handle Alt+key */
		if (c == ESC) {
			timeout(0);
			i = get_wch(&c);
			if (i != ERR) {
				if (c == ESC)
					c = CONTROL('L');
				else {
					unget_wch(c);
					c = ';';
				}
				settimeout();
			} else if (escaped) {
				settimeout();
				c = CONTROL('Q');
			} else {
#ifndef NOFIFO
				if (!g_state.fifomode)
					notify_fifo(TRUE); /* Send hovered path to NNN_FIFO */
#endif
				escaped = TRUE;
				settimeout();
				goto try_quit;
			}
		}

		if (i == ERR && presel == MSGWAIT)
			c = (cfg.filtermode || filterset()) ? FILTER : CONTROL('L');
		else if (c == FILTER || c == CONTROL('L'))
			/* Clear previous filter when manually starting */
			clearfilter();
	}

	if (i == ERR) {
		++idle;

		/*
		 * Do not check for directory changes in du mode.
		 * A redraw forces du calculation.
		 * Check for changes every odd second.
		 */
#ifdef LINUX_INOTIFY
		if (!cfg.blkorder && inotify_wd >= 0 && (idle & 1)) {
			struct inotify_event *event;
			char inotify_buf[EVENT_BUF_LEN] = {0};

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
						c = handle_event();
						break;
					}
				}
				DPRINTF_S("inotify read done");
			}
		}
#elif defined(BSD_KQUEUE)
		if (!cfg.blkorder && event_fd >= 0 && (idle & 1)) {
			struct kevent event_data[NUM_EVENT_SLOTS] = {0};

			if (kevent(kq, events_to_monitor, NUM_EVENT_SLOTS,
				   event_data, NUM_EVENT_FDS, &gtimeout) > 0)
				c = handle_event();
		}
#elif defined(HAIKU_NM)
		if (!cfg.blkorder && haiku_nm_active && (idle & 1) && haiku_is_update_needed(haiku_hnd))
			c = handle_event();
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

	if (cfg.fileinfo && ndents && get_output("file", "-b", pdents[cur].name, -1, FALSE))
		mvaddstr(xlines - 2, 2, g_buf);
	else {
		snprintf(info + i, REGEX_MAX - i - 1, "  %s [/], %4s [:]",
			 (cfg.regex ? "reg" : "str"),
			 ((fnstrstr == &strcasestr) ? "ic" : "noic"));
	}

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
	alignas(max_align_t) wchar_t wln[REGEX_MAX];
	char *ln = g_ctx[cfg.curctx].c_fltr;
	wint_t ch[1];
	int r, total = ndents, len;
	char *pln = g_ctx[cfg.curctx].c_fltr + 1;

	DPRINTF_S(__func__);

	if (ndents && (ln[0] == FILTER || ln[0] == RFILTER) && *pln) {
		if (matches(pln) != -1) {
			move_cursor(dentfind(lastname, ndents), 0);
			redraw(path);
		}

		if (!cfg.filtermode) {
			statusbar(path);
			return 0;
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
		case 0: // fallthrough
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
			} else {
				*ch = FILTER;
				goto end;
			}
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
		{
			MEVENT event = {0};
			getmouse(&event);
			if (event.bstate == 0)
				continue;
			ungetmouse(&event);
			goto end;
		}
#endif
		case ESC:
			if (handle_alt_key(ch) != ERR) {
				if (*ch == ESC) /* Handle Alt+Esc */
					*ch = 'q'; /* Quit context */
				else {
					unget_wch(*ch);
					*ch = ';'; /* Run plugin */
				}
			}
			goto end;
		}

		if (r != OK) /* Handle Fn keys in main loop */
			break;

		/* Handle all control chars in main loop */
		if (*ch < ASCII_MAX && keyname(*ch)[0] == '^' && *ch != '^')
			goto end;

		if (len == 1) {
			if (*ch == '?') /* Help and config key, '?' is an invalid regex */
				goto end;

			if (cfg.filtermode) {
				switch (*ch) {
				case '\'': // fallthrough /* Go to first non-dir file */
				case '+': // fallthrough /* Toggle file selection */
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

			/* Toggle string or regex filter */
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
#ifdef MATCHFLTR
		r = matches(pln);
		if (r <= 0) {
			!r ? unget_wch(KEY_BACKSPACE) : showfilter(ln);
#else
		if (matches(pln) == -1) {
			showfilter(ln);
#endif
			continue;
		}

		/* If the only match is a dir, auto-enter and cd into it */
		if ((ndents == 1) && cfg.autoenter && (pdents[0].flags & DIR_OR_DIRLNK)) {
			*ch = KEY_ENTER;
			cur = 0;
			goto end;
		}

		/*
		 * redraw() should be above the auto-enter optimization, for
		 * the case where there's an issue with dir auto-enter, say,
		 * due to a permission problem. The transition is _jumpy_ in
		 * case of such an error. However, we optimize for successful
		 * cases where the dir has permissions. This skips a redraw().
		 */
		redraw(path);
		showfilter(ln);
	}
end:

	/* Save last working filter in-filter */
	if (ln[1])
		ln[REGEX_MAX - 1] = ln[1];

	/* Save current */
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
	wint_t ch[1];
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
		if (pos > (size_t)(xcols - x)) {
			mvaddnwstr(xlines - 1, x, buf + (pos - (xcols - x) + 1), xcols - x);
			move(xlines - 1, xcols - 1);
		} else {
			mvaddnwstr(xlines - 1, x, buf, len + 1);
			move(xlines - 1, x + wcswidth(buf, pos));
		}
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
				}
				continue;
			case '\t':
				if (!(len || pos) && ndents)
					len = pos = mbstowcs(buf, pdents[cur].name, READLINE_MAX);
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
			case ESC: /* Exit prompt on Esc, but just filter out Alt+key */
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
			case KEY_UP: // fallthrough
			case KEY_DOWN:
				if (prompt && lastcmd && (xstrcmp(prompt, PROMPT) == 0)) {
					printmsg(prompt);
					len = pos = mbstowcs(buf, lastcmd, READLINE_MAX); // fallthrough
				}
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
static int xlink(char *prefix, char *path, char *curfname, char *buf, int type)
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

	if (choice == 'c' || (nselected == 1)) {
		prefix = abspath(prefix, path, lnpath); /* Generate link path */
		if (!prefix)
			return -1;

		if (choice == 'c')
			mkpath(path, curfname, buf); /* Generate target file path */

		if (!link_fn((choice == 'c') ? buf : pselbuf, lnpath)) {
			if (choice == 's')
				clearselection();
			return 1; /* One link created */
		}
		return 0;
	}

	r = xstrsncpy(buf, prefix, NAME_MAX + 1); /* Copy prefix */

	while (pos < selbufpos) {
		len = xstrlen(psel);
		fname = xbasename(psel);

		xstrsncpy(buf + r - 1, fname, NAME_MAX - r); /* Suffix target file name */
		mkpath(path, buf, lnpath); /* Generate link path */

		if (!link_fn(psel, lnpath))
			++count;

		pos += len + 1;
		psel += len + 1;
	}

	if (count == nselected) /* Clear selection if all links are generated */
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

	for (int r = 0; r < max && kvarr[r].key; ++r) {
		if (kvarr[r].key == key) {
			/* Do not allocate new memory for plugin */
			if (id == NNN_PLUG)
				return pluginstr + kvarr[r].off;

			val = bmstr + kvarr[r].off;
			bool tilde = convert_tilde(val, g_buf);
			return abspath((tilde ? g_buf : val), NULL, buf);
		}
	}

	DPRINTF_S("Invalid key");
	return NULL;
}

static int get_kv_key(kv *kvarr, char *val, uchar_t max, uchar_t id)
{
	if (!kvarr)
		return -1;

	if (id != NNN_ORDER) /* For now this function supports only order string */
		return -1;

	for (int r = 0; r < max && kvarr[r].key; ++r) {
		if (xstrcmp((orderstr + kvarr[r].off), val) == 0)
			return kvarr[r].key;
	}

	return -1;
}

static void resetdircolor(int flags)
{
	/* Directories are always shown on top, clear the color when moving to first file */
	if (g_state.dircolor && !(flags & DIR_OR_DIRLNK)) {
		attroff(COLOR_PAIR(cfg.curctx + 1) | A_BOLD);
		g_state.dircolor = 0;
	}
}

/*
 * Replace escape characters in a string with '?'
 * Adjust string length to maxcols if > 0;
 * Max supported str length: NAME_MAX;
 */
#ifdef NOLC
static char *unescape(const char *str, uint_t maxcols)
{
	char * const wbuf = g_buf;
	char *buf = wbuf;

	xstrsncpy(wbuf, str, maxcols);
#else
static wchar_t *unescape(const char *str, uint_t maxcols)
{
	wchar_t * const wbuf = (wchar_t *)g_buf;
	wchar_t *buf = wbuf;
	size_t len = mbstowcs(wbuf, str, maxcols); /* Convert multi-byte to wide char */

	len = wcswidth(wbuf, len);

	if (len >= maxcols) {
		size_t lencount = maxcols;

		while (len > maxcols) /* Reduce wide chars one by one till it fits */
			len = wcswidth(wbuf, --lencount);

		wbuf[lencount] = L'\0';
	}
#endif

	while (*buf) {
		if (*buf <= '\x1f' || *buf == '\x7f')
			*buf = '\?';

		++buf;
	}

	return wbuf;
}

static off_t get_size(off_t size, off_t *pval, int comp)
{
	off_t rem = *pval;
	off_t quo = rem / 10;

	if ((rem - (quo * 10)) >= 5) {
		rem = quo + 1;
		if (rem == comp) {
			++size;
			rem = 0;
		}
	} else
		rem = quo;

	*pval = rem;
	return size;
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
		size = get_size(size, &rem, 10);
	} else if (i == 2) {
		rem = (rem * 1000) >> 10;
		size = get_size(size, &rem, 100);
	} else if (i > 2) {
		rem = (rem * 10000) >> 10;
		size = get_size(size, &rem, 1000);
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
static struct icon get_icon(const struct entry *ent)
{
	for (size_t i = 0; i < ELEMENTS(icons_name); ++i)
		if (strcasecmp(ent->name, icons_name[i].match) == 0)
			return (struct icon){ icons_name[i].icon, icons_name[i].color };

	if (ent->flags & DIR_OR_DIRLNK)
		return dir_icon;

	char *tmp = xextension(ent->name, ent->nlen);

	if (tmp) {
		uint16_t z, k, h = icon_ext_hash(++tmp); /* ++tmp to skip '.' */
		for (k = 0; k < ICONS_PROBE_MAX; ++k) {
			z = (h + k) % ELEMENTS(icons_ext);
			if (strcasecmp(tmp, icons_ext[z].match) == 0)
				return (struct icon){ icons_ext_uniq[icons_ext[z].idx], icons_ext[z].color };
		}
	}

	/* If there's no match and the file is executable, icon that */
	if (ent->mode & 0100)
		return exec_icon;
	return file_icon;
}

static void print_icon(const struct entry *ent, const int attrs)
{
	const struct icon icon = get_icon(ent);
	addstr(ICON_PADDING_LEFT);
	if (icon.color)
		attron(COLOR_PAIR(C_UND + 1 + icon.color));
	else if (attrs)
		attron(attrs);
	addstr(icon.icon);
	if (icon.color)
		attroff(COLOR_PAIR(C_UND + 1 + icon.color));
	else if (attrs)
		attroff(attrs);
	addstr(ICON_PADDING_RIGHT);
}
#endif

static void print_time(const time_t *timep, const uchar_t flags)
{
	struct tm t;

	/* Highlight timestamp for entries 5 minutes young */
	if (flags & FILE_YOUNG)
		attron(A_REVERSE);

	localtime_r(timep, &t);
	printw("%s-%02d-%02d %02d:%02d",
		xitoa(t.tm_year + 1900), t.tm_mon + 1, t.tm_mday, t.tm_hour, t.tm_min);

	if (flags & FILE_YOUNG)
		attroff(A_REVERSE);
}

static char get_detail_ind(const mode_t mode)
{
	switch (mode & S_IFMT) {
	case S_IFDIR:  // fallthrough
	case S_IFREG:  return ' ';
	case S_IFLNK:  return '@';
	case S_IFSOCK: return '=';
	case S_IFIFO:  return '|';
	case S_IFBLK:  return 'b';
	case S_IFCHR:  return 'c';
	}
	return '?';
}

/* Note: attribute and indicator values must be initialized to 0 */
static uchar_t get_color_pair_name_ind(const struct entry *ent, char *pind, int *pattr)
{
	switch (ent->mode & S_IFMT) {
	case S_IFREG:
		if (!ent->size) {
			if (ent->mode & 0100)
				*pind = '*';
			return C_UND;
		}
		if (ent->flags & HARD_LINK) {
			if (ent->mode & 0100)
				*pind = '*';
			return C_HRD;
		}
		if (ent->mode & 0100) {
			*pind = '*';
			return C_EXE;
		}
		return C_FIL;
	case S_IFDIR:
		*pind = '/';
		if (g_state.oldcolor)
			return C_DIR;
		*pattr |= A_BOLD;
		return g_state.dirctx ? cfg.curctx + 1 : C_DIR;
	case S_IFLNK:
		if (ent->flags & DIR_OR_DIRLNK) {
			*pind = '/';
			*pattr |= g_state.oldcolor ? A_DIM : A_BOLD;
		} else {
			*pind = '@';
			if (g_state.oldcolor)
				*pattr |= A_DIM;
		}
		if (!g_state.oldcolor || cfg.showdetail)
			return (ent->flags & SYM_ORPHAN) ? C_ORP : C_LNK;
		return 0;
	case S_IFSOCK:
		*pind = '=';
		return C_SOC;
	case S_IFIFO:
		*pind = '|';
		return C_PIP;
	case S_IFBLK:
		return C_BLK;
	case S_IFCHR:
		return C_CHR;
	}

	*pind = '?';
	return C_UND;
}

static void printent(int pdents_index, uint_t namecols, bool sel)
{
	const struct entry *ent = &pdents[pdents_index];
	char ind = '\0';
	int attrs;

	if (cfg.showdetail) {
		int type = ent->mode & S_IFMT;
		char perms[6] = {' ', ' ', (char)('0' + ((ent->mode >> 6) & 7)),
				(char)('0' + ((ent->mode >> 3) & 7)),
				(char)('0' + (ent->mode & 7)), '\0'};

		addch(' ');
		attrs = g_state.oldcolor ? (resetdircolor(ent->flags), A_DIM)
					 : (fcolors[C_MIS] ? COLOR_PAIR(C_MIS) : 0);
		if (attrs)
			attron(attrs);

		/* Print details */
		print_time(&ent->sec, ent->flags);

		printw("%s%9s ", perms, (type == S_IFREG || type == S_IFDIR)
			? coolsize(cfg.blkorder ? (blkcnt_t)ent->blocks << blk_shift : ent->size)
			: (type = (uchar_t)get_detail_ind(ent->mode), (char *)&type));

		if (attrs)
			attroff(attrs);
	}

	if (g_state.showlines) {
		ptrdiff_t rel_num = pdents_index - cur;
		printw(rel_num == 0 ? "%4td" : "%+4td", rel_num);
	}

	attrs = 0;

	uchar_t color_pair = get_color_pair_name_ind(ent, &ind, &attrs);

	addch((ent->flags & FILE_SELECTED) ? '+' | A_REVERSE | A_BOLD : ' ');

	if (g_state.oldcolor)
		resetdircolor(ent->flags);
	else {
		if (ent->flags & FILE_MISSING)
			color_pair = C_MIS;
		if (color_pair && fcolors[color_pair])
			attrs |= COLOR_PAIR(color_pair);
#ifdef ICONS_ENABLED
		print_icon(ent, attrs);
#endif
	}

	if (sel)
		attrs |= A_REVERSE;
	if (attrs)
		attron(attrs);
	if (!ind)
		++namecols;

#ifndef NOLC
	addwstr(unescape(ent->name, namecols));
#else
	addstr(unescape(ent->name, MIN(namecols, ent->nlen) + 1));
#endif

	if (attrs)
		attroff(attrs);
	if (ind)
		addch(ind);
}

/**
 * Sets the global cfg variable and restores related state to match the new
 * cfg.
 */
static void setcfg(settings newcfg)
{
	cfg = newcfg;
	/* Synchronize the global function pointers to match the new cfg. */
	entrycmpfn = cfg.reverse ? &reventrycmp : &entrycmp;
	namecmpfn = cfg.version ? &xstrverscasecmp : &xstricmp;
}

static void savecurctx(char *path, char *curname, int nextctx)
{
	settings tmpcfg = cfg;
	context *ctxr = &g_ctx[nextctx];

	/* Save current context */
	if (curname)
		xstrsncpy(g_ctx[tmpcfg.curctx].c_name, curname, NAME_MAX + 1);
	else
		g_ctx[tmpcfg.curctx].c_name[0] = '\0';

	g_ctx[tmpcfg.curctx].c_cfg = tmpcfg;

	if (ctxr->c_cfg.ctxactive) { /* Switch to saved context */
		tmpcfg = ctxr->c_cfg;
		/* Skip ordering an open context */
		if (order) {
			cfgsort[CTX_MAX] = cfgsort[nextctx];
			cfgsort[nextctx] = '0';
		}
	} else { /* Set up a new context from current context */
		ctxr->c_cfg.ctxactive = 1;
		xstrsncpy(ctxr->c_path, path, PATH_MAX);
		ctxr->c_last[0] = ctxr->c_name[0] = ctxr->c_fltr[0] = ctxr->c_fltr[1] = '\0';
		ctxr->c_cfg = tmpcfg;
		/* If already in an ordered dir, clear ordering for the new context and let it order */
		if (cfgsort[cfg.curctx] == 'z')
			cfgsort[nextctx] = 'z';
	}

	tmpcfg.curctx = nextctx;
	setcfg(tmpcfg);
}

#ifndef NOSSN
static void save_session(const char *sname, int *presel)
{
	int fd, i;
	session_header_t header = {0};
	bool status = FALSE;
	char ssnpath[PATH_MAX];
	char spath[PATH_MAX];

	header.ver = SESSIONS_VERSION;

	for (i = 0; i < CTX_MAX; ++i) {
		if (g_ctx[i].c_cfg.ctxactive) {
			if (cfg.curctx == i && ndents)
				/* Update current file name, arrows don't update it */
				xstrsncpy(g_ctx[i].c_name, pdents[cur].name, NAME_MAX + 1);
			header.pathln[i] = MIN(xstrlen(g_ctx[i].c_path), PATH_MAX) + 1;
			header.lastln[i] = MIN(xstrlen(g_ctx[i].c_last), PATH_MAX) + 1;
			header.nameln[i] = MIN(xstrlen(g_ctx[i].c_name), NAME_MAX) + 1;
			header.fltrln[i] = REGEX_MAX;
		}
	}

	mkpath(cfgpath, toks[TOK_SSN], ssnpath);
	mkpath(ssnpath, sname, spath);

	fd = open(spath, O_CREAT | O_WRONLY | O_TRUNC, S_IWUSR | S_IRUSR);
	if (fd == -1) {
		printwait(messages[MSG_SEL_MISSING], presel);
		return;
	}

	if ((write(fd, &header, sizeof(header)) != (ssize_t)sizeof(header))
		|| (write(fd, &cfg, sizeof(cfg)) != (ssize_t)sizeof(cfg)))
		goto END;

	for (i = 0; i < CTX_MAX; ++i)
		if ((write(fd, &g_ctx[i].c_cfg, sizeof(settings)) != (ssize_t)sizeof(settings))
			|| (write(fd, &g_ctx[i].color, sizeof(uint_t)) != (ssize_t)sizeof(uint_t))
			|| (header.nameln[i] > 0
			    && write(fd, g_ctx[i].c_name, header.nameln[i]) != (ssize_t)header.nameln[i])
			|| (header.lastln[i] > 0
			    && write(fd, g_ctx[i].c_last, header.lastln[i]) != (ssize_t)header.lastln[i])
			|| (header.fltrln[i] > 0
			    && write(fd, g_ctx[i].c_fltr, header.fltrln[i]) != (ssize_t)header.fltrln[i])
			|| (header.pathln[i] > 0
			    && write(fd, g_ctx[i].c_path, header.pathln[i]) != (ssize_t)header.pathln[i]))
			goto END;

	status = TRUE;

END:
	close(fd);

	if (!status)
		printwait(messages[MSG_FAILED], presel);
}

static bool load_session(const char *sname, char **path, char **lastdir, char **lastname, bool restore)
{
	int fd, i = 0;
	session_header_t header;
	bool has_loaded_dynamically = !(sname || restore);
	bool status = (sname && g_state.picker); /* Picker mode with session program option */
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
		save_session("@", NULL);

	fd = open(spath, O_RDONLY, S_IWUSR | S_IRUSR);
	if (fd == -1) {
		if (!status) {
			printmsg(messages[MSG_SEL_MISSING]);
			xdelay(XDELAY_INTERVAL_MS);
		}
		return FALSE;
	}

	status = FALSE;

	if ((read(fd, &header, sizeof(header)) != (ssize_t)sizeof(header))
		|| (header.ver != SESSIONS_VERSION)
		|| (read(fd, &cfg, sizeof(cfg)) != (ssize_t)sizeof(cfg)))
		goto END;

	g_ctx[cfg.curctx].c_name[0] = g_ctx[cfg.curctx].c_last[0]
		= g_ctx[cfg.curctx].c_fltr[0] = g_ctx[cfg.curctx].c_fltr[1] = '\0';

	for (; i < CTX_MAX; ++i)
		if ((read(fd, &g_ctx[i].c_cfg, sizeof(settings)) != (ssize_t)sizeof(settings))
			|| (read(fd, &g_ctx[i].color, sizeof(uint_t)) != (ssize_t)sizeof(uint_t))
			|| (header.nameln[i] > 0
			    && read(fd, g_ctx[i].c_name, header.nameln[i]) != (ssize_t)header.nameln[i])
			|| (header.lastln[i] > 0
			    && read(fd, g_ctx[i].c_last, header.lastln[i]) != (ssize_t)header.lastln[i])
			|| (header.fltrln[i] > 0
			    && read(fd, g_ctx[i].c_fltr, header.fltrln[i]) != (ssize_t)header.fltrln[i])
			|| (header.pathln[i] > 0
			    && read(fd, g_ctx[i].c_path, header.pathln[i]) != (ssize_t)header.pathln[i]))
			goto END;

	*path = g_ctx[cfg.curctx].c_path;
	*lastdir = g_ctx[cfg.curctx].c_last;
	*lastname = g_ctx[cfg.curctx].c_name;
	set_sort_flags('\0'); /* Set correct sort options */
	status = TRUE;

END:
	close(fd);

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

/* ctx is absolute: 1 to 4, + for smart context */
static void set_smart_ctx(int ctx, char *nextpath, char **path, char *file, char **lastname, char **lastdir)
{
	if (ctx == '+') /* Get smart context */
		ctx = (int)(get_free_ctx() + 1);

	if (ctx == 0 || ctx == cfg.curctx + 1) { /* Same context */
		clearfilter();
		xstrsncpy(*lastdir, *path, PATH_MAX);
		xstrsncpy(*path, nextpath, PATH_MAX);
	} else { /* New context */
		--ctx;
		/* Deactivate the new context and build from scratch */
		g_ctx[ctx].c_cfg.ctxactive = 0;
		DPRINTF_S(nextpath);
		savecurctx(nextpath, file, ctx);
		*path = g_ctx[ctx].c_path;
		*lastdir = g_ctx[ctx].c_last;
		*lastname = g_ctx[ctx].c_name;
	}
}

/*
 * This function does one of the following depending on the values of `fdout` and `page`:
 *  1) fdout == -1 && !page: Write up to CMD_LEN_MAX bytes of command output into g_buf
 *  2) fdout == -1 && page: Create a temp file, write full command output into it and show in pager.
 *  3) fdout != -1 && !page: Write full command output into the provided file.
 *  4) fdout != -1 && page: Don't use! Returns FALSE.
 *
 * g_buf is modified only in case 1.
 * g_tmpfpath is modified only in case 2.
 */
static bool get_output(char *file, char *arg1, char *arg2, int fdout, bool page)
{
	pid_t pid;
	int pipefd[2];
	int index = 0, flags;
	bool ret = FALSE;
	bool have_file = fdout != -1;
	int cmd_in_fd = -1;
	int cmd_out_fd = -1;
	ssize_t len;

	/*
	 * In this case the logic of the function dictates that we should write the output of the command
	 * to `fd` and show it in the pager. But since we didn't open the file descriptor we have no right
	 * to close it, the caller must do it. We don't even know the path to pass to the pager and
	 * it's a real hassle to get it. In general this just invites problems so we are blocking it.
	 */
	if (have_file && page) {
		DPRINTF_S("invalid get_ouptput() call");
		return FALSE;
	}

	/* Setup file descriptors for child command */
	if (!have_file && page) {
		// Case 2
		fdout = create_tmp_file();
		if (fdout == -1)
			return FALSE;

		cmd_in_fd = STDIN_FILENO;
		cmd_out_fd = fdout;
	} else if (have_file) {
		// Case 3
		cmd_in_fd = STDIN_FILENO;
		cmd_out_fd = fdout;
	} else {
		// Case 1
		if (pipe(pipefd) == -1)
			errexit();

		for (index = 0; index < 2; ++index) {
			/* Get previous flags */
			flags = fcntl(pipefd[index], F_GETFL, 0);

			/* Set bit for non-blocking flag */
			flags |= O_NONBLOCK;

			/* Change flags on fd */
			fcntl(pipefd[index], F_SETFL, flags);
		}

		cmd_in_fd = pipefd[0];
		cmd_out_fd = pipefd[1];
	}

	pid = fork();
	if (pid == 0) {
		/* In child */
		close(cmd_in_fd);
		dup2(cmd_out_fd, STDOUT_FILENO);
		dup2(cmd_out_fd, STDERR_FILENO);
		close(cmd_out_fd);

		spawn(file, arg1, arg2, NULL, F_MULTI);
		_exit(EXIT_SUCCESS);
	}

	/* In parent */
	waitpid(pid, NULL, 0);

	/* Do what each case should do */
	if (!have_file && page) {
		// Case 2
		close(fdout);

		spawn(pager, g_tmpfpath, NULL, NULL, F_CLI | F_TTY);

		unlink(g_tmpfpath);
		return TRUE;
	}

	if (have_file)
		// Case 3
		return TRUE;

	// Case 1
	len = read(pipefd[0], g_buf, CMD_LEN_MAX - 1);
	if (len > 0)
		ret = TRUE;

	close(pipefd[0]);
	close(pipefd[1]);
	return ret;
}

/*
 * Follows the stat(1) output closely
 */
static bool show_stats(char *fpath)
{
	static char * const cmds[] = {
#ifdef FILE_MIME_OPTS
		("file " FILE_MIME_OPTS),
#endif
		"file -b",
#if defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__) || defined(__APPLE__)
		"stat -x",
#else
		"stat",
#endif
	};

	size_t r = ELEMENTS(cmds);
	int fd = create_tmp_file();
	if (fd == -1)
		return FALSE;

	while (r)
		get_output(cmds[--r], fpath, NULL, fd, FALSE);

	close(fd);

	spawn(pager, g_tmpfpath, NULL, NULL, F_CLI | F_TTY);
	unlink(g_tmpfpath);
	return TRUE;
}

static bool xchmod(const char *fpath, mode_t *mode)
{
	/* (Un)set (S_IXUSR | S_IXGRP | S_IXOTH) */
	(0100 & *mode) ? (*mode &= ~0111) : (*mode |= 0111);

	return (chmod(fpath, *mode) == 0);
}

static size_t get_fs_info(const char *path, uchar_t type)
{
	struct statvfs svb;

	if (statvfs(path, &svb) == -1)
		return 0;

	if (type == VFS_AVAIL)
		return (size_t)svb.f_bavail << ffs((int)(svb.f_frsize >> 1));

	if (type == VFS_USED)
		return ((size_t)svb.f_blocks - (size_t)svb.f_bfree) << ffs((int)(svb.f_frsize >> 1));

	return (size_t)svb.f_blocks << ffs((int)(svb.f_frsize >> 1)); /* VFS_SIZE */
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
		int fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, S_IWUSR | S_IRUSR); /* Forced create mode for files */

		if (fd == -1 && errno != EEXIST) {
			DPRINTF_S("open!");
			DPRINTF_S(strerror(errno));
			return FALSE;
		}

		close(fd);
	}

	return TRUE;
}

/* List or extract archive */
static bool handle_archive(char *fpath /* in-out param */, char op)
{
	char arg[] = "-tvf"; /* options for tar/bsdtar to list files */
	char *util, *outdir = NULL;
	bool x_to = FALSE;
	bool is_atool = (!g_state.usebsdtar && getutil(utils[UTIL_ATOOL]));

	if (op == 'x') {
		outdir = xreadline(is_atool ? "." : xbasename(fpath), messages[MSG_NEW_PATH]);
		if (!outdir || !*outdir) { /* Cancelled */
			printwait(messages[MSG_CANCEL], NULL);
			return FALSE;
		}
		/* Do not create smart context for current dir */
		if (!(*outdir == '.' && outdir[1] == '\0')) {
			if (!xmktree(outdir, TRUE) || (chdir(outdir) == -1)) {
				printwarn(NULL);
				return FALSE;
			}
			/* Copy the new dir path to open it in smart context */
			outdir = getcwd(NULL, 0);
			x_to = TRUE;
		}
	}

	if (is_atool) {
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
		spawn(util, arg, fpath, NULL, F_NORMAL | F_MULTI);
	else /* list */
		get_output(util, arg, fpath, -1, TRUE);

	if (x_to) {
		if (chdir(xdirname(fpath)) == -1) {
			printwarn(NULL);
			free(outdir);
			return FALSE;
		}
		xstrsncpy(fpath, outdir, PATH_MAX);
		free(outdir);
	} else if (op == 'x')
		fpath[0] = '\0';

	return TRUE;
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

static bool archive_mount(char *newpath)
{
	char *dir, *cmd = xgetenv("NNN_ARCHMNT", utils[UTIL_ARCHMNT]);
	char *name = pdents[cur].name;
	size_t len = pdents[cur].nlen;
	char mntpath[PATH_MAX];

	if (!getutil(cmd)) {
		printmsg("install utility");
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
	if (spawn(cmd, name, newpath, NULL, F_NORMAL)) {
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
		if (spawn(env, tmp, newpath, NULL, flag)) {
			printmsg(messages[MSG_FAILED]);
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
 * Returns TRUE if directory needs to be refreshed *.
 */
static bool unmount(char *name, char *newpath, int *presel, char *currentpath)
{
#if defined(__APPLE__) || defined(__FreeBSD__)
	static char cmd[] = "umount";
#else
	static char cmd[] = "fusermount3"; /* Arch Linux utility */
	static bool found = FALSE;
#endif
	char *tmp = name;
	struct stat sb, psb;
	bool child = FALSE;
	bool parent = FALSE;
	bool hovered = FALSE;
	char mntpath[PATH_MAX];

#if !defined(__APPLE__) && !defined(__FreeBSD__)
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
		if (name && (tmp[0] == '-') && (tmp[1] == '\0')) {
			mkpath(currentpath, name, newpath);
			hovered = TRUE;
		}
	}

	if (!hovered)
		mkpath(mntpath, tmp, newpath);

	if (!xdiraccess(newpath)) {
		*presel = MSGWAIT;
		return FALSE;
	}

#if defined(__APPLE__) || defined(__FreeBSD__)
	if (spawn(cmd, newpath, NULL, NULL, F_NORMAL)) {
#else
	if (spawn(cmd, "-qu", newpath, NULL, F_NORMAL)) {
#endif
		if (!xconfirm(get_input(messages[MSG_LAZY])))
			return FALSE;

#ifdef __APPLE__
		if (spawn(cmd, "-l", newpath, NULL, F_NORMAL)) {
#elif defined(__FreeBSD__)
		if (spawn(cmd, "-f", newpath, NULL, F_NORMAL)) {
#else
		if (spawn(cmd, "-quz", newpath, NULL, F_NORMAL)) {
#endif
			printwait(messages[MSG_FAILED], presel);
			return FALSE;
		}
	}

	if (rmdir(newpath) == -1) {
		printwarn(presel);
		return FALSE;
	}

	return TRUE;
}

static void lock_terminal(void)
{
	spawn(xgetenv("NNN_LOCKER", utils[UTIL_LOCKER]), NULL, NULL, NULL, F_CLI);
}

static void printkv(kv *kvarr, FILE *f, uchar_t max, uchar_t id)
{
	char *val = (id == NNN_BMS) ? bmstr : pluginstr;

	for (uchar_t i = 0; i < max && kvarr[i].key; ++i)
		fprintf(f, " %c: %s\n", (char)kvarr[i].key, val + kvarr[i].off);
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
	int fd = '\r';
	size_t r;

	if (maxbm || bmark) {
		r = xstrsncpy(g_buf, messages[MSG_KEYS], CMD_LEN_MAX);

		if (bmark) { /* There is a marked directory */
			g_buf[--r] = ' ';
			g_buf[++r] = ',';
			g_buf[++r] = '\0';
			++r;
		}
		printkeys(bookmark, g_buf + r - 1, maxbm);
		printmsg(g_buf);
		fd = get_input(NULL);
	}

	r = FALSE;
	if (fd == ',') /* Visit marked directory */
		bmark ? xstrsncpy(newpath, bmark, PATH_MAX) : (r = MSG_NOT_SET);
	else if (fd == '\r') { /* Visit bookmarks directory */
		mkpath(cfgpath, toks[TOK_BM], newpath);
		g_state.selbm = 1;
	} else if (!get_kv_val(bookmark, newpath, fd, maxbm, NNN_BMS))
		r = MSG_INVALID_KEY;

	if (!r && chdir(newpath) == -1) {
		r = MSG_ACCESS;
		if (g_state.selbm)
			g_state.selbm = 0;
	}

	return r;
}

static void add_bookmark(char *path, char *newpath, int *presel)
{
	char *dir = xbasename(path);

	dir = xreadline(dir[0] ? dir : NULL, messages[MSG_BM_NAME]);
	if (dir && *dir) {
		size_t r = mkpath(cfgpath, toks[TOK_BM], newpath);

		newpath[r - 1] = '/';
		xstrsncpy(newpath + r, dir, PATH_MAX - r);
		printwait((symlink(path, newpath) == -1) ? strerror(errno) : newpath, presel);
	} else
		printwait(messages[MSG_CANCEL], presel);
}

/*
 * The help string tokens (each line) start with a HEX value which indicates
 * the number of spaces to print before the particular token. In the middle,
 * %NN can be used to insert a run of spaces, e.g %10 will print 10 spaces.
 * %NN MUST be 2 characters long, e.g %05 for 5 spaces.
 *
 * This method was chosen instead of a flat string because the number of bytes
 * in help was increasing the binary size by around a hundred bytes. This would
 * only have increased as we keep adding new options.
 */
static void show_help(const char *path)
{
	static const char helpstr[] = {
	"2|V\\_\n"
	"2/. \\\\\n"
	"1(;^; ||\n"
	"3/___3\n"
	"2(___n))\n"
	"0\n"
	"1NAVIGATION\n"
	       "9Up k  Up%16PgUp ^U  Page up\n"
	       "9Dn j  Down%14PgDn ^D  Page down\n"
	       "9Lt h  Parent%12~ ` @ -  ~, /, start, prev\n"
	   "5Ret Rt l  Open%20'  First file/match\n"
	       "9g ^A  Top%21J  Jump to entry/offset\n"
	       "9G ^E  End%20^J  Toggle auto-advance on open\n"
	      "8B (,)  Book(mark)%11b ^/  Select bookmark\n"
		"a1-4  Context%11(Sh)Tab  Cycle/new context\n"
	    "62Esc ^Q  Quit%19^y  Next young\n"
		 "b^G  QuitCD%18Q  Pick/err, quit\n"
		  "cq  Quit context\n"
	"0\n"
	"1FILTER & PROMPT\n"
		  "c/  Filter%17^N  Toggle type-to-nav\n"
		"aEsc  Exit prompt%12^L  Toggle last filter\n"
		  "c.  Toggle hidden%05Alt+Esc  Unfilter, quit context\n"
	"0\n"
	"1FILES\n"
	       "9o ^O  Open with%15n  Create new/link\n"
	       "9f ^F  File stats%14d  Detail mode toggle\n"
		 "b^R  Rename/dup%14r  Batch rename\n"
		  "cz  Archive%17e  Edit file\n"
		  "c*  Toggle exe%14>  Export list\n"
	    "6Space +  (Un)select%12m-m  Select range/clear\n"
	          "ca  Select all%14A  Invert sel\n"
	       "9p ^P  Copy here%12w ^W  Cp/mv sel as\n"
	       "9v ^V  Move here%15E  Edit sel list\n"
	       "9x ^X  Delete or trash%09S  Listed sel size\n"
		  "cX  Delete (rm -rf)%07Esc  Send to FIFO\n"
	"0\n"
	"1MISC\n"
	      "8Alt ;  Select plugin%11=  Launch app\n"
	       "9! ^]  Shell%19]  Cmd prompt\n"
		  "cc  Connect remote%10u  Unmount remote/archive\n"
	       "9t ^T  Sort toggles%12s  Manage session\n"
		  "cT  Set time type%110  Lock\n"
		 "b^L  Redraw%18?  Help, conf\n"
	};

	int fd = create_tmp_file();
	if (fd == -1)
		return;
	FILE *f = fdopen(fd, "wb");
	if (f == NULL) {
		close(fd);
		unlink(g_tmpfpath);
		return;
	}

	char *prog = xgetenv(env_cfg[NNN_HELP], NULL);
	if (prog)
		get_output(prog, NULL, NULL, fd, FALSE);

	bool hex = true;
	const char *end = helpstr + sizeof(helpstr) - 1;

	for (const char *s = helpstr; s < end; ++s) {
		if (hex) {
			for (int k = 0, n = xchartohex(*s); k < n; ++k)
				fputc(' ', f);
		} else if (*s == '%') {
			int n = ((s[1] - '0') * 10) + (s[2] - '0');
			for (int k = 0; k < n; ++k)
				fputc(' ', f);
			s += 2;
		} else {
			fputc(*s, f);
		}
		hex = (*s == '\n');
	}

	fprintf(f, "\nLOCATIONS\n");
	for (uchar_t i = 0; i < CTX_MAX; ++i)
		if (g_ctx[i].c_cfg.ctxactive)
			fprintf(f, " %u: %s\n", i + 1, g_ctx[i].c_path);

	fprintf(f, "\nVOLUME: avail:%s ", coolsize(get_fs_info(path, VFS_AVAIL)));
	fprintf(f, "used:%s ", coolsize(get_fs_info(path, VFS_USED)));
	fprintf(f, "size:%s\n\n", coolsize(get_fs_info(path, VFS_SIZE)));

	if (bookmark) {
		fprintf(f, "BOOKMARKS\n");
		printkv(bookmark, f, maxbm, NNN_BMS);
		fprintf(f, "\n");
	}

	if (plug) {
		fprintf(f, "PLUGIN KEYS\n");
		printkv(plug, f, maxplug, NNN_PLUG);
		fprintf(f, "\n");
	}

	for (uchar_t i = NNN_OPENER; i <= NNN_TRASH; ++i) {
		char *s = getenv(env_cfg[i]);
		if (s)
			fprintf(f, "%s: %s\n", env_cfg[i], s);
	}

	if (selpath)
		fprintf(f, "SELECTION FILE: %s\n", selpath);

	fprintf(f, "\nv%s\n%s\n", VERSION, GENERAL_INFO);
	fclose(f); // also closes fd

	spawn(pager, g_tmpfpath, NULL, NULL, F_CLI | F_TTY);
	unlink(g_tmpfpath);
}

static void setexports(const char *path)
{
	char dvar[] = "d0";
	char fvar[] = "f0";

	if (ndents) {
		setenv(envs[ENV_NCUR], pdents[cur].name, 1);
		xstrsncpy(g_ctx[cfg.curctx].c_name, pdents[cur].name, NAME_MAX + 1);
	} else if (g_ctx[cfg.curctx].c_name[0])
		g_ctx[cfg.curctx].c_name[0] = '\0';

	for (uchar_t i = 0; i < CTX_MAX; ++i) {
		if (g_ctx[i].c_cfg.ctxactive) {
			dvar[1] = fvar[1] = '1' + i;
			setenv(dvar, g_ctx[i].c_path, 1);

			if (g_ctx[i].c_name[0]) {
				mkpath(g_ctx[i].c_path, g_ctx[i].c_name, g_buf);
				setenv(fvar, g_buf, 1);
			}
		}
	}
	setenv("NNN_INCLUDE_HIDDEN", xitoa(cfg.showhidden), 1);
	setenv("NNN_PREFER_SELECTION", xitoa(cfg.prefersel), 1);
	setenv("PWD", path, 1);
}

static void run_cmd_as_plugin(const char *file, uchar_t flags)
{
	size_t len;

	xstrsncpy(g_buf, file, PATH_MAX);

	len = xstrlen(g_buf);
	if (len > 1 && g_buf[len - 1] == '*') {
		flags &= ~F_CONFIRM; /* Skip user confirmation */
		g_buf[len - 1] = '\0'; /* Get rid of trailing no confirmation symbol */
		--len;
	}

	if (flags & F_PAGE)
		get_output(utils[UTIL_SH_EXEC], g_buf, NULL, -1, TRUE);
	else
		spawn(utils[UTIL_SH_EXEC], g_buf, NULL, NULL, flags);
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
		spawn(utils[UTIL_RM_RF], listpath, NULL, NULL, F_NOTRACE | F_MULTI);
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

static char *readpipe(int fd, char *ctxnum, char **path)
{
	char ctx, *nextpath = NULL;

	if (read_nointr(fd, g_buf, 1) != 1)
		return NULL;

	if (g_buf[0] == '-') { /* Clear selection on '-' */
		clearselection();
		if (read_nointr(fd, g_buf, 1) != 1)
			return NULL;
	}

	if (g_buf[0] == '+')
		ctx = (char)(get_free_ctx() + 1);
	else if (g_buf[0] < '0')
		return NULL;
	else {
		ctx = g_buf[0] - '0';
		if (ctx > CTX_MAX)
			return NULL;
	}

	if (read_nointr(fd, g_buf, 1) != 1)
		return NULL;

	char op = g_buf[0];

	if (op == 'c') {
		ssize_t len = read_nointr(fd, g_buf, PATH_MAX);

		if (len <= 0)
			return NULL;

		g_buf[len] = '\0'; /* Terminate the path read */
		if (g_buf[0] == '/') {
			nextpath = g_buf;
			len = xstrlen(g_buf);
			while (--len && (g_buf[len] == '/')) /* Trim all trailing '/' */
				g_buf[len] = '\0';
		}
	} else if (op == 'l') {
		rmlistpath(); /* Remove last list mode path, if any */
		nextpath = load_input(fd, *path);
	} else if (op == 'p') {
		free(selpath);
		selpath = NULL;
		clearselection();
		g_state.picker = 0;
		g_state.picked = 1;
	}

	*ctxnum = ctx;

	return nextpath;
}

static bool run_plugin(char **path, const char *file, char *runfile, char **lastname, char **lastdir)
{
	pid_t p;
	char ctx = 0;
	uchar_t flags = 0;
	bool cmd_as_plugin = FALSE;
	char *nextpath;

	if (!g_state.pluginit) {
		plctrl_init();
		g_state.pluginit = 1;
	}

	setexports(*path);

	/* Check for run-cmd-as-plugin mode */
	if (*file == '!') {
		flags = F_MULTI | F_CONFIRM;
		++file;

		if (*file == '|') { /* Check if output should be paged */
			flags |= F_PAGE;
			++file;
		} else if (*file == '&') { /* Check if GUI flags are to be used */
			flags = F_MULTI | F_NOTRACE | F_NOWAIT;
			++file;
		}

		if (!*file)
			return FALSE;

		if ((flags & F_NOTRACE) || (flags & F_PAGE)) {
			run_cmd_as_plugin(file, flags);
			return TRUE;
		}

		cmd_as_plugin = TRUE;
	}

	if (mkfifo(g_pipepath, 0600) != 0)
		return FALSE;

	exitcurses();

	p = fork();

	if (!p) { // In child
		int wfd = open(g_pipepath, O_WRONLY | O_CLOEXEC);

		if (wfd == -1)
			_exit(EXIT_FAILURE);

		if (!cmd_as_plugin) {
			char *sel = NULL;
			char std[2] = "-";

			/* Generate absolute path to plugin */
			mkpath(plgpath, file, g_buf);

			if (g_state.picker)
				sel = selpath ? selpath : std;

			if (runfile && runfile[0]) {
				xstrsncpy(*lastname, runfile, NAME_MAX);
				spawn(g_buf, *lastname, *path, sel, 0);
			} else
				spawn(g_buf, NULL, *path, sel, 0);
		} else
			run_cmd_as_plugin(file, flags);

		close(wfd);
		_exit(EXIT_SUCCESS);
	}

	int rfd;

	do
		rfd = open(g_pipepath, O_RDONLY);
	while (rfd == -1 && errno == EINTR);

	nextpath = readpipe(rfd, &ctx, path);
	if (nextpath)
		set_smart_ctx(ctx, nextpath, path, runfile, lastname, lastdir);

	close(rfd);

	/* wait for the child to finish. no zombies allowed */
	waitpid(p, NULL, 0);

	refresh();

	unlink(g_pipepath);

	return TRUE;
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
		spawn(tmp, (r == F_NORMAL) ? "0" : NULL, NULL, NULL, r);

	return FALSE;
}

/* Returns TRUE if at least one command was run */
static bool prompt_run(void)
{
	bool ret = FALSE;
	char *cmdline, *next;
	int cnt_j, cnt_J, cmd_ret;
	size_t len;

	const char *xargs_j = "xargs -0 -I{} %s < %s";
	const char *xargs_J = "xargs -0 %s < %s";
	char cmd[CMD_LEN_MAX + 32]; // 32 for xargs format strings

	while (1) {
#ifndef NORL
		if (g_state.picker || g_state.xprompt) {
#endif
			cmdline = xreadline(NULL, PROMPT);
#ifndef NORL
		} else
			cmdline = getreadline("\n"PROMPT);
#endif
		// Check for an empty command
		if (!cmdline || !cmdline[0])
			break;

		free(lastcmd);
		lastcmd = xstrdup(cmdline);
		ret = TRUE;

		len = xstrlen(cmdline);

		cnt_j = 0;
		next = cmdline;
		while ((next = strstr(next, "%j"))) {
			++cnt_j;

			// replace %j with {} for xargs later
			next[0] = '{';
			next[1] = '}';

			++next;
		}

		cnt_J = 0;
		next = cmdline;
		while ((next = strstr(next, "%J"))) {
			++cnt_J;

			// %J should be the last thing in the command
			if (next == cmdline + len - 2) {
				cmdline[len - 2] = '\0';
			}

			++next;
		}

		// We can't handle both %j and %J in a single command
		if (cnt_j && cnt_J)
			break;

		if (cnt_j)
			snprintf(cmd, CMD_LEN_MAX + 32, xargs_j, cmdline, selpath);
		else if (cnt_J)
			snprintf(cmd, CMD_LEN_MAX + 32, xargs_J, cmdline, selpath);

		cmd_ret = spawn(shell, "-c", (cnt_j || cnt_J) ? cmd : cmdline, NULL, F_CLI | F_CONFIRM);
		if ((cnt_j || cnt_J) && cmd_ret == 0)
			clearselection();
	}

	return ret;
}

static bool handle_cmd(enum action sel, char *path, char *newpath)
{
	endselection(FALSE);

	if (sel == SEL_LAUNCH)
		return launch_app(newpath);

	setexports(path);

	if (sel == SEL_PROMPT)
		return prompt_run();

	/* Set nnn nesting level */
	char *tmp = getenv(env_cfg[NNNLVL]);
	int r = tmp ? atoi(tmp) : 0;

	setenv(env_cfg[NNNLVL], xitoa(r + 1), 1);
	spawn(shell, NULL, NULL, NULL, F_CLI);
	setenv(env_cfg[NNNLVL], xitoa(r), 1);
	return TRUE;
}

static void dentfree(void)
{
	free(pnamebuf);
	free(pdents);
	free(mark);

	/* Thread data cleanup */
	free(core_blocks);
	free(core_data);
	free(core_files);
}

static void *du_thread(void *p_data)
{
	thread_data *pdata = (thread_data *)p_data;
	char *path[2] = {pdata->path, NULL};
	ullong_t tfiles = 0;
	blkcnt_t tblocks = 0;
	struct stat *sb;
	FTS *tree = fts_open(path, FTS_PHYSICAL | FTS_XDEV | FTS_NOCHDIR, 0);
	FTSENT *node;

	while ((node = fts_read(tree))) {
		if (node->fts_info & FTS_D) {
			if (g_state.interrupt)
				break;
			continue;
		}

		sb = node->fts_statp;

		if (cfg.apparentsz) {
			if (sb->st_size && DU_TEST)
				tblocks += sb->st_size;
		} else if (sb->st_blocks && DU_TEST)
			tblocks += sb->st_blocks;

		++tfiles;
	}

	fts_close(tree);

	if (pdata->entnum >= 0)
		pdents[pdata->entnum].blocks = tblocks;

	if (!pdata->mntpoint) {
		core_blocks[pdata->core] += tblocks;
		core_files[pdata->core] += tfiles;
	} else
		core_files[pdata->core] += 1;

	pthread_mutex_lock(&running_mutex);
	threadbmp |= (1 << pdata->core);
	--active_threads;
	pthread_mutex_unlock(&running_mutex);

	return NULL;
}

static void dirwalk(char *path, int entnum, bool mountpoint)
{
	/* Loop till any core is free */
	while (active_threads == NUM_DU_THREADS);

	if (g_state.interrupt)
		return;

	pthread_mutex_lock(&running_mutex);
	int core = ffs(threadbmp) - 1;

	threadbmp &= ~(1 << core);
	++active_threads;
	pthread_mutex_unlock(&running_mutex);

	xstrsncpy(core_data[core].path, path, PATH_MAX);
	core_data[core].entnum = entnum;
	core_data[core].core = (ushort_t)core;
	core_data[core].mntpoint = mountpoint;

	pthread_t tid = 0;

	pthread_create(&tid, NULL, du_thread, (void *)&(core_data[core]));

	tolastln();
	addstr(xbasename(path));
	addstr(" [^C aborts]\n");
	refresh();
}

static bool prep_threads(void)
{
	if (!g_state.duinit) {
		/* drop MSB 1s */
		threadbmp >>= (32 - NUM_DU_THREADS);

		if (!core_blocks)
			core_blocks = calloc(NUM_DU_THREADS, sizeof(blkcnt_t));
		if (!core_data)
			core_data = calloc(NUM_DU_THREADS, sizeof(thread_data));
		if (!core_files)
			core_files = calloc(NUM_DU_THREADS, sizeof(ullong_t));

		if (!core_blocks || !core_data || !core_files) {
			printwarn(NULL);
			return FALSE;
		}
#ifndef __APPLE__
		/* Increase current open file descriptor limit */
		max_openfds();
#endif
		g_state.duinit = TRUE;
	} else {
		memset(core_blocks, 0, NUM_DU_THREADS * sizeof(blkcnt_t));
		memset(core_data, 0, NUM_DU_THREADS * sizeof(thread_data));
		memset(core_files, 0, NUM_DU_THREADS * sizeof(ullong_t));
	}
	return TRUE;
}

/* Skip self and parent */
static inline bool selforparent(const char *path)
{
	return path[0] == '.' && (path[1] == '\0' || (path[1] == '.' && path[2] == '\0'));
}

static int dentfill(char *path, struct entry **ppdents)
{
	uchar_t entflags = 0;
	int flags = 0;
	struct dirent *dp;
	char *namep, *pnb, *buf;
	struct entry *dentp;
	size_t off = 0, namebuflen = NAMEBUF_INCR;
	struct stat sb_path, sb;
	DIR *dirp = opendir(path);

	ndents = 0;
	gtimesecs = time(NULL);

	DPRINTF_S(__func__);

	if (!dirp)
		return 0;

	int fd = dirfd(dirp);

	if (cfg.blkorder) {
		num_files = 0;
		dir_blocks = 0;
		buf = g_buf;

		if (fstatat(fd, path, &sb_path, 0) == -1)
			goto exit;

		if (!ihashbmp) {
			ihashbmp = calloc(1, HASH_OCTETS << 3);
			if (!ihashbmp)
				goto exit;
		} else
			memset(ihashbmp, 0, HASH_OCTETS << 3);

		if (!prep_threads())
			goto exit;

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
					mkpath(path, namep, buf); // NOLINT
					dirwalk(buf, -1, FALSE);

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

		if (ndents == total_dents) {
			if (cfg.blkorder)
				while (active_threads);

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

				for (int count = 1; count < ndents; ++dentp, ++count)
					/* Current file name starts at last file name start + length */
					(dentp + 1)->name = (char *)((size_t)dentp->name + dentp->nlen);
			}
		}

		dentp = *ppdents + ndents;

		/* Selection file name */
		dentp->name = (char *)((size_t)pnamebuf + off);
		dentp->nlen = xstrsncpy(dentp->name, namep, NAME_MAX + 1);
		off += dentp->nlen;

		/* Copy other fields */
		if (cfg.timetype == T_MOD) {
			dentp->sec = sb.st_mtime;
#ifdef __APPLE__
			dentp->nsec = (uint_t)sb.st_mtimespec.tv_nsec;
#else
			dentp->nsec = (uint_t)sb.st_mtim.tv_nsec;
#endif
		} else if (cfg.timetype == T_ACCESS) {
			dentp->sec = sb.st_atime;
#ifdef __APPLE__
			dentp->nsec = (uint_t)sb.st_atimespec.tv_nsec;
#else
			dentp->nsec = (uint_t)sb.st_atim.tv_nsec;
#endif
		} else {
			dentp->sec = sb.st_ctime;
#ifdef __APPLE__
			dentp->nsec = (uint_t)sb.st_ctimespec.tv_nsec;
#else
			dentp->nsec = (uint_t)sb.st_ctim.tv_nsec;
#endif
		}

		if ((gtimesecs - sb.st_mtime <= 300) || (gtimesecs - sb.st_ctime <= 300))
			entflags |= FILE_YOUNG;

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
				mkpath(path, namep, buf); // NOLINT

				/* Need to show the disk usage of this dir */
				dirwalk(buf, ndents, (sb_path.st_dev != sb.st_dev)); // NOLINT

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
				dentp->flags |= DIR_OR_DIRLNK;
#if !(defined(__sun) || defined(__HAIKU__)) /* no d_type */
		} else if (dp->d_type == DT_DIR || ((dp->d_type == DT_LNK
			   || dp->d_type == DT_UNKNOWN) && S_ISDIR(sb.st_mode))) {
			dentp->flags |= DIR_OR_DIRLNK;
#endif
		}

		++ndents;
	} while ((dp = readdir(dirp)));

exit:
	if (g_state.duinit && cfg.blkorder) {
		while (active_threads);

		attroff(COLOR_PAIR(cfg.curctx + 1));
		for (int i = 0; i < NUM_DU_THREADS; ++i) {
			num_files += core_files[i];
			dir_blocks += core_blocks[i];
		}
	}

	/* Should never be null */
	if (closedir(dirp) == -1)
		errexit();

	return ndents;
}

static void populate(char *path, char *lastname)
{
#ifdef DEBUG
	struct timespec ts1, ts2;

	clock_gettime(CLOCK_REALTIME, &ts1); /* Use CLOCK_MONOTONIC on FreeBSD */
#endif

	ndents = dentfill(path, &pdents);
	if (!ndents)
		return;

#ifndef NOSORT
	ENTSORT(pdents, ndents, entrycmpfn);
#endif

#ifdef DEBUG
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

	if (!force && !memcmp(&lastentry, &pdents[cur], sizeof(struct entry))) // NOLINT
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

static void send_to_explorer(int *presel)
{
	if (nselected) {
		int fd = open(fifopath, O_WRONLY|O_NONBLOCK|O_CLOEXEC, 0600);
		if ((fd == -1) || (seltofile(fd, NULL, NEWLINE) != (size_t)(selbufpos)))
			printwarn(presel);
		else {
			resetselind();
			clearselection();
		}
		if (fd > 1)
			close(fd);
	} else
		notify_fifo(TRUE); /* Send opened path to NNN_FIFO */
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
	if (!g_state.fifomode)
		notify_fifo(FALSE); /* Send hovered path to NNN_FIFO */
#endif
}

static void handle_screen_move(enum action sel)
{
	int onscreen;

	switch (sel) {
	case SEL_NEXT:
		if (cfg.rollover || (cur != ndents - 1))
			move_cursor((cur + 1) % ndents, 0);
		break;
	case SEL_PREV:
		if (cfg.rollover || cur)
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
	case SEL_PGUP:
		onscreen = xlines - 4;
		move_cursor(curscroll, 1);
		curscroll -= onscreen - 1;
		break;
	case SEL_CTRL_U:
		onscreen = xlines - 4;
		move_cursor(curscroll, 1);
		curscroll -= onscreen >> 1;
		break;
	case SEL_JUMP:
	{
		char *input = xreadline(NULL, "jump (+n/-n/n): ");

		if (!input || !*input)
			break;
		if (input[0] == '-') {
			cur -= atoi(input + 1);
			if (cur < 0)
				cur = 0;
		} else if (input[0] == '+') {
			cur += atoi(input + 1);
			if (cur >= ndents)
				cur = ndents - 1;
		} else {
			int index = atoi(input);

			if ((index < 1) || (index > ndents))
				break;
			cur = index - 1;
		}
		onscreen = xlines - 4;
		move_cursor(cur, 1);
		curscroll -= onscreen >> 1;
		break;
	}
	case SEL_HOME:
		move_cursor(0, 1);
		break;
	case SEL_END:
		move_cursor(ndents - 1, 1);
		break;
	case SEL_YOUNG:
	{
		for (int r = cur;;) {
			if (++r >= ndents)
				r = 0;
			if (r == cur)
				break;
			if (pdents[r].flags & FILE_YOUNG) {
				move_cursor(r, 0);
				break;
			}
		}
		break;
	}
	default: /* case SEL_FIRST */
	{
		int c = get_input(messages[MSG_FIRST]);

		if (!c)
			break;

		c = TOUPPER(c);

		int r = (c == TOUPPER(*pdents[cur].name)) ? (cur + 1) : 0;

		for (; r < ndents; ++r) {
			if (((c == '\'') && !(pdents[r].flags & DIR_OR_DIRLNK))
			    || (c == TOUPPER(*pdents[r].name))) {
				move_cursor((r) % ndents, 0);
				break;
			}
		}
		break;
	}
	}
}

static void handle_openwith(const char *path, const char *name, char *newpath, char *tmp)
{
	/* Confirm if app is CLI or GUI */
	int r = get_input(messages[MSG_CLI_MODE]);

	r = (r == 'c' ? F_CLI :
	     ((r == 'g' || r == '\r') ? (F_NOWAIT | F_NOTRACE | F_MULTI) : 0));
	if (r) {
		mkpath(path, name, newpath);
		spawn(tmp, newpath, NULL, NULL, r);
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
		else {
			do /* Attempt to create a new context */
				r = (r + 1) & ~CTX_MAX;
			while (g_ctx[r].c_cfg.ctxactive && (r != cfg.curctx));

			if (r == cfg.curctx) /* If all contexts are active, reverse cycle */
				do
					r = (r + (CTX_MAX - 1)) & (CTX_MAX - 1);
				while (!g_ctx[r].c_cfg.ctxactive);
		} // fallthrough
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
	}

	return r;
}

static int set_sort_flags(int r)
{
	bool session = (r == '\0');
	bool reverse = FALSE;

	if (ISUPPER_(r) && (r != 'R') && (r != 'C')) {
		reverse = TRUE;
		r = TOLOWER(r);
	}

	/* Set the correct input in case of a session load */
	if (session) {
		if (cfg.apparentsz) {
			cfg.apparentsz = 0;
			r = 'a';
		} else if (cfg.blkorder) {
			cfg.blkorder = 0;
			r = 'd';
		}

		/* Ensure function pointers are in sync with cfg. */
		entrycmpfn = cfg.reverse ? &reventrycmp : &entrycmp;
		namecmpfn = cfg.version ? &xstrverscasecmp : &xstricmp;
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
			cfg.blkorder = 1;
			blk_shift = 0;
		} else
			cfg.blkorder = 0;
		// fallthrough
	case 'd': /* Disk usage */
		if (r == 'd') {
			if (!cfg.apparentsz)
				cfg.blkorder ^= 1;
			cfg.apparentsz = 0;
			blk_shift = ffs(S_BLKSIZE) - 1;
		}

		if (cfg.blkorder)
			cfg.showdetail = 1;
		cfg.timeorder = 0;
		cfg.sizeorder = 0;
		cfg.extnorder = 0;
		if (!session) {
			cfg.reverse = 0;
			entrycmpfn = &entrycmp;
		}
		endselection(TRUE); /* We are going to reload dir */
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

	if (reverse) {
		cfg.reverse = 1;
		entrycmpfn = &reventrycmp;
	}

	cfgsort[cfg.curctx] = (uchar_t)r;

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
			r = MSG_NOCHANGE;
	} else
		r = MSG_INVALID_KEY;

	if (!ret)
		printwait(messages[r], presel);

	return ret;
}

static void statusbar(char *path)
{
	int i = 0, len = 0;
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
			len = i - (ptr - pent->name);
		if (!ptr || len > 5 || len < 2)
			ptr = "\b";
	} else
		ptr = "\b";

	attron(COLOR_PAIR(cfg.curctx + 1));

	if (cfg.fileinfo && get_output("file", "-b", pdents[cur].name, -1, FALSE))
		mvaddstr(xlines - 2, 2, g_buf);

	tolastln();

	printw("%d/%s ", cur + 1, xitoa(ndents));

	if (g_state.selmode || nselected) {
		attron(A_REVERSE);
		addch(' ');
		if (g_state.rangesel)
			addch('*');
		else if (g_state.selmode)
			addch('+');
		if (nselected)
			addstr(xitoa(nselected));
		addch(' ');
		attroff(A_REVERSE);
		addch(' ');
	}

	if (cfg.blkorder) { /* du mode */
		char buf[24];

		xstrsncpy(buf, coolsize(dir_blocks << blk_shift), 12);

		printw("%cu:%s avail:%s files:%llu %lluB %s\n",
		       (cfg.apparentsz ? 'a' : 'd'), buf, coolsize(get_fs_info(path, VFS_AVAIL)),
		       num_files, (ullong_t)pent->blocks << blk_shift, ptr);
	} else { /* light or detail mode */
		char sort[] = "\0\0\0\0\0";

		if (getorderstr(sort))
			addstr(sort);

		/* Timestamp */
		print_time(&pent->sec, pent->flags);

		addch(' ');
		addstr(get_lsperms(pent->mode));
		addch(' ');
#ifndef NOUG
		if (g_state.uidgid) {
			addstr(getpwname(pent->uid));
			addch(':');
			addstr(getgrname(pent->gid));
			addch(' ');
		}
#endif
		if (S_ISLNK(pent->mode)) {
			if (!cfg.fileinfo) {
				i = readlink(pent->name, g_buf, PATH_MAX);
				addstr(coolsize(i >= 0 ? i : pent->size)); /* Show symlink size */
				if (i > 1) { /* Show symlink target */
					int y;

					addstr(" ->");
					getyx(stdscr, len, y);
					i = MIN(i, xcols - y);
					g_buf[i] = '\0';
					addstr(g_buf);
				}
			}
		} else {
			addstr(coolsize(pent->size));
			addch(' ');
			addstr(ptr);
			if (pent->flags & HARD_LINK) {
				struct stat sb;

				if (stat(pent->name, &sb) != -1) {
					addch(' ');
					addstr(xitoa((int)sb.st_nlink)); /* Show number of links */
					addch('-');
					addstr(xitoa((int)sb.st_ino)); /* Show inode number */
				}
			}
		}
		clrtoeol();
	}

	attroff(COLOR_PAIR(cfg.curctx + 1));
	/* Place HW cursor on current for Braille systems */
	tocursor();
}

static inline void markhovered(void)
{
	if (cfg.showdetail && ndents) { /* Bold forward arrowhead */
		tocursor();
		addch('>' | A_BOLD);
	}
}

static int adjust_cols(int n)
{
	/* Calculate the number of cols available to print entry name */
#ifdef ICONS_ENABLED
	n -= (g_state.oldcolor ? 0 : ICON_SIZE + ICON_PADDING_LEFT_LEN + ICON_PADDING_RIGHT_LEN);
#endif
	if (cfg.showdetail) {
		/* Fallback to light mode if less than 35 columns */
		if (n < 36)
			cfg.showdetail ^= 1;
		else /* 2 more accounted for below */
			n -= 32;
	}

	/* 2 columns for preceding space and indicator */
	return (n - 2);
}

static void draw_line(int ncols)
{
	bool dir = FALSE;

	ncols = adjust_cols(ncols);

	if (g_state.oldcolor && (pdents[last].flags & DIR_OR_DIRLNK)) {
		attron(COLOR_PAIR(cfg.curctx + 1) | A_BOLD);
		dir = TRUE;
	}

	move(2 + last - curscroll, 0);
	macos_icons_hack();
	printent(last, ncols, FALSE);

	if (g_state.oldcolor && (pdents[cur].flags & DIR_OR_DIRLNK)) {
		if (!dir)  {/* First file is not a directory */
			attron(COLOR_PAIR(cfg.curctx + 1) | A_BOLD);
			dir = TRUE;
		}
	} else if (dir) { /* Second file is not a directory */
		attroff(COLOR_PAIR(cfg.curctx + 1) | A_BOLD);
		dir = FALSE;
	}

	move(2 + cur - curscroll, 0);
	macos_icons_hack();
	printent(cur, ncols, TRUE);

	/* Must reset e.g. no files in dir */
	if (dir)
		attroff(COLOR_PAIR(cfg.curctx + 1) | A_BOLD);

	markhovered();
}

static void redraw(char *path)
{
	getmaxyx(stdscr, xlines, xcols);

	int ncols = (xcols <= PATH_MAX) ? xcols : PATH_MAX;
	int onscreen = xlines - 4;
	int i, j = 1;

	// Fast redraw
	if (g_state.move) {
		g_state.move = 0;

		if (ndents && (last_curscroll == curscroll))
			return draw_line(ncols);
	}

	DPRINTF_S(__func__);

	/* Clear screen */
	erase();

	/* Enforce scroll/cursor invariants */
	move_cursor(cur, 1);

	/* Fail redraw if < than 10 columns, context info prints 10 chars */
	if (ncols <= MIN_DISPLAY_COL) {
		printmsg(messages[MSG_FEW_COLUMNS]);
		return;
	}

	//DPRINTF_D(cur);
	DPRINTF_S(path);

	for (i = 0; i < CTX_MAX; ++i) { /* 8 chars printed for contexts - "1 2 3 4 " */
		if (!g_ctx[i].c_cfg.ctxactive)
			addch(i + '1');
		else
			addch((i + '1') | (COLOR_PAIR(i + 1) | A_BOLD
				/* active: underline, current: reverse */
				| ((cfg.curctx != i) ? A_UNDERLINE : A_REVERSE)));

		addch(' ');
	}

	attron(A_UNDERLINE | COLOR_PAIR(cfg.curctx + 1));

	/* Print path */
	bool in_home = set_tilde_in_path(path);
	char *ptr = in_home ? &path[homelen - 1] : path;

	i = (int)xstrlen(ptr);
	if ((i + MIN_DISPLAY_COL) <= ncols)
		addnstr(ptr, ncols - MIN_DISPLAY_COL);
	else {
		char *base = xmemrchr((uchar_t *)ptr, '/', i);

		if (in_home) {
			addch(*ptr);
			++ptr;
			i = 1;
		} else
			i = 0;

		if (ptr && (base != ptr)) {
			while (ptr < base) {
				if (*ptr == '/') {
					i += 2; /* 2 characters added */
					if (ncols < i + MIN_DISPLAY_COL) {
						base = NULL; /* Can't print more characters */
						break;
					}

					addch(*ptr);
					addch(*(++ptr));
				}
				++ptr;
			}
		}

		if (base)
			addnstr(base, ncols - (MIN_DISPLAY_COL + i));
	}

	if (in_home)
		reset_tilde_in_path(path);

	attroff(A_UNDERLINE | COLOR_PAIR(cfg.curctx + 1));

	/* Go to first entry */
	if (curscroll > 0) {
		move(1, 0);
#ifdef ICONS_ENABLED
		addstr(ICON_ARROW_UP);
#else
		addch('^');
#endif
	}

	if (g_state.oldcolor) {
		attron(COLOR_PAIR(cfg.curctx + 1) | A_BOLD);
		g_state.dircolor = 1;
	}

	onscreen = MIN(onscreen + curscroll, ndents);

	ncols = adjust_cols(ncols);

	int len = scanselforpath(path, FALSE);

	/* Print listing */
	for (i = curscroll; i < onscreen; ++i) {
		move(++j, 0);

		if (len)
			findmarkentry(len, &pdents[i]);

		printent(i, ncols, i == cur);
	}

	/* Must reset e.g. no files in dir */
	if (g_state.dircolor) {
		attroff(COLOR_PAIR(cfg.curctx + 1) | A_BOLD);
		g_state.dircolor = 0;
	}

	/* Go to last entry */
	if (onscreen < ndents) {
		move(xlines - 2, 0);
#ifdef ICONS_ENABLED
		addstr(ICON_ARROW_DOWN);
#else
		addch('v');
#endif
	}

	markhovered();
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

static void showselsize(const char *path)
{
	off_t sz = 0;
	int len = scanselforpath(path, FALSE);

	for (int r = 0, selcount = nselected; (r < ndents) && selcount; ++r)
		if (findinsel(findselpos, len + xstrsncpy(g_sel + len, pdents[r].name, pdents[r].nlen))) {
			sz += cfg.blkorder ? pdents[r].blocks : pdents[r].size;
			--selcount;
		}

	printmsg(coolsize(cfg.blkorder ? sz << blk_shift : sz));
}

static bool browse(char *ipath, const char *session, int pkey)
{
	alignas(max_align_t) char newpath[PATH_MAX];
	alignas(max_align_t) char runfile[NAME_MAX + 1];
	char *path, *lastdir, *lastname, *dir, *tmp;
	pEntry pent;
	enum action sel;
	struct stat sb;
	int r = -1, presel, selstartid = 0, selendid = 0;
	const uchar_t opener_flags = (cfg.cliopener ? F_CLI : (F_NOTRACE | F_NOSTDIN | F_NOWAIT));
	bool watch = FALSE, cd = TRUE;
	ino_t inode = 0;

#ifndef NOMOUSE
	MEVENT event = {0};
	struct timespec mousetimings[2] = {{.tv_sec = 0, .tv_nsec = 0}, {.tv_sec = 0, .tv_nsec = 0}};
	int mousedent[2] = {-1, -1};
	bool currentmouse = 1, rightclicksel = 0;
#endif

	atexit(dentfree);

	getmaxyx(stdscr, xlines, xcols);

#ifndef NOSSN
	/* set-up first context */
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

		lastname = g_ctx[0].c_name; /* last visited file name */

		xstrsncpy(g_ctx[0].c_path, ipath, PATH_MAX);
		/* If the initial path is a file, retain a way to return to start dir */
		if (g_state.initfile) {
			free(initpath);
			initpath = ipath = getcwd(NULL, 0);
		}
		path = g_ctx[0].c_path; /* current directory */

		g_ctx[0].c_fltr[0] = g_ctx[0].c_fltr[1] = '\0';
		g_ctx[0].c_cfg = cfg; /* current configuration */
#ifndef NOSSN
	}
#endif

	newpath[0] = runfile[0] = '\0';

	presel = pkey ? ((pkey == CREATE_NEW_KEY) ? 'n' : ';') : ((cfg.filtermode
			|| (session && (g_ctx[cfg.curctx].c_fltr[0] == FILTER
				|| g_ctx[cfg.curctx].c_fltr[0] == RFILTER)
				&& g_ctx[cfg.curctx].c_fltr[1])) ? FILTER : 0);

	pdents = xrealloc(pdents, total_dents * sizeof(struct entry));
	if (!pdents)
		errexit();

	/* Allocate buffer to hold names */
	pnamebuf = (char *)xrealloc(pnamebuf, NAMEBUF_INCR);
	if (!pnamebuf)
		errexit();

	/* The following call is added to handle a broken window at start */
	if (presel == FILTER)
		handle_key_resize();

begin:
	/*
	 * Can fail when permissions change while browsing.
	 * It's assumed that path IS a directory when we are here.
	 */
	if (chdir(path) == -1) {
		DPRINTF_S("directory inaccessible");
		valid_parent(path, lastname);
		setdirwatch();
	}

#ifndef NOX11
	xterm_cfg(path);
#endif

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

	if (order && cd) {
		if (cfgsort[cfg.curctx] != '0') {
			if (cfgsort[cfg.curctx] == 'z')
				set_sort_flags('c');
			if ((!cfgsort[cfg.curctx] || (cfgsort[cfg.curctx] == 'c'))
			    && ((r = get_kv_key(order, path, maxorder, NNN_ORDER)) > 0)) { // NOLINT
				set_sort_flags(r);
				cfgsort[cfg.curctx] = 'z';
			}
		} else
			cfgsort[cfg.curctx] = cfgsort[CTX_MAX];
	}
	cd = TRUE;

	populate(path, lastname);
	if (g_state.interrupt) {
		g_state.interrupt = cfg.apparentsz = cfg.blkorder = 0;
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
		if ((presel != FILTER) || !filterset()) {
			redraw(path);
			statusbar(path);
		}

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
					savecurctx(path, ndents ? pdents[cur].name : NULL, r);

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
				move_cursor((!cfg.rollover && cur < scroll_lines
						? 0 : (cur + ndents - scroll_lines) % ndents), 0);
				break;
			}

			/* Scroll down */
			if (event.bstate == BUTTON5_PRESSED && ndents
			    && (cfg.rollover || (cur != ndents - 1))) {
				move_cursor((!cfg.rollover && cur >= ndents - scroll_lines)
						? (ndents - 1) : ((cur + scroll_lines) % ndents), 0);
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
				copycurname();
				cd = FALSE;
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
				else if ((event.bstate == BUTTON1_PRESSED) && !g_state.fifomode)
					notify_fifo(TRUE); /* Send clicked path to NNN_FIFO */
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
					> DBLCLK_INTERVAL_NS))
					break;
				/* Double click */
				mousetimings[currentmouse].tv_sec = 0;
				mousedent[currentmouse] = -1;
				sel = SEL_OPEN;
			} else {
				if (cfg.filtermode || filterset())
					presel = FILTER;
				copycurname();
				goto nochange;
			}
#endif
			// fallthrough
		case SEL_NAV_IN: // fallthrough
		case SEL_OPEN:
			/* Cannot descend in empty directories */
			if (!ndents) {
				cd = FALSE;
				g_state.selbm = g_state.runplugin = 0;
				goto begin;
			}

			pent = &pdents[cur];
			if (!g_state.selbm || !(S_ISLNK(pent->mode) &&
			                        bmtarget(pent->name, path, newpath) &&
			                        xstrsncpy(path, lastdir, PATH_MAX)))
				mkpath(path, pent->name, newpath);
			g_state.selbm = 0;
			DPRINTF_S(newpath);

			/* Visit directory */
			if (pent->flags & DIR_OR_DIRLNK) {
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

			/* Handle plugin selection mode */
			if (g_state.runplugin) {
				g_state.runplugin = 0;
				/* Must be in plugin dir and same context to select plugin */
				if ((g_state.runctx == cfg.curctx) && !strcmp(path, plgpath)) {
					endselection(FALSE);
					/* Copy path so we can return back to earlier dir */
					xstrsncpy(path, lastdir, PATH_MAX);
					clearfilter();

					if (chdir(path) == -1
					    || !run_plugin(&path, pent->name, runfile, &lastname, &lastdir)) {
						DPRINTF_S("plugin failed!");
					}

					if (g_state.picked)
						return EXIT_SUCCESS;

					if (runfile[0]) {
						xstrsncpy(lastname, runfile, NAME_MAX + 1);
						runfile[0] = '\0';
					}
					setdirwatch();
					goto begin;
				}
			}

#ifndef NOFIFO
			if (g_state.fifomode && (sel == SEL_OPEN)) {
				send_to_explorer(&presel); /* Write selection to explorer fifo */
				break;
			}
#endif
			/* If opened as vim plugin and Enter/^M pressed, pick */
			if (g_state.picker && (sel == SEL_OPEN)) {
				if (nselected == 0) /* Pick if none selected */
					appendfpath(newpath, mkpath(path, pent->name, newpath));
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

					cdprep(lastdir, NULL, path, newpath)
					       ? (presel = FILTER) : (watch = TRUE);
					xstrsncpy(lastname, pent->name, NAME_MAX + 1);
					goto begin;
				}

				/* Open file disabled on right arrow or `l` */
				if (cfg.nonavopen)
					goto nochange;
			}

			if (!sb.st_size) {
				printwait(messages[MSG_EMPTY_FILE], &presel);
				goto nochange;
			}

			if (cfg.useeditor
#ifdef FILE_MIME_OPTS
			    && get_output("file", FILE_MIME_OPTS, newpath, -1, FALSE)
			    && is_prefix(g_buf, "text/", 5)
#else
			    /* no MIME option; guess from description instead */
			    && get_output("file", "-bL", newpath, -1, FALSE)
			    && strstr(g_buf, "text")
#endif
			) {
				spawn(editor, newpath, NULL, NULL, F_CLI);
				if (cfg.filtermode) {
					presel = FILTER;
					clearfilter();
				}
				continue;
			}

			/* Get the extension for regex match */
			tmp = xextension(pent->name, pent->nlen - 1);
#ifdef PCRE
			if (tmp && !pcre_exec(archive_pcre, NULL, tmp,
					      pent->nlen - (tmp - pent->name) - 1, 0, 0, NULL, 0)) {
#else
			if (tmp && !regexec(&archive_re, tmp, 0, NULL, 0)) {
#endif
				r = get_input(messages[MSG_ARCHIVE_OPTS]);
				if (r == '\r')
					r = 'l';
				if (r == 'l' || r == 'x') {
					mkpath(path, pent->name, newpath);
					if (!handle_archive(newpath, r)) {
						presel = MSGWAIT;
						goto nochange;
					}
					if (r == 'l') {
						statusbar(path);
						goto nochange;
					}
				}

				if ((r == 'm') && !archive_mount(newpath)) {
					presel = MSGWAIT;
					goto nochange;
				}

				if (r == 'x' || r == 'm') {
					if (newpath[0])
						set_smart_ctx('+', newpath, &path,
							      ndents ? pdents[cur].name : NULL,
							      &lastname, &lastdir);
					else
						copycurname();
					clearfilter();
					goto begin;
				}

				if (r != 'o') {
					printwait(messages[MSG_INVALID_KEY], &presel);
					goto nochange;
				}
			}

			/* Invoke desktop opener as last resort */
			spawn(opener, newpath, NULL, NULL, opener_flags);

			/* Move cursor to the next entry if not the last entry */
			if (g_state.autonext && cur != ndents - 1)
				move_cursor((cur + 1) % ndents, 0);
			if (cfg.filtermode) {
				presel = FILTER;
				clearfilter();
			}
			continue;
		case SEL_NEXT: // fallthrough
		case SEL_PREV: // fallthrough
		case SEL_PGDN: // fallthrough
		case SEL_CTRL_D: // fallthrough
		case SEL_PGUP: // fallthrough
		case SEL_CTRL_U: // fallthrough
		case SEL_HOME: // fallthrough
		case SEL_END: // fallthrough
		case SEL_FIRST: // fallthrough
		case SEL_YOUNG:
			if (ndents) {
				g_state.move = 1;
				handle_screen_move(sel);
			}
			break;
		case SEL_JUMP:
			if (ndents) {
				g_state.showlines = 1;
				redraw(path);
				handle_screen_move(sel);
				g_state.showlines = 0;
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

			g_state.selbm = 0;
			if (strcmp(path, dir) == 0) {
				if (dir == ipath) {
					if (cfg.filtermode)
						presel = FILTER;
					goto nochange;
				}
				dir = lastdir; /* Go to last dir on home/root key repeat */
			}

			if (chdir(dir) == -1) {
				presel = MSGWAIT;
				goto nochange;
			}

			/* SEL_CDLAST: dir pointing to lastdir */
			xstrsncpy(newpath, dir, PATH_MAX); // fallthrough
		case SEL_BMOPEN:
			if (sel == SEL_BMOPEN) {
				r = (int)handle_bookmark(mark, newpath);
				if (r) {
					printwait(messages[r], &presel);
					goto nochange;
				}

				if (g_state.selbm == 1) /* Allow filtering in bookmarks directory */
					presel = FILTER;
				if (strcmp(path, newpath) == 0)
					break;
			}

			/* In list mode, retain the last file name to highlight it, if possible */
			cdprep(lastdir, listpath && sel == SEL_CDLAST ? NULL : lastname, path, newpath)
			       ? (presel = FILTER) : (watch = TRUE);
			goto begin;
		case SEL_REMOTE:
			if ((sel == SEL_REMOTE) && !remote_mount(newpath)) {
				presel = MSGWAIT;
				goto nochange;
			}

			set_smart_ctx('+', newpath, &path,
				      ndents ? pdents[cur].name : NULL, &lastname, &lastdir);
			clearfilter();
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
			savecurctx(path, ndents ? pdents[cur].name : NULL, r);

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
		case SEL_BMARK:
			add_bookmark(path, newpath, &presel);
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
			if (presel == FILTER) { /* Refresh dir and filter again */
				cd = FALSE;
				goto begin;
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
				copycurname();
				cd = FALSE;
				goto begin;
			case SEL_DETAIL:
				cfg.showdetail ^= 1;
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

				if (r == 'd' || r == 'a') {
					presel = 0;
					goto begin;
				}

				ENTSORT(pdents, ndents, entrycmpfn);
				move_cursor(ndents ? dentfind(lastname, ndents) : 0, 0);
			}
			continue;
		case SEL_STATS: // fallthrough
		case SEL_CHMODX:
			if (ndents) {
				tmp = (listpath && xstrcmp(path, listpath) == 0) ? listroot : path;
				mkpath(tmp, pdents[cur].name, newpath);

				if ((sel == SEL_STATS && !show_stats(newpath))
				    || (lstat(newpath, &sb) == -1)
				    || (sel == SEL_CHMODX && !xchmod(newpath, &sb.st_mode))) {
					printwarn(&presel);
					goto nochange;
				}

				if (sel == SEL_CHMODX)
					pdents[cur].mode = sb.st_mode;
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
				endselection(TRUE);
				setenv("NNN_INCLUDE_HIDDEN", xitoa(cfg.showhidden), 1);
				setenv("NNN_PREFER_SELECTION", xitoa(cfg.prefersel), 1);
				setenv("NNN_LIST", listpath ? listroot : "", 1);

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
				copycurname();
				goto nochange;
			case SEL_EDIT:
				if (!(g_state.picker || g_state.fifomode))
					spawn(editor, newpath, NULL, NULL, F_CLI);
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
			copycurname();
			/* Repopulate as directory content may have changed */
			cd = FALSE;
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
				--nselected;
				rmfromselbuf(mkpath(path, pdents[cur].name, g_sel));
			}

#ifndef NOX11
			if (cfg.x11)
				plugscript(utils[UTIL_CBCP], F_NOWAIT | F_NOTRACE);
#endif
#ifndef NOMOUSE
			if (rightclicksel)
				rightclicksel = 0;
			else
#endif
				/* move cursor to the next entry if this is not the last entry */
				if (!g_state.stayonsel && (cur != ndents - 1))
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
				inode = sb.st_ino;
				selstartid = cur;
				continue;
			}

			if (inode != sb.st_ino) {
				printwait(messages[MSG_DIR_CHANGED], &presel);
				goto nochange;
			}

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

			if ((nselected > LARGESEL) || (nselected && (ndents > LARGESEL))) {
				printmsg("processing...");
				refresh();
			}

			r = scanselforpath(path, TRUE); /* Get path length suffixed by '/' */
			((sel == SEL_SELINV) && findselpos)
				? invertselbuf(r) : addtoselbuf(r, selstartid, selendid);

#ifndef NOX11
			if (cfg.x11)
				plugscript(utils[UTIL_CBCP], F_NOWAIT | F_NOTRACE);
#endif
			continue;
		case SEL_SELEDIT:
			r = editselection();
			if (r <= 0) {
				r = !r ? MSG_0_SELECTED : MSG_FAILED;
				printwait(messages[r], &presel);
			} else {
#ifndef NOX11
				if (cfg.x11)
					plugscript(utils[UTIL_CBCP], F_NOWAIT | F_NOTRACE);
#endif
				cfg.filtermode ?  presel = FILTER : statusbar(path);
			}
			goto nochange;
		case SEL_CP: // fallthrough
		case SEL_MV: // fallthrough
		case SEL_CPMVAS: // fallthrough
		case SEL_TRASH: // fallthrough
		case SEL_RM_ONLY:
		{
			if (sel == SEL_TRASH || sel == SEL_RM_ONLY) {
				r = get_cur_or_sel();
				if (!r) {
					statusbar(path);
					goto nochange;
				}

				if (r == 'c') {
					tmp = (listpath && xstrcmp(path, listpath) == 0)
					      ? listroot : path;
					mkpath(tmp, pdents[cur].name, newpath);
					if (!xrm(newpath, g_state.trash && sel == SEL_TRASH))
						continue;

					xrmfromsel(tmp, newpath);

					copynextname(lastname);

					if (cfg.filtermode || filterset())
						presel = FILTER;
					cd = FALSE;
					goto begin;
				}
			}

			(nselected == 1 && (sel == SEL_CP || sel == SEL_MV))
				? mkpath(path, xbasename(pselbuf), newpath)
				: (newpath[0] = '\0');

			endselection(TRUE);

			if (!cpmvrm_selection(sel, path)) {
				presel = MSGWAIT;
				goto nochange;
			}

			if (cfg.filtermode)
				presel = FILTER;
			clearfilter();

#ifndef NOX11
			/* Show notification on operation complete */
			if (cfg.x11)
				plugscript(utils[UTIL_NTFY], F_NOWAIT | F_NOTRACE);
#endif

			if (newpath[0] && !access(newpath, F_OK))
				xstrsncpy(lastname, xbasename(newpath), NAME_MAX+1);
			else
				copycurname();
			cd = FALSE;
			goto begin;
		}
		case SEL_ARCHIVE: // fallthrough
		case SEL_OPENWITH: // fallthrough
		case SEL_NEW: // fallthrough
		case SEL_RENAME:
		{
			int ret = 'n';
			size_t len;

			if (!ndents && (sel == SEL_OPENWITH || sel == SEL_RENAME))
				break;

			if (sel != SEL_OPENWITH)
				endselection(TRUE);

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
#ifndef NORL
				if (g_state.picker || g_state.xprompt) {
#endif
					tmp = xreadline(NULL, messages[MSG_OPEN_WITH]);
#ifndef NORL
				} else
					tmp = getreadline(messages[MSG_OPEN_WITH]);
#endif
				break;
			case SEL_NEW:
				if (!pkey) {
					r = get_input(messages[MSG_NEW_OPTS]);
					if (r == '\r')
						r = 'f';
					tmp = NULL;
				} else {
					r = 'f';
					tmp = g_ctx[0].c_name;
					pkey = '\0';
				}

				if (r == 'f' || r == 'd')
					tmp = xreadline(tmp, messages[MSG_NEW_PATH]);
				else if (r == 's' || r == 'h')
					tmp = xreadline((nselected == 1 && cfg.prefersel) ? xbasename(pselbuf) : NULL,
						messages[nselected <= 1 ? MSG_NEW_PATH : MSG_LINK_PREFIX]);
				else
					tmp = NULL;
				break;
			default: /* SEL_RENAME */
				tmp = xreadline(pdents[cur].name, "");
				break;
			}

			if (!tmp || !*tmp || is_bad_len_or_dir(tmp))
				break;

			switch (sel) {
			case SEL_ARCHIVE:
				if (r == 'c' && strcmp(tmp, pdents[cur].name) == 0)
					continue; /* Cannot overwrite the hovered file */

				tmp = abspath(tmp, NULL, newpath);
				if (!tmp)
					continue;
				if (access(tmp, F_OK) == 0) {
					if (!xconfirm(get_input(messages[MSG_OVERWRITE]))) {
						statusbar(path);
						goto nochange;
					}
				}

				(r == 's') ? archive_selection(get_archive_cmd(tmp), tmp)
					   : spawn(get_archive_cmd(tmp), tmp, pdents[cur].name,
						   NULL, F_CLI | F_CONFIRM);

				if (tmp && (access(tmp, F_OK) == 0)) { /* File created */
					if (r == 's')
						clearselection(); /* Archive operation complete */

					/* Check if any entry is created in the current directory */
					tmp = get_cwd_entry(path, tmp, &len);
					if (tmp) {
						xstrsncpy(lastname, tmp, len + 1);
						clearfilter(); /* Archive name may not match */
					} if (cfg.filtermode)
						presel = FILTER;
					cd = FALSE;
					goto begin;
				}
				continue;
			case SEL_OPENWITH:
				handle_openwith(path, pdents[cur].name, newpath, tmp);

				cfg.filtermode ?  presel = FILTER : statusbar(path);
				copycurname();
				goto nochange;
			case SEL_RENAME:
				r = 0;
				/* Skip renaming to same name */
				if (strcmp(tmp, pdents[cur].name) == 0) {
					tmp = xreadline(pdents[cur].name, messages[MSG_COPY_NAME]);
					if (!tmp || !tmp[0] || is_bad_len_or_dir(tmp)
					    || !strcmp(tmp, pdents[cur].name)) {
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

			if (!(r == 's' || r == 'h')) {
				tmp = abspath(tmp, path, newpath);
				if (!tmp) {
					printwarn(&presel);
					goto nochange;
				}
			}

			/* Check if another file with same name exists */
			if (lstat(tmp, &sb) == 0) {
				if ((sel == SEL_RENAME) || ((r == 'f') && (S_ISREG(sb.st_mode)))) {
					/* Overwrite file with same name? */
					if (!xconfirm(get_input(messages[MSG_OVERWRITE])))
						break;
				} else {
					/* Do nothing for SEL_NEW if a non-regular entry exists */
					printwait(messages[MSG_EXISTS], &presel);
					goto nochange;
				}
			}

			if (sel == SEL_RENAME) {
				/* Rename the file */
				if (ret == 'd')
					spawn("cp -rp --", pdents[cur].name, tmp, NULL, F_SILENT);
				else if (rename(pdents[cur].name, tmp) != 0) {
					printwarn(&presel);
					goto nochange;
				}

				/* Check if any entry is created in the current directory */
				tmp = get_cwd_entry(path, tmp, &len);
				if (tmp)
					xstrsncpy(lastname, tmp, len + 1);
				/* Directory must be reloeaded for rename case */
			} else { /* SEL_NEW */
				presel = 0;

				/* Check if it's a dir or file */
				if (r == 'f' || r == 'd') {
					ret = xmktree(tmp, r == 'f' ? FALSE : TRUE);
				} else if (r == 's' || r == 'h') {
					if (nselected > 1 && tmp[0] == '@' && tmp[1] == '\0')
						tmp[0] = '\0';
					ret = xlink(tmp, path, (ndents ? pdents[cur].name : NULL), newpath, r);
				}

				if (!ret)
					printwarn(&presel);

				if (ret <= 0)
					goto nochange;

				if (r == 'f' || r == 'd') {
					tmp = get_cwd_entry(path, tmp, &len);
					if (tmp)
						xstrsncpy(lastname, tmp, len + 1);
					else
						continue; /* No change in directory */
				} else if (ndents) {
					if (cfg.filtermode)
						presel = FILTER;
					copycurname();
				}
			}
			clearfilter();

			cd = FALSE;
			goto begin;
		}
		case SEL_PLUGIN:
			/* Check if directory is accessible */
			if (!xdiraccess(plgpath)) {
				printwarn(&presel);
				goto nochange;
			}

			if (!pkey) {
				r = xstrsncpy(g_buf, messages[MSG_KEYS], CMD_LEN_MAX);
				printkeys(plug, g_buf + r - 1, maxplug);
				printmsg(g_buf);
				r = get_input(NULL);
			} else {
				r = pkey;
				pkey = '\0';
			}

			if (r != '\r') {
				endselection(FALSE);
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

				if (!run_plugin(&path, tmp, (ndents ? pdents[cur].name : NULL),
							 &lastname, &lastdir)) {
					printwait(messages[MSG_FAILED], &presel);
					goto nochange;
				}

				if (g_state.picked)
					return EXIT_SUCCESS;

				copycurname();

				if (!r) {
					cfg.filtermode ? presel = FILTER : statusbar(path);
					goto nochange;
				}
			} else { /* 'Return/Enter' enters the plugin directory */
				g_state.runplugin ^= 1;
				if (!g_state.runplugin) {
					/*
					 * If toggled, and still in the plugin dir,
					 * switch to original directory
					 */
					if (strcmp(path, plgpath) == 0) {
						xstrsncpy(path, lastdir, PATH_MAX);
						xstrsncpy(lastname, runfile, NAME_MAX + 1);
						runfile[0] = '\0';
						setdirwatch();
						goto begin;
					}

					/* Otherwise, initiate choosing plugin again */
					g_state.runplugin = 1;
				}

				xstrsncpy(lastdir, path, PATH_MAX);
				xstrsncpy(path, plgpath, PATH_MAX);
				if (ndents)
					xstrsncpy(runfile, pdents[cur].name, NAME_MAX);
				g_state.runctx = cfg.curctx;
				lastname[0] = '\0';
				clearfilter();
			}
			setdirwatch();
			if (g_state.runplugin == 1) /* Allow filtering in plugins directory */
				presel = FILTER;
			goto begin;
		case SEL_SELSIZE:
			showselsize(path);
			goto nochange;
		case SEL_SHELL: // fallthrough
		case SEL_LAUNCH: // fallthrough
		case SEL_PROMPT:
			r = handle_cmd(sel, path, newpath);

			/* Continue in type-to-nav mode, if enabled */
			if (cfg.filtermode)
				presel = FILTER;

			/* Save current */
			copycurname();

			if (!r)
				goto nochange;

			/* Repopulate as directory content may have changed */
			cd = FALSE;
			goto begin;
		case SEL_UMOUNT:
			presel = MSG_ZERO;
			if (!unmount((ndents ? pdents[cur].name : NULL), newpath, &presel, path)) {
				if (presel == MSG_ZERO)
					statusbar(path);
				goto nochange;
			}

			/* Dir removed, go to next entry */
			copynextname(lastname);
			cd = FALSE;
			goto begin;
#ifndef NOSSN
		case SEL_SESSIONS:
			r = get_input(messages[MSG_SSN_OPTS]);

			if (r == 's') {
				tmp = xreadline(NULL, messages[MSG_SSN_NAME]);
				if (tmp && *tmp)
					save_session(tmp, &presel);
			} else if (r == 'l' || r == 'r') {
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
			cd = FALSE;
			goto begin;
		case SEL_QUITCTX: // fallthrough
		case SEL_QUITCD: // fallthrough
		case SEL_QUIT:
		case SEL_QUITERR:
			if (sel == SEL_QUITCTX) {
				int ctx = cfg.curctx;

				for (r = (ctx - 1) & (CTX_MAX - 1);
				     (r != ctx) && !g_ctx[r].c_cfg.ctxactive;
				     r = ((r - 1) & (CTX_MAX - 1))) {
				};

				if (r != ctx) {
					g_ctx[ctx].c_cfg.ctxactive = 0;

					/* Switch to next active context */
					path = g_ctx[r].c_path;
					lastdir = g_ctx[r].c_last;
					lastname = g_ctx[r].c_name;

					g_ctx[r].c_cfg.curctx = r;
					setcfg(g_ctx[r].c_cfg);

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

			/* CD on Quit */
			tmp = getenv("NNN_TMPFILE");
			if ((sel == SEL_QUITCD) || tmp) {
				write_lastdir(path, tmp);
				/* ^G is a way to quit picker mode without picking anything */
				if ((sel == SEL_QUITCD) && g_state.picker)
					selbufpos = 0;
			}

			if (sel != SEL_QUITERR)
				return EXIT_SUCCESS;

			if (selbufpos && !g_state.picker) {
				/* Pick files to stdout and exit */
				g_state.picker = 1;
				free(selpath);
				selpath = NULL;
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

		if (access(tmpdir, F_OK)) /* Create directory if it doesn't exist */
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
	size_t i, chunk_count = 1, chunk = 512UL * 1024 /* 512 KiB chunk size */, entries = 0;
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

	while (chunk_count < LIST_INPUT_MAX / chunk && !msgnum) {
		input_read = read(fd, input + total_read, chunk);
		if (input_read < 0) {
			if (errno == EINTR)
				continue;

			DPRINTF_S(strerror(errno));
			goto malloc_1;
		}

		if (input_read == 0)
			break;

		total_read += input_read;

		while (off < total_read) {
			next = memchr(input + off, '\0', total_read - off);
			if (!next)
				break;
			++next;

			if (next - input == off + 1) {
				off = next - input;
				continue;
			}

			if (entries == LIST_FILES_MAX) {
				msgnum = MSG_FILE_LIMIT;
				break;
			}

			offsets[entries++] = off;
			off = next - input;
		}

		/* We don't need to allocate another chunk */
		if (chunk_count > (total_read + chunk - 1) / chunk)
			continue;

		/* We can never need more than one additional chunk */
		++chunk_count;
		if (chunk_count > LIST_INPUT_MAX / chunk) {
			msgnum = MSG_SIZE_LIMIT;
			break;
		}

		input = xrealloc(input, chunk_count * chunk);
		if (!input)
			goto malloc_1;
	}

	/* We close fd outside this function. Any extra data is left to the kernel to handle */

	if (off != total_read) {
		if (entries == LIST_FILES_MAX)
			msgnum = MSG_FILE_LIMIT;
		else
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

		paths[i] = abspath(paths[i], cwd, NULL);
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
	for (i = 0; i < entries; ++i)
		free(paths[i]);
malloc_1:
	if (msgnum) { /* Check if we are past init stage and show msg */
		if (home) {
			printmsg(messages[msgnum]);
			xdelay(XDELAY_INTERVAL_MS << 2);
		} else {
			msg(messages[msgnum]);
			usleep(XDELAY_INTERVAL_MS << 2);
		}
	}

	free(input);
	free(paths);
	return tmpdir;
}

static void check_key_collision(void)
{
	wint_t key;
	bool bitmap[KEY_MAX] = {FALSE};

	for (ullong_t i = 0; i < ELEMENTS(bindings); ++i) {
		key = bindings[i].sym;

		if (bitmap[key])
			fprintf(stderr, "key collision! [%s]\n", keyname(key));
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
		" -A      no dir auto-enter during filter\n"
		" -b key  open bookmark key (trumps -s/S)\n"
		" -B      use bsdtar for archives\n"
		" -c      cli-only NNN_OPENER (trumps -e)\n"
		" -C      8-color scheme\n"
		" -d      detail mode\n"
		" -D      dirs in context color\n"
		" -e      text in $VISUAL/$EDITOR/vi\n"
		" -E      internal edits in EDITOR\n"
#ifndef NORL
		" -f      use readline history file\n"
#endif
#ifndef NOFIFO
		" -F val  fifo mode [0:preview 1:explore]\n"
#endif
		" -g      regex filters\n"
		" -H      show hidden files\n"
		" -i      show current file info\n"
		" -J      no auto-advance on selection\n"
		" -K      detect key collision and exit\n"
		" -l val  set scroll lines\n"
		" -n      type-to-nav mode\n"
#ifndef NORL
		" -N      use native prompt\n"
#endif
		" -o      open files only on Enter\n"
		" -p file selection file [-:stdout]\n"
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
#ifndef NOX11
		" -x      notis, selection sync, xterm title\n"
#endif
		" -0      null separator in picker mode\n"
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
		if (tilde_is_home(xdgcfg)) {
			r = xstrsncpy(g_buf, home, PATH_MAX);
			xstrsncpy(g_buf + r - 1, xdgcfg + 1, PATH_MAX);
			xdgcfg = g_buf;
			DPRINTF_S(xdgcfg);
		}

		if (!xdiraccess(xdgcfg)) {
			xerror();
			return FALSE;
		}

		len = xstrlen(xdgcfg) + xstrlen("/nnn/bookmarks") + 1;
		xdg = TRUE;
	}

	if (!xdg)
		len = xstrlen(home) + xstrlen("/.config/nnn/bookmarks") + 1;

	cfgpath = (char *)malloc(len);
	plgpath = (char *)malloc(len);
	if (!cfgpath || !plgpath) {
		xerror();
		return FALSE;
	}

	if (xdg) {
		xstrsncpy(cfgpath, xdgcfg, len);
		r = len - xstrlen("/nnn/bookmarks");
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

	/* Create bookmarks, sessions, mounts and plugins directories */
	for (r = 0; r < ELEMENTS(toks); ++r) {
		mkpath(cfgpath, toks[r], plgpath);
		/* The dirs are created on first run, check if they already exist */
		if (access(plgpath, F_OK) && !xmktree(plgpath, TRUE)) {
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
		msg("set TMPDIR");
		return FALSE;
	}

	tmpfplen = (uchar_t)xstrsncpy(g_tmpfpath, path, TMP_LEN_MAX);
	DPRINTF_S(g_tmpfpath);
	DPRINTF_U(tmpfplen);

	return TRUE;
}

static void cleanup(void)
{
#ifndef NOX11
	if (cfg.x11 && !g_state.picker) {
		printf("\033[23;0t"); /* reset terminal window title */
		fflush(stdout);
	}
#endif
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
	free(lastcmd);
#ifndef NOFIFO
	if (g_state.autofifo)
		unlink(fifopath);
#endif
	if (g_state.pluginit)
		unlink(g_pipepath);
#ifdef DEBUG
	disabledbg();
#endif
}

int main(int argc, char *argv[])
{
	char *arg = NULL;
	char *session = NULL;
	int fd, opt, sort = 0, pkey = '\0'; /* Plugin key */
	bool sepnul = FALSE;
#ifndef NOMOUSE
	mmask_t mask;
	char *middle_click_env = xgetenv(env_cfg[NNN_MCLICK], "\0");

	middle_click_key = (middle_click_env[0] == '^' && middle_click_env[1])
			    ? CONTROL(middle_click_env[1])
			    : (uchar_t)middle_click_env[0];
#endif

	const char * const env_opts = xgetenv(env_cfg[NNN_OPTS], NULL);
	int env_opts_id = env_opts ? (int)xstrlen(env_opts) : -1;
#ifndef NORL
	bool rlhist = FALSE;
#endif

	while ((opt = (env_opts_id > 0
		       ? env_opts[--env_opts_id]
		       : getopt(argc, argv, "aAb:BcCdDeEfF:gHiJKl:nNop:P:QrRs:St:T:uUVx0h"))) != -1) {
		switch (opt) {
#ifndef NOFIFO
		case 'a':
			g_state.autofifo = 1;
			break;
#endif
		case 'A':
			cfg.autoenter = 0;
			break;
		case 'b':
			if (env_opts_id < 0)
				arg = optarg;
			break;
		case 'B':
			g_state.usebsdtar = 1;
			break;
		case 'c':
			cfg.cliopener = 1;
			break;
		case 'C':
			g_state.oldcolor = 1;
			break;
		case 'd':
			cfg.showdetail = 1;
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
#ifndef NOFIFO
		case 'F':
			if (env_opts_id < 0) {
				fd = atoi(optarg);
				if ((fd < 0) || (fd > 1))
					return EXIT_FAILURE;
				g_state.fifomode = fd;
			}
			break;
#endif
		case 'g':
			cfg.regex = 1;
			filterfn = &visible_re;
			break;
		case 'H':
			cfg.showhidden = 1;
			break;
		case 'i':
			cfg.fileinfo = 1;
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
#ifndef NORL
		case 'N':
			g_state.xprompt = 1;
			break;
#endif
		case 'o':
			cfg.nonavopen = 1;
			break;
		case 'p':
			if (env_opts_id >= 0)
				break;

			g_state.picker = 1;
			if (!(optarg[0] == '-' && optarg[1] == '\0')) {
				fd = open(optarg, O_WRONLY | O_CREAT, 0600);
				if (fd == -1) {
					xerror();
					return EXIT_FAILURE;
				}

				close(fd);
				selpath = abspath(optarg, NULL, NULL);
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
			memcpy(cp, PROGRESS_CP, sizeof PROGRESS_CP);
			memcpy(mv, PROGRESS_MV, sizeof PROGRESS_MV);
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
			g_state.prstssn = 1;
			if (!session) /* Support named persistent sessions */
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
		case 'x':
			cfg.x11 = 1;
			break;
		case '0':
			sepnul = TRUE;
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

#ifdef DEBUG
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
		if (!isatty(STDOUT_FILENO)) {
			fd = open(ctermid(NULL), O_RDONLY, 0400);
			dup2(fd, STDIN_FILENO);
			close(fd);
		} else
			dup2(STDOUT_FILENO, STDIN_FILENO);

		if (session)
			session = NULL;
	}

	home = getenv("HOME");
	if (!home) {
		msg("set HOME");
		return EXIT_FAILURE;
	}
	DPRINTF_S(home);
	homelen = (uchar_t)xstrlen(home);

	if (!setup_config())
		return EXIT_FAILURE;

	/* Get custom opener, if set */
	opener = xgetenv(env_cfg[NNN_OPENER], utils[UTIL_OPENER]);
	DPRINTF_S(opener);

	/* Parse bookmarks string */
	if (!parsekvpair(&bookmark, &bmstr, NNN_BMS, &maxbm)) {
		msg(env_cfg[NNN_BMS]);
		return EXIT_FAILURE;
	}

	/* Parse plugins string */
	if (!parsekvpair(&plug, &pluginstr, NNN_PLUG, &maxplug)) {
		msg(env_cfg[NNN_PLUG]);
		return EXIT_FAILURE;
	}

	/* Parse order string */
	if (!parsekvpair(&order, &orderstr, NNN_ORDER, &maxorder)) {
		msg(env_cfg[NNN_ORDER]);
		return EXIT_FAILURE;
	}

	if (!initpath) {
		if (arg) { /* Open a bookmark directly */
			if (!arg[1]) /* Bookmarks keys are single char */
				initpath = get_kv_val(bookmark, NULL, *arg, maxbm, NNN_BMS);

			if (!initpath) {
				msg(messages[MSG_INVALID_KEY]);
				return EXIT_FAILURE;
			}

			if (session)
				session = NULL;
		} else if (argc == optind) {
			/* Start in the current directory */
			char *startpath = getenv("PWD");

			initpath = (startpath && *startpath) ? xstrdup(startpath) : getcwd(NULL, 0);
			if (!initpath)
				initpath = "/";
		} else { /* Open a file */
			arg = argv[optind];
			DPRINTF_S(arg);
			size_t len = xstrlen(arg);

			if (len > 7 && is_prefix(arg, "file://", 7)) {
				arg = arg + 7;
				len -= 7;
			}
			initpath = abspath(arg, NULL, NULL);
			DPRINTF_S(initpath);
			if (!initpath) {
				xerror();
				return EXIT_FAILURE;
			}

			/* If the file is hidden, enable hidden option */
			if (*xbasename(initpath) == '.')
				cfg.showhidden = 1;

			/*
			 * If nnn is set as the file manager, applications may try to open
			 * files by invoking nnn. In that case pass the file path to the
			 * desktop opener and exit.
			 */
			struct stat sb;

			if (stat(initpath, &sb) == -1) {
				bool dir = (arg[len - 1] == '/');

				if (!dir) {
					arg = xbasename(initpath);
					initpath = xdirname(initpath);

					pkey = CREATE_NEW_KEY; /* Override plugin key */
					g_state.initfile = 1;
				}
				if (dir || (arg != initpath)) { /* We have a directory */
					if (!xdiraccess(initpath) && !xmktree(initpath, TRUE)) {
						xerror(); /* Fail if directory cannot be created */
						return EXIT_FAILURE;
					}
					if (!dir) /* Restore the complete path */
						*--arg = '/';
				}
			} else if (!S_ISDIR(sb.st_mode))
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
		msg(messages[MSG_INVALID_REG]);
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
	inotify_fd = inotify_init1(IN_NONBLOCK | IN_CLOEXEC);
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

#ifndef NOLC
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

#ifndef NOX11
	if (cfg.x11 && !g_state.picker) {
		/* Save terminal window title */
		printf("\033[22;0t");
		fflush(stdout);
		gethostname(hostname, sizeof(hostname));
		hostname[sizeof(hostname) - 1] = '\0';
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

#ifndef NOSSN
	if (session && g_state.prstssn)
		save_session(session, NULL);
#endif

#ifndef NOMOUSE
	mousemask(mask, NULL);
#endif

	exitcurses();

#ifndef NORL
	if (rlhist && !g_state.xprompt) {
		mkpath(cfgpath, ".history", g_buf);
		write_history(g_buf);
	}
#endif

	if (g_state.picker) {
		if (selbufpos) {
			fd = selpath ? open(selpath, O_WRONLY | O_CREAT | O_TRUNC, 0600) : STDOUT_FILENO;
			if ((fd == -1) || (seltofile(fd, NULL, sepnul ? "\0" : NEWLINE) != (size_t)(selbufpos)))
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
	if (!g_state.fifomode)
		notify_fifo(FALSE);
	if (fifofd != -1)
		close(fifofd);
#endif

	return opt;
}
