/* See LICENSE file for copyright and license details. */
#define CWD   "cwd: "
#define CURSR " > "
#define EMPTY "   "

static int mtimeorder  = 0; /* Set to 1 to sort by time modified */
static int sizeorder   = 0; /* Set to 1 to sort by file size */
static int idletimeout = 0; /* Screensaver timeout in seconds, 0 to disable */
static int showhidden  = 0; /* Set to 1 to show hidden files by default */
static int showdetail  = 0; /* Set to show additional file info */
static char *idlecmd   = "rain"; /* The screensaver program */

struct assoc assocs[] = {
	//{ "\\.(avi|mp4|mkv|mp3|ogg|flac|mov)$", "mpv" },
	{ "\\.(c|cpp|h|txt|log|sh)$", "vi" },
	{ "\\.(wma|mp3|ogg|flac)$", "mpv" },
	//{ "\\.(png|jpg|gif)$", "feh" },
	//{ "\\.(html|svg)$", "firefox" },
	{ "\\.pdf$", "zathura" },
	//{ "\\.sh$", "sh" },
	//{ ".", "less" },
};

struct key bindings[] = {
	/* Quit */
	{ 'q',            SEL_QUIT,      "\0",  "\0" },
	/* Back */
	{ KEY_BACKSPACE,  SEL_BACK,      "\0",  "\0" },
	{ KEY_LEFT,       SEL_BACK,      "\0",  "\0" },
	{ 'h',            SEL_BACK,      "\0",  "\0" },
	{ CONTROL('H'),   SEL_BACK,      "\0",  "\0" },
	/* Inside */
	{ KEY_ENTER,      SEL_GOIN,      "\0",  "\0" },
	{ '\r',           SEL_GOIN,      "\0",  "\0" },
	{ KEY_RIGHT,      SEL_GOIN,      "\0",  "\0" },
	{ 'l',            SEL_GOIN,      "\0",  "\0" },
	/* Filter */
	{ '/',            SEL_FLTR,      "\0",  "\0" },
	{ '&',            SEL_FLTR,      "\0",  "\0" },
	/* Next */
	{ 'j',            SEL_NEXT,      "\0",  "\0" },
	{ KEY_DOWN,       SEL_NEXT,      "\0",  "\0" },
	{ CONTROL('N'),   SEL_NEXT,      "\0",  "\0" },
	/* Previous */
	{ 'k',            SEL_PREV,      "\0",  "\0" },
	{ KEY_UP,         SEL_PREV,      "\0",  "\0" },
	{ CONTROL('P'),   SEL_PREV,      "\0",  "\0" },
	/* Page down */
	{ KEY_NPAGE,      SEL_PGDN,      "\0",  "\0" },
	{ CONTROL('D'),   SEL_PGDN,      "\0",  "\0" },
	/* Page up */
	{ KEY_PPAGE,      SEL_PGUP,      "\0",  "\0" },
	{ CONTROL('U'),   SEL_PGUP,      "\0",  "\0" },
	/* Home */
	{ KEY_HOME,       SEL_HOME,      "\0",  "\0" },
	{ CONTROL('A'),   SEL_HOME,      "\0",  "\0" },
	{ '^',            SEL_HOME,      "\0",  "\0" },
	/* End */
	{ KEY_END,        SEL_END,       "\0",  "\0" },
	{ CONTROL('E'),   SEL_END,       "\0",  "\0" },
	{ '$',            SEL_END,       "\0",  "\0" },
	/* Change dir */
	{ 'c',            SEL_CD,        "\0",  "\0" },
	{ '~',            SEL_CDHOME,    "\0",  "\0" },
	/* Toggle hide .dot files */
	{ '.',            SEL_TOGGLEDOT, "\0",  "\0" },
	/* Detailed listing */
	{ 'd',            SEL_DETAIL,    "\0",  "\0" },
	/* File details */
	{ 'D',            SEL_STATS,     "\0",  "\0" },
	/* Toggle sort by size */
	{ 's',            SEL_FSIZE,     "\0",  "\0" },
	/* Toggle sort by time */
	{ 't',            SEL_MTIME,     "\0",   "\0" },
	{ CONTROL('L'),   SEL_REDRAW,    "\0",   "\0" },
	/* Copy currently selected file path */
	{ CONTROL('K'),   SEL_COPY,      "\0",   "\0" },
	/* Run command */
	{ 'z',            SEL_RUN,       "top",  "\0" },
	{ '!',            SEL_RUN,       "sh",   "SHELL" },
	/* Run command with argument */
	{ 'e',            SEL_RUNARG,    "vi",   "EDITOR" },
	{ 'p',            SEL_RUNARG,    "less", "PAGER" },
};
