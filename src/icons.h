#include "icons-in-terminal.h"

struct icon_pair {
	const char *match;
	const char *icon;
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

static const char *dir_icon = FA_FOLDER;
static const char *file_icon = FA_FILE;

/* All entries are case-insensitive */

static const struct icon_pair icons_name[] = {
	{".git",         FA_GITHUB_SQUARE},
	{"Desktop",      FA_HOME},
	{"Documents",    FA_LIST_ALT},
	{"Downloads",    FA_DOWNLOAD},
	{"Music",        FA_MUSIC},
	{"node_modules", MFIZZ_NPM},
	{"Pictures",     FA_IMAGE},
	{"Public",       FA_INBOX},
	{"Templates",    FA_COG},
	{"Videos",       FA_FILM},
};

/* 
 * New entries should bu added such that the first character of the extension is in the correct group .
 * This is done for performance reason so that the correct icon can be found faster.
 * All entries are case-insensitive
 */

static const struct icon_pair icons_ext[] = {
	/* Numbers */
	{"7z",       FA_FILE_ARCHIVE_O},

	/* A */
	{"a",        FILE_MANPAGE},
	{"apk",      FA_FILE_ARCHIVE_O},
	{"asm",      FILE_NASM},
	{"aup",      FA_FILE_AUDIO_O},
	{"avi",      FA_FILE_MOVIE_O},

	/* B */
	{"bat",      MFIZZ_SCRIPT},
	{"bmp",      FA_FILE_IMAGE_O},
	{"bz2",      FA_FILE_ARCHIVE_O},

	 /* C */
	{"c",        MFIZZ_C},
	{"c++",      MFIZZ_CPLUSPLUS},
	{"cab",      FA_FILE_ARCHIVE_O},
	{"cbr",      FA_FILE_ARCHIVE_O},
	{"cbz",      FA_FILE_ARCHIVE_O},
	{"cc",       MFIZZ_CPLUSPLUS},
	{"class",    MFIZZ_JAVA},
	{"clj",      MFIZZ_CLOJURE},
	{"cljc",     MFIZZ_CLOJURE},
	{"cljs",     MFIZZ_CLOJURE},
	{"cmake",    FILE_CMAKE},
	{"coffee",   MFIZZ_COFFEE_BEAN},
	{"conf",     FA_COGS},
	{"cpio",     FA_FILE_ARCHIVE_O},
	{"cpp",      MFIZZ_CPLUSPLUS},
	{"css",      MFIZZ_CSS3},
	{"cue",      FA_FILE_AUDIO_O},
	{"cvs",      FA_COGS},
	{"cxx",      MFIZZ_CPLUSPLUS},

	/* D */
	{"db",       MFIZZ_DATABASE_ALT2},
	{"deb",      MFIZZ_DEBIAN},
	{"dll",      FILE_MANPAGE},
	{"doc",      FILE_WORD},
	{"docx",     FILE_WORD},

	 /* E */
	{"ejs",      FA_FILE_CODE_O},
	{"elf",      FA_LINUX},
	{"epub",     FA_FILE_PDF_O},
	{"exe",      FA_WINDOWS},
	
	/* F */
	{"f#",       DEV_FSHARP},
	{"flac",     FA_FILE_AUDIO_O},
	{"flv",      FA_FILE_MOVIE_O},
	{"fs",       DEV_FSHARP},
	{"fsi",      DEV_FSHARP},
	{"fsscript", DEV_FSHARP},
	{"fsx",      DEV_FSHARP},

	/* G */
	{"gem",      FA_FILE_ARCHIVE_O},
	{"gif",      FA_FILE_IMAGE_O},
	{"go",       MFIZZ_GO},
	{"gz",       FA_FILE_ARCHIVE_O},
	{"gzip",     FA_FILE_ARCHIVE_O},

	/* H */
	{"h",        MFIZZ_C},
	{"hh",       MFIZZ_CPLUSPLUS},
	{"htaccess", FA_COGS},
	{"htpasswd", FA_COGS},
	{"htm",      FA_FILE_CODE_O},
	{"html",     FA_FILE_CODE_O},
	{"hxx",      MFIZZ_CPLUSPLUS},

	/* I */
	{"ico",      FA_FILE_IMAGE_O},
	{"img",      FA_FILE_IMAGE_O},
	{"ini",      FA_COGS},
	{"iso",      LINEA_MUSIC_CD},

	/* J */
	{"jar",      MFIZZ_JAVA},
	{"java",     MFIZZ_JAVA},
	{"jl",       FA_COGS},
	{"jpeg",     FA_FILE_IMAGE_O},
	{"jpg",      FA_FILE_IMAGE_O},
	{"js",       DEV_JAVASCRIPT_BADGE},
	{"json",     MFIZZ_JAVASCRIPT},
	{"jsx",      FILE_JSX},

	/* K */

	/* L */
	{"lha",      FA_FILE_ARCHIVE_O},
	{"log",      FA_FILE_TEXT_O},
	{"lua",      FILE_LUA},
	{"lzh",      FA_FILE_ARCHIVE_O},
	{"lzma",     FA_FILE_ARCHIVE_O},

	/* M */
	{"m4a",      FA_FILE_AUDIO_O},
	{"m4v",      FA_FILE_MOVIE_O},
	{"markdown", OCT_MARKDOWN},
	{"md",       OCT_MARKDOWN},
	{"mkv",      FA_FILE_MOVIE_O},
	{"mov",      FA_FILE_MOVIE_O},
	{"mp3",      FA_FILE_AUDIO_O},
	{"mp4",      FA_FILE_MOVIE_O},
	{"mpeg",     FA_FILE_MOVIE_O},
	{"mpg",      FA_FILE_MOVIE_O},
	{"msi",      FA_WINDOWS},

	/* N */

	/* O */
	{"o",        FILE_MANPAGE},
	{"ogg",      FA_FILE_AUDIO_O},
	{"out",      FA_LINUX},

	/* P */
	{"pdf",      FA_FILE_PDF_O},
	{"php",      MFIZZ_PHP},
	{"png",      FA_FILE_IMAGE_O},
	{"ppt",      FILE_POWERPOINT},
	{"pptx",     FILE_POWERPOINT},
	{"psb",      DEV_PHOTOSHOP},
	{"psd",      DEV_PHOTOSHOP},
	{"py",       MFIZZ_PYTHON},
	{"pyc",      MFIZZ_PYTHON},
	{"pyd",      MFIZZ_PYTHON},
	{"pyo",      MFIZZ_PYTHON},

	/* Q */

	/* R */
	{"rar",      FA_FILE_ARCHIVE_O},
	{"rc",       FA_COGS},
	{"rom",      FA_LOCK},
	{"rpm",      FA_FILE_ARCHIVE_O},
	{"rss",      FA_RSS_SQUARE},
	{"rtf",      FA_FILE_PDF_O},

	/* S */
	{"so",       FILE_MANPAGE},
	{"scala",    MFIZZ_SCALA},
	{"sh",       MFIZZ_SCRIPT},
	{"slim",     FA_FILE_CODE_O},
	{"sln",      DEV_VISUALSTUDIO},
	{"sql",      MFIZZ_MYSQL},
	{"svg",      FA_FILE_IMAGE_O},

	/* T */
	{"tar",      FA_FILE_ARCHIVE_O},
	{"tex",      FILE_TEX},
	{"tgz",      FA_FILE_ARCHIVE_O},
	{"ts",       FILE_TS},
	{"tsx",      FILE_TSX},
	{"txt",      FA_FILE_TEXT_O},

	/* U */

	/* V */
	{"vim",      DEV_VIM},
	{"vimrc",    DEV_VIM},

	/* W */
	{"wav",      FA_FILE_AUDIO_O},
	{"webm",     FA_FILE_MOVIE_O},

	/* X */
	{"xbps",     FA_FILE_ARCHIVE_O},
	{"xhtml",    FA_FILE_CODE_O},
	{"xls",      FILE_EXCEL},
	{"xlsx",     FILE_EXCEL},
	{"xml",      FA_FILE_CODE_O},
	{"xz",       FA_FILE_ARCHIVE_O},

	/* Y */
	{"yaml",     FA_COGS},
	{"yml",      FA_COGS},

	/* Z */
	{"zip",      FA_FILE_ARCHIVE_O}

	/* Other */
};
