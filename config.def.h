#define CWD "cwd: "
#define CURSR " > "
#define EMPTY "   "

int mtimeorder = 0; /* Set to 1 to sort by time in the default case */

struct assoc assocs[] = {
	{ "\\.(avi|mp4|mkv|mp3|ogg|flac)$", "mplayer" },
	{ "\\.(png|jpg|gif)$", "feh" },
	{ "\\.(html|svg)$", "firefox" },
	{ "\\.pdf$", "mupdf" },
	{ "\\.sh$", "sh" },
	{ ".", "less" },
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
	/* Filter as you type */
	{ '?',            SEL_TYPE },
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
	/* Change dir */
	{ 'c',            SEL_CD },
	/* Toggle sort by time */
	{ 't',            SEL_MTIME },
	{ CONTROL('L'),   SEL_REDRAW },
	/* Run command */
	{ '!',            SEL_RUN,    "sh" },
	{ 'z',            SEL_RUN,    "top" },
	/* Run command with argument */
	{ 'e',            SEL_RUNARG, "vi" },
};
