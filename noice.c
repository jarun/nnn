#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <errno.h>
#include <fcntl.h>
#include <dirent.h>
#include <curses.h>
#include <libgen.h>
#include <limits.h>
#include <locale.h>
#include <regex.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>

#include "queue.h"
#include "util.h"

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
#define CONTROL(c) ((c) ^ 0x40)

struct assoc {
	char *regex; /* Regex to match on filename */
	char *bin;   /* Program */
};

#include "config.h"

struct entry {
	char *name;
	mode_t mode;
};

struct history {
	int pos;
	SLIST_ENTRY(history) entry;
};

SLIST_HEAD(histhead, history) histhead = SLIST_HEAD_INITIALIZER(histhead);

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

void printmsg(char *msg);
void printwarn(void);
void printerr(int ret, char *prefix);

void *
xmalloc(size_t size)
{
	void *p;

	p = malloc(size);
	if (p == NULL)
		printerr(1, "malloc");
	return p;
}

void *
xrealloc(void *p, size_t size)
{
	p = realloc(p, size);
	if (p == NULL)
		printerr(1, "realloc");
	return p;
}

char *
xstrdup(const char *s)
{
	char *p;

	p = strdup(s);
	if (p == NULL)
		printerr(1, "strdup");
	return p;
}

char *
xrealpath(const char *path)
{
	char *p;

	p = realpath(path, NULL);
	if (p == NULL)
		printerr(1, "realpath");
	return p;
}

char *
xdirname(const char *path)
{
	char *p, *tmp;

	/* Some implementations of dirname(3) may modify `path' and some
	 * return a pointer inside `path` and we cannot free(3) the
	 * original string if we lose track of it. */
	tmp = xstrdup(path);
	p = dirname(tmp);
	if (p == NULL) {
		free(tmp);
		printerr(1, "dirname");
	}
	/* Make sure this is a malloc(3)-ed string */
	p = xstrdup(p);
	free(tmp);
	return p;
}

void
spawn(const char *file, const char *arg)
{
	pid_t pid;
	int status;

	pid = fork();
	if (pid == 0) {
		execlp(file, file, arg, NULL);
		_exit(1);
	} else {
		/* Ignore interruptions */
		while (waitpid(pid, &status, 0) == -1)
			DPRINTF_D(status);
		DPRINTF_D(pid);
	}
}

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
setfilter(regex_t *regex, char *filter)
{
	char *errbuf;
	int r;

	r = regcomp(regex, filter, REG_NOSUB | REG_EXTENDED);
	if (r != 0) {
		errbuf = xmalloc(COLS * sizeof(char));
		regerror(r, regex, errbuf, COLS * sizeof(char));
		printmsg(errbuf);
		free(errbuf);
	}

	return r;
}

int
visible(regex_t *regex, char *file)
{
	if (regexec(regex, file, 0, NULL, 0) != REG_NOMATCH)
		return 1;
	return 0;
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
	fprintf(stderr, "%s: %s\n", prefix, strerror(errno));
	exit(ret);
}

/* Clear the last line */
void
clearprompt(void)
{
	printmsg("");
}

/* Print prompt on the last line */
void
printprompt(char *str)
{
	clearprompt();
	printw(str);
}

/*
 * Returns 0 normally
 * On movement it updates *cur
 * Returns SEL_{QUIT,BACK,GOIN,FLTR,SH,CD} otherwise
 */
enum {
	SEL_QUIT = 1,
	SEL_BACK,
	SEL_GOIN,
	SEL_FLTR,
	SEL_SH,
	SEL_CD,
};

int
nextsel(int *cur, int max)
{
	int c;

	c = getch();
	switch (c) {
	case 'q':
		return SEL_QUIT;
	/* Back */
	case KEY_BACKSPACE:
	case KEY_LEFT:
	case 'h':
		return SEL_BACK;
	/* Inside */
	case KEY_ENTER:
	case '\r':
	case KEY_RIGHT:
	case 'l':
		return SEL_GOIN;
	/* Filter */
	case '/':
	case '&':
		return SEL_FLTR;
	/* Next */
	case 'j':
	case KEY_DOWN:
	case CONTROL('N'):
		if (*cur < max - 1)
			(*cur)++;
		break;
	/* Previous */
	case 'k':
	case KEY_UP:
	case CONTROL('P'):
		if (*cur > 0)
			(*cur)--;
		break;
	/* Page down */
	case KEY_NPAGE:
	case CONTROL('D'):
		if (*cur < max -1)
			(*cur) += MIN((LINES - 4) / 2, max - 1 - *cur);
		break;
	/* Page up */
	case KEY_PPAGE:
	case CONTROL('U'):
		if (*cur > 0)
			(*cur) -= MIN((LINES - 4) / 2, *cur);
		break;
	case '!':
		return SEL_SH;
	case 'c':
		return SEL_CD;
	}

	return 0;
}

char *
readln(void)
{
	int c;
	int i = 0;
	char *ln = NULL;
	int y, x, x0;

	echo();
	curs_set(TRUE);

	/* Starting point */
	getyx(stdscr, y, x);
	x0 = x;

	while (c = getch()) {
		if (c == KEY_ENTER || c == '\r')
			break;
		if (c == KEY_BACKSPACE) {
			getyx(stdscr, y, x);
			if (x >= x0) {
				if (i > 0) {
					ln = xrealloc(ln, (i - 1) * sizeof(*ln));
					i--;
				} else {
					free(ln);
					ln = NULL;
				}
				move(y, x);
				printw("%c", ' ');
				move(y, x);
			} else {
				move(y, x0);
			}
			continue;
		}
		ln = xrealloc(ln, (i + 1) * sizeof(*ln));
		ln[i] = c;
		i++;
	}
	if (ln != NULL) {
		ln = xrealloc(ln, (i + 1) * sizeof(*ln));
		ln[i] = '\0';
	}

	curs_set(FALSE);
	noecho();

	return ln;
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
	name = xstrdup(ent->name);

	if (S_ISDIR(ent->mode)) {
		cm = '/';
		maxlen--;
	} else if (S_ISLNK(ent->mode)) {
		cm = '@';
		maxlen--;
	} else if (ent->mode & S_IXUSR) {
		cm = '*';
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

int
dentfill(DIR *dirp, struct entry **dents,
	 int (*filter)(regex_t *, char *), regex_t *re)
{
	struct dirent *dp;
	struct stat sb;
	int n = 0;
	int r;

	while ((dp = readdir(dirp)) != NULL) {
		/* Skip self and parent */
		if (strcmp(dp->d_name, ".") == 0
		    || strcmp(dp->d_name, "..") == 0)
			continue;
		if (filter(re, dp->d_name) == 0)
			continue;
		*dents = xrealloc(*dents, (n + 1) * sizeof(**dents));
		(*dents)[n].name = xstrdup(dp->d_name);
		/* Get mode flags */
		r = fstatat(dirfd(dirp), dp->d_name, &sb,
			    AT_SYMLINK_NOFOLLOW);
		if (r == -1)
			printerr(1, "stat");
		(*dents)[n].mode = sb.st_mode;
		n++;
	}

	return n;
}

void
dentfree(struct entry *dents, int n)
{
	int i;

	for (i = 0; i < n; i++)
		free(dents[i].name);
	free(dents);
}

void
browse(const char *ipath, const char *ifilter)
{
	DIR *dirp;
	struct entry *dents;
	int i, n, cur;
	int r, ret;
	char *path = xrealpath(ipath);
	char *filter = xstrdup(ifilter);
	regex_t filter_re;
	char *cwd;
	struct stat sb;

	cur = 0;
begin:
	dirp = opendir(path);
	if (dirp == NULL) {
		printwarn();
		goto nochange;
	} else {
		if (chdir(path) == -1)
			printwarn();
	}

	/* Search filter */
	r = setfilter(&filter_re, filter);
	if (r != 0)
		goto nochange;

	dents = NULL;
	n = dentfill(dirp, &dents, visible, &filter_re);

	/* Make sure cur is in range */
	cur = MIN(cur, n - 1);

	qsort(dents, n, sizeof(*dents), entrycmp);

	for (;;) {
		int nlines;
		int odd;
		char *pathnew;
		char *name;
		char *bin;
		char *dir;
		char *tmp;
		regex_t re;
		struct history *hist;

redraw:
		nlines = MIN(LINES - 4, n);

		/* Clean screen */
		erase();

		DPRINTF_D(cur);
		DPRINTF_S(path);

		/* No text wrapping in cwd line */
		cwd = xmalloc(COLS * sizeof(char));
		strlcpy(cwd, path, COLS * sizeof(char));
		cwd[COLS - strlen(CWD) - 1] = '\0';

		printw(CWD "%s\n\n", cwd);

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
		switch (ret) {
		case SEL_QUIT:
			free(path);
			free(filter);
			/* Forget history */
			while (!SLIST_EMPTY(&histhead)) {
				hist = SLIST_FIRST(&histhead);
				SLIST_REMOVE_HEAD(&histhead, entry);
				free(hist);
			}
			return;
		case SEL_BACK:
			/* There is no going back */
			if (strcmp(path, "/") == 0) {
				goto nochange;
			} else {
				dir = xdirname(path);
				free(path);
				path = dir;
				free(filter);
				filter = xstrdup(ifilter); /* Reset filter */
				/* Recall history */
				hist = SLIST_FIRST(&histhead);
				if (hist != NULL) {
					cur = hist->pos;
					DPRINTF_D(hist->pos);
					SLIST_REMOVE_HEAD(&histhead, entry);
					free(hist);
				} else {
					cur = 0;
				}
				goto out;
			}
		case SEL_GOIN:
			/* Cannot descend in empty directories */
			if (n == 0)
				goto nochange;

			name = dents[cur].name;

			/* Handle root case */
			if (strcmp(path, "/") == 0)
				asprintf(&pathnew, "/%s", name);
			else
				asprintf(&pathnew, "%s/%s", path, name);

			DPRINTF_S(name);
			DPRINTF_S(pathnew);

			/* Get path info */
			r = stat(pathnew, &sb);
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
				free(filter);
				filter = xstrdup(ifilter); /* Reset filter */
				/* Save history */
				hist = xmalloc(sizeof(struct history));
				hist->pos = cur;
				SLIST_INSERT_HEAD(&histhead, hist, entry);
				cur = 0;
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
				spawn(bin, pathnew);
				initcurses();
				free(pathnew);
				goto redraw;
			}
			/* All the rest */
			printmsg("Unsupported file");
			free(pathnew);
			goto nochange;
		case SEL_FLTR:
			/* Read filter */
			printprompt("filter: ");
			tmp = readln();
			if (tmp == NULL) {
				clearprompt();
				goto nochange;
			}
			r = setfilter(&re, tmp);
			if (r != 0) {
				free(tmp);
				goto nochange;
			}
			free(filter);
			filter = tmp;
			filter_re = re;
			DPRINTF_S(filter);
			cur = 0;
			goto out;
		case SEL_SH:
			exitcurses();
			spawn("/bin/sh", NULL);
			initcurses();
			break;
		case SEL_CD:
			/* Read target dir */
			printprompt("chdir: ");
			tmp = readln();
			if (tmp == NULL) {
				clearprompt();
				goto nochange;
			}
			if (testopendir(tmp) == 0) {
				printwarn();
				goto nochange;
			} else {
				free(path);
				path = xrealpath(tmp);
				free(tmp);
				free(filter);
				filter = xstrdup(ifilter); /* Reset filter */
				/* Forget history */
				while (!SLIST_EMPTY(&histhead)) {
					hist = SLIST_FIRST(&histhead);
					SLIST_REMOVE_HEAD(&histhead, entry);
					free(hist);
				}
				DPRINTF_S(path);
				cur = 0;
				goto out;
			}
		}
	}

out:
	dentfree(dents, n);

	/* Should never be null */
	r = closedir(dirp);
	if (r == -1)
		printerr(1, "closedir");

	goto begin;
}

int
main(int argc, char *argv[])
{
	char cwd[PATH_MAX], *ipath;
	char *ifilter;

	if (getuid() == 0)
		ifilter = ".*";
	else
		ifilter = "^[^.].*"; /* Hide dotfiles */

	if (argv[1] != NULL) {
		ipath = argv[1];
	} else {
		ipath = getcwd(cwd, sizeof(cwd));
		if (ipath == NULL)
			ipath = "/";
	}

	/* Test initial path */
	if (!testopendir(ipath))
		printerr(1, ipath);

	/* Set locale before curses setup */
	setlocale(LC_ALL, "");

	initcurses();

	browse(ipath, ifilter);

	exitcurses();

	return 0;
}
