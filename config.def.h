/* See LICENSE file for copyright and license details. */
#define CWD   "cwd: "
#define CURSR " > "
#define EMPTY "   "

static int filtermode;      /* Set to 1 to enter filter mode */
static int mtimeorder;      /* Set to 1 to sort by time modified */
static int sizeorder;       /* Set to 1 to sort by file size */
static int bsizeorder;      /* Set to 1 to sort by blocks used (disk usage) */
static int idletimeout;     /* Idle timeout in seconds, 0 to disable */
static int showhidden;      /* Set to 1 to show hidden files by default */
static int showdetail  = 1; /* Set to 0 to show fewer file info */

static struct assoc assocs[] = {
	{ "\\.(c|cpp|h|log|md|py|sh|txt)$", "text" },
};

static struct key bindings[] = {
	/* Quit */
	{ 'q',            SEL_QUIT,      "",     "" },
	{ CONTROL('Q'),   SEL_QUIT,      "",     "" },
	/* Change dir on quit */
	{ 'Q',            SEL_CDQUIT,    "",     "" },
	/* Back */
	{ KEY_BACKSPACE,  SEL_BACK,      "",     "" },
	{ KEY_LEFT,       SEL_BACK,      "",     "" },
	{ 'h',            SEL_BACK,      "",     "" },
	{ CONTROL('H'),   SEL_BACK,      "",     "" },
	/* Inside */
	{ KEY_ENTER,      SEL_GOIN,      "",     "" },
	{ '\r',           SEL_GOIN,      "",     "" },
	{ KEY_RIGHT,      SEL_GOIN,      "",     "" },
	{ 'l',            SEL_GOIN,      "",     "" },
	/* Filter */
	{ '/',            SEL_FLTR,      "",     "" },
	/* Toggle filter mode */
	{ KEY_IC,         SEL_MFLTR,     "",     "" },
	/* Desktop search */
	{ CONTROL('_'),   SEL_SEARCH,    "",     "" },
	/* Next */
	{ 'j',            SEL_NEXT,      "",     "" },
	{ KEY_DOWN,       SEL_NEXT,      "",     "" },
	{ CONTROL('N'),   SEL_NEXT,      "",     "" },
	/* Previous */
	{ 'k',            SEL_PREV,      "",     "" },
	{ KEY_UP,         SEL_PREV,      "",     "" },
	{ CONTROL('P'),   SEL_PREV,      "",     "" },
	/* Page down */
	{ KEY_NPAGE,      SEL_PGDN,      "",     "" },
	{ CONTROL('D'),   SEL_PGDN,      "",     "" },
	/* Page up */
	{ KEY_PPAGE,      SEL_PGUP,      "",     "" },
	{ CONTROL('U'),   SEL_PGUP,      "",     "" },
	/* First entry */
	{ KEY_HOME,       SEL_HOME,      "",     "" },
	{ 'g',            SEL_HOME,      "",     "" },
	{ CONTROL('A'),   SEL_HOME,      "",     "" },
	{ '^',            SEL_HOME,      "",     "" },
	/* Last entry */
	{ KEY_END,        SEL_END,       "",     "" },
	{ 'G',            SEL_END,       "",     "" },
	{ CONTROL('E'),   SEL_END,       "",     "" },
	{ '$',            SEL_END,       "",     "" },
	/* Change dir */
	{ 'c',            SEL_CD,        "",     "" },
	/* HOME */
	{ '~',            SEL_CDHOME,    "",     "" },
	/* Initial directory */
	{ '&',            SEL_CDBEGIN,   "",     "" },
	/* Last visited dir */
	{ '-',            SEL_CDLAST,    "",     "" },
	/* Change dir using bookmark */
	{ 'b',            SEL_CDBM,      "",     "" },
	/* Toggle hide .dot files */
	{ '.',            SEL_TOGGLEDOT, "",     "" },
	/* Detailed listing */
	{ 'd',            SEL_DETAIL,    "",     "" },
	/* File details */
	{ 'D',            SEL_STATS,     "",     "" },
	/* Show mediainfo short */
	{ 'm',            SEL_MEDIA,     "",     "" },
	/* Show mediainfo full */
	{ 'M',            SEL_FMEDIA,    "",     "" },
	/* Open dir in desktop file manager */
	{ 'o',            SEL_DFB,       "",     "" },
	/* Toggle sort by size */
	{ 's',            SEL_FSIZE,     "",     "" },
	/* Sort by total block size including dir contents */
	{ 'S',            SEL_BSIZE,    "",     "" },
	/* Toggle sort by time */
	{ 't',            SEL_MTIME,     "",     "" },
	{ CONTROL('L'),   SEL_REDRAW,    "",     "" },
	/* Copy currently selected file path */
	{ CONTROL('K'),   SEL_COPY,      "",     "" },
	/* Show help */
	{ '?',            SEL_HELP,      "",     "" },
	/* Run command */
	{ '!',            SEL_RUN,       "sh",   "SHELL" },
	/* Run command with argument */
	{ 'e',            SEL_RUNARG,    "vi",   "EDITOR" },
	{ 'p',            SEL_RUNARG,    "less", "PAGER" },
};
