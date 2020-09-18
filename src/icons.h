#pragma once

#if defined(ICONS)
#include "icons-in-terminal.h"
#elif defined(NERD)
#include "icons-nerdfont.h"
#endif

struct icon_pair {
	const char *match;
	const char *icon;
	/*
	 * Hex xterm 256 color code, 0 to follow file specific (if any)
	 * Codes: https://jonasjacek.github.io/colors/
	 * Spectrum sorted: https://upload.wikimedia.org/wikipedia/commons/1/15/Xterm_256color_chart.svg
	 */
	const unsigned char color;
};

/*
 * Define a string to be printed before and after the icon
 * Adjust if the icons are not printed properly
 */

#define ICON_PADDING_LEFT  ""
#define ICON_PADDING_RIGHT " "

#define COLOR_VIDEO        93  /* Purple */
#define COLOR_AUDIO        220 /* Gold1 */
#define COLOR_IMAGE        82  /* Chartreuse2 */
#define COLOR_DOCS         202 /* OrangeRed1 */
#define COLOR_ARCHIVE      209 /* Salmon1 */
#define COLOR_C            81  /* SteelBlue1 */
#define COLOR_JAVA         32  /* DeepSkyBlue3 */
#define COLOR_JAVASCRIPT   47  /* SpringGreen2 */
#define COLOR_REACT        39  /* DeepSkyBlue1 */
#define COLOR_CSS          199 /* DeepPink1 */
#define COLOR_PYTHON       227 /* LightGoldenrod1 */
#define COLOR_LUA          19  /* LightGoldenrod1 */
#define COLOR_DOCUMENT     15  /* WHITE */
#define COLOR_FSHARP       31  /* DeepSkyBlue3 */
#define COLOR_RUBY         160 /* Red3 */
#define COLOR_SCALA        196 /* Red1 */
#define COLOR_VIM          28  /* Green4 */

/*
 * Using symbols defined in icons-in-terminal.h, or even using icons-in-terminal is not necessary.
 * You can use whatever pathched font you like. You just have to put the desired icon as a string.
 * If you are using icons-in-terminal the creator recommends that you do use the symbols in the generated header.
 */

#if defined(ICONS)
static const struct icon_pair dir_icon  = {"", FA_FOLDER, 0};
static const struct icon_pair file_icon = {"", FA_FILE_O, 0};
static const struct icon_pair exec_icon = {"", FA_COG,    0};
#elif defined(NERD)
static const struct icon_pair dir_icon  = {"", ICON_DIRECTORY, 0};
static const struct icon_pair file_icon = {"", ICON_FILE,      0};
static const struct icon_pair exec_icon = {"", ICON_EXEC,      0};
#endif

/* All entries are case-insensitive */

static const struct icon_pair icons_name[] = {
#if defined(ICONS)
	{".git",         FA_GIT,        0},
	{"Desktop",      FA_DESKTOP,    0},
	{"Documents",    FA_BRIEFCASE,  0},
	{"Downloads",    FA_DOWNLOAD,   0},
	{"Music",        FA_MUSIC,      0},
	{"Pictures",     MD_CAMERA_ALT, 0},
	{"Public",       FA_INBOX,      0},
	{"Templates",    FA_PAPERCLIP,  0},
	{"Videos",       FA_FILM,       0},
	{"CHANGELOG",    FA_HISTORY,    COLOR_DOCS},
	{"configure",    FILE_CONFIG,   0},
	{"License",      FA_COPYRIGHT,  COLOR_DOCS},
	{"Makefile",     FILE_CMAKE,    0},
#elif defined(NERD)
	{".git",         ICON_GIT,       0},
	{"Desktop",      ICON_DESKTOP,   0},
	{"Documents",    ICON_BRIEFCASE, 0},
	{"Downloads",    ICON_DOWNLOADS, 0},
	{"Music",        ICON_MUSIC,     0},
	{"Pictures",     ICON_PICTURES,  0},
	{"Public",       ICON_PUBLIC,    0},
	{"Templates",    ICON_TEMPLATES, 0},
	{"Videos",       ICON_VIDEOS,    0},
	{"CHANGELOG",    ICON_CHANGELOG, COLOR_DOCS},
	{"configure",    ICON_CONFIGURE, 0},
	{"License",      ICON_LICENSE,   COLOR_DOCS},
	{"Makefile",     ICON_MAKEFILE,  0},
#endif
};

/*
 * New entries should bu added such that the first character of the extension is in the correct group .
 * This is done for performance reason so that the correct icon can be found faster.
 * All entries are case-insensitive
 */

static const struct icon_pair icons_ext[] = {
#if defined(ICONS)
	/* Numbers */
	{"1",        FILE_MANPAGE,         COLOR_DOCS},
	{"7z",       FA_FILE_ARCHIVE_O,    COLOR_ARCHIVE},

	/* A */
	{"a",        FILE_MANPAGE,         0},
	{"apk",      FA_FILE_ARCHIVE_O,    COLOR_ARCHIVE},
	{"asm",      FILE_NASM,            0},
	{"aup",      FA_FILE_AUDIO_O,      COLOR_AUDIO},
	{"avi",      FA_FILE_MOVIE_O,      COLOR_VIDEO},

	/* B */
	{"bat",      MFIZZ_SCRIPT,         0},
	{"bin",      OCT_FILE_BINARY,      0},
	{"bmp",      FA_FILE_IMAGE_O,      COLOR_IMAGE},
	{"bz2",      FA_FILE_ARCHIVE_O,    COLOR_ARCHIVE},

	 /* C */
	{"c",        MFIZZ_C,              0},
	{"c++",      MFIZZ_CPLUSPLUS,      0},
	{"cab",      FA_FILE_ARCHIVE_O,    COLOR_ARCHIVE},
	{"cbr",      FA_FILE_ARCHIVE_O,    COLOR_ARCHIVE},
	{"cbz",      FA_FILE_ARCHIVE_O,    COLOR_ARCHIVE},
	{"cc",       MFIZZ_CPLUSPLUS,      0},
	{"class",    MFIZZ_JAVA,           0},
	{"clj",      MFIZZ_CLOJURE,        0},
	{"cljc",     MFIZZ_CLOJURE,        0},
	{"cljs",     MFIZZ_CLOJURE,        0},
	{"cmake",    FILE_CMAKE,           0},
	{"coffee",   MFIZZ_COFFEE_BEAN,    0},
	{"conf",     FA_COGS,              0},
	{"cpio",     FA_FILE_ARCHIVE_O,    COLOR_ARCHIVE},
	{"cpp",      MFIZZ_CPLUSPLUS,      0},
	{"css",      MFIZZ_CSS3,           0},
	{"cue",      FA_FILE_AUDIO_O,      COLOR_AUDIO},
	{"cvs",      FA_COGS,              0},
	{"cxx",      MFIZZ_CPLUSPLUS,      0},

	/* D */
	{"db",       MFIZZ_DATABASE_ALT2,  0},
	{"deb",      MFIZZ_DEBIAN,         COLOR_ARCHIVE},
	{"diff",     FILE_DIFF,            0},
	{"dll",      FILE_MANPAGE,         0},
	{"doc",      FILE_WORD,            0},
	{"docx",     FILE_WORD,            0},

	 /* E */
	{"ejs",      FA_FILE_CODE_O,       0},
	{"elf",      FA_LINUX,             0},
	{"epub",     FA_FILE_PDF_O,        COLOR_DOCS},
	{"exe",      FA_WINDOWS,           0},

	/* F */
	{"f#",       DEV_FSHARP,           0},
	{"flac",     FA_FILE_AUDIO_O,      COLOR_AUDIO},
	{"flv",      FA_FILE_MOVIE_O,      COLOR_VIDEO},
	{"fs",       DEV_FSHARP,           0},
	{"fsi",      DEV_FSHARP,           0},
	{"fsscript", DEV_FSHARP,           0},
	{"fsx",      DEV_FSHARP,           0},

	/* G */
	{"gem",      FA_FILE_ARCHIVE_O,    0},
	{"gif",      FA_FILE_IMAGE_O,      COLOR_IMAGE},
	{"go",       MFIZZ_GO,             0},
	{"gz",       FA_FILE_ARCHIVE_O,    COLOR_ARCHIVE},
	{"gzip",     FA_FILE_ARCHIVE_O,    COLOR_ARCHIVE},

	/* H */
	{"h",        MFIZZ_C,              0},
	{"hh",       MFIZZ_CPLUSPLUS,      0},
	{"htaccess", FA_COGS,              0},
	{"htpasswd", FA_COGS,              0},
	{"htm",      FA_FILE_CODE_O,       0},
	{"html",     FA_FILE_CODE_O,       0},
	{"hxx",      MFIZZ_CPLUSPLUS,      0},

	/* I */
	{"ico",      FA_FILE_IMAGE_O,      COLOR_IMAGE},
	{"img",      FA_FILE_ARCHIVE_O,    COLOR_ARCHIVE},
	{"ini",      FA_COGS,              0},
	{"iso",      LINEA_MUSIC_CD,       COLOR_ARCHIVE},

	/* J */
	{"jar",      MFIZZ_JAVA,           0},
	{"java",     MFIZZ_JAVA,           0},
	{"jl",       FA_COGS,              0},
	{"jpeg",     FA_FILE_IMAGE_O,      COLOR_IMAGE},
	{"jpg",      FA_FILE_IMAGE_O,      COLOR_IMAGE},
	{"js",       DEV_JAVASCRIPT_BADGE, 0},
	{"json",     MFIZZ_JAVASCRIPT,     0},
	{"jsx",      FILE_JSX,             0},

	/* K */

	/* L */
	{"lha",      FA_FILE_ARCHIVE_O,    COLOR_ARCHIVE},
	{"log",      FA_FILE_TEXT_O,       0},
	{"lua",      FILE_LUA,             0},
	{"lzh",      FA_FILE_ARCHIVE_O,    COLOR_ARCHIVE},
	{"lzma",     FA_FILE_ARCHIVE_O,    COLOR_ARCHIVE},

	/* M */
	{"m4a",      FA_FILE_AUDIO_O,      COLOR_AUDIO},
	{"m4v",      FA_FILE_MOVIE_O,      COLOR_VIDEO},
	{"markdown", DEV_MARKDOWN,         COLOR_DOCS},
	{"md",       DEV_MARKDOWN,         COLOR_DOCS},
	{"mk",       FILE_CMAKE,           0},
	{"mkv",      FA_FILE_MOVIE_O,      COLOR_VIDEO},
	{"mov",      FA_FILE_MOVIE_O,      COLOR_VIDEO},
	{"mp3",      FA_FILE_AUDIO_O,      COLOR_AUDIO},
	{"mp4",      FA_FILE_MOVIE_O,      COLOR_VIDEO},
	{"mpeg",     FA_FILE_MOVIE_O,      COLOR_VIDEO},
	{"mpg",      FA_FILE_MOVIE_O,      COLOR_VIDEO},
	{"msi",      FA_WINDOWS,           0},

	/* N */

	/* O */
	{"o",          FILE_MANPAGE,       0},
	{"ogg",        FA_FILE_AUDIO_O,    COLOR_AUDIO},
	{"opdownload", FA_DOWNLOAD,        0},
	{"out",        FA_LINUX,           0},

	/* P */
	{"part",     FA_DOWNLOAD,          0},
	{"patch",    FILE_PATCH,           0},
	{"pdf",      FA_FILE_PDF_O,        COLOR_DOCS},
	{"php",      MFIZZ_PHP,            0},
	{"png",      FA_FILE_IMAGE_O,      COLOR_IMAGE},
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
	{"rar",      FA_FILE_ARCHIVE_O,    COLOR_ARCHIVE},
	{"rc",       FA_COGS,              0},
	{"rom",      FA_LOCK,              0},
	{"rpm",      FA_FILE_ARCHIVE_O,    COLOR_ARCHIVE},
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
	{"svg",      FA_FILE_IMAGE_O,      COLOR_IMAGE},

	/* T */
	{"tar",      FA_FILE_ARCHIVE_O,    COLOR_ARCHIVE},
	{"tex",      FILE_TEX,             0},
	{"tgz",      FA_FILE_ARCHIVE_O,    COLOR_ARCHIVE},
	{"ts",       FILE_TS,              0},
	{"tsx",      FILE_TSX,             0},
	{"txt",      FA_FILE_TEXT_O,       0},
	{"txz",      FA_FILE_ARCHIVE_O,    COLOR_ARCHIVE},

	/* U */

	/* V */
	{"vid",      FA_FILE_MOVIE_O,      COLOR_VIDEO},
	{"vim",      DEV_VIM,              0},
	{"vimrc",    DEV_VIM,              0},
	{"vtt",      FA_COMMENTS_O,        0},

	/* W */
	{"wav",      FA_FILE_AUDIO_O,      COLOR_AUDIO},
	{"webm",     FA_FILE_MOVIE_O,      COLOR_VIDEO},
	{"wma",      FA_FILE_AUDIO_O,      COLOR_AUDIO},
	{"wmv",      FA_FILE_MOVIE_O,      COLOR_VIDEO},

	/* X */
	{"xbps",     FA_FILE_ARCHIVE_O,    COLOR_ARCHIVE},
	{"xhtml",    FA_FILE_CODE_O,       0},
	{"xls",      FILE_EXCEL,           0},
	{"xlsx",     FILE_EXCEL,           0},
	{"xml",      FA_FILE_CODE_O,       0},
	{"xz",       FA_FILE_ARCHIVE_O,    COLOR_ARCHIVE},

	/* Y */
	{"yaml",     FA_COGS,              0},
	{"yml",      FA_COGS,              0},

	/* Z */
	{"zip",      FA_FILE_ARCHIVE_O,    COLOR_ARCHIVE},

	/* Other */
#elif defined(NERD)
	/* Numbers */
	{"1",          ICON_EXT_1,         COLOR_DOCS},
	{"7z",         ICON_EXT_7Z,        COLOR_ARCHIVE},

	/* A */
	{"a",          ICON_EXT_A,         0},
	{"apk",        ICON_EXT_APK,       COLOR_ARCHIVE},
	{"asm",        ICON_EXT_ASM,       0},
	{"aup",        ICON_EXT_AUP,       COLOR_AUDIO},
	{"avi",        ICON_EXT_AVI,       COLOR_VIDEO},

	/* B */
	{"bat",        ICON_EXT_BAT,       0},
	{"bin",        ICON_EXT_BIN,       0},
	{"bmp",        ICON_EXT_BMP,       COLOR_IMAGE},
	{"bz2",        ICON_EXT_BZ2,       COLOR_ARCHIVE},

	 /* C */
	{"c",          ICON_EXT_C,         COLOR_C},
	{"c++",        ICON_EXT_CPLUSPLUS, COLOR_C},
	{"cab",        ICON_EXT_CAB,       COLOR_ARCHIVE},
	{"cbr",        ICON_EXT_CBR,       COLOR_ARCHIVE},
	{"cbz",        ICON_EXT_CBZ,       COLOR_ARCHIVE},
	{"cc",         ICON_EXT_CC,        COLOR_C},
	{"class",      ICON_EXT_CLASS,     COLOR_JAVA},
	{"clj",        ICON_EXT_CLJ,       0},
	{"cljc",       ICON_EXT_CLJC,      0},
	{"cljs",       ICON_EXT_CLJS,      0},
	{"cmake",      ICON_EXT_CMAKE,     0},
	{"coffee",     ICON_EXT_COFFEE,    0},
	{"conf",       ICON_EXT_CONF,      0},
	{"cpio",       ICON_EXT_CPIO,      COLOR_ARCHIVE},
	{"cpp",        ICON_EXT_CPP,       0},
	{"css",        ICON_EXT_CSS,       COLOR_CSS},
	{"cue",        ICON_EXT_CUE,       COLOR_AUDIO},
	{"cvs",        ICON_EXT_CVS,       0},
	{"cxx",        ICON_EXT_CXX,       COLOR_C},

	/* D */
	{"db",         ICON_EXT_DB,        0},
	{"deb",        ICON_EXT_DEB,       COLOR_ARCHIVE},
	{"diff",       ICON_EXT_DIFF,      0},
	{"dll",        ICON_EXT_DLL,       0},
	{"doc",        ICON_EXT_DOC,       COLOR_DOCUMENT},
	{"docx",       ICON_EXT_DOCX,      COLOR_DOCUMENT},

	 /* E */
	{"ejs",        ICON_EXT_EJS,       COLOR_JAVASCRIPT},
	{"elf",        ICON_EXT_ELF,       0},
	{"epub",       ICON_EXT_EPUB,      COLOR_DOCS},
	{"exe",        ICON_EXT_EXE,       0},

	/* F */
	{"f#",         ICON_EXT_FSHARP,    COLOR_FSHARP},
	{"flac",       ICON_EXT_FLAC,      COLOR_AUDIO},
	{"flv",        ICON_EXT_FLV,       COLOR_VIDEO},
	{"fs",         ICON_EXT_FS,        COLOR_FSHARP},
	{"fsi",        ICON_EXT_FSI,       COLOR_FSHARP},
	{"fsscript",   ICON_EXT_FSSCRIPT,  COLOR_FSHARP},
	{"fsx",        ICON_EXT_FSX,       COLOR_FSHARP},

	/* G */
	{"gem",        ICON_EXT_GEM,       COLOR_RUBY},
	{"gif",        ICON_EXT_GIF,       COLOR_IMAGE},
	{"go",         ICON_EXT_GO,        0},
	{"gz",         ICON_EXT_GZ,        COLOR_ARCHIVE},
	{"gzip",       ICON_EXT_GZIP,      COLOR_ARCHIVE},

	/* H */
	{"h",          ICON_EXT_H,         COLOR_C},
	{"hh",         ICON_EXT_HH,        COLOR_C},
	{"htaccess",   ICON_EXT_HTACCESS,  0},
	{"htpasswd",   ICON_EXT_HTPASSWD,  0},
	{"htm",        ICON_EXT_HTM,       0},
	{"html",       ICON_EXT_HTML,      0},
	{"hxx",        ICON_EXT_HXX,       COLOR_C},

	/* I */
	{"ico",        ICON_EXT_ICO,       COLOR_IMAGE},
	{"img",        ICON_EXT_IMG,       COLOR_ARCHIVE},
	{"ini",        ICON_EXT_INI,       0},
	{"iso",        ICON_EXT_ISO,       COLOR_ARCHIVE},

	/* J */
	{"jar",        ICON_EXT_JAR,       COLOR_JAVA},
	{"java",       ICON_EXT_JAVA,      COLOR_JAVA},
	{"jl",         ICON_EXT_JL,        0},
	{"jpeg",       ICON_EXT_JPEG,      COLOR_IMAGE},
	{"jpg",        ICON_EXT_JPG,       COLOR_IMAGE},
	{"js",         ICON_EXT_JS,        COLOR_JAVASCRIPT},
	{"json",       ICON_EXT_JSON,      COLOR_JAVASCRIPT},
	{"jsx",        ICON_EXT_JSX,       COLOR_REACT},

	/* K */

	/* L */
	{"lha",        ICON_EXT_LHA,       COLOR_ARCHIVE},
	{"log",        ICON_EXT_LOG,       0},
	{"lua",        ICON_EXT_LUA,       0},
	{"lzh",        ICON_EXT_LZH,       COLOR_ARCHIVE},
	{"lzma",       ICON_EXT_LZMA,      COLOR_ARCHIVE},

	/* M */
	{"m4a",        ICON_EXT_M4A,       COLOR_AUDIO},
	{"m4v",        ICON_EXT_M4V,       COLOR_VIDEO},
	{"markdown",   ICON_EXT_MD,        COLOR_DOCS},
	{"md",         ICON_EXT_MD,        COLOR_DOCS},
	{"mk",         ICON_EXT_MK,        0},
	{"mkv",        ICON_EXT_MKV,       COLOR_VIDEO},
	{"mov",        ICON_EXT_MOV,       COLOR_VIDEO},
	{"mp3",        ICON_EXT_MP3,       COLOR_AUDIO},
	{"mp4",        ICON_EXT_MP4,       COLOR_VIDEO},
	{"mpeg",       ICON_EXT_MPEG,      COLOR_VIDEO},
	{"mpg",        ICON_EXT_MPG,       COLOR_VIDEO},
	{"msi",        ICON_EXT_MSI,       0},

	/* N */

	/* O */
	{"o",          ICON_EXT_O,         0},
	{"ogg",        ICON_EXT_OGG,       COLOR_AUDIO},
	{"opdownload", ICON_EXT_ODOWNLOAD, 0},
	{"out",        ICON_EXT_OUT,       0},

	/* P */
	{"part",       ICON_EXT_PART,      0},
	{"patch",      ICON_EXT_PATCH,     0},
	{"pdf",        ICON_EXT_PDF,       COLOR_DOCS},
	{"php",        ICON_EXT_PHP,       0},
	{"png",        ICON_EXT_PNG,       COLOR_IMAGE},
	{"ppt",        ICON_EXT_PPT,       0},
	{"pptx",       ICON_EXT_PPTX,      0},
	{"psb",        ICON_EXT_PSB,       0},
	{"psd",        ICON_EXT_PSD,       0},
	{"py",         ICON_EXT_PY,        COLOR_PYTHON},
	{"pyc",        ICON_EXT_PYC,       COLOR_PYTHON},
	{"pyd",        ICON_EXT_PYD,       COLOR_PYTHON},
	{"pyo",        ICON_EXT_PYO,       COLOR_PYTHON},

	/* Q */

	/* R */
	{"rar",        ICON_EXT_RAR,       COLOR_ARCHIVE},
	{"rc",         ICON_EXT_RC,        0},
	{"rom",        ICON_EXT_ROM,       0},
	{"rpm",        ICON_EXT_RPM,       COLOR_ARCHIVE},
	{"rss",        ICON_EXT_RSS,       0},
	{"rtf",        ICON_EXT_RTF,       0},

	/* S */
	{"sass",       ICON_EXT_SASS,      COLOR_CSS},
	{"scss",       ICON_EXT_SCSS,      COLOR_CSS},
	{"so",         ICON_EXT_SO,        0},
	{"scala",      ICON_EXT_SCALA,     COLOR_SCALA},
	{"sh",         ICON_EXT_SH,        0},
	{"slim",       ICON_EXT_SLIM,      COLOR_DOCUMENT},
	{"sln",        ICON_EXT_SLN,       0},
	{"sql",        ICON_EXT_SQL,       0},
	{"srt",        ICON_EXT_SRT,       0},
	{"sub",        ICON_EXT_SUB,       0},
	{"svg",        ICON_EXT_SVG,       COLOR_IMAGE},

	/* T */
	{"tar",        ICON_EXT_TAR,       COLOR_ARCHIVE},
	{"tex",        ICON_EXT_TEX,       0},
	{"tgz",        ICON_EXT_TGZ,       COLOR_ARCHIVE},
	{"ts",         ICON_EXT_TS,        COLOR_JAVASCRIPT},
	{"tsx",        ICON_EXT_TSX,       COLOR_REACT},
	{"txt",        ICON_EXT_TXT,       COLOR_DOCUMENT},
	{"txz",        ICON_EXT_TXZ,       COLOR_ARCHIVE},

	/* U */

	/* V */
	{"vid",        ICON_EXT_VID,       COLOR_VIDEO},
	{"vim",        ICON_EXT_VIM,       COLOR_VIM},
	{"vimrc",      ICON_EXT_VIMRC,     COLOR_VIM},
	{"vtt",        ICON_EXT_SRT,       0},

	/* W */
	{"wav",        ICON_EXT_WAV,       COLOR_AUDIO},
	{"webm",       ICON_EXT_WEBM,      COLOR_VIDEO},
	{"wma",        ICON_EXT_WMA,       COLOR_AUDIO},
	{"wmv",        ICON_EXT_WMV,       COLOR_VIDEO},

	/* X */
	{"xbps",       ICON_EXT_XBPS,      COLOR_ARCHIVE},
	{"xhtml",      ICON_EXT_XHTML,     0},
	{"xls",        ICON_EXT_XLS,       0},
	{"xlsx",       ICON_EXT_XLSX,      0},
	{"xml",        ICON_EXT_XML,       0},
	{"xz",         ICON_EXT_XZ,        COLOR_ARCHIVE},

	/* Y */
	{"yaml",       ICON_EXT_YAML,      COLOR_DOCUMENT},
	{"yml",        ICON_EXT_YML,       COLOR_DOCUMENT},

	/* Z */
	{"zip",        ICON_EXT_ZIP,       COLOR_ARCHIVE},

	/* Other */
#endif
};
