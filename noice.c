#include <sys/types.h>

#include <errno.h>
#include <fcntl.h>
#include <dirent.h>
#include <curses.h>
#include <libgen.h>
#include <locale.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>

#ifdef DEBUG
#define DPRINTF_D(x) printw(#x "=%d\n", x)
#define DPRINTF_S(x) printw(#x "=%s\n", x)
#define DPRINTF_P(x) printw(#x "=0x%p\n", x)
#else
#define DPRINTF_D(x)
#define DPRINTF_S(x)
#define DPRINTF_P(x)
#endif /* DEBUG */

#define LEN(x) (sizeof(x) / sizeof(*(x)))

/*
 * Layout:
 * .---------
 * | cwd: /mnt/path
 * |
 * |  > file0
 * |    file1
 *      ...
 * |    filen
 * |
 * | msg: invalid extension
 * '------
 */

int die = 0;

struct assoc {
	char *ext;
	char *bin;
} assocs[] = {
	{ "avi", "mplayer" },
	{ "mp4", "mplayer" },
	{ "mkv", "mplayer" },
	{ "mp3", "mplayer" },
	{ "ogg", "mplayer" },
	{ "srt", "less" },
	{ "txt", "less" },
};

char *
extension(char *file)
{
	char *dot;

	dot = strrchr(file, '.');
	if (dot == NULL || dot == file)
		return NULL;
	else
		return dot + 1;
}

char *
openwith(char *ext)
{
	int i;

	for (i = 0; i < LEN(assocs); i++)
		if (strncmp(assocs[i].ext, ext, strlen(ext)) == 0)
			return assocs[i].bin;
	return NULL;
}

int
dentcmp(const void *va, const void *vb)
{
	const struct dirent *a, *b;

	a = *(struct dirent **)va;
	b = *(struct dirent **)vb;

	return strcmp(a->d_name, b->d_name);
}

/* Warning shows up at the bottom */
void
printwarn(char *prefix)
{
	move(LINES - 1, 0);
	printw("%s: %s\n", prefix, strerror(errno));
}

/* Kill curses and display error before exiting */
void
printerr(int ret, char *prefix)
{
	endwin();
	printf("%s: %s\n", prefix, strerror(errno));
	exit(ret);
}

/*
 * Returns 0 normally
 * On movement it updates *cur
 * Returns 1 on quit
 * Returns 2 on go in
 * Returns 3 on go up
 */
int
nextsel(int *cur, int max)
{
	int c;

	c = getch();
	switch (c) {
	case 'q':
		return 1;
	/* go up */
	case KEY_BACKSPACE:
	case KEY_LEFT:
	case 'h':
		return 2;
	/* go in */
	case KEY_ENTER:
	case '\r':
	case KEY_RIGHT:
	case 'l':
		return 3;
	/* next */
	case 'j':
	case KEY_DOWN:
		if (*cur < max - 1)
			(*cur)++;
		break;
	/* prev */
	case 'k':
	case KEY_UP:
		if (*cur > 0)
			(*cur)--;
		break;
	}

	return 0;
}

int
testopen(char *path)
{
	int fd;

	fd = open(path, O_RDONLY);
	if (fd == -1) {
		return 0;
	} else {
		close(fd);
		return 1;
	}
}

int
testopendir(char *path)
{
	DIR *dirp;

	dirp = opendir(path);
	if (dirp == NULL) {
		return 0;
	} else {
		closedir(dirp);
		return 1;
	}
}

void
browse(const char *ipath)
{
	DIR *dirp;
	struct dirent *dp;
	struct dirent **dents;
	int i, n, cur;
	int r, ret;
	char *path = strdup(ipath);

begin:
	/* Path is a malloc(3)-ed string */
	n = 0;
	cur = 0;
	dents = NULL;

	dirp = opendir(path); 
	if (dirp == NULL) {
		printwarn("opendir");
		goto nochange;
	}

	while ((dp = readdir(dirp)) != NULL) {
		/* Skip self and parent */
		if (strncmp(dp->d_name, ".", 2) == 0
		    || strncmp(dp->d_name, "..", 3) == 0)
			continue;
		dents = realloc(dents, (n + 1) * sizeof(*dents));
		if (dents == NULL)
			printerr(1, "realloc");
		dents[n] = dp;
		n++;
	}

	qsort(dents, n, sizeof(*dents), dentcmp);

	for (;;) {
redraw:
		/* Clean screen */
		erase();

		/* Strip slashes */
		for (i = strlen(path) - 1; i > -1; i--)
			if (path[i] == '/')
				path[i] = '\0';
			else
				break;

		DPRINTF_D(cur);
		DPRINTF_S(path);

		/* Print cwd */
		printw("cwd: %s%s\n\n",
		    strncmp(path, "", 1) == 0 ? "/" : "",
		    path);

		/* Print listing */
		for (i = 0; i < n; i++)
			printw(" %s %s\n",
			    i == cur ? ">" : " ",
			    dents[i]->d_name);

nochange:
		ret = nextsel(&cur, n);
		if (ret == 1) {
			free(path);
			return;
		}
		if (ret == 2) {
			/* Handle root case */
			if (strncmp(path, "", 1) == 0) {
				goto nochange;
			} else {
				path = dirname(path);
				goto out;
			}
		}
		if (ret == 3) {
			char *name, *file = NULL;
			char *newpath;
			char *ext, *bin;
			pid_t pid;

			name = dents[cur]->d_name;

			switch (dents[cur]->d_type) {
			case DT_DIR:
				newpath = malloc(strlen(path) + 1
				    + strlen(name) + 1);
				sprintf(newpath, "%s/%s", path, name);
				if (testopen(newpath)) {
					free(path);
					path = newpath;
					goto out;
				} else {
					printwarn(newpath);
					free(newpath);
					goto nochange;
				}
			case DT_REG:
				file = malloc(strlen(path) + 1
				    + strlen(name) + 1);
				sprintf(file, "%s/%s", path, name);
				DPRINTF_S(file);

				/* Open with */
				ext = extension(name);
				if (ext == NULL) {
					printwarn("invalid extension\n");
					goto nochange;
				}
				bin = openwith(ext);
				if (bin == NULL) {
					printwarn("no association\n");
					goto nochange;
				}
				DPRINTF_S(ext);
				DPRINTF_S(bin);

				/* Run program */
				pid = fork();
				if (pid == 0)
					execlp(bin, bin, file, NULL);
				else
					waitpid(pid, NULL, 0);

				free(file);

				/* Screen may be messed up */
				clear();
				/* Some programs reset this */
				keypad(stdscr, TRUE);
				goto redraw;
			default:
				DPRINTF_D(dents[cur]->d_type);
			}
		}
	}

out:
	free(dents);

	r = closedir(dirp); 
	if (r == -1)
		printerr(1, "closedir");

	goto begin;
}

int
main(int argc, char *argv[])
{
	char *ipath = argv[1] != NULL ? argv[1] : "/";

	/* Test initial path */
	if (!testopendir(ipath))
		printerr(1, ipath);

	/* Set locale before curses setup */
	setlocale(LC_ALL, "");

	/* Init curses */
	initscr();
	cbreak();
	noecho();
	nonl();
	intrflush(stdscr, FALSE);
	keypad(stdscr, TRUE);
	curs_set(FALSE); /* Hide cursor */

	browse(ipath);

	endwin(); /* Restore terminal */

	return 0;
}
