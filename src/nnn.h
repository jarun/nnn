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

#pragma once

#include <curses.h>

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

#define CONTROL(c) ((c) & 0x1f)
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

#define MIN_DISPLAY_COLS ((CTX_MAX * 2) + 2) /* Two chars for [ and ] */
#define LONG_SIZE sizeof(ulong)
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
typedef unsigned long ulong;
typedef unsigned int uint;
typedef unsigned char uchar;
typedef unsigned short ushort;
typedef long long ll;
typedef unsigned long long ull;

/* STRUCTURES */

/* Supported actions */
enum action {
	SEL_BACK = 1,
	SEL_GOIN,
	SEL_NAV_IN,
	SEL_NEXT,
	SEL_PREV,
	SEL_PGDN,
	SEL_PGUP,
	SEL_CTRL_D,
	SEL_CTRL_U,
	SEL_HOME,
	SEL_END,
	SEL_FIRST,
	SEL_CDHOME,
	SEL_CDBEGIN,
	SEL_CDLAST,
	SEL_CDROOT,
	SEL_BOOKMARK,
	SEL_REMOTE,
	SEL_CYCLE,
	SEL_CYCLER,
	SEL_CTX1,
	SEL_CTX2,
	SEL_CTX3,
	SEL_CTX4,
#ifdef CTX8
	SEL_CTX5,
	SEL_CTX6,
	SEL_CTX7,
	SEL_CTX8,
#endif
	SEL_MARK,
	SEL_FLTR,
	SEL_MFLTR,
	SEL_HIDDEN,
	SEL_DETAIL,
	SEL_STATS,
	SEL_CHMODX,
	SEL_ARCHIVE,
	SEL_SORT,
	SEL_REDRAW,
	SEL_SEL,
	SEL_SELMUL,
	SEL_SELALL,
	SEL_SELEDIT,
	SEL_CP,
	SEL_MV,
	SEL_CPMVAS,
	SEL_RM,
	SEL_OPENWITH,
	SEL_NEW,
	SEL_RENAME,
	SEL_RENAMEMUL,
	SEL_UMOUNT,
	SEL_HELP,
	SEL_AUTONEXT,
	SEL_EDIT,
	SEL_PLUGIN,
	SEL_SHELL,
	SEL_LAUNCH,
	SEL_RUNCMD,
	SEL_LOCK,
	SEL_SESSIONS,
	SEL_EXPORT,
	SEL_TIMETYPE,
	SEL_QUITCTX,
	SEL_QUITCD,
	SEL_QUIT,
	SEL_QUITFAIL,
#ifndef NOFIFO
	SEL_FIFO,
#endif
#ifndef NOMOUSE
	SEL_CLICK,
#endif
};

/* Associate a pressed key to an action */
struct key {
	int sym;         /* Key pressed */
	enum action act; /* Action */
};

static struct key bindings[] = {
	/* Back */
	{ KEY_LEFT,       SEL_BACK },
	{ 'h',            SEL_BACK },
	/* Inside or select */
	{ KEY_ENTER,      SEL_GOIN },
	{ '\r',           SEL_GOIN },
	/* Pure navigate inside */
	{ KEY_RIGHT,      SEL_NAV_IN },
	{ 'l',            SEL_NAV_IN },
	/* Next */
	{ 'j',            SEL_NEXT },
	{ KEY_DOWN,       SEL_NEXT },
	/* Previous */
	{ 'k',            SEL_PREV },
	{ KEY_UP,         SEL_PREV },
	/* Page down */
	{ KEY_NPAGE,      SEL_PGDN },
	/* Page up */
	{ KEY_PPAGE,      SEL_PGUP },
	/* Ctrl+D */
	{ CONTROL('D'),   SEL_CTRL_D },
	/* Ctrl+U */
	{ CONTROL('U'),   SEL_CTRL_U },
	/* First entry */
	{ KEY_HOME,       SEL_HOME },
	{ 'g',            SEL_HOME },
	{ CONTROL('A'),   SEL_HOME },
	/* Last entry */
	{ KEY_END,        SEL_END },
	{ 'G',            SEL_END },
	{ CONTROL('E'),   SEL_END },
	/* Go to first file */
	{ '\'',           SEL_FIRST },
	/* HOME */
	{ '~',            SEL_CDHOME },
	/* Initial directory */
	{ '@',            SEL_CDBEGIN },
	/* Last visited dir */
	{ '-',            SEL_CDLAST },
	/* Go to / */
	{ '`',            SEL_CDROOT },
	/* Leader key */
	{ 'b',            SEL_BOOKMARK },
	{ CONTROL('_'),   SEL_BOOKMARK },
	/* Connect to server over SSHFS */
	{ 'c',            SEL_REMOTE },
	/* Cycle contexts in forward direction */
	{ '\t',           SEL_CYCLE },
	/* Cycle contexts in reverse direction */
	{ KEY_BTAB,       SEL_CYCLER },
	/* Go to/create context N */
	{ '1',            SEL_CTX1 },
	{ '2',            SEL_CTX2 },
	{ '3',            SEL_CTX3 },
	{ '4',            SEL_CTX4 },
#ifdef CTX8
	{ '5',            SEL_CTX5 },
	{ '6',            SEL_CTX6 },
	{ '7',            SEL_CTX7 },
	{ '8',            SEL_CTX8 },
#endif
	/* Mark a path to visit later */
	{ ',',            SEL_MARK },
	/* Filter */
	{ '/',            SEL_FLTR },
	/* Toggle filter mode */
	{ CONTROL('N'),   SEL_MFLTR },
	/* Toggle hide .dot files */
	{ '.',            SEL_HIDDEN },
	/* Detailed listing */
	{ 'd',            SEL_DETAIL },
	/* File details */
	{ 'f',            SEL_STATS },
	{ CONTROL('F'),   SEL_STATS },
	/* Toggle executable status */
	{ '*',            SEL_CHMODX },
	/* Create archive */
	{ 'z',            SEL_ARCHIVE },
	/* Sort toggles */
	{ 't',            SEL_SORT },
	{ CONTROL('T'),   SEL_SORT },
	/* Redraw window */
	{ CONTROL('L'),   SEL_REDRAW },
	/* Select current file path */
	{ CONTROL('J'),   SEL_SEL },
	{ ' ',            SEL_SEL },
	/* Toggle select multiple files */
	{ 'm',            SEL_SELMUL },
	{ CONTROL('K'),   SEL_SELMUL },
	/* Select all files in current dir */
	{ 'a',            SEL_SELALL },
	/* List, edit selection */
	{ 'E',            SEL_SELEDIT },
	/* Copy from selection buffer */
	{ 'p',            SEL_CP },
	{ CONTROL('P'),   SEL_CP },
	/* Move from selection buffer */
	{ 'v',            SEL_MV },
	{ CONTROL('V'),   SEL_MV },
	/* Copy/move from selection buffer and rename */
	{ 'w',            SEL_CPMVAS },
	{ CONTROL('W'),   SEL_CPMVAS },
	/* Delete from selection buffer */
	{ 'x',            SEL_RM },
	{ CONTROL('X'),   SEL_RM },
	/* Open in a custom application */
	{ 'o',            SEL_OPENWITH },
	{ CONTROL('O'),   SEL_OPENWITH },
	/* Create a new file */
	{ 'n',            SEL_NEW },
	/* Show rename prompt */
	{ CONTROL('R'),   SEL_RENAME },
	/* Rename contents of current dir */
	{ 'r',            SEL_RENAMEMUL },
	/* Disconnect a SSHFS mount point */
	{ 'u',            SEL_UMOUNT },
	/* Show help */
	{ '?',            SEL_HELP },
	/* Quit a context */
	{ '+',            SEL_AUTONEXT },
	/* Edit in EDITOR */
	{ 'e',            SEL_EDIT },
	/* Run a plugin */
	{ ';',            SEL_PLUGIN },
	/* Run command */
	{ '!',            SEL_SHELL },
	{ CONTROL(']'),   SEL_SHELL },
	/* Launcher */
	{ '=',            SEL_LAUNCH },
	/* Run a command */
	{ ']',            SEL_RUNCMD },
	/* Lock screen */
	{ '0',            SEL_LOCK },
	/* Manage sessions */
	{ 's',            SEL_SESSIONS },
	/* Export list */
	{ '>',            SEL_EXPORT },
	/* Set time type */
	{ 'T',            SEL_TIMETYPE },
	/* Quit a context */
	{ 'q',            SEL_QUITCTX },
	/* Change dir on quit */
	{ CONTROL('G'),   SEL_QUITCD },
	/* Quit */
	{ CONTROL('Q'),   SEL_QUIT },
	/* Quit with an error code */
	{ 'Q',            SEL_QUITFAIL },
#ifndef NOFIFO
	/* Send hovered path to NNN_FIFO */
	{ 27,            SEL_FIFO },
#endif
#ifndef NOMOUSE
	{ KEY_MOUSE,      SEL_CLICK },
#endif
};

/* Directory entry */
typedef struct entry {
	char *name;
	time_t t;
	off_t size;
	blkcnt_t blocks; /* number of 512B blocks allocated */
	mode_t mode;
	ushort nlen; /* Length of file name */
	uchar flags; /* Flags specific to the file */
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
	uint filtermode : 1;  /* Set to enter filter mode */
	uint timeorder  : 1;  /* Set to sort by time */
	uint sizeorder  : 1;  /* Set to sort by file size */
	uint apparentsz : 1;  /* Set to sort by apparent size (disk usage) */
	uint blkorder   : 1;  /* Set to sort by blocks used (disk usage) */
	uint extnorder  : 1;  /* Order by extension */
	uint showhidden : 1;  /* Set to show hidden files */
	uint reserved0  : 1;
	uint showdetail : 1;  /* Clear to show lesser file info */
	uint ctxactive  : 1;  /* Context active or not */
	uint reverse    : 1;  /* Reverse sort */
	uint version    : 1;  /* Version sort */
	uint reserved1  : 1;
	/* The following settings are global */
	uint curctx     : 3;  /* Current context number */
	uint prefersel  : 1;  /* Prefer selection over current, if exists */
	uint reserved2  : 1;
	uint nonavopen  : 1;  /* Open file on right arrow or `l` */
	uint autoselect : 1;  /* Auto-select dir in type-to-nav mode */
	uint cursormode : 1;  /* Move hardware cursor with selection */
	uint useeditor  : 1;  /* Use VISUAL to open text files */
	uint reserved3  : 3;
	uint regex      : 1;  /* Use regex filters */
	uint x11        : 1;  /* Copy to system clipboard and show notis */
	uint timetype   : 2;  /* Time sort type (0: access, 1: change, 2: modification) */
	uint cliopener  : 1;  /* All-CLI app opener */
	uint waitedit   : 1;  /* For ops that can't be detached, used EDITOR */
	uint rollover   : 1;  /* Roll over at edges */
} settings;

/* Non-persistent program-internal states */
typedef struct {
	uint pluginit   : 1;  /* Plugin framework initialized */
	uint interrupt  : 1;  /* Program received an interrupt */
	uint rangesel   : 1;  /* Range selection on */
	uint move       : 1;  /* Move operation */
	uint autonext   : 1;  /* Auto-proceed on open */
	uint fortune    : 1;  /* Show fortune messages in help */
	uint trash      : 1;  /* Use trash to delete files */
	uint forcequit  : 1;  /* Do not prompt on quit */
	uint autofifo   : 1;  /* Auto-create NNN_FIFO */
	uint initfile   : 1;  /* Positional arg is a file */
	uint dircolor   : 1;  /* Current status of dir color */
	uint picker     : 1;  /* Write selection to user-specified file */
	uint pickraw    : 1;  /* Write selection to sdtout before exit */
	uint runplugin  : 1;  /* Choose plugin mode */
	uint runctx     : 2;  /* The context in which plugin is to be run */
	uint selmode    : 1;  /* Set when selecting files */
	uint oldcolor   : 1;  /* Show dirs in context colors */
	uint reserved   : 14;
} runstate;

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
static uint idletimeout, selbufpos, lastappendpos, selbuflen;
static ushort xlines, xcols;
static ushort idle;
static uchar maxbm, maxplug;
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
static ull *ihashbmp;
static struct entry *pdents;
static blkcnt_t ent_blocks;
static blkcnt_t dir_blocks;
static ulong num_files;
static kv *bookmark;
static kv *plug;
static uchar tmpfplen;
static uchar blk_shift = BLK_SHIFT_512;
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
#define UTIL_ARCHIVEMOUNT 9
#define UTIL_SSHFS 10
#define UTIL_RCLONE 11
#define UTIL_VI 12
#define UTIL_LESS 13
#define UTIL_SH 14
#define UTIL_FZF 15
#define UTIL_NTFY 16
#define UTIL_CBCP 17
#define UTIL_NMV 18

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
	"archivemount",
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
#define MSG_LIMIT 10
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
#define MSG_FIRST 39
#define MSG_RM_TMP 40
#define MSG_NOCHNAGE 41
#define MSG_CANCEL 42
#define MSG_0_ENTRIES 43
#ifndef DIR_LIMITED_SELECTION
#define MSG_DIR_CHANGED 44 /* Must be the last entry */
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
	"rm -rf %s file%s? [Esc cancels]",
	"limit exceeded",
	"'f'ile / 'd'ir / 's'ym / 'h'ard?",
	"'c'li / 'g'ui?",
	"overwrite?",
	"'s'ave / 'l'oad / 'r'estore?",
	"Quit all contexts?",
	"remote name ('-' for hovered): ",
	"archive name: ",
	"open with: ",
	"relative path: ",
	"link prefix [@ for none]: ",
	"copy name: ",
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
	"'a'u / 'd'u / 'e'xtn / 'r'ev / 's'ize / 't'ime / 'v'er / 'c'lear?",
	"unmount failed! try lazy?",
	"first file (\')/char?",
	"remove tmp file?",
	"unchanged",
	"cancelled",
	"0 entries",
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
	"sed -i 's|^\\(\\(.*/\\)\\(.*\\)$\\)|#\\1\\n\\3|' %s",
	"sed 's|^\\([^#/][^/]\\?.*\\)$|%s/\\1|;s|^#\\(/.*\\)$|\\1|' "
		"%s | tr '\\n' '\\0' | xargs -0 -n2 sh -c '%s \"$0\" \"$@\" < /dev/tty'",
	"\\.(bz|bz2|gz|tar|taz|tbz|tbz2|tgz|z|zip)$",
	"sed -i 's|^%s\\(.*\\)$|%s\\1|' %s",
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

static char gcolors[] = "c1e2272e006033f7c6d6abc4";
static uint fcolors[C_UND + 1] = {0};

/* Event handling */
#ifdef LINUX_INOTIFY
#define NUM_EVENT_SLOTS 32 /* Make room for 32 events */
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
#define tocursor() move(cur + 2, 0)
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

#ifdef __GNUC__
#define UNUSED(x) UNUSED_##x __attribute__((__unused__))
#else
#define UNUSED(x) UNUSED_##x
#endif /* __GNUC__ */

/* HELPER FUNCTIONS */

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

/* Return the integer value of a char representing HEX */
static uchar xchartohex(uchar c)
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
static bool test_set_bit(uint nr)
{
	nr &= HASH_BITS;

	ull *m = ((ull *)ihashbmp) + (nr >> 6);

	if (*m & (1 << (nr & 63)))
		return FALSE;

	*m |= 1 << (nr & 63);

	return TRUE;
}

#if 0
static bool test_clear_bit(uint nr)
{
	nr &= HASH_BITS;

	ull *m = ((ull *) ihashbmp) + (nr >> 6);

	if (!(*m & (1 << (nr & 63))))
		return FALSE;

	*m &= ~(1 << (nr & 63));
	return TRUE;
}
#endif

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
static void *xmemrchr(uchar *restrict s, uchar ch, size_t n)
{
#if defined(__GLIBC__) || defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__)
	return memrchr(s, ch, n);
#else

	if (!s || !n)
		return NULL;

	uchar *ptr = s + n;

	do
		if (*--ptr == ch)
			return ptr;
	while (s != ptr);

	return NULL;
#endif
}

/* A very simplified implementation, changes path */
static char *xdirname(char *path)
{
	char *base = xmemrchr((uchar *)path, '/', xstrlen(path));

	if (base == path)
		path[1] = '\0';
	else
		*base = '\0';

	return path;
}

static char *xbasename(char *path)
{
	char *base = xmemrchr((uchar *)path, '/', xstrlen(path)); // NOLINT

	return base ? base + 1 : path;
}

static char *xextension(const char *fname, size_t len)
{
	return xmemrchr((uchar *)fname, '.', len);
}

/*
 * Updates out with "dir/name or "/name"
 * Returns the number of bytes copied including the terminating NULL byte
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
				dst = xmemrchr((uchar *)resolved_path, '/', dst - resolved_path);
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

/* PRINT I/O FUNCTIONS */

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

	if (r == 27)
		return '\0'; /* cancel */
	if (r == 'y' || r == 'Y')
		return 'f'; /* forceful */
	return 'i'; /* interactive */
}

/* FORK FUNCTIONS */

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
	sigaction(SIGHUP, &oldsighup, NULL);
	sigaction(SIGTSTP, &oldsigtstp, NULL);

	return status;
}

/*
 * Spawns a child process. Behaviour can be controlled using flag.
 * Limited to 2 arguments to a program, flag works on bit set.
 */
static int spawn(char *file, char *arg1, char *arg2, uchar flag)
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
			while (getchar() != '\n');
		}

		if (flag & F_NORMAL)
			refresh();

		free(cmd);
	}

	return retstatus;
}

/* SELECTION HANDLER FUNCTIONS */

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
static size_t seltofile(int fd, uint *pcount)
{
	uint lastpos, count = 0;
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
