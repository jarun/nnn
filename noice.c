#include <sys/stat.h>
#include <sys/types.h>

#include <errno.h>
#include <fcntl.h>
#include <dirent.h>
#include <curses.h>
#include <libgen.h>
#include <locale.h>
#include <regex.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>

#ifdef LINUX
#include <bsd/string.h>
#endif

#ifdef DEBUG
#define DEBUG_FD 8
#define DPRINTF_D(x) dprintf(DEBUG_FD, #x "=%d\n", x)
#define DPRINTF_U(x) dprintf(DEBUG_FD, #x "=%u\n", x)
#define DPRINTF_S(x) dprintf(DEBUG_FD, #x "=%s\n", x)
#define DPRINTF_P(x) dprintf(DEBUG_FD, #x "=0x%p\n", x)
#else
#define DPRINTF_D(x)
#define DPRINTF_U(x)
#define DPRINTF_S(x)
#define DPRINTF_P(x)
#endif /* DEBUG */

#define LEN(x) (sizeof(x) / sizeof(*(x)))
#define MIN(x, y) ((x) < (y) ? (x) : (y))
#define ISODD(x) ((x) & 1)

struct assoc {
	char *regex; /* Regex to match on filename */
	char *bin;   /* Program */
};

/* Configuration */
struct assoc assocs[] = {
	{ "\\.(avi|mp4|mkv|mp3|ogg)$", "mplayer" },
	{ "\\.srt$", "less" },
	{ "\\.txt$", "less" },
	{ "\\.sh$", "sh" },
	{ "^README$", "less" },
	{ ".*", "less" },
};

struct entry {
	char *name;
	mode_t mode;
};

#define CWD "cwd: "
#define CURSR " > "
#define EMPTY "   "

/*
 * Layout:
 * .---------
 * | cwd: /mnt/path
 * |
 * |    file0
 * |    file1
 * |  > file2
 * |    file3
 * |    file4
 *      ...
 * |    filen
 * |
 * | Permission denied
 * '------
 */

int die = 0;

char *
openwith(char *file)
{
	regex_t regex;
	char *bin = NULL;
	int i;

	for (i = 0; i < LEN(assocs); i++) {
		if (regcomp(&regex, assocs[i].regex,
			    REG_NOSUB | REG_EXTENDED) != 0)
			continue;
		if (regexec(&regex, file, 0, NULL, 0) != REG_NOMATCH) {
			bin = assocs[i].bin;
			break;
		}
	}
	DPRINTF_S(bin);

	return bin;
}

int
entrycmp(const void *va, const void *vb)
{
	const struct entry *a, *b;

	a = (struct entry *)va;
	b = (struct entry *)vb;

	return strcmp(a->name, b->name);
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
	exitcurses();
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
printent(struct entry *ent, int active)
{
	char *name;
	unsigned int maxlen = COLS - strlen(CURSR) - 1;
	char cm = 0;

	/* Copy name locally */
	name = strdup(ent->name);
	if (name == NULL)
		printerr(1, "strdup name");

	if (S_ISDIR(ent->mode)) {
		cm = '/';
		maxlen--;
	} else if (S_ISLNK(ent->mode)) {
		cm = '@';
		maxlen--;
	}

	/* No text wrapping in entries */
	if (strlen(name) > maxlen)
		name[maxlen] = '\0';

	if (cm == 0)
		printw("%s%s\n", active ? CURSR : EMPTY, name);
	else
		printw("%s%s%c\n", active ? CURSR : EMPTY, name, cm);

	free(name);
}

void
browse(const char *ipath)
{
	DIR *dirp;
	int dfd;
	struct dirent *dp;
	struct entry *dents;
	int i, n, cur;
	int r, ret;
	char *path = strdup(ipath);
	char *cwd;
	struct stat sb;

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
		char *name;

		/* Skip self and parent */
		if (strcmp(dp->d_name, ".") == 0
		    || strcmp(dp->d_name, "..") == 0)
			continue;
		/* Deep copy because readdir(3) reuses the entries */
		dents = realloc(dents, (n + 1) * sizeof(*dents));
		if (dents == NULL)
			printerr(1, "realloc");
		dents[n].name = strdup(dp->d_name);
		if (dents[n].name == NULL)
			printerr(1, "strdup");
		/* Get mode flags */
		asprintf(&name, "%s/%s", path, dents[n].name);
		r = lstat(name, &sb);
		free(name);
		if (r == -1)
			printerr(1, "stat");
		dents[n].mode = sb.st_mode;
		n++;
	}

	qsort(dents, n, sizeof(*dents), entrycmp);

	for (;;) {
		int nlines;
		int maxlen;
		int odd;

redraw:
		nlines = MIN(LINES - 4, n);

		/* Clean screen */
		erase();

		/* Strip trailing slashes */
		for (i = strlen(path) - 1; i > -1; i--)
			if (path[i] == '/')
				path[i] = '\0';
			else
				break;

		DPRINTF_D(cur);
		DPRINTF_S(path);

		/* No text wrapping in cwd line */
		cwd = malloc(COLS * sizeof(char));
		strlcpy(cwd, path, COLS * sizeof(char));
		cwd[COLS - strlen(CWD) - 1] = '\0';

		/* Print cwd.  If empty we are on the root.  We store it
		 * as an empty string so that when we navigate in /mnt
		 * is doesn't come up as //mnt. */
		printw(CWD "%s%s\n\n",
		    strcmp(cwd, "") == 0 ? "/" : "",
		    cwd);

		/* Print listing */
		odd = ISODD(nlines);
		if (cur < nlines / 2) {
			for (i = 0; i < nlines; i++)
				printent(&dents[i], i == cur);
		} else if (cur >= n - nlines / 2) {
			for (i = n - nlines; i < n; i++)
				printent(&dents[i], i == cur);
		} else {
			for (i = cur - nlines / 2;
			     i < cur + nlines / 2 + odd; i++)
				printent(&dents[i], i == cur);
		}

nochange:
		ret = nextsel(&cur, n);
		if (ret == 1) {
			free(path);
			return;
		}
		if (ret == 2) {
			/* Handle root case */
			if (strcmp(path, "") == 0) {
				goto nochange;
			} else {
				char *dir, *tmp;

				dir = dirname(path);
				tmp = malloc(strlen(dir) + 1);
				strlcpy(tmp, dir, strlen(dir) + 1);
				free(path);
				path = tmp;
				goto out;
			}
		}
		if (ret == 3) {
			char *pathnew;
			char *name;
			char *bin;
			pid_t pid;
			int fd;

			/* Cannot descend in empty directories */
			if (n == 0)
				goto nochange;

			name = dents[cur].name;

			asprintf(&pathnew, "%s/%s", path, name);

			DPRINTF_S(name);
			DPRINTF_S(pathnew);

			/* Get path info */
			fd = open(pathnew, O_RDONLY | O_NONBLOCK);
			if (fd == -1) {
				printwarn();
				free(pathnew);
				goto nochange;
			}
			r = fstat(fd, &sb);
			close(fd);
			if (r == -1) {
				printwarn();
				free(pathnew);
				goto nochange;
			}
			DPRINTF_U(sb.st_mode);
			/* Directory */
			if (S_ISDIR(sb.st_mode)) {
				free(path);
				path = pathnew;
				goto out;
			}
			/* Regular file */
			if (S_ISREG(sb.st_mode)) {
				/* Open with */
				bin = openwith(name);
				if (bin == NULL) {
					printmsg("No association");
					free(pathnew);
					goto nochange;
				}

				exitcurses();

				/* Run program */
				pid = fork();
				if (pid == 0)
					execlp(bin, bin, pathnew, NULL);
				else
					waitpid(pid, NULL, 0);

				initcurses();

				free(pathnew);
				goto redraw;
			}
			/* All the rest */
			printmsg("Unsupported file");
			free(pathnew);
			goto nochange;
		}
	}

out:
	for (i = 0; i < n; i++)
		free(dents[i].name);
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
