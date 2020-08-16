#include "icons-in-terminal.h"

struct icon_pair {
	const char *match;
	const char *icon;
	const unsigned char color;
};

/* 
 * Define a string to be printed before and after the icon
 * Adjust if the icons are not printed properly
 */

#define ICON_PADDING_LEFT  ""
#define ICON_PADDING_RIGHT " "

/*
 * Using symbols defined in icons-in-terminal.h, or even using icons-in-terminal is not necessary.
 * You can use whatever pathched font you like. You just have to put the desired icon as a string.
 * If you are using icons-in-terminal the creator recommends that you do use the symbols in the generated header.
 */

static const struct icon_pair dir_icon  = {"", FA_FOLDER, 0};
static const struct icon_pair file_icon = {"", FA_FILE,   0};

/* All entries are case-insensitive */

static const struct icon_pair icons_name[] = {
	{".git",         FA_GITHUB_SQUARE, 0},
	{"Desktop",      FA_HOME,          0},
	{"Documents",    FA_LIST_ALT,      0},
	{"Downloads",    FA_DOWNLOAD,      0},
	{"Music",        FA_MUSIC,         0},
	{"node_modules", MFIZZ_NPM,        0},
	{"Pictures",     FA_IMAGE,         0},
	{"Public",       FA_INBOX,         0},
	{"Templates",    FA_COG,           0},
	{"Videos",       FA_FILM,          0},
};

/* 
 * New entries should bu added such that the first character of the extension is in the correct group .
 * This is done for performance reason so that the correct icon can be found faster.
 * All entries are case-insensitive
 */

static const struct icon_pair icons_ext[] = {
	/* Numbers */
	{"7z",       FA_FILE_ARCHIVE_O,    0},

	/* A */
	{"a",        FILE_MANPAGE,         0},
	{"apk",      FA_FILE_ARCHIVE_O,    0},
	{"asm",      FILE_NASM,            0},
	{"aup",      FA_FILE_AUDIO_O,      0},
	{"avi",      FA_FILE_MOVIE_O,      0},

	/* B */
	{"bat",      MFIZZ_SCRIPT,         0},
	{"bmp",      FA_FILE_IMAGE_O,      0},
	{"bz2",      FA_FILE_ARCHIVE_O,    0},

	 /* C */
	{"c",        MFIZZ_C,              0},
	{"c++",      MFIZZ_CPLUSPLUS,      0},
	{"cab",      FA_FILE_ARCHIVE_O,    0},
	{"cbr",      FA_FILE_ARCHIVE_O,    0},
	{"cbz",      FA_FILE_ARCHIVE_O,    0},
	{"cc",       MFIZZ_CPLUSPLUS,      0},
	{"class",    MFIZZ_JAVA,           0},
	{"clj",      MFIZZ_CLOJURE,        0},
	{"cljc",     MFIZZ_CLOJURE,        0},
	{"cljs",     MFIZZ_CLOJURE,        0},
	{"cmake",    FILE_CMAKE,           0},
	{"coffee",   MFIZZ_COFFEE_BEAN,    0},
	{"conf",     FA_COGS,              0},
	{"cpio",     FA_FILE_ARCHIVE_O,    0},
	{"cpp",      MFIZZ_CPLUSPLUS,      0},
	{"css",      MFIZZ_CSS3,           0},
	{"cue",      FA_FILE_AUDIO_O,      0},
	{"cvs",      FA_COGS,              0},
	{"cxx",      MFIZZ_CPLUSPLUS,      0},

	/* D */
	{"db",       MFIZZ_DATABASE_ALT2,  0},
	{"deb",      MFIZZ_DEBIAN,         0},
	{"dll",      FILE_MANPAGE,         0},
	{"doc",      FILE_WORD,            0},
	{"docx",     FILE_WORD,            0},

	 /* E */
	{"ejs",      FA_FILE_CODE_O,       0},
	{"elf",      FA_LINUX,             0},
	{"epub",     FA_FILE_PDF_O,        0},
	{"exe",      FA_WINDOWS,           0},
	
	/* F */
	{"f#",       DEV_FSHARP,           0},
	{"flac",     FA_FILE_AUDIO_O,      0},
	{"flv",      FA_FILE_MOVIE_O,      0},
	{"fs",       DEV_FSHARP,           0},
	{"fsi",      DEV_FSHARP,           0},
	{"fsscript", DEV_FSHARP,           0},
	{"fsx",      DEV_FSHARP,           0},

	/* G */
	{"gem",      FA_FILE_ARCHIVE_O,    0},
	{"gif",      FA_FILE_IMAGE_O,      0},
	{"go",       MFIZZ_GO,             0},
	{"gz",       FA_FILE_ARCHIVE_O,    0},
	{"gzip",     FA_FILE_ARCHIVE_O,    0},

	/* H */
	{"h",        MFIZZ_C,              0},
	{"hh",       MFIZZ_CPLUSPLUS,      0},
	{"htaccess", FA_COGS,              0},
	{"htpasswd", FA_COGS,              0},
	{"htm",      FA_FILE_CODE_O,       0},
	{"html",     FA_FILE_CODE_O,       0},
	{"hxx",      MFIZZ_CPLUSPLUS,      0},

	/* I */
	{"ico",      FA_FILE_IMAGE_O,      0},
	{"img",      FA_FILE_IMAGE_O,      0},
	{"ini",      FA_COGS,              0},
	{"iso",      LINEA_MUSIC_CD,       0},

	/* J */
	{"jar",      MFIZZ_JAVA,           0},
	{"java",     MFIZZ_JAVA,           0},
	{"jl",       FA_COGS,              0},
	{"jpeg",     FA_FILE_IMAGE_O,      0},
	{"jpg",      FA_FILE_IMAGE_O,      0},
	{"js",       DEV_JAVASCRIPT_BADGE, 0},
	{"json",     MFIZZ_JAVASCRIPT,     0},
	{"jsx",      FILE_JSX,             0},

	/* K */

	/* L */
	{"lha",      FA_FILE_ARCHIVE_O,    0},
	{"log",      FA_FILE_TEXT_O,       0},
	{"lua",      FILE_LUA,             0},
	{"lzh",      FA_FILE_ARCHIVE_O,    0},
	{"lzma",     FA_FILE_ARCHIVE_O,    0},

	/* M */
	{"m4a",      FA_FILE_AUDIO_O,      0},
	{"m4v",      FA_FILE_MOVIE_O,      0},
	{"markdown", OCT_MARKDOWN,         0},
	{"md",       OCT_MARKDOWN,         0},
	{"mkv",      FA_FILE_MOVIE_O,      0},
	{"mov",      FA_FILE_MOVIE_O,      0},
	{"mp3",      FA_FILE_AUDIO_O,      0},
	{"mp4",      FA_FILE_MOVIE_O,      0},
	{"mpeg",     FA_FILE_MOVIE_O,      0},
	{"mpg",      FA_FILE_MOVIE_O,      0},
	{"msi",      FA_WINDOWS,           0},

	/* N */

	/* O */
	{"o",        FILE_MANPAGE,         0},
	{"ogg",      FA_FILE_AUDIO_O,      0},
	{"out",      FA_LINUX,             0},

	/* P */
	{"pdf",      FA_FILE_PDF_O,        0},
	{"php",      MFIZZ_PHP,            0},
	{"png",      FA_FILE_IMAGE_O,      0},
	{"ppt",      FILE_POWERPOINT,      0},
	{"pptx",     FILE_POWERPOINT,      0},
	{"psb",      DEV_PHOTOSHOP,        0},
	{"psd",      DEV_PHOTOSHOP,        0},
	{"py",       MFIZZ_PYTHON,         0},
	{"pyc",      MFIZZ_PYTHON,         0},
	{"pyd",      MFIZZ_PYTHON,         0},
	{"pyo",      MFIZZ_PYTHON,         0},

	/* Q */

	/* R */
	{"rar",      FA_FILE_ARCHIVE_O,    0},
	{"rc",       FA_COGS,              0},
	{"rom",      FA_LOCK,              0},
	{"rpm",      FA_FILE_ARCHIVE_O,    0},
	{"rss",      FA_RSS_SQUARE,        0},
	{"rtf",      FA_FILE_PDF_O,        0},

	/* S */
	{"so",       FILE_MANPAGE,         0},
	{"scala",    MFIZZ_SCALA,          0},
	{"sh",       MFIZZ_SCRIPT,         0},
	{"slim",     FA_FILE_CODE_O,       0},
	{"sln",      DEV_VISUALSTUDIO,     0},
	{"sql",      MFIZZ_MYSQL,          0},
	{"svg",      FA_FILE_IMAGE_O,      0},

	/* T */
	{"tar",      FA_FILE_ARCHIVE_O,    0},
	{"tex",      FILE_TEX,             0},
	{"tgz",      FA_FILE_ARCHIVE_O,    0},
	{"ts",       FILE_TS,              0},
	{"tsx",      FILE_TSX,             0},
	{"txt",      FA_FILE_TEXT_O,       0},

	/* U */

	/* V */
	{"vim",      DEV_VIM,              0},
	{"vimrc",    DEV_VIM,              0},

	/* W */
	{"wav",      FA_FILE_AUDIO_O,      0},
	{"webm",     FA_FILE_MOVIE_O,      0},

	/* X */
	{"xbps",     FA_FILE_ARCHIVE_O,    0},
	{"xhtml",    FA_FILE_CODE_O,       0},
	{"xls",      FILE_EXCEL,           0},
	{"xlsx",     FILE_EXCEL,           0},
	{"xml",      FA_FILE_CODE_O,       0},
	{"xz",       FA_FILE_ARCHIVE_O,    0},

	/* Y */
	{"yaml",     FA_COGS,              0},
	{"yml",      FA_COGS,              0},

	/* Z */
	{"zip",      FA_FILE_ARCHIVE_O,    0}

	/* Other */
};
