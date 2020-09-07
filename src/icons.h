/*
 * This file loads in variables either from icons-in-terminal.h or nerdfont.h.
 * Only include one and not the other.
 * If you are a Vim user you are likely already using a patched font and Nerdfont will be easier
*/

// #include "icons-in-terminal.h"
#include "icons-nerdfont.h"

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
#define COLOR_MUSIC        220 /* Gold1 */
#define COLOR_PICTURE      82  /* Chartreuse2 */
#define COLOR_MANUAL       202 /* OrangeRed1 */
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
#define RUBY_COLOR         160 /* Red3 */
#define COLOR_SCALA        196 /* Red1 */
#define COLOR_VIM          28  /* Green4 */

/*
 * Using symbols defined in icons-in-terminal.h, or even using icons-in-terminal is not necessary.
 * You can use whatever pathched font you like. You just have to put the desired icon as a string.
 * If you are using icons-in-terminal the creator recommends that you do use the symbols in the generated header.
 */

static const struct icon_pair dir_icon  = {"", ICON_DIRECTORY, 0};
static const struct icon_pair file_icon = {"", ICON_FILE, 0};
static const struct icon_pair exec_icon = {"", ICON_EXEC,    0};

/* All entries are case-insensitive */

static const struct icon_pair icons_name[] = {
	{".git",         ICON_GIT,       0},
	{"Desktop",      ICON_DESKTOP,   0},
	{"Documents",    ICON_BRIEFCASE, 0},
	{"Downloads",    ICON_DOWNLOADS, 0},
	{"Music",        ICON_MUSIC,     0},
	{"Pictures",     ICON_PICTURES,  0},
	{"Public",       ICON_PUBLIC,    0},
	{"Templates",    ICON_TEMPLATES, 0},
	{"Videos",       ICON_VIDEOS,    0},
	{"CHANGELOG",    ICON_CHANGELOG, 0},
	{"configure",    ICON_CONFIGURE, 0},
	{"License",      ICON_LICENSE,   0},
	{"Makefile",     ICON_MAKEFILE,  0},
};

/*
 * New entries should bu added such that the first character of the extension is in the correct group .
 * This is done for performance reason so that the correct icon can be found faster.
 * All entries are case-insensitive
 */

static const struct icon_pair icons_ext[] = {
	/* Numbers */
	{"1",          ICON_EXT_1,         0},
	{"7z",         ICON_EXT_7Z,        COLOR_ARCHIVE},

	/* A */
	{"a",          ICON_EXT_A,         0},
	{"apk",        ICON_EXT_APK,       COLOR_ARCHIVE},
	{"asm",        ICON_EXT_ASM,       0},
	{"aup",        ICON_EXT_AUP,       COLOR_MUSIC},
	{"avi",        ICON_EXT_AVI,       COLOR_VIDEO},

	/* B */
	{"bat",        ICON_EXT_BAT,       0},
	{"bin",        ICON_EXT_BIN,       0},
	{"bmp",        ICON_EXT_BMP,       COLOR_PICTURE},
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
	{"cue",        ICON_EXT_CUE,       COLOR_MUSIC},
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
	{"epub",       ICON_EXT_EPUB,      COLOR_MANUAL},
	{"exe",        ICON_EXT_EXE,       0},

	/* F */
	{"f#",         ICON_EXT_FSHARP,    COLOR_FSHARP},
	{"flac",       ICON_EXT_FLAC,      COLOR_MUSIC},
	{"flv",        ICON_EXT_FLV,       COLOR_VIDEO},
	{"fs",         ICON_EXT_FS,        COLOR_FSHARP},
	{"fsi",        ICON_EXT_FSI,       COLOR_FSHARP},
	{"fsscript",   ICON_EXT_FSSCRIPT,  COLOR_FSHARP},
	{"fsx",        ICON_EXT_FSX,       COLOR_FSHARP},

	/* G */
	{"gem",        ICON_EXT_GEM,       RUBY_COLOR},
	{"gif",        ICON_EXT_GIF,       COLOR_PICTURE},
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
	{"ico",        ICON_EXT_ICO,       COLOR_PICTURE},
	{"img",        ICON_EXT_IMG,       COLOR_ARCHIVE},
	{"ini",        ICON_EXT_INI,       0},
	{"iso",        ICON_EXT_ISO,       COLOR_ARCHIVE},

	/* J */
	{"jar",        ICON_EXT_JAR,       COLOR_JAVA},
	{"java",       ICON_EXT_JAVA,      COLOR_JAVA},
	{"jl",         ICON_EXT_JL,        0},
	{"jpeg",       ICON_EXT_JPEG,      COLOR_PICTURE},
	{"jpg",        ICON_EXT_JPG,       COLOR_PICTURE},
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
	{"m4a",        ICON_EXT_M4A,       COLOR_MUSIC},
	{"m4v",        ICON_EXT_M4V,       COLOR_VIDEO},
	{"markdown",   ICON_EXT_MD,        0},
	{"md",         ICON_EXT_MD,        0},
	{"mk",         ICON_EXT_MK,        0},
	{"mkv",        ICON_EXT_MKV,       COLOR_VIDEO},
	{"mov",        ICON_EXT_MOV,       COLOR_VIDEO},
	{"mp3",        ICON_EXT_MP3,       COLOR_MUSIC},
	{"mp4",        ICON_EXT_MP4,       COLOR_VIDEO},
	{"mpeg",       ICON_EXT_MPEG,      COLOR_VIDEO},
	{"mpg",        ICON_EXT_MPG,       COLOR_VIDEO},
	{"msi",        ICON_EXT_MSI,       0},

	/* N */

	/* O */
	{"o",          ICON_EXT_O,         0},
	{"ogg",        ICON_EXT_OGG,       COLOR_MUSIC},
	{"opdownload", ICON_EXT_ODOWNLOAD, 0},
	{"out",        ICON_EXT_OUT,       0},

	/* P */
	{"part",       ICON_EXT_PART,      0},
	{"patch",      ICON_EXT_PATCH,     0},
	{"pdf",        ICON_EXT_PDF,       COLOR_MANUAL},
	{"php",        ICON_EXT_PHP,       0},
	{"png",        ICON_EXT_PNG,       COLOR_PICTURE},
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
	{"svg",        ICON_EXT_SVG,       COLOR_PICTURE},

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

	/* W */
	{"wav",        ICON_EXT_WAV,       COLOR_MUSIC},
	{"webm",       ICON_EXT_WEBM,      COLOR_VIDEO},
	{"wma",        ICON_EXT_WMA,       COLOR_MUSIC},
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
	{"zip",        ICON_EXT_ZIP,       COLOR_ARCHIVE}

	/* Other */
};
