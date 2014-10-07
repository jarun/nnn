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
#define MIN(x, y) ((x) < (y) ? (x) : (y))
#define ISODD(x) ((x) & 1)

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

struct entry {
	char name[MAXNAMLEN + 1];
};

struct assoc {
	char *ext; /* Extension */
	char *bin; /* Program */
} assocs[] = {
	{ ".avi", "mplayer" },
	{ ".mp4", "mplayer" },
	{ ".mkv", "mplayer" },
	{ ".mp3", "mplayer" },
	{ ".ogg", "mplayer" },
	{ ".srt", "less" },
	{ ".txt", "less" },
	{ "README", "less" },
};

char *
openwith(char *file)
{
	char *ext = NULL;
	char *bin = NULL;
	int i;

	ext = strrchr(file, '.');
	if (ext == NULL)
		ext = file;
	DPRINTF_S(ext);

	for (i = 0; i < LEN(assocs); i++)
		if (strncmp(assocs[i].ext, ext, strlen(ext) + 1) == 0)
			bin = assocs[i].bin;
	DPRINTF_S(bin);

	return bin;
}

int
dentcmp(const void *va, const void *vb)
{
	const struct dirent *a, *b;

	a = *(struct dirent **)va;
	b = *(struct dirent **)vb;

	return strcmp(a->d_name, b->d_name);
}

void
initcurses(void)
{
	initscr();
	cbreak();
	noecho();
	nonl();
	intrflush(stdscr, FALSE);
	keypad(stdscr, TRUE);
	curs_set(FALSE); /* Hide cursor */
}

void
exitcurses(void)
{
	endwin(); /* Restore terminal */
}

/* Messages show up at the bottom */
void
printmsg(char *msg)
{
	move(LINES - 1, 0);
	printw("%s\n", msg);
}

/* Display warning as a message */
void
printwarn(void)
{
	printmsg(strerror(errno));
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
	char *cwd;

begin:
	/* Path should be a malloc(3)-ed string at all times */
	n = 0;
	cur = 0;
	dents = NULL;

	dirp = opendir(path); 
	if (dirp == NULL) {
		printwarn();
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
		int nlines;
		struct entry *tmpents;
		int odd;

redraw:
		nlines = MIN(LINES - 4, n);

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

#define CWD "cwd: "
#define CURSR " > "
#define EMPTY "   "

		/* No text wrapping in cwd line */
		cwd = malloc(COLS * sizeof(char));
		strncpy(cwd, path, COLS);
		cwd[COLS - strlen(CWD) - 1] = '\0';

		/* No text wrapping in entries */
		tmpents = malloc(n * sizeof(*tmpents));
		for (i = 0; i < n; i++) {
			strncpy(tmpents[i].name, dents[i]->d_name,
			    sizeof(tmpents[i].name));
			tmpents[i].name[COLS - strlen(CURSR) - 1] = '\0';
		}

		/* Print cwd */
		printw(CWD "%s%s\n\n",
		    strncmp(cwd, "", 1) == 0 ? "/" : "",
		    cwd);

		/* Print listing */
		odd = ISODD(nlines);
		if (cur < nlines / 2) {
			for (i = 0; i < nlines; i++)
				printw("%s%s\n",
				    i == cur ? CURSR : EMPTY,
				    tmpents[i].name);
		} else if (cur >= n - nlines / 2) {
			for (i = n - nlines; i < n; i++)
				printw("%s%s\n",
				    i == cur ? CURSR : EMPTY,
				    tmpents[i].name);
		} else {
			for (i = cur - nlines / 2;
			     i < cur + nlines / 2 + odd; i++)
				printw("%s%s\n",
				    i == cur ? CURSR : EMPTY,
				    tmpents[i].name);
		}

		free(tmpents);

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
				char *dir, *tmp;

				dir = dirname(path);
				tmp = malloc(strlen(dir) + 1);
				strncpy(tmp, dir, strlen(dir) + 1);
				free(path);
				path = tmp;
				goto out;
			}
		}
		if (ret == 3) {
			char *name, *file = NULL;
			char *newpath;
			char *bin;
			pid_t pid;

			/* Cannot descend in empty directories */
			if (n == 0)
				goto nochange;

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
					printwarn();
					free(newpath);
					goto nochange;
				}
			case DT_REG:
				file = malloc(strlen(path) + 1
				    + strlen(name) + 1);
				sprintf(file, "%s/%s", path, name);
				DPRINTF_S(file);

				/* Open with */
				bin = openwith(name);
				if (bin == NULL) {
					printmsg("No association");
					goto nochange;
				}

				exitcurses();

				/* Run program */
				pid = fork();
				if (pid == 0)
					execlp(bin, bin, file, NULL);
				else
					waitpid(pid, NULL, 0);

				initcurses();

				free(file);

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

	initcurses();

	browse(ipath);

	exitcurses();

	return 0;
}
