#include "icons-in-terminal.h"

struct icon_pair {
	const char *match;
	const char *icon;
	/*
	 * Hex xterm 256 color code, 0 to follow file specific (if any)
	 * Codes: https://jonasjacek.github.io/colors/
	 */
	const unsigned char color;
};

/*
 * Define a string to be printed before and after the icon
 * Adjust if the icons are not printed properly
 */

#define ICON_PADDING_LEFT  ""
#define ICON_PADDING_RIGHT " "

#define VIDEO_COLOR    93 /* Purple */
#define AUDIO_COLOR   220 /* Gold1 */
#define IMAGE_COLOR    82 /* Chartreuse2 */
#define BOOKS_COLOR   202 /* OrangeRed1 */
#define ARCHIVE_COLOR 209 /* Salmon1 */

/*
 * Using symbols defined in icons-in-terminal.h, or even using icons-in-terminal is not necessary.
 * You can use whatever pathched font you like. You just have to put the desired icon as a string.
 * If you are using icons-in-terminal the creator recommends that you do use the symbols in the generated header.
 */

static const struct icon_pair dir_icon  = {"", FA_FOLDER, 0};
static const struct icon_pair file_icon = {"", FA_FILE_O, 0};
static const struct icon_pair exec_icon = {"", FA_COG,    0};

/* All entries are case-insensitive */

static const struct icon_pair icons_name[] = {
	{".git",         FA_GIT,        0},
	{"Desktop",      FA_DESKTOP,    0},
	{"Documents",    FA_BRIEFCASE,  0},
	{"Downloads",    FA_DOWNLOAD,   0},
	{"Music",        FA_MUSIC,      0},
	{"Pictures",     MD_CAMERA_ALT, 0},
	{"Public",       FA_INBOX,      0},
	{"Templates",    FA_PAPERCLIP,  0},
	{"Videos",       FA_FILM,       0},
	{"CHANGELOG",    FA_HISTORY,    0},
	{"configure",    FILE_CONFIG,   0},
	{"License",      FA_COPYRIGHT,  0},
	{"Makefile",     FILE_CMAKE,    0},
};

/*
 * New entries should bu added such that the first character of the extension is in the correct group .
 * This is done for performance reason so that the correct icon can be found faster.
 * All entries are case-insensitive
 */

static const struct icon_pair icons_ext[] = {
	/* Numbers */
	{"1",        FILE_MANPAGE,         0},
	{"7z",       FA_FILE_ARCHIVE_O,    ARCHIVE_COLOR},

	/* A */
	{"a",        FILE_MANPAGE,         0},
	{"apk",      FA_FILE_ARCHIVE_O,    ARCHIVE_COLOR},
	{"asm",      FILE_NASM,            0},
	{"aup",      FA_FILE_AUDIO_O,      AUDIO_COLOR},
	{"avi",      FA_FILE_MOVIE_O,      VIDEO_COLOR},

	/* B */
	{"bat",      MFIZZ_SCRIPT,         0},
	{"bin",      OCT_FILE_BINARY,      0},
	{"bmp",      FA_FILE_IMAGE_O,      IMAGE_COLOR},
	{"bz2",      FA_FILE_ARCHIVE_O,    ARCHIVE_COLOR},

	 /* C */
	{"c",        MFIZZ_C,              0},
	{"c++",      MFIZZ_CPLUSPLUS,      0},
	{"cab",      FA_FILE_ARCHIVE_O,    ARCHIVE_COLOR},
	{"cbr",      FA_FILE_ARCHIVE_O,    ARCHIVE_COLOR},
	{"cbz",      FA_FILE_ARCHIVE_O,    ARCHIVE_COLOR},
	{"cc",       MFIZZ_CPLUSPLUS,      0},
	{"class",    MFIZZ_JAVA,           0},
	{"clj",      MFIZZ_CLOJURE,        0},
	{"cljc",     MFIZZ_CLOJURE,        0},
	{"cljs",     MFIZZ_CLOJURE,        0},
	{"cmake",    FILE_CMAKE,           0},
	{"coffee",   MFIZZ_COFFEE_BEAN,    0},
	{"conf",     FA_COGS,              0},
	{"cpio",     FA_FILE_ARCHIVE_O,    ARCHIVE_COLOR},
	{"cpp",      MFIZZ_CPLUSPLUS,      0},
	{"css",      MFIZZ_CSS3,           0},
	{"cue",      FA_FILE_AUDIO_O,      AUDIO_COLOR},
	{"cvs",      FA_COGS,              0},
	{"cxx",      MFIZZ_CPLUSPLUS,      0},

	/* D */
	{"db",       MFIZZ_DATABASE_ALT2,  0},
	{"deb",      MFIZZ_DEBIAN,         ARCHIVE_COLOR},
	{"diff",     FILE_DIFF,            0},
	{"dll",      FILE_MANPAGE,         0},
	{"doc",      FILE_WORD,            0},
	{"docx",     FILE_WORD,            0},

	 /* E */
	{"ejs",      FA_FILE_CODE_O,       0},
	{"elf",      FA_LINUX,             0},
	{"epub",     FA_FILE_PDF_O,        BOOKS_COLOR},
	{"exe",      FA_WINDOWS,           0},

	/* F */
	{"f#",       DEV_FSHARP,           0},
	{"flac",     FA_FILE_AUDIO_O,      AUDIO_COLOR},
	{"flv",      FA_FILE_MOVIE_O,      VIDEO_COLOR},
	{"fs",       DEV_FSHARP,           0},
	{"fsi",      DEV_FSHARP,           0},
	{"fsscript", DEV_FSHARP,           0},
	{"fsx",      DEV_FSHARP,           0},

	/* G */
	{"gem",      FA_FILE_ARCHIVE_O,    0},
	{"gif",      FA_FILE_IMAGE_O,      IMAGE_COLOR},
	{"go",       MFIZZ_GO,             0},
	{"gz",       FA_FILE_ARCHIVE_O,    ARCHIVE_COLOR},
	{"gzip",     FA_FILE_ARCHIVE_O,    ARCHIVE_COLOR},

	/* H */
	{"h",        MFIZZ_C,              0},
	{"hh",       MFIZZ_CPLUSPLUS,      0},
	{"htaccess", FA_COGS,              0},
	{"htpasswd", FA_COGS,              0},
	{"htm",      FA_FILE_CODE_O,       0},
	{"html",     FA_FILE_CODE_O,       0},
	{"hxx",      MFIZZ_CPLUSPLUS,      0},

	/* I */
	{"ico",      FA_FILE_IMAGE_O,      IMAGE_COLOR},
	{"img",      FA_FILE_ARCHIVE_O,    ARCHIVE_COLOR},
	{"ini",      FA_COGS,              0},
	{"iso",      LINEA_MUSIC_CD,       ARCHIVE_COLOR},

	/* J */
	{"jar",      MFIZZ_JAVA,           0},
	{"java",     MFIZZ_JAVA,           0},
	{"jl",       FA_COGS,              0},
	{"jpeg",     FA_FILE_IMAGE_O,      IMAGE_COLOR},
	{"jpg",      FA_FILE_IMAGE_O,      IMAGE_COLOR},
	{"js",       DEV_JAVASCRIPT_BADGE, 0},
	{"json",     MFIZZ_JAVASCRIPT,     0},
	{"jsx",      FILE_JSX,             0},

	/* K */

	/* L */
	{"lha",      FA_FILE_ARCHIVE_O,    ARCHIVE_COLOR},
	{"log",      FA_FILE_TEXT_O,       0},
	{"lua",      FILE_LUA,             0},
	{"lzh",      FA_FILE_ARCHIVE_O,    ARCHIVE_COLOR},
	{"lzma",     FA_FILE_ARCHIVE_O,    ARCHIVE_COLOR},

	/* M */
	{"m4a",      FA_FILE_AUDIO_O,      AUDIO_COLOR},
	{"m4v",      FA_FILE_MOVIE_O,      VIDEO_COLOR},
	{"markdown", DEV_MARKDOWN,         0},
	{"md",       DEV_MARKDOWN,         0},
	{"mk",       FILE_CMAKE,           0},
	{"mkv",      FA_FILE_MOVIE_O,      VIDEO_COLOR},
	{"mov",      FA_FILE_MOVIE_O,      VIDEO_COLOR},
	{"mp3",      FA_FILE_AUDIO_O,      AUDIO_COLOR},
	{"mp4",      FA_FILE_MOVIE_O,      VIDEO_COLOR},
	{"mpeg",     FA_FILE_MOVIE_O,      VIDEO_COLOR},
	{"mpg",      FA_FILE_MOVIE_O,      VIDEO_COLOR},
	{"msi",      FA_WINDOWS,           0},

	/* N */

	/* O */
	{"o",          FILE_MANPAGE,       0},
	{"ogg",        FA_FILE_AUDIO_O,    AUDIO_COLOR},
	{"opdownload", FA_DOWNLOAD,        0},
	{"out",        FA_LINUX,           0},

	/* P */
	{"part",     FA_DOWNLOAD,          0},
	{"patch",    FILE_PATCH,           0},
	{"pdf",      FA_FILE_PDF_O,        BOOKS_COLOR},
	{"php",      MFIZZ_PHP,            0},
	{"png",      FA_FILE_IMAGE_O,      IMAGE_COLOR},
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
	{"rar",      FA_FILE_ARCHIVE_O,    ARCHIVE_COLOR},
	{"rc",       FA_COGS,              0},
	{"rom",      FA_LOCK,              0},
	{"rpm",      FA_FILE_ARCHIVE_O,    ARCHIVE_COLOR},
	{"rss",      FA_RSS_SQUARE,        0},
	{"rtf",      FA_FILE_PDF_O,        0},

	/* S */
	{"so",       FILE_MANPAGE,         0},
	{"scala",    MFIZZ_SCALA,          0},
	{"sh",       MFIZZ_SCRIPT,         0},
	{"slim",     FA_FILE_CODE_O,       0},
	{"sln",      DEV_VISUALSTUDIO,     0},
	{"sql",      MFIZZ_MYSQL,          0},
	{"srt",      FA_COMMENTS_O,        0},
	{"sub",      FA_COMMENTS_O,        0},
	{"svg",      FA_FILE_IMAGE_O,      IMAGE_COLOR},

	/* T */
	{"tar",      FA_FILE_ARCHIVE_O,    ARCHIVE_COLOR},
	{"tex",      FILE_TEX,             0},
	{"tgz",      FA_FILE_ARCHIVE_O,    ARCHIVE_COLOR},
	{"ts",       FILE_TS,              0},
	{"tsx",      FILE_TSX,             0},
	{"txt",      FA_FILE_TEXT_O,       0},
	{"txz",      FA_FILE_ARCHIVE_O,    ARCHIVE_COLOR},

	/* U */

	/* V */
	{"vid",      FA_FILE_MOVIE_O,      VIDEO_COLOR},
	{"vim",      DEV_VIM,              0},
	{"vimrc",    DEV_VIM,              0},

	/* W */
	{"wav",      FA_FILE_AUDIO_O,      AUDIO_COLOR},
	{"webm",     FA_FILE_MOVIE_O,      VIDEO_COLOR},
	{"wma",      FA_FILE_AUDIO_O,      AUDIO_COLOR},
	{"wmv",      FA_FILE_MOVIE_O,      VIDEO_COLOR},

	/* X */
	{"xbps",     FA_FILE_ARCHIVE_O,    ARCHIVE_COLOR},
	{"xhtml",    FA_FILE_CODE_O,       0},
	{"xls",      FILE_EXCEL,           0},
	{"xlsx",     FILE_EXCEL,           0},
	{"xml",      FA_FILE_CODE_O,       0},
	{"xz",       FA_FILE_ARCHIVE_O,    ARCHIVE_COLOR},

	/* Y */
	{"yaml",     FA_COGS,              0},
	{"yml",      FA_COGS,              0},

	/* Z */
	{"zip",      FA_FILE_ARCHIVE_O,    ARCHIVE_COLOR}

	/* Other */
};
