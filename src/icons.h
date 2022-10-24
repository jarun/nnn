#ifndef INCLUDE_ICONS_H
#define INCLUDE_ICONS_H

#if defined(ICONS_GENERATE) || defined(ICONS_ENABLED)

/*
 * 1st arg = ICONS_IN_TERM
 *
 * 2nd arg = NERD ICONS
 * You can find hex codes for nerd fonts here: https://www.nerdfonts.com/cheat-sheet
 *
 * 3rd arg = EMOJIS
 * You can find a list of emoji here: https://unicode.org/Public/emoji/5.0/emoji-test.txt
 *
 * Any entry with empty icon gets removed by the hash-table generator
 */
#if defined(ICONS_IN_TERM)
	#define ICON_STR(I, N, E) I
	#include "icons-in-terminal.h"
#elif defined(NERD)
	#define ICON_STR(I, N, E) N
#elif defined(EMOJI)
	#define ICON_STR(I, N, E) E
#endif

/*
 * Define a string to be printed before and after the icon
 * Adjust if the icons are not printed properly
 */
#if defined(EMOJI)
	/*
	 * NOTE: As some emojis take up two cells, all of the emoji icons must
	 * be of width 2. Therefore, right pad single-width emoji with a space.
	 */
	#define ICON_SIZE 2
	#define ICON_PADDING_RIGHT " "
#else
	#define ICON_SIZE 1
	#define ICON_PADDING_RIGHT "  "
#endif
#define ICON_PADDING_LEFT  ""
#define ICON_PADDING_LEFT_LEN  (sizeof ICON_PADDING_LEFT  - 1)
#define ICON_PADDING_RIGHT_LEN (sizeof ICON_PADDING_RIGHT - 1)

/* ARROWS */
#define ICON_ARROW_UP      ICON_STR(MD_ARROW_UPWARD, "\uf55c", "⬆")
#define ICON_ARROW_FORWARD ICON_STR(MD_ARROW_FORWARD, "\uf553", "➡")
#define ICON_ARROW_DOWN    ICON_STR(MD_ARROW_DOWNWARD, "\uf544", "⬇")

/* GENERIC */
#define ICON_DIRECTORY     ICON_STR(FA_FOLDER, "\ue5ff", "📂")
#define ICON_FILE          ICON_STR(FA_FILE, "\uf713", "📃")
#define ICON_EXEC          ICON_STR(FA_COG, "\uf144", "⚙️ ")

/* Top level and common icons */
#define ICON_ARCHIVE       ICON_STR("", "\uf410"," 📦")
#define ICON_BRIEFCASE     ICON_STR(FA_BRIEFCASE, "\uf5d5", "💼")
#define ICON_C             ICON_STR(MFIZZ_C, "\ue61e", "🇨 ")
#define ICON_CHANGELOG     ICON_STR(FA_HISTORY, "\uf7d9", "🔺")
#define ICON_CHEADER       ICON_STR(MFIZZ_C, "\uf0fd", ICON_C)
#define ICON_CHESS         ICON_STR("", "\uf639", "")
#define ICON_CLOJURE       ICON_STR(MFIZZ_CLOJURE, "\ue76a", "")
#define ICON_CONFIGURE     ICON_STR(FILE_CONFIG, "\uf423", "🔧")
#define ICON_CONFFOLDER    ICON_STR(FILE_CONFIG, "\ue5fc", "🔧")
#define ICON_CPLUSPLUS     ICON_STR(MFIZZ_CPLUSPLUS, "\ue61d", ICON_C)
#define ICON_CSHARP        ICON_STR(MFIZZ_CSHARP, "\uf81a", "")
#define ICON_DATABASE      ICON_STR(MFIZZ_DATABASE_ALT2, "\uf6b7", "🗃️ ")
#define ICON_DESKTOP       ICON_STR(FA_DESKTOP, "\ufcbe", "🖥️ ")
#define ICON_DOCKER        ICON_STR(FILE_DOCKER, "\uf308", "")
#define ICON_DOCUMENT      ICON_STR(FA_FILE_TEXT_O, "\uf718", "🗒 ")
#define ICON_DOWNLOADS     ICON_STR(FA_DOWNLOAD, "\uf5d7", "📥")
#define ICON_DOWNLOAD      ICON_STR(FA_DOWNLOAD, "\uf43a", "")
#define ICON_ELIXIR        ICON_STR(MFIZZ_ELIXIR, "\ue62d", "💧")
#define ICON_EMACS         ICON_STR(DEV_GNU, "\ue779", "")
#define ICON_ENCRYPT       ICON_STR("", "\uf805", "🔒")
#define ICON_FONT          ICON_STR(FILE_FONT, "\uf031", "")
#define ICON_FSHARP        ICON_STR(DEV_FSHARP, "\ue7a7", "")
#define ICON_GIT           ICON_STR(FA_GIT, "\uf1d3", "")
#define ICON_GIT_FOLDER    ICON_STR(FA_GIT, "\ue5fb", "🌱")
#define ICON_GITHUB        ICON_STR(FA_GITHUB, "\uf408", "")
#define ICON_GITHUB_FOLDER ICON_STR(FA_GITHUB, "\ue5fd", "")
#define ICON_GRUNT         ICON_STR(FILE_GRUNT, "\ue611", "")
#define ICON_GULP          ICON_STR(FILE_GULP, "\ue610", "")
#define ICON_HASKELL       ICON_STR("", "\ue777", "")
#define ICON_HTML          ICON_STR(FA_FILE_CODE_O, "\uf72d", "")
#define ICON_JAVA          ICON_STR(MFIZZ_JAVA, "\ue738", "☕")
#define ICON_JAVASCRIPT    ICON_STR(FA_FILE_CODE_O, "\ue74e", "")
#define ICON_KEY           ICON_STR(FA_KEY, "\ue60a", "")
#define ICON_LICENSE       ICON_STR(FA_COPYRIGHT, "\uf718", "⚖️ ")
#define ICON_LINUX         ICON_STR(FA_LINUX, "\uf83c", "🐧")
#define ICON_LISP          ICON_STR(DEV_GNU, "\uf671", "")
#define ICON_LOCK          ICON_STR(FA_LOCK, "\uf023", "🔒")
#define ICON_MAKEFILE      ICON_STR(FILE_CMAKE, "\uf68c", "🛠 ")
#define ICON_MANUAL        ICON_STR(FILE_MANPAGE, "\uf5bd", "❓")
#define ICON_MS_EXCEL      ICON_STR(FILE_EXCEL, "\uf1c3", ICON_WORDDOC)
#define ICON_MUSIC         ICON_STR(FA_MUSIC, "\uf832", "🎧")
#define ICON_MUSICFILE     ICON_STR(FA_FILE_AUDIO_O, "\uf886", ICON_MUSIC)
#define ICON_NPM           ICON_STR(DEV_NPM, "\ue71e", "")
#define ICON_NODE          ICON_STR(DEV_NODEJS, "\ue718", "")
#define ICON_NODE_FOLDER   ICON_STR(DEV_NODEJS, "\ue5fa", "")
#define ICON_OPTICALDISK   ICON_STR(LINEA_MUSIC_CD, "\ue271", "💿")
#define ICON_PDF           ICON_STR(FA_FILE_PDF_O, "\uf724", "📕")
#define ICON_PKG           ICON_STR(FA_FILE_ARCHIVE_O, "\uf53b", "📦")
#define ICON_PHOTOSHOP     ICON_STR(DEV_PHOTOSHOP, "\ue7b8", ICON_PICTUREFILE)
#define ICON_PICTUREFILE   ICON_STR(FA_FILE_IMAGE_O, "\uf71e", ICON_PICTURES)
#define ICON_PICTURES      ICON_STR(MD_CAMERA_ALT, "\uf753", "🎨")
#define ICON_PLAYLIST      ICON_STR(ICON_MUSICFILE, "\uf831", "")
#define ICON_POWERPOINT    ICON_STR(FILE_POWERPOINT, "\uf726", "📊")
#define ICON_PUBLIC        ICON_STR(FA_INBOX, "\ue5ff", "👀")
#define ICON_PYTHON        ICON_STR(MFIZZ_PYTHON, "\ue235", "🐍")
#define ICON_R             ICON_STR(FILE_R, "\ufcd2", "")
#define ICON_REACT         ICON_STR(FILE_JSX, "\ue625", ICON_JAVASCRIPT)
#define ICON_RUBY          ICON_STR(MFIZZ_RUBY, "\ue23e", "💎")
#define ICON_RUST          ICON_STR(DEV_RUST, "\ue7a8", "")
#define ICON_SASS          ICON_STR(DEV_SASS, "\ue603", "")
#define ICON_SCRIPT        ICON_STR(MFIZZ_SCRIPT, "\ue795", "📜")
#define ICON_SUBTITLE      ICON_STR(FA_COMMENTS_O, "\uf679", "💬")
#define ICON_SWIFT         ICON_STR(DEV_SWIFT, "\ue755", "")
#define ICON_TEMPLATES     ICON_STR(FA_PAPERCLIP, "\ufac6", "📎")
#define ICON_TEX           ICON_STR(FILE_TEX, "\ufb68", ICON_DOCUMENT)
#define ICON_TORRENT       ICON_STR("", "\uf98c", "")
#define ICON_VIDEOFILE     ICON_STR(FA_FILE_MOVIE_O, "\uf72a", ICON_VIDEOS)
#define ICON_VIDEOS        ICON_STR(FA_FILM, "\uf72f", "🎞 ")
#define ICON_VIM           ICON_STR(DEV_VIM, "\ue62b", "")
#define ICON_VUE           ICON_STR("", "\ufd42", "")
#define ICON_WEBPACK       ICON_STR(FILE_WEBPACK, "\ufc29", "")
#define ICON_WORDDOC       ICON_STR(FILE_WORD, "\uf72b", "📘")

#define ICON_EXT_O         ICON_STR("", "\ue624", "")
#define ICON_EXT_ASM       ICON_STR(FILE_NASM, "\ue614", "")
#define ICON_EXT_BIN       ICON_STR(OCT_FILE_BINARY, "\uf471", "📓")
#define ICON_EXT_COFFEE    ICON_STR(MFIZZ_COFFEE_BEAN, "\ue751", "")
#define ICON_EXT_CSS       ICON_STR(MFIZZ_CSS3, "\ue749", "🦋")
#define ICON_EXT_DEB       ICON_STR(MFIZZ_DEBIAN, "\ue77d", ICON_LINUX)
#define ICON_EXT_DIFF      ICON_STR(FILE_DIFF, "\uf440", "📋")
#define ICON_EXT_GO        ICON_STR(MFIZZ_GO, "\ufcd1", "")
#define ICON_EXT_JSON      ICON_STR(ICON_JAVASCRIPT, "\ufb25", ICON_JAVASCRIPT)
#define ICON_EXT_LUA       ICON_STR(FILE_LUA, "\ue620", "🌘")
#define ICON_EXT_M         ICON_STR("", "\ufd1c", "📊")
#define ICON_EXT_MAT       ICON_STR("", "\uf0ce", "")
#define ICON_EXT_MD        ICON_STR(DEV_MARKDOWN, "\ue609", "📝")
#define ICON_EXT_MSI       ICON_STR(FA_WINDOWS, "\uf871", "🪟")
#define ICON_EXT_NIX       ICON_STR("", "\uf313", "")
#define ICON_EXT_PATCH     ICON_STR(FILE_PATCH, "\uf440", "🩹")
#define ICON_EXT_PHP       ICON_STR(MFIZZ_PHP, "\ue608", "🌐")
#define ICON_EXT_ROM       ICON_STR(FA_LOCK, "\uf795", "")
#define ICON_EXT_RSS       ICON_STR(FA_RSS_SQUARE, "\uf143", "📡")
#define ICON_EXT_RTF       ICON_STR(ICON_PDF, "\uf724", ICON_PDF)
#define ICON_EXT_SCALA     ICON_STR(MFIZZ_SCALA, "\ue737", "")
#define ICON_EXT_SLN       ICON_STR(DEV_VISUALSTUDIO, "\ue70c", "")
#define ICON_EXT_TS        ICON_STR(FILE_TS, "\ue628", "")


/*
 * Hex xterm 256 color code, 0 to follow file specific (if any)
 * Codes: https://jonasjacek.github.io/colors/
 * Spectrum sorted: https://upload.wikimedia.org/wikipedia/commons/1/15/Xterm_256color_chart.svg
 * Color names: https://www.ditig.com/256-colors-cheat-sheet
 */
#define COLOR_LIST \
	COLOR_X(COLOR_VIDEO,         45)  /* Turquoise2 */ \
	COLOR_X(COLOR_VIDEO1,       226)  /* Yellow1 */ \
	COLOR_X(COLOR_AUDIO,        220)  /* Gold1 */ \
	COLOR_X(COLOR_AUDIO1,       205)  /* HotPink */ \
	COLOR_X(COLOR_IMAGE,         82)  /* Chartreuse2 */ \
	COLOR_X(COLOR_DOCS,         202)  /* OrangeRed1 */ \
	COLOR_X(COLOR_ARCHIVE,      111)  /* SkyBlue2 */ \
	COLOR_X(COLOR_PKG,          209)  /* Salmon1 */ \
	COLOR_X(COLOR_C,             81)  /* SteelBlue1 */ \
	COLOR_X(COLOR_CPLUSPLUS,    151)  /* DarkSeaGreen2 */ \
	COLOR_X(COLOR_JAVA,          32)  /* DeepSkyBlue3 */ \
	COLOR_X(COLOR_JAVASCRIPT,    47)  /* SpringGreen2 */ \
	COLOR_X(COLOR_REACT,         39)  /* DeepSkyBlue1 */ \
	COLOR_X(COLOR_CSS,          199)  /* DeepPink1 */ \
	COLOR_X(COLOR_PYTHON,       227)  /* LightGoldenrod1 */ \
	COLOR_X(COLOR_LUA,           19)  /* Blue3 */ \
	COLOR_X(COLOR_DOCUMENT,      15)  /* White */ \
	COLOR_X(COLOR_FSHARP,        31)  /* DeepSkyBlue3 */ \
	COLOR_X(COLOR_RUBY,         160)  /* Red3 */ \
	COLOR_X(COLOR_SCALA,        196)  /* Red1 */ \
	COLOR_X(COLOR_SHELL,         47)  /* SpringGreen2 */ \
	COLOR_X(COLOR_VIM,           28)  /* Green4 */ \
	COLOR_X(COLOR_ELIXIR,       104)  /* MediumPurple */ \
	COLOR_X(COLOR_EMACS,        165)  /* Magenta2 */ \
	COLOR_X(COLOR_RUST,         203)  /* IndianRed1 */ \
	COLOR_X(COLOR_KEY,          185)  /* Khaki3 */ \
	COLOR_X(COLOR_LOCK,         102)  /* Gray53 */ \
	COLOR_X(COLOR_CONFIGURE,    181)  /* MistyRose3 */ \
	COLOR_X(COLOR_ASM,          220)  /* Gold1 */ \
	COLOR_X(COLOR_O,            243)  /* Gray46 */ \
	COLOR_X(COLOR_HTML,         147)  /* LightSteelBlue */ \

/* X-Macro: https://en.wikipedia.org/wiki/X_Macro */
#define COLOR_X(N, V) N = (V),
enum { COLOR_LIST };
#undef COLOR_X
#define COLOR_X(N, V) N,
static const unsigned char init_colors[] = { COLOR_LIST };
#undef COLOR_X

#ifdef ICONS_GENERATE
	/* temporary struct using `char *`. the hash-table generator will
	 * output a more optimized version which uses `char[]` instead reducing
	 * indirection and the total binary size.
	 */
	struct icon_pair { const char *match; const char *icon; unsigned char color; };
#endif

struct icon { const char *icon; unsigned char color; };

static const struct icon dir_icon  = {ICON_DIRECTORY, 0};
static const struct icon file_icon = {ICON_FILE, 0};
static const struct icon exec_icon = {ICON_EXEC, 0};

static const struct icon_pair icons_name[] = {
	{".bash_history",      ICON_CONFIGURE,     COLOR_CONFIGURE},
	{".bash_logout",       ICON_SCRIPT,        COLOR_SHELL},
	{".bash_profile",      ICON_CONFIGURE,     COLOR_CONFIGURE},
	{".bashrc",            ICON_SCRIPT,        COLOR_SHELL},
	{".cargo",             ICON_RUST,          COLOR_RUST},
	{".config",            ICON_CONFFOLDER,    COLOR_CONFIGURE},
	{".doom.d",            ICON_EMACS,         COLOR_EMACS},
	{".emacs.d",           ICON_EMACS,         COLOR_EMACS},
	{".git",               ICON_GIT_FOLDER,    0},
	{".github",            ICON_GITHUB_FOLDER, 0},
	{".gitignore",         ICON_GIT,           0},
	{".gnupg",             ICON_LOCK,          COLOR_LOCK},
	{".local",             ICON_CONFFOLDER,    COLOR_CONFIGURE},
	{".node_repl_history", ICON_NODE,          COLOR_JAVASCRIPT},
	{".npmignore",         ICON_NPM,           0},
	{".python_history",    ICON_PYTHON,        COLOR_PYTHON},
	{".release.toml",      ICON_RUST,          COLOR_RUST},
	{".ssh",               ICON_LOCK,          COLOR_LOCK},
	{".viminfo",           ICON_VIM,           COLOR_VIM},
	{".xinitrc",           ICON_CONFIGURE,     COLOR_CONFIGURE},
	{".Xresources",        ICON_CONFIGURE,     COLOR_CONFIGURE},
	{"authorized_keys",    ICON_KEY,           COLOR_KEY},
	{"bspwmrc",            ICON_CONFIGURE,     COLOR_CONFIGURE},
	{"Cargo.toml",         ICON_RUST,          COLOR_RUST},
	{"CHANGELOG",          ICON_CHANGELOG,     COLOR_DOCS},
	{"composer.json",      ICON_EXT_PHP,       COLOR_CPLUSPLUS},
	{"conf.d",             ICON_CONFFOLDER,    COLOR_CONFIGURE},
	{"configure",          ICON_CONFIGURE,     COLOR_CONFIGURE},
	{"config.el",          ICON_EMACS,         COLOR_EMACS},
	{"custom.el",          ICON_EMACS,         COLOR_EMACS},
	{"Desktop",            ICON_DESKTOP,       0},
	{"docker-compose.yml", ICON_DOCKER,        0},
	{"dockerfile",         ICON_DOCKER,        0},
	{"Documents",          ICON_BRIEFCASE,     0},
	{"Downloads",          ICON_DOWNLOADS,     COLOR_O},
	{"funding.yml",        ICON_GITHUB,        0},
	{"gruntfile.coffee",   ICON_GRUNT,         0},
	{"gruntfile.js",       ICON_GRUNT,         0},
	{"gruntfile.ls",       ICON_GRUNT,         0},
	{"gulpfile.coffee",    ICON_GULP,          0},
	{"gulpfile.js",        ICON_GULP,          0},
	{"gulpfile.ls",        ICON_GULP,          0},
	{"init.el",            ICON_EMACS,         COLOR_EMACS},
	{"known_hosts",        ICON_CONFIGURE,     COLOR_CONFIGURE},
	{"License",            ICON_LICENSE,       COLOR_DOCS},
	{"Makefile",           ICON_MAKEFILE,      COLOR_CONFIGURE},
	{"Music",              ICON_MUSIC,         COLOR_AUDIO1},
	{"node_modules",       ICON_NODE_FOLDER,   0},
	{"packages.el",        ICON_EMACS,         COLOR_EMACS},
	{"package.json",       ICON_NODE,          COLOR_JAVASCRIPT},
	{"package-lock.json",  ICON_NODE,          COLOR_JAVASCRIPT},
	{"Pictures",           ICON_PICTURES,      COLOR_IMAGE},
	{"Public",             ICON_PUBLIC,        0},
	{"Rakefile",           ICON_RUBY,          COLOR_RUBY},
	{"rc.lua",             ICON_CONFIGURE,     COLOR_LUA},
	{"sxhkdrc",            ICON_CONFIGURE,     COLOR_CONFIGURE},
	{"Templates",          ICON_TEMPLATES,     0},
	{"Vagrantfile",        ICON_CONFIGURE,     COLOR_CONFIGURE},
	{"Videos",             ICON_VIDEOS,        COLOR_VIDEO},
	{"webpack.config.js",  ICON_WEBPACK,       COLOR_JAVASCRIPT},
	{"xmonad.hs",          ICON_CONFIGURE,     COLOR_CONFIGURE},
};

#ifdef ICONS_GENERATE
/*
 * All entries are case-insensitive
 */
static const struct icon_pair icons_ext[] = {
	/* Numbers */
	{"1",          ICON_MANUAL,         COLOR_DOCS},
	{"7z",         ICON_ARCHIVE,        COLOR_ARCHIVE},

	/* A */
	{"a",          ICON_MANUAL,         0},
	{"ape",        ICON_MUSICFILE,      COLOR_AUDIO},
	{"apk",        ICON_ARCHIVE,        COLOR_ARCHIVE},
	{"asc",        ICON_LOCK,           COLOR_LOCK},
	{"asm",        ICON_EXT_ASM,        COLOR_ASM},
	{"aup",        ICON_MUSICFILE,      COLOR_AUDIO},
	{"avi",        ICON_VIDEOFILE,      COLOR_VIDEO},

	/* B */
	{"bash",       ICON_SCRIPT,         COLOR_SHELL},
	{"bat",        ICON_SCRIPT,         0},
	{"bib",        ICON_TEX,            0},
	{"bin",        ICON_EXT_BIN,        0},
	{"bmp",        ICON_PICTUREFILE,    COLOR_IMAGE},
	{"bz2",        ICON_ARCHIVE,        COLOR_ARCHIVE},

	/* C */
	{"c",          ICON_C,              COLOR_C},
	{"c++",        ICON_CPLUSPLUS,      COLOR_CPLUSPLUS},
	{"cabal",      ICON_HASKELL,        COLOR_VIDEO},
	{"cab",        ICON_ARCHIVE,        COLOR_ARCHIVE},
	{"cbr",        ICON_ARCHIVE,        COLOR_ARCHIVE},
	{"cbz",        ICON_ARCHIVE,        COLOR_ARCHIVE},
	{"cc",         ICON_CPLUSPLUS,      COLOR_CPLUSPLUS},
	{"class",      ICON_JAVA,           COLOR_JAVA},
	{"cl",         ICON_LISP,           0},
	{"clj",        ICON_CLOJURE,        0},
	{"cljc",       ICON_CLOJURE,        0},
	{"cljs",       ICON_CLOJURE,        0},
	{"cls",        ICON_TEX,            0},
	{"cmake",      ICON_MAKEFILE,       0},
	{"coffee",     ICON_EXT_COFFEE,     0},
	{"conf",       ICON_CONFIGURE,      COLOR_CONFIGURE},
	{"cpio",       ICON_ARCHIVE,        COLOR_ARCHIVE},
	{"cpp",        ICON_CPLUSPLUS,      COLOR_CPLUSPLUS},
	{"crate",      ICON_PKG,            COLOR_ARCHIVE},
	{"cs",         ICON_CSHARP,         COLOR_C},
	{"csproj",     ICON_CSHARP,         COLOR_C},
	{"css",        ICON_EXT_CSS,        COLOR_CSS},
	{"csx",        ICON_CSHARP,         COLOR_C},
	{"cue",        ICON_PLAYLIST,       COLOR_AUDIO},
	{"cvs",        ICON_CONFIGURE,      0},
	{"cxx",        ICON_CPLUSPLUS,      COLOR_CPLUSPLUS},

	/* D */
	{"db",         ICON_DATABASE,       0},
	{"deb",        ICON_EXT_DEB,        COLOR_ARCHIVE},
	{"desktop",    ICON_DESKTOP,        0},
	{"diff",       ICON_EXT_DIFF,       0},
	{"dll",        ICON_SCRIPT,         0},
	{"dmg",        ICON_OPTICALDISK,    COLOR_ARCHIVE},
	{"doc",        ICON_WORDDOC,        COLOR_DOCUMENT},
	{"docx",       ICON_WORDDOC,        COLOR_DOCUMENT},
	{"download",   ICON_DOWNLOAD,       COLOR_O},
	{"dump",       ICON_DATABASE,       0},

	/* E */
	{"ejs",        ICON_JAVASCRIPT,     COLOR_JAVASCRIPT},
	{"el",         ICON_LISP,           COLOR_EMACS},
	{"elc",        ICON_LISP,           COLOR_EMACS},
	{"elf",        ICON_LINUX,          0},
	{"epub",       ICON_PDF,            COLOR_DOCS},
	{"exe",        ICON_EXEC,           0},
	{"ex",         ICON_ELIXIR,         COLOR_ELIXIR},
	{"eex",        ICON_ELIXIR,         COLOR_ELIXIR},
	{"exs",        ICON_ELIXIR,         COLOR_ELIXIR},

	/* F */
	{"f#",         ICON_FSHARP,         COLOR_FSHARP},
	{"fen",        ICON_CHESS,          0},
	{"flac",       ICON_MUSICFILE,      COLOR_AUDIO1},
	{"flv",        ICON_VIDEOFILE,      COLOR_VIDEO},
	{"fs",         ICON_FSHARP,         COLOR_FSHARP},
	{"fsi",        ICON_FSHARP,         COLOR_FSHARP},
	{"fsscript",   ICON_FSHARP,         COLOR_FSHARP},
	{"fsx",        ICON_FSHARP,         COLOR_FSHARP},

	/* G */
	{"gem",        ICON_RUBY,           COLOR_RUBY},
	{"gif",        ICON_PICTUREFILE,    COLOR_IMAGE},
	{"go",         ICON_EXT_GO,         COLOR_C},
	{"gpg",        ICON_ENCRYPT,        COLOR_ARCHIVE},
	{"gz",         ICON_ARCHIVE,        COLOR_ARCHIVE},
	{"gzip",       ICON_ARCHIVE,        COLOR_ARCHIVE},

	/* H */
	{"h",          ICON_CHEADER,        COLOR_C},
	{"hh",         ICON_CHEADER,        COLOR_C},
	{"hpp",        ICON_CHEADER,        COLOR_C},
	{"hxx",        ICON_CHEADER,        COLOR_C},
	{"hs",         ICON_HASKELL,        COLOR_VIM},
	{"htaccess",   ICON_CONFIGURE,      COLOR_CONFIGURE},
	{"htpasswd",   ICON_CONFIGURE,      COLOR_CONFIGURE},
	{"htm",        ICON_HTML,           COLOR_HTML},
	{"html",       ICON_HTML,           COLOR_HTML},
	{"heex",       ICON_ELIXIR,         COLOR_ELIXIR},

	/* I */
	{"ico",        ICON_PICTUREFILE,    COLOR_IMAGE},
	{"ini",        ICON_CONFIGURE,      COLOR_CONFIGURE},
	{"img",        ICON_OPTICALDISK,    COLOR_ARCHIVE},
	{"iso",        ICON_OPTICALDISK,    COLOR_ARCHIVE},

	/* J */
	{"jar",        ICON_JAVA,           COLOR_JAVA},
	{"java",       ICON_JAVA,           COLOR_JAVA},
	{"jl",         ICON_CONFIGURE,      COLOR_CONFIGURE},
	{"jpeg",       ICON_PICTUREFILE,    COLOR_IMAGE},
	{"jpg",        ICON_PICTUREFILE,    COLOR_IMAGE},
	{"js",         ICON_JAVASCRIPT,     COLOR_JAVASCRIPT},
	{"json",       ICON_EXT_JSON,       COLOR_JAVASCRIPT},
	{"jsx",        ICON_REACT,          COLOR_REACT},

	/* K */
	{"kbx",        ICON_KEY,            COLOR_KEY},
	{"key",        ICON_KEY,            COLOR_KEY},
	{"ksh",        ICON_SCRIPT,         COLOR_SHELL},

	/* L */
	{"lha",        ICON_ARCHIVE,        COLOR_ARCHIVE},
	{"lhs",        ICON_HASKELL,        COLOR_VIM},
	{"lisp",       ICON_LISP,           0},
	{"lock",       ICON_LOCK,           COLOR_LOCK},
	{"log",        ICON_DOCUMENT,       0},
	{"lua",        ICON_EXT_LUA,        COLOR_LUA},
	{"lzh",        ICON_ARCHIVE,        COLOR_ARCHIVE},
	{"lzma",       ICON_ARCHIVE,        COLOR_ARCHIVE},

	/* M */
	{"m",          ICON_EXT_M,          COLOR_C},
	{"m4a",        ICON_MUSICFILE,      COLOR_AUDIO},
	{"m4v",        ICON_VIDEOFILE,      COLOR_VIDEO},
	{"markdown",   ICON_EXT_MD,         COLOR_DOCS},
	{"mat",        ICON_EXT_MAT,        COLOR_C},
	{"md",         ICON_EXT_MD,         COLOR_DOCS},
	{"md5",        ICON_LOCK,           COLOR_LOCK},
	{"mk",         ICON_MAKEFILE,       0},
	{"mkv",        ICON_VIDEOFILE,      COLOR_VIDEO},
	{"mobi",       ICON_PDF,            COLOR_DOCS},
	{"mov",        ICON_VIDEOFILE,      COLOR_VIDEO},
	{"mp3",        ICON_MUSICFILE,      COLOR_AUDIO},
	{"mp4",        ICON_VIDEOFILE,      COLOR_VIDEO1},
	{"mpeg",       ICON_VIDEOFILE,      COLOR_VIDEO},
	{"mpg",        ICON_VIDEOFILE,      COLOR_VIDEO},
	{"msi",        ICON_EXT_MSI,        0},

	/* N */
	{"nix",        ICON_EXT_NIX,        COLOR_FSHARP},

	/* O */
	{"o",          ICON_EXT_O,          COLOR_O},
	{"ogg",        ICON_MUSICFILE,      COLOR_AUDIO},
	{"org",        ICON_EMACS,          COLOR_EMACS},
	{"opus",       ICON_MUSICFILE,      COLOR_AUDIO},
	{"opdownload", ICON_DOWNLOAD,       COLOR_O},
	{"otf",        ICON_FONT,           COLOR_O},
	{"out",        ICON_LINUX,          0},

	/* P */
	{"part",       ICON_DOWNLOAD,       COLOR_O},
	{"patch",      ICON_EXT_PATCH,      0},
	{"pdf",        ICON_PDF,            COLOR_DOCS},
	{"pgn",        ICON_CHESS,          0},
	{"php",        ICON_EXT_PHP,        COLOR_CPLUSPLUS},
	{"pkg",        ICON_PKG,            COLOR_ARCHIVE},
	{"png",        ICON_PICTUREFILE,    COLOR_IMAGE},
	{"ppt",        ICON_POWERPOINT,     0},
	{"pptx",       ICON_POWERPOINT,     0},
	{"psb",        ICON_PHOTOSHOP,      COLOR_IMAGE},
	{"psd",        ICON_PHOTOSHOP,      COLOR_IMAGE},
	{"pub",        ICON_KEY,            COLOR_KEY},
	{"py",         ICON_PYTHON,        COLOR_PYTHON},
	{"pyc",        ICON_PYTHON,        COLOR_PYTHON},
	{"pyd",        ICON_PYTHON,        COLOR_PYTHON},
	{"pyo",        ICON_PYTHON,        COLOR_PYTHON},

	/* Q */
	{"qcow2",      ICON_OPTICALDISK,   COLOR_ARCHIVE},
	{"qss",        ICON_EXT_CSS,       COLOR_CSS},

	/* R */
	{"r",          ICON_R,              0},
	{"rar",        ICON_ARCHIVE,        COLOR_ARCHIVE},
	{"rb",         ICON_RUBY,           COLOR_RUBY},
	{"rc",         ICON_CONFIGURE,      COLOR_CONFIGURE},
	{"rdata",      ICON_R,              0},
	{"rlib",       ICON_RUST,           COLOR_RUST},
	{"rmeta",      ICON_RUST,           COLOR_RUST},
	{"rom",        ICON_EXT_ROM,        0},
	{"rpm",        ICON_PKG,            COLOR_ARCHIVE},
	{"rs",         ICON_RUST,           COLOR_RUST},
	{"rspec",      ICON_RUBY,           COLOR_RUBY},
	{"rss",        ICON_EXT_RSS,        0},
	{"rtf",        ICON_EXT_RTF,        0},

	/* S */
	{"s",          ICON_EXT_ASM,        COLOR_ASM},
	{"sass",       ICON_SASS,           COLOR_CSS},
	{"scss",       ICON_SASS,           COLOR_CSS},
	{"so",         ICON_EXT_O,          COLOR_O},
	{"scala",      ICON_EXT_SCALA,      COLOR_SCALA},
	{"sh",         ICON_SCRIPT,         COLOR_SHELL},
	{"sha1",       ICON_LOCK,           COLOR_LOCK},
	{"sha256",     ICON_LOCK,           COLOR_LOCK},
	{"sig",        ICON_KEY,            COLOR_KEY},
	{"slim",       ICON_SCRIPT,         COLOR_DOCUMENT},
	{"sln",        ICON_EXT_SLN,        0},
	{"sql",        ICON_DATABASE,       0},
	{"srt",        ICON_SUBTITLE,       0},
	{"sty",        ICON_TEX,            0},
	{"sub",        ICON_SUBTITLE,       0},
	{"svg",        ICON_PICTUREFILE,    COLOR_IMAGE},
	{"swift",      ICON_SWIFT,          0},
	{"swp",        ICON_VIM,            COLOR_VIM},
	{"sym",        ICON_EXT_O,          COLOR_O},

	/* T */
	{"tar",        ICON_ARCHIVE,        COLOR_ARCHIVE},
	{"tex",        ICON_TEX,            0},
	{"tiff",       ICON_PICTUREFILE,    COLOR_IMAGE},
	{"tgz",        ICON_ARCHIVE,        COLOR_ARCHIVE},
	{"toml",       ICON_EXT_JSON,       0},
	{"torrent",    ICON_TORRENT,        COLOR_ARCHIVE},
	{"ts",         ICON_EXT_TS,         COLOR_JAVASCRIPT},
	{"tsx",        ICON_REACT,          COLOR_REACT},
	{"ttc",        ICON_FONT,           COLOR_O},
	{"ttf",        ICON_FONT,           COLOR_O},
	{"txt",        ICON_DOCUMENT,       COLOR_DOCUMENT},
	{"txz",        ICON_ARCHIVE,        COLOR_ARCHIVE},

	/* U */

	/* V */
	{"vid",        ICON_VIDEOFILE,      COLOR_VIDEO},
	{"vim",        ICON_VIM,            COLOR_VIM},
	{"vimrc",      ICON_VIM,            COLOR_VIM},
	{"vtt",        ICON_SUBTITLE,       0},
	{"vue",        ICON_VUE,            COLOR_JAVASCRIPT},

	/* W */
	{"wasm",       ICON_EXT_ASM,        COLOR_HTML},
	{"wat",        ICON_EXT_BIN,        COLOR_HTML},
	{"wav",        ICON_MUSICFILE,      COLOR_AUDIO},
	{"webm",       ICON_VIDEOFILE,      COLOR_VIDEO},
	{"webp",       ICON_PICTUREFILE,    COLOR_IMAGE},
	{"wma",        ICON_VIDEOFILE,      COLOR_AUDIO},
	{"wmv",        ICON_VIDEOFILE,      COLOR_VIDEO},

	/* X */
	{"xbps",       ICON_PKG,            COLOR_ARCHIVE},
	{"xcf",        ICON_PICTUREFILE,    COLOR_IMAGE},
	{"xhtml",      ICON_HTML,           COLOR_HTML},
	{"xls",        ICON_MS_EXCEL,       0},
	{"xlsx",       ICON_MS_EXCEL,       0},
	{"xml",        ICON_HTML,           COLOR_HTML},
	{"xz",         ICON_ARCHIVE,        COLOR_ARCHIVE},

	/* Y */
	{"yaml",       ICON_CONFIGURE,      COLOR_DOCUMENT},
	{"yml",        ICON_CONFIGURE,      COLOR_DOCUMENT},

	/* Z */
	{"zip",        ICON_ARCHIVE,        COLOR_ARCHIVE},
	{"zsh",        ICON_SCRIPT,         COLOR_SHELL},
	{"zshrc",      ICON_SCRIPT,         COLOR_SHELL},
	{"zsh-theme",  ICON_SCRIPT,         COLOR_SHELL},
	{"zst",        ICON_ARCHIVE,        COLOR_ARCHIVE},

	/* Other */
};
#endif

#endif /* defined(ICONS_GENERATE) || defined(ICONS_ENABLED) */

#endif /* INCLUDE_ICONS_H */
