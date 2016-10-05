/* See LICENSE file for copyright and license details. */
#define CWD   "cwd: "
#define CURSR " > "
#define EMPTY "   "

int mtimeorder  = 0; /* Set to 1 to sort by time modified */
int idletimeout = 0; /* Screensaver timeout in seconds, 0 to disable */
int showhidden  = 0; /* Set to 1 to show hidden files by default */
char *idlecmd   = "rain"; /* The screensaver program */

struct assoc assocs[] = {
	//{ "\\.(avi|mp4|mkv|mp3|ogg|flac|mov)$", "mpv" },
	{ "\\.(c|cpp|h|txt|log)$", "vim" },
	{ "\\.(wma|mp3|ogg|flac)$", "fmedia" },
	//{ "\\.(png|jpg|gif)$", "feh" },
	//{ "\\.(html|svg)$", "firefox" },
	{ "\\.pdf$", "zathura" },
	{ "\\.sh$", "sh" },
	//{ ".", "less" },
};

struct key bindings[] = {
	/* Quit */
	{ 'q',            SEL_QUIT },
	/* Back */
	{ KEY_BACKSPACE,  SEL_BACK },
	{ KEY_LEFT,       SEL_BACK },
	{ 'h',            SEL_BACK },
	{ CONTROL('H'),   SEL_BACK },
	/* Inside */
	{ KEY_ENTER,      SEL_GOIN },
	{ '\r',           SEL_GOIN },
	{ KEY_RIGHT,      SEL_GOIN },
	{ 'l',            SEL_GOIN },
	/* Filter */
	{ '/',            SEL_FLTR },
	{ '&',            SEL_FLTR },
	/* Next */
	{ 'j',            SEL_NEXT },
	{ KEY_DOWN,       SEL_NEXT },
	{ CONTROL('N'),   SEL_NEXT },
	/* Previous */
	{ 'k',            SEL_PREV },
	{ KEY_UP,         SEL_PREV },
	{ CONTROL('P'),   SEL_PREV },
	/* Page down */
	{ KEY_NPAGE,      SEL_PGDN },
	{ CONTROL('D'),   SEL_PGDN },
	/* Page up */
	{ KEY_PPAGE,      SEL_PGUP },
	{ CONTROL('U'),   SEL_PGUP },
	/* Home */
	{ KEY_HOME,       SEL_HOME },
	{ CONTROL('A'),   SEL_HOME },
	{ '^',            SEL_HOME },
	/* End */
	{ KEY_END,        SEL_END },
	{ CONTROL('E'),   SEL_END },
	{ '$',            SEL_END },
	/* Change dir */
	{ 'c',            SEL_CD },
	{ '~',            SEL_CDHOME },
	/* Toggle hide .dot files */
	{ '.',            SEL_TOGGLEDOT },
	/* Toggle sort by time */
	{ 't',            SEL_MTIME },
	{ CONTROL('L'),   SEL_REDRAW },
	/* Run command */
	{ 'z',            SEL_RUN, "top" },
	{ '!',            SEL_RUN, "sh", "SHELL" },
	/* Run command with argument */
	{ 'e',            SEL_RUNARG, "vi", "EDITOR" },
	{ 'p',            SEL_RUNARG, "less", "PAGER" },
};
