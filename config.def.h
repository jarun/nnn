#define CWD "cwd: "
#define CURSR " > "
#define EMPTY "   "

struct assoc assocs[] = {
	{ "\\.(avi|mp4|mkv|mp3|ogg|flac)$", "mplayer" },
	{ "\\.(png|jpg|gif)$", "feh" },
	{ "\\.(html|svg)$", "firefox" },
	{ "\\.pdf$", "mupdf" },
	{ "\\.sh$", "sh" },
	{ ".*", "less" },
};
