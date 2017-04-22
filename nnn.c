/* See LICENSE file for copyright and license details. */
#include <readline/readline.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/statvfs.h>
#include <sys/resource.h>

#include <ctype.h>
#include <curses.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <grp.h>
#include <limits.h>
#include <locale.h>
#include <pwd.h>
#include <regex.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define __USE_XOPEN_EXTENDED
#include <ftw.h>

#ifdef DEBUG
static int
xprintf(int fd, const char *fmt, ...)
{
	char buf[BUFSIZ];
	int r;
	va_list ap;

	va_start(ap, fmt);
	r = vsnprintf(buf, sizeof(buf), fmt, ap);
	if (r > 0)
		r = write(fd, buf, r);
	va_end(ap);
	return r;
}

#define DEBUG_FD 8
#define DPRINTF_D(x) xprintf(DEBUG_FD, #x "=%d\n", x)
#define DPRINTF_U(x) xprintf(DEBUG_FD, #x "=%u\n", x)
#define DPRINTF_S(x) xprintf(DEBUG_FD, #x "=%s\n", x)
#define DPRINTF_P(x) xprintf(DEBUG_FD, #x "=0x%p\n", x)
#else
#define DPRINTF_D(x)
#define DPRINTF_U(x)
#define DPRINTF_S(x)
#define DPRINTF_P(x)
#endif /* DEBUG */

#define VERSION "v1.0"
#define LEN(x) (sizeof(x) / sizeof(*(x)))
#undef MIN
#define MIN(x, y) ((x) < (y) ? (x) : (y))
#define ISODD(x) ((x) & 1)
#define CONTROL(c) ((c) ^ 0x40)
#define TOUPPER(ch) \
	(((ch) >= 'a' && (ch) <= 'z') ? ((ch) - 'a' + 'A') : (ch))
#define MAX_CMD_LEN 5120
#define CURSYM(flag) (flag ? CURSR : EMPTY)

struct assoc {
	char *regex; /* Regex to match on filename */
	char *bin;   /* Program */
};

/* Supported actions */
enum action {
	SEL_QUIT = 1,
	SEL_CDQUIT,
	SEL_BACK,
	SEL_GOIN,
	SEL_FLTR,
	SEL_NEXT,
	SEL_PREV,
	SEL_PGDN,
	SEL_PGUP,
	SEL_HOME,
	SEL_END,
	SEL_CD,
	SEL_CDHOME,
	SEL_LAST,
	SEL_TOGGLEDOT,
	SEL_DETAIL,
	SEL_STATS,
	SEL_MEDIA,
	SEL_FMEDIA,
	SEL_DFB,
	SEL_FSIZE,
	SEL_BSIZE,
	SEL_MTIME,
	SEL_REDRAW,
	SEL_COPY,
	SEL_HELP,
	SEL_RUN,
	SEL_RUNARG,
};

struct key {
	int sym;         /* Key pressed */
	enum action act; /* Action */
	char *run;       /* Program to run */
	char *env;       /* Environment variable to run */
};

#include "config.h"

typedef struct entry {
	char name[NAME_MAX];
	mode_t mode;
	time_t t;
	off_t size;
	off_t bsize;
} *pEntry;

typedef unsigned long ulong;

/* Externs */
#ifdef __APPLE__
extern int add_history(const char *);
#else
extern void add_history(const char *string);
#endif

/* Global context */
static struct entry *dents;
static int ndents, cur, total_dents;
static int idle;
static char *opener;
static char *fallback_opener;
static char *copier;
static char *desktop_manager;
static off_t blk_size;
static size_t fs_free;
static int open_max;
static const double div_2_pow_10 = 1.0 / 1024.0;
static const char *size_units[] = {"B", "K", "M", "G", "T", "P", "E", "Z", "Y"};

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

static void printmsg(char *);
static void printwarn(void);
static void printerr(int, char *);

static rlim_t
max_openfds()
{
	struct rlimit rl;
        rlim_t limit;

	limit = getrlimit(RLIMIT_NOFILE, &rl);
	if (limit != 0)
		return 32;

	limit = rl.rlim_cur;
	rl.rlim_cur = rl.rlim_max;

	if (setrlimit(RLIMIT_NOFILE, &rl) == 0)
		return rl.rlim_max - 64;

	if (limit > 128)
		return limit - 64;

	return 32;
}

/* Just a safe strncpy(3) */
static void
xstrlcpy(char *dest, const char *src, size_t n)
{
	strncpy(dest, src, n - 1);
	dest[n - 1] = '\0';
}

/*
 * The poor man's implementation of memrchr(3).
 * We are only looking for '/' in this program.
 */
static void *
xmemrchr(const void *s, int c, size_t n)
{
	unsigned char *p;
	unsigned char ch = (unsigned char)c;

	if (!s || !n)
		return NULL;

	p = (unsigned char *)s + n - 1;

	while(n--)
		if ((*p--) == ch)
			return ++p;

	return NULL;
}

/*
 * The following dirname(3) implementation does not
 * modify the input. We use a copy of the original.
 *
 * Modified from the glibc (GNU LGPL) version.
 */
static char *
xdirname(const char *path)
{
	static char buf[PATH_MAX];
	static char *last_slash;

	xstrlcpy(buf, path, PATH_MAX);

	/* Find last '/'. */
	last_slash = strrchr(buf, '/');

	if (last_slash != NULL && last_slash != buf && last_slash[1] == '\0') {
		/* Determine whether all remaining characters are slashes. */
		char *runp;

		for (runp = last_slash; runp != buf; --runp)
			if (runp[-1] != '/')
				break;

		/* The '/' is the last character, we have to look further. */
		if (runp != buf)
			last_slash = xmemrchr(buf, '/', runp - buf);
	}

	if (last_slash != NULL) {
		/* Determine whether all remaining characters are slashes. */
		char *runp;

		for (runp = last_slash; runp != buf; --runp)
			if (runp[-1] != '/')
				break;

		/* Terminate the buffer. */
		if (runp == buf) {
			/* The last slash is the first character in the string.
			   We have to return "/". As a special case we have to
			   return "//" if there are exactly two slashes at the
			   beginning of the string. See XBD 4.10 Path Name
			   Resolution for more information. */
			if (last_slash == buf + 1)
				++last_slash;
			else
				last_slash = buf + 1;
		} else
			last_slash = runp;

		last_slash[0] = '\0';
	} else {
		/* This assignment is ill-designed but the XPG specs require to
		   return a string containing "." in any case no directory part
		   is found and so a static and constant string is required. */
		buf[0] = '.';
		buf[1] = '\0';
	}

	return buf;
}

/*
 * Spawns a child process. Behaviour can be controlled using flag:
 * flag = 1: draw a marker to indicate nnn spawned e.g., a shell
 * flag = 2: do not wait in parent for child process e.g. DE file manager
 */
static void
spawn(char *file, char *arg, char *dir, int flag)
{
	pid_t pid;
	int status;

	pid = fork();
	if (pid == 0) {
		if (dir != NULL)
			status = chdir(dir);
		if (flag == 1)
			fprintf(stdout, "\n +-++-++-+\n | n n n |\n +-++-++-+\n\n");
		execlp(file, file, arg, NULL);
		_exit(1);
	} else {
		if (flag != 2) {
			/* Ignore interruptions */
			while (waitpid(pid, &status, 0) == -1)
				DPRINTF_D(status);
		}
		DPRINTF_D(pid);
	}
}

static char *
xgetenv(char *name, char *fallback)
{
	char *value;

	if (name == NULL)
		return fallback;
	value = getenv(name);
	return value && value[0] ? value : fallback;
}

/*
 * We assume none of the strings are NULL.
 *
 * Let's have the logic to sort numeric names in numeric order.
 * E.g., the order '1, 10, 2' doesn't make sense to human eyes.
 *
 * If the absolute numeric values are same, we fallback to alphasort.
 */
static int
xstricmp(const char *s1, const char *s2)
{
	static char *c1, *c2;
	static long long num1, num2;

	num1 = strtoll(s1, &c1, 10);
	num2 = strtoll(s2, &c2, 10);

	if (*c1 == '\0' && *c2 == '\0') {
		if (num1 != num2) {
			if (num1 > num2)
				return 1;
			else
				return -1;
		}
	} else if (*c1 == '\0' && *c2 != '\0')
		return -1;
	else if (*c1 != '\0' && *c2 == '\0')
		return 1;

	while (*s2 && *s1 && TOUPPER(*s1) == TOUPPER(*s2))
		s1++, s2++;

	/* In case of alphabetically same names, make sure
	   lower case one comes before upper case one */
	if (!*s1 && !*s2)
		return 1;

	return (int) (TOUPPER(*s1) - TOUPPER(*s2));
}

/* Trim all white space from both ends */
static char *
strstrip(char *s)
{
	size_t size;
	char *end;

	size = strlen(s);

	if (!size)
		return s;

	end = s + size - 1;
	while (end >= s && isspace(*end))
		end--;
	*(end + 1) = '\0';

	while (*s && isspace(*s))
		s++;

	return s;
}

static char *
openwith(char *file)
{
	regex_t regex;
	char *bin = NULL;
	unsigned int i;

	for (i = 0; i < LEN(assocs); i++) {
		if (regcomp(&regex, assocs[i].regex,
			    REG_NOSUB | REG_EXTENDED | REG_ICASE) != 0)
			continue;
		if (regexec(&regex, file, 0, NULL, 0) == 0) {
			bin = assocs[i].bin;
			break;
		}
	}
	DPRINTF_S(bin);
	return bin;
}

static int
setfilter(regex_t *regex, char *filter)
{
	static char errbuf[LINE_MAX];
	static size_t len;
	static int r;

	r = regcomp(regex, filter, REG_NOSUB | REG_EXTENDED | REG_ICASE);
	if (r != 0) {
		len = COLS;
		if (len > sizeof(errbuf))
			len = sizeof(errbuf);
		regerror(r, regex, errbuf, len);
		printmsg(errbuf);
	}
	return r;
}

static void
initfilter(int dot, char **ifilter)
{
	*ifilter = dot ? "." : "^[^.]";
}

static int
visible(regex_t *regex, char *file)
{
	return regexec(regex, file, 0, NULL, 0) == 0;
}

static int
entrycmp(const void *va, const void *vb)
{
	static pEntry pa, pb;

	pa = (pEntry)va;
	pb = (pEntry)vb;

	/* Sort directories first */
	if (S_ISDIR(pb->mode) && !S_ISDIR(pa->mode))
		return 1;
	else if (S_ISDIR(pa->mode) && !S_ISDIR(pb->mode))
		return -1;

	/* Do the actual sorting */
	if (mtimeorder)
		return pb->t - pa->t;

	if (sizeorder) {
		if (pb->size > pa->size)
			return 1;
		else if (pb->size < pa->size)
			return -1;
	}

	if (bsizeorder) {
		if (pb->bsize > pa->bsize)
			return 1;
		else if (pb->bsize < pa->bsize)
			return -1;
	}

	return xstricmp(pa->name, pb->name);
}

static void
initcurses(void)
{
	if (initscr() == NULL) {
		char *term = getenv("TERM");
		if (term != NULL)
			fprintf(stderr, "error opening terminal: %s\n", term);
		else
			fprintf(stderr, "failed to initialize curses\n");
		exit(1);
	}
	cbreak();
	noecho();
	nonl();
	intrflush(stdscr, FALSE);
	keypad(stdscr, TRUE);
	curs_set(FALSE); /* Hide cursor */
	timeout(1000); /* One second */
}

static void
exitcurses(void)
{
	endwin(); /* Restore terminal */
}

/* Messages show up at the bottom */
static void
printmsg(char *msg)
{
	move(LINES - 1, 0);
	printw("%s\n", msg);
}

/* Display warning as a message */
static void
printwarn(void)
{
	printmsg(strerror(errno));
}

/* Kill curses and display error before exiting */
static void
printerr(int ret, char *prefix)
{
	exitcurses();
	fprintf(stderr, "%s: %s\n", prefix, strerror(errno));
	exit(ret);
}

/* Clear the last line */
static void
clearprompt(void)
{
	printmsg("");
}

/* Print prompt on the last line */
static void
printprompt(char *str)
{
	clearprompt();
	printw(str);
}

/* Returns SEL_* if key is bound and 0 otherwise.
 * Also modifies the run and env pointers (used on SEL_{RUN,RUNARG}) */
static int
nextsel(char **run, char **env)
{
	int c;
	unsigned int i;

	c = getch();
	if (c == -1)
		idle++;
	else
		idle = 0;

	for (i = 0; i < LEN(bindings); i++)
		if (c == bindings[i].sym) {
			*run = bindings[i].run;
			*env = bindings[i].env;
			return bindings[i].act;
		}
	return 0;
}

static char *
readln(void)
{
	static char ln[LINE_MAX];

	timeout(-1);
	echo();
	curs_set(TRUE);
	memset(ln, 0, sizeof(ln));
	wgetnstr(stdscr, ln, sizeof(ln) - 1);
	noecho();
	curs_set(FALSE);
	timeout(1000);
	return ln[0] ? ln : NULL;
}

static int
canopendir(char *path)
{
	static DIR *dirp;

	dirp = opendir(path);
	if (dirp == NULL)
		return 0;
	closedir(dirp);
	return 1;
}

/*
 * Returns "dir/name or "/name"
 */
static char *
mkpath(char *dir, char *name, char *out, size_t n)
{
	/* Handle absolute path */
	if (name[0] == '/')
		xstrlcpy(out, name, n);
	else {
		/* Handle root case */
		if (strcmp(dir, "/") == 0)
			snprintf(out, n, "/%s", name);
		else
			snprintf(out, n, "%s/%s", dir, name);
	}
	return out;
}

static void
printent(struct entry *ent, int active)
{
	static int ncols;
	static char str[PATH_MAX + 16];

	if (COLS > PATH_MAX + 16)
		ncols = PATH_MAX + 16;
	else
		ncols = COLS;

	if (S_ISDIR(ent->mode))
		snprintf(str, ncols, "%s%s/", CURSYM(active), ent->name);
	else if (S_ISLNK(ent->mode))
		snprintf(str, ncols, "%s%s@", CURSYM(active), ent->name);
	else if (S_ISSOCK(ent->mode))
		snprintf(str, ncols, "%s%s=", CURSYM(active), ent->name);
	else if (S_ISFIFO(ent->mode))
		snprintf(str, ncols, "%s%s|", CURSYM(active), ent->name);
	else if (ent->mode & S_IXUSR)
		snprintf(str, ncols, "%s%s*", CURSYM(active), ent->name);
	else
		snprintf(str, ncols, "%s%s", CURSYM(active), ent->name);

	printw("%s\n", str);
}

static void (*printptr)(struct entry *ent, int active) = &printent;

static char*
coolsize(off_t size)
{
	static char size_buf[12]; /* Buffer to hold human readable size */
	static int i;
	static off_t fsize, tmp;
	static long double rem;

	i = 0;
	fsize = size;
	rem = 0;

	while (fsize > 1024) {
		tmp = fsize;
		//fsize *= div_2_pow_10;
		fsize >>= 10;
		rem = tmp - (fsize << 10);
		i++;
	}

	snprintf(size_buf, 12, "%.*Lf%s", i, fsize + rem * div_2_pow_10, size_units[i]);
	return size_buf;
}

static void
printent_long(struct entry *ent, int active)
{
	static int ncols;
	static char str[PATH_MAX + 32];
	static char buf[18];

	if (COLS > PATH_MAX + 32)
		ncols = PATH_MAX + 32;
	else
		ncols = COLS;

	strftime(buf, 18, "%d %m %Y %H:%M", localtime(&ent->t));

	if (active)
		attron(A_REVERSE);

	if (!bsizeorder) {
		if (S_ISDIR(ent->mode))
			snprintf(str, ncols, "%s%-16.16s        /  %s/",
				 CURSYM(active), buf, ent->name);
		else if (S_ISLNK(ent->mode))
			snprintf(str, ncols, "%s%-16.16s        @  %s@",
				 CURSYM(active), buf, ent->name);
		else if (S_ISSOCK(ent->mode))
			snprintf(str, ncols, "%s%-16.16s        =  %s=",
				 CURSYM(active), buf, ent->name);
		else if (S_ISFIFO(ent->mode))
			snprintf(str, ncols, "%s%-16.16s        |  %s|",
				 CURSYM(active), buf, ent->name);
		else if (S_ISBLK(ent->mode))
			snprintf(str, ncols, "%s%-16.16s        b  %s",
				 CURSYM(active), buf, ent->name);
		else if (S_ISCHR(ent->mode))
			snprintf(str, ncols, "%s%-16.16s        c  %s",
				 CURSYM(active), buf, ent->name);
		else if (ent->mode & S_IXUSR)
			snprintf(str, ncols, "%s%-16.16s %8.8s* %s*",
				 CURSYM(active), buf, coolsize(ent->size), ent->name);
		else
			snprintf(str, ncols, "%s%-16.16s %8.8s  %s",
				 CURSYM(active), buf, coolsize(ent->size), ent->name);
	} else {
		if (S_ISDIR(ent->mode))
			snprintf(str, ncols, "%s%-16.16s %8.8s/ %s/",
				 CURSYM(active), buf, coolsize(ent->bsize << 9), ent->name);
		else if (S_ISLNK(ent->mode))
			snprintf(str, ncols, "%s%-16.16s        @  %s@",
				 CURSYM(active), buf, ent->name);
		else if (S_ISSOCK(ent->mode))
			snprintf(str, ncols, "%s%-16.16s        =  %s=",
				 CURSYM(active), buf, ent->name);
		else if (S_ISFIFO(ent->mode))
			snprintf(str, ncols, "%s%-16.16s        |  %s|",
				 CURSYM(active), buf, ent->name);
		else if (S_ISBLK(ent->mode))
			snprintf(str, ncols, "%s%-16.16s        b  %s",
				 CURSYM(active), buf, ent->name);
		else if (S_ISCHR(ent->mode))
			snprintf(str, ncols, "%s%-16.16s        c  %s",
				 CURSYM(active), buf, ent->name);
		else if (ent->mode & S_IXUSR)
			snprintf(str, ncols, "%s%-16.16s %8.8s* %s*",
				 CURSYM(active), buf, coolsize(ent->bsize << 9), ent->name);
		else
			snprintf(str, ncols, "%s%-16.16s %8.8s  %s",
				 CURSYM(active), buf, coolsize(ent->bsize << 9), ent->name);
	}

	printw("%s\n", str);

	if (active)
		attroff(A_REVERSE);
}

static char
get_fileind(mode_t mode, char *desc)
{
	static char c;

	if (S_ISREG(mode)) {
		c = '-';
		sprintf(desc, "%s", "regular file");
		if (mode & S_IXUSR)
			strcat(desc, ", executable");
	} else if (S_ISDIR(mode)) {
		c = 'd';
		sprintf(desc, "%s", "directory");
	} else if (S_ISBLK(mode)) {
		c = 'b';
		sprintf(desc, "%s", "block special device");
	} else if (S_ISCHR(mode)) {
		c = 'c';
		sprintf(desc, "%s", "character special device");
#ifdef S_ISFIFO
	} else if (S_ISFIFO(mode)) {
		c = 'p';
		sprintf(desc, "%s", "FIFO");
#endif  /* S_ISFIFO */
#ifdef S_ISLNK
	} else if (S_ISLNK(mode)) {
		c = 'l';
		sprintf(desc, "%s", "symbolic link");
#endif  /* S_ISLNK */
#ifdef S_ISSOCK
	} else if (S_ISSOCK(mode)) {
		c = 's';
		sprintf(desc, "%s", "socket");
#endif  /* S_ISSOCK */
#ifdef S_ISDOOR
    /* Solaris 2.6, etc. */
	} else if (S_ISDOOR(mode)) {
		c = 'D';
		desc[0] = '\0';
#endif  /* S_ISDOOR */
	} else {
		/* Unknown type -- possibly a regular file? */
		c = '?';
		desc[0] = '\0';
	}

	return(c);
}

/* Convert a mode field into "ls -l" type perms field. */
static char *
get_lsperms(mode_t mode, char *desc)
{
	static const char *rwx[] = {"---", "--x", "-w-", "-wx",
				    "r--", "r-x", "rw-", "rwx"};
	static char bits[11];

	bits[0] = get_fileind(mode, desc);
	strcpy(&bits[1], rwx[(mode >> 6) & 7]);
	strcpy(&bits[4], rwx[(mode >> 3) & 7]);
	strcpy(&bits[7], rwx[(mode & 7)]);

	if (mode & S_ISUID)
		bits[3] = (mode & S_IXUSR) ? 's' : 'S';
	if (mode & S_ISGID)
		bits[6] = (mode & S_IXGRP) ? 's' : 'l';
	if (mode & S_ISVTX)
		bits[9] = (mode & S_IXOTH) ? 't' : 'T';

	bits[10] = '\0';

	return(bits);
}

static char *
get_output(char *buf, size_t bytes)
{
	char *ret;
	FILE *pf = popen(buf, "r");
	if (pf) {
		ret = fgets(buf, bytes, pf);
		pclose(pf);
		return ret;
	}

	return NULL;
}

/*
 * Follows the stat(1) output closely
 */
static int
show_stats(char* fpath, char* fname, struct stat *sb)
{
	char buf[PATH_MAX + 16];
	char *perms = get_lsperms(sb->st_mode, buf);
	char *p, *begin = buf;

	char tmp[] = "/tmp/nnnXXXXXX";
	int fd = mkstemp(tmp);
	if (fd == -1)
		return -1;

	/* Show file name or 'symlink' -> 'target' */
	if (perms[0] == 'l') {
		char symtgt[PATH_MAX];
		ssize_t len = readlink(fpath, symtgt, PATH_MAX);
		if (len != -1) {
			symtgt[len] = '\0';
			dprintf(fd, "    File: '%s' -> '%s'", fname, symtgt);
		}
	} else
		dprintf(fd, "    File: '%s'", fname);

	/* Show size, blocks, file type */
	dprintf(fd, "\n    Size: %-15ld Blocks: %-10ld IO Block: %-6ld %s",
	       sb->st_size, sb->st_blocks, sb->st_blksize, buf);

	/* Show containing device, inode, hardlink count */
	sprintf(buf, "%lxh/%lud", (ulong)sb->st_dev, (ulong)sb->st_dev);
	dprintf(fd, "\n  Device: %-15s Inode: %-11lu Links: %-9lu",
	       buf, sb->st_ino, sb->st_nlink);

	/* Show major, minor number for block or char device */
	if (perms[0] == 'b' || perms[0] == 'c')
		dprintf(fd, " Device type: %x,%x",
		       major(sb->st_rdev), minor(sb->st_rdev));

	/* Show permissions, owner, group */
	dprintf(fd, "\n  Access: 0%d%d%d/%s Uid: (%u/%s)  Gid: (%u/%s)",
	       (sb->st_mode >> 6) & 7, (sb->st_mode >> 3) & 7, sb->st_mode & 7,
	       perms,
	       sb->st_uid, (getpwuid(sb->st_uid))->pw_name,
	       sb->st_gid, (getgrgid(sb->st_gid))->gr_name);

	/* Show last access time */
	strftime(buf, 40, "%a %d-%b-%Y %T %z,%Z", localtime(&sb->st_atime));
	dprintf(fd, "\n\n  Access: %s", buf);

	/* Show last modification time */
	strftime(buf, 40, "%a %d-%b-%Y %T %z,%Z", localtime(&sb->st_mtime));
	dprintf(fd, "\n  Modify: %s", buf);

	/* Show last status change time */
	strftime(buf, 40, "%a %d-%b-%Y %T %z,%Z", localtime(&sb->st_ctime));
	dprintf(fd, "\n  Change: %s", buf);

	if (S_ISREG(sb->st_mode)) {
		/* Show file(1) output */
		sprintf(buf, "file -b \"%s\" 2>&1", fpath);
		p = get_output(buf, PATH_MAX + 16);
		if (p) {
			dprintf(fd, "\n\n ");
			while (*p) {
				if (*p == ',') {
					*p = '\0';
					dprintf(fd, " %s\n", begin);
					begin = p + 1;
				}

				p++;
			}
			dprintf(fd, " %s", begin);
		}
	}

	dprintf(fd, "\n\n");
	close(fd);

	sprintf(buf, "cat %s | less", tmp);
	fd = system(buf);

	unlink(tmp);
	return fd;
}

static int
show_mediainfo(const char* fpath, int full)
{
	static char buf[MAX_CMD_LEN];

	snprintf(buf, MAX_CMD_LEN, "which mediainfo");
	if (get_output(buf, MAX_CMD_LEN) == NULL)
		return -1;

	if (full)
		sprintf(buf, "mediainfo -f \"%s\" 2>&1 | less", fpath);
	else
		sprintf(buf, "mediainfo \"%s\" 2>&1 | less", fpath);

	return system(buf);
}

static int
show_help(void)
{
	char helpstr[] = ("echo \"\
    << Key >>                   << Function >>\n\n\
    [Up], k, ^P                 Previous entry\n\
    [Down], j, ^N               Next entry\n\
    [PgUp], ^U                  Scroll half page up\n\
    [PgDn], ^D                  Scroll half page down\n\
    [Home], g, ^, ^A            Jump to first entry\n\
    [End], G, $, ^E             Jump to last entry\n\
    [Right], [Enter], l, ^M     Open file or enter dir\n\
    [Left], [Backspace], h, ^H  Go to parent dir\n\
    ~                           Jump to HOME dir\n\
    -                           Jump to last visited dir\n\
    o                           Open dir in NNN_DE_FILE_MANAGER\n\
    /, &                        Filter dir contents\n\
    c                           Show change dir prompt\n\
    d                           Toggle detail view\n\
    D                           Toggle current file details screen\n\
    m                           Show concise mediainfo\n\
    M                           Show full mediainfo\n\
    .                           Toggle hide .dot files\n\
    s                           Toggle sort by file size\n\
    S                           Toggle disk usage analyzer mode\n\
    t                           Toggle sort by modified time\n\
    !                           Spawn SHELL in PWD (fallback sh)\n\
    z                           Run top\n\
    e                           Edit entry in EDITOR (fallback vi)\n\
    p                           Open entry in PAGER (fallback less)\n\
    ^K                          Invoke file name copier\n\
    ^L                          Force a redraw\n\
    ?                           Toggle help screen\n\
    q                           Quit\n\
    Q                           Quit and change directory\n\n\" | less");

	return system(helpstr);
}

static int
sum_bsizes(const char *fpath, const struct stat *sb, int typeflag, struct FTW *ftwbuf)
{
	/* Handle permission problems */
	if(typeflag == FTW_NS) {
		printmsg("No stats (permissions ?)");
		return 0;
	}

	blk_size += sb->st_blocks;

	return 0;
}

static int
getorder(size_t size)
{
	switch (size) {
	case 4096:
		return 12;
	case 512:
		return 9;
	case 8192:
		return 13;
	case 16384:
		return 14;
	case 32768:
		return 15;
	case 65536:
		return 16;
	case 131072:
		return 17;
	case 262144:
		return 18;
	case 524288:
		return 19;
	case 1048576:
		return 20;
	case 2048:
		return 11;
	case 1024:
		return 10;
	default:
		return 0;
	}
}

static int
dentfill(char *path, struct entry **dents,
	 int (*filter)(regex_t *, char *), regex_t *re)
{
	static char newpath[PATH_MAX];
	static DIR *dirp;
	static struct dirent *dp;
	static struct stat sb;
	static struct statvfs svb;
	static int r, n;
	r = n = 0;

	dirp = opendir(path);
	if (dirp == NULL)
		return 0;

	while ((dp = readdir(dirp)) != NULL) {
		/* Skip self and parent */
		if ((dp->d_name[0] == '.' && (dp->d_name[1] == '\0' ||
		    (dp->d_name[1] == '.' && dp->d_name[2] == '\0'))))
			continue;

		if (filter(re, dp->d_name) == 0)
			continue;

		if (n == total_dents) {
			total_dents += 64;
			*dents = realloc(*dents, total_dents * sizeof(**dents));
			if (*dents == NULL)
				printerr(1, "realloc");
		}

		xstrlcpy((*dents)[n].name, dp->d_name, sizeof((*dents)[n].name));
		/* Get mode flags */
		mkpath(path, dp->d_name, newpath, sizeof(newpath));
		r = lstat(newpath, &sb);
		if (r == -1) {
			if (*dents)
				free(*dents);
			printerr(1, "lstat");
		}
		(*dents)[n].mode = sb.st_mode;
		(*dents)[n].t = sb.st_mtime;
		(*dents)[n].size = sb.st_size;

		if (bsizeorder) {
			if (S_ISDIR(sb.st_mode)) {
				blk_size = 0;
				if (nftw(newpath, sum_bsizes, open_max, FTW_MOUNT | FTW_PHYS) == -1) {
					printmsg("nftw(3) failed");
					(*dents)[n].bsize = sb.st_blocks;
				} else
					(*dents)[n].bsize = blk_size;
			} else
				(*dents)[n].bsize = sb.st_blocks;
		}

		n++;
	}

	if (bsizeorder) {
		r = statvfs(path, &svb);
		if (r == -1)
			fs_free = 0;
		else
			fs_free = svb.f_bavail << getorder(svb.f_bsize);
	}

	/* Should never be null */
	r = closedir(dirp);
	if (r == -1) {
		if (*dents)
			free(*dents);
		printerr(1, "closedir");
	}
	return n;
}

static void
dentfree(struct entry *dents)
{
	free(dents);
}

/* Return the position of the matching entry or 0 otherwise */
static int
dentfind(struct entry *dents, int n, char *path)
{
	if (!path)
		return 0;

	static int i;
	static char *p;

	p = xmemrchr(path, '/', strlen(path));
	if (!p)
		p = path;
	else
		/* We are assuming an entry with actual
		   name ending in '/' will not appear */
		p++;

	DPRINTF_S(p);

	for (i = 0; i < n; i++)
		if (strcmp(p, dents[i].name) == 0)
			return i;

	return 0;
}

static int
populate(char *path, char *oldpath, char *fltr)
{
	static regex_t re;
	static int r;

	/* Can fail when permissions change while browsing */
	if (canopendir(path) == 0)
		return -1;

	/* Search filter */
	r = setfilter(&re, fltr);
	if (r != 0)
		return -1;

	ndents = dentfill(path, &dents, visible, &re);

	qsort(dents, ndents, sizeof(*dents), entrycmp);

	/* Find cur from history */
	cur = dentfind(dents, ndents, oldpath);
	return 0;
}

static void
redraw(char *path)
{
	static char cwd[PATH_MAX];
	static int nlines, odd;
	static int i;
	static size_t ncols;

	nlines = MIN(LINES - 4, ndents);

	/* Clean screen */
	erase();

	/* Strip trailing slashes */
	for (i = strlen(path) - 1; i > 0; i--)
		if (path[i] == '/')
			path[i] = '\0';
		else
			break;

	DPRINTF_D(cur);
	DPRINTF_S(path);

	/* No text wrapping in cwd line */
	if (!realpath(path, cwd)) {
		printmsg("Cannot resolve path");
		return;
	}

	ncols = COLS;
	if (ncols > PATH_MAX)
		ncols = PATH_MAX;
	cwd[ncols - strlen(CWD) - 1] = '\0';
	printw(CWD "%s\n\n", cwd);

	/* Print listing */
	odd = ISODD(nlines);
	if (cur < (nlines >> 1)) {
		for (i = 0; i < nlines; i++)
			printptr(&dents[i], i == cur);
	} else if (cur >= ndents - (nlines >> 1)) {
		for (i = ndents - nlines; i < ndents; i++)
			printptr(&dents[i], i == cur);
	} else {
		nlines >>= 1;
		for (i = cur - nlines; i < cur + nlines + odd; i++)
			printptr(&dents[i], i == cur);
	}

	if (showdetail) {
		if (ndents) {
			static char ind[2] = "\0\0";
			static char sort[17];

			if (mtimeorder)
				sprintf(sort, "by time ");
			else if (sizeorder)
				sprintf(sort, "by size ");
			else
				sort[0] = '\0';

			if (S_ISDIR(dents[cur].mode))
				ind[0] = '/';
			else if (S_ISLNK(dents[cur].mode))
				ind[0] = '@';
			else if (S_ISSOCK(dents[cur].mode))
				ind[0] = '=';
			else if (S_ISFIFO(dents[cur].mode))
				ind[0] = '|';
			else if (dents[cur].mode & S_IXUSR)
				ind[0] = '*';
			else
				ind[0] = '\0';

			if (!bsizeorder)
				sprintf(cwd, "total %d %s[%s%s]", ndents, sort,
					dents[cur].name, ind);
			else
				sprintf(cwd, "total %d by disk usage, %s free [%s%s]",
					ndents, coolsize(fs_free), dents[cur].name, ind);

			printmsg(cwd);
		} else
			printmsg("0 items");
	}
}

static void
browse(char *ipath, char *ifilter)
{
	static char path[PATH_MAX], oldpath[PATH_MAX], newpath[PATH_MAX];
	static char lastdir[PATH_MAX];
	static char fltr[LINE_MAX];
	char *bin, *dir, *tmp, *run, *env;
	struct stat sb;
	regex_t re;
	int r, fd;
	enum action sel = SEL_RUNARG + 1;

	xstrlcpy(path, ipath, sizeof(path));
	xstrlcpy(lastdir, ipath, sizeof(lastdir));
	xstrlcpy(fltr, ifilter, sizeof(fltr));
	oldpath[0] = '\0';
	newpath[0] = '\0';
begin:

	if (sel == SEL_GOIN && S_ISDIR(sb.st_mode))
		r = populate(path, NULL, fltr);
	else
		r = populate(path, oldpath, fltr);
	if (r == -1) {
		printwarn();
		goto nochange;
	}

	for (;;) {
		redraw(path);
nochange:
		sel = nextsel(&run, &env);
		switch (sel) {
		case SEL_CDQUIT:
		{
			char *tmpfile = "/tmp/nnn";
			if ((tmp = getenv("NNN_TMPFILE")) != NULL)
				tmpfile = tmp;

			FILE *fp = fopen(tmpfile, "w");
			if (fp) {
				fprintf(fp, "cd \"%s\"", path);
				fclose(fp);
			}
		}
		case SEL_QUIT:
			dentfree(dents);
			return;
		case SEL_BACK:
			/* There is no going back */
			if (strcmp(path, "/") == 0 ||
			    strchr(path, '/') == NULL) {
				printmsg("You are at /");
				goto nochange;
			}
			dir = xdirname(path);
			if (canopendir(dir) == 0) {
				printwarn();
				goto nochange;
			}

			/* Save history */
			xstrlcpy(oldpath, path, sizeof(oldpath));

			/* Save last working directory */
			xstrlcpy(lastdir, path, sizeof(lastdir));
			xstrlcpy(path, dir, sizeof(path));
			/* Reset filter */
			xstrlcpy(fltr, ifilter, sizeof(fltr));
			goto begin;
		case SEL_GOIN:
			/* Cannot descend in empty directories */
			if (ndents == 0)
				goto nochange;

			mkpath(path, dents[cur].name, newpath, sizeof(newpath));
			DPRINTF_S(newpath);

			/* Get path info */
			fd = open(newpath, O_RDONLY | O_NONBLOCK);
			if (fd == -1) {
				printwarn();
				goto nochange;
			}
			r = fstat(fd, &sb);
			if (r == -1) {
				printwarn();
				close(fd);
				goto nochange;
			}
			close(fd);
			DPRINTF_U(sb.st_mode);

			switch (sb.st_mode & S_IFMT) {
			case S_IFDIR:
				if (canopendir(newpath) == 0) {
					printwarn();
					goto nochange;
				}

				/* Save last working directory */
				xstrlcpy(lastdir, path, sizeof(lastdir));

				xstrlcpy(path, newpath, sizeof(path));
				/* Reset filter */
				xstrlcpy(fltr, ifilter, sizeof(fltr));
				goto begin;
			case S_IFREG:
			{
				static char cmd[MAX_CMD_LEN];
				static char *runvi = "vi";

				/* If default mime opener is set, use it */
				if (opener) {
					snprintf(cmd, MAX_CMD_LEN,
						 "%s \"%s\" > /dev/null 2>&1",
						 opener, newpath);
					r = system(cmd);
					continue;
				}

				/* Try custom applications */
				bin = openwith(newpath);

				/* If custom app doesn't exist try fallback */
				snprintf(cmd, MAX_CMD_LEN, "which \"%s\"", bin);
				if (get_output(cmd, MAX_CMD_LEN) == NULL)
					bin = NULL;

				if (bin == NULL) {
					/* If a custom handler application is
					   not set, open plain text files with
					   vi, then try fallback_opener */
					snprintf(cmd, MAX_CMD_LEN,
						 "file \"%s\"", newpath);
					if (get_output(cmd, MAX_CMD_LEN) == NULL)
						goto nochange;

					if (strstr(cmd, "ASCII text") != NULL)
						bin = runvi;
					else if (fallback_opener) {
						snprintf(cmd, MAX_CMD_LEN,
							 "%s \"%s\" > \
							 /dev/null 2>&1",
							 fallback_opener,
							 newpath);
						r = system(cmd);
						continue;
					} else {
						printmsg("No association");
						goto nochange;
					}
				}
				exitcurses();
				spawn(bin, newpath, NULL, 0);
				initcurses();
				continue;
			}
			default:
				printmsg("Unsupported file");
				goto nochange;
			}
		case SEL_FLTR:
			/* Read filter */
			printprompt("filter: ");
			tmp = readln();
			if (tmp == NULL)
				tmp = ifilter;
			/* Check and report regex errors */
			r = setfilter(&re, tmp);
			if (r != 0)
				goto nochange;
			xstrlcpy(fltr, tmp, sizeof(fltr));
			DPRINTF_S(fltr);
			/* Save current */
			if (ndents > 0)
				mkpath(path, dents[cur].name, oldpath, sizeof(oldpath));
			goto begin;
		case SEL_NEXT:
			if (cur < ndents - 1)
				cur++;
			else if (ndents)
				/* Roll over, set cursor to first entry */
				cur = 0;
			break;
		case SEL_PREV:
			if (cur > 0)
				cur--;
			else if (ndents)
				/* Roll over, set cursor to last entry */
				cur = ndents - 1;
			break;
		case SEL_PGDN:
			if (cur < ndents - 1)
				cur += MIN((LINES - 4) / 2, ndents - 1 - cur);
			break;
		case SEL_PGUP:
			if (cur > 0)
				cur -= MIN((LINES - 4) / 2, cur);
			break;
		case SEL_HOME:
			cur = 0;
			break;
		case SEL_END:
			cur = ndents - 1;
			break;
		case SEL_CD:
		{
			/* Read target dir */
			tmp = getcwd(newpath, PATH_MAX);
			if (tmp == NULL) {
				printwarn();
				goto nochange;
			}

			if (chdir(path) == -1) {
				printwarn();
				goto nochange;
			}

			exitcurses();
			char *tmp = readline("chdir: ");
			initcurses();

			if (chdir(newpath) == -1)
				printwarn();

			/* Save current */
			if (ndents > 0)
				mkpath(path, dents[cur].name, oldpath, sizeof(oldpath));

			if (tmp[0] == '\0')
				break;
			else
				add_history(tmp);

			char *input = tmp;
			tmp = strstrip(tmp);
			if (tmp[0] == '\0') {
				free(input);
				break;
			}

			if (tmp[0] == '~') {
				char *home = getenv("HOME");
				if (home)
					snprintf(newpath, PATH_MAX,
						"%s%s", home, tmp + 1);
				else
					mkpath(path, tmp, newpath, sizeof(newpath));

				oldpath[0] = '\0';
			} else if (tmp[0] == '-' && tmp[1] == '\0') {
				xstrlcpy(newpath, lastdir, sizeof(newpath));
				oldpath[0] = '\0';
			} else if (tmp[0] == '.' && tmp[1] == '\0')
				xstrlcpy(newpath, path, sizeof(newpath));
			else if (tmp[0] == '.' && tmp[1] == '.' && tmp[2] == '\0') {
				/* There is no going back */
				if (strcmp(path, "/") == 0 ||
				    strchr(path, '/') == NULL) {
					printmsg("You are at /");
					free(input);
					goto nochange;
				}
				dir = xdirname(path);
				if (canopendir(dir) == 0) {
					printwarn();
					free(input);
					goto nochange;
				}

				/* Save history */
				xstrlcpy(oldpath, path, sizeof(oldpath));
				xstrlcpy(newpath, dir, sizeof(newpath));
			} else {
				mkpath(path, tmp, newpath, sizeof(newpath));
				oldpath[0] = '\0';
			}

			if (canopendir(newpath) == 0) {
				/* Save current */
				if (ndents > 0)
					mkpath(path, dents[cur].name, oldpath, sizeof(oldpath));

				printwarn();
				free(input);
				break;
			}

			/* Save last working directory */
			xstrlcpy(lastdir, path, sizeof(lastdir));

			xstrlcpy(path, newpath, sizeof(path));
			/* Reset filter */
			xstrlcpy(fltr, ifilter, sizeof(fltr));
			DPRINTF_S(path);
			free(input);
			goto begin;
		}
		case SEL_CDHOME:
			tmp = getenv("HOME");
			if (tmp == NULL) {
				clearprompt();
				goto nochange;
			}
			if (canopendir(tmp) == 0) {
				printwarn();
				goto nochange;
			}

			/* Save last working directory */
			xstrlcpy(lastdir, path, sizeof(lastdir));

			xstrlcpy(path, tmp, sizeof(path));
			/* Reset filter */
			xstrlcpy(fltr, ifilter, sizeof(fltr));
			DPRINTF_S(path);
			goto begin;
		case SEL_LAST:
			xstrlcpy(newpath, lastdir, sizeof(newpath));
			xstrlcpy(lastdir, path, sizeof(lastdir));
			xstrlcpy(path, newpath, sizeof(path));
			oldpath[0] = '\0';
			DPRINTF_S(path);
			goto begin;
		case SEL_TOGGLEDOT:
			showhidden ^= 1;
			initfilter(showhidden, &ifilter);
			xstrlcpy(fltr, ifilter, sizeof(fltr));
			goto begin;
		case SEL_DETAIL:
			showdetail = !showdetail;
			showdetail ? (printptr = &printent_long)
				   : (printptr = &printent);
			/* Save current */
			if (ndents > 0)
				mkpath(path, dents[cur].name, oldpath, sizeof(oldpath));
			goto begin;
		case SEL_STATS:
		{
			struct stat sb;

			if (ndents > 0)
				mkpath(path, dents[cur].name, oldpath, sizeof(oldpath));

			r = lstat(oldpath, &sb);
			if (r == -1) {
				if (dents)
					dentfree(dents);
				printerr(1, "lstat");
			} else {
				exitcurses();
				r = show_stats(oldpath, dents[cur].name, &sb);
				initcurses();
				if (r < 0) {
					printmsg(strerror(errno));
					goto nochange;
				}
			}

			break;
		}
		case SEL_MEDIA:
			if (ndents > 0)
				mkpath(path, dents[cur].name, oldpath, sizeof(oldpath));

			exitcurses();
			r = show_mediainfo(oldpath, FALSE);
			initcurses();
			if (r < 0) {
				printmsg("mediainfo missing");
				goto nochange;
			}
			break;
		case SEL_FMEDIA:
			if (ndents > 0)
				mkpath(path, dents[cur].name, oldpath, sizeof(oldpath));

			exitcurses();
			r = show_mediainfo(oldpath, TRUE);
			initcurses();
			if (r < 0) {
				printmsg("mediainfo missing");
				goto nochange;
			}
			break;
		case SEL_DFB:
			if (!desktop_manager) {
				printmsg("NNN_DE_FILE_MANAGER not set");
				goto nochange;
			}

			spawn(desktop_manager, path, path, 2);
			break;
		case SEL_FSIZE:
			sizeorder = !sizeorder;
			mtimeorder = 0;
			bsizeorder = 0;
			/* Save current */
			if (ndents > 0)
				mkpath(path, dents[cur].name, oldpath, sizeof(oldpath));
			goto begin;
		case SEL_BSIZE:
			bsizeorder = !bsizeorder;
			if (bsizeorder) {
				showdetail = 1;
				printptr = &printent_long;
			}
			mtimeorder = 0;
			sizeorder = 0;
			/* Save current */
			if (ndents > 0)
				mkpath(path, dents[cur].name, oldpath, sizeof(oldpath));
			goto begin;
		case SEL_MTIME:
			mtimeorder = !mtimeorder;
			sizeorder = 0;
			bsizeorder = 0;
			/* Save current */
			if (ndents > 0)
				mkpath(path, dents[cur].name, oldpath, sizeof(oldpath));
			goto begin;
		case SEL_REDRAW:
			/* Save current */
			if (ndents > 0)
				mkpath(path, dents[cur].name, oldpath, sizeof(oldpath));
			goto begin;
		case SEL_COPY:
			if (copier && ndents) {
				if (strcmp(path, "/") == 0)
					snprintf(newpath, PATH_MAX, "/%s",
						 dents[cur].name);
				else
					snprintf(newpath, PATH_MAX, "%s/%s",
						 path, dents[cur].name);
				spawn(copier, newpath, NULL, 0);
				printmsg(newpath);
			} else if (!copier)
					printmsg("NNN_COPIER is not set");
			goto nochange;
		case SEL_HELP:
			exitcurses();
			show_help();
			initcurses();
			break;
		case SEL_RUN:
			run = xgetenv(env, run);
			exitcurses();
			spawn(run, NULL, path, 1);
			initcurses();
			/* Repopulate as directory content may have changed */
			goto begin;
		case SEL_RUNARG:
			run = xgetenv(env, run);
			exitcurses();
			spawn(run, dents[cur].name, path, 0);
			initcurses();
			break;
		}
		/* Screensaver */
		if (idletimeout != 0 && idle == idletimeout) {
			idle = 0;
			exitcurses();
			spawn(idlecmd, NULL, NULL, 0);
			initcurses();
		}
	}
}

static void
usage(void)
{
	fprintf(stdout, "usage: nnn [-d] [-S] [-v] [h] [PATH]\n\n\
The missing terminal file browser for X.\n\n\
positional arguments:\n\
  PATH           directory to open [default: current dir]\n\n\
optional arguments:\n\
  -d             start in detail view mode\n\
  -S             start in disk usage analyzer mode\n\
  -v             show program version and exit\n\
  -h             show this help and exit\n\n\
Version: %s\n\
License: BSD 2-Clause\n\
Webpage: https://github.com/jarun/nnn\n", VERSION);

	exit(0);
}

int
main(int argc, char *argv[])
{
	char cwd[PATH_MAX], *ipath;
	char *ifilter;
	int opt = 0;

	/* Confirm we are in a terminal */
	if (!isatty(0) || !isatty(1)) {
		fprintf(stderr, "stdin or stdout is not a tty\n");
		exit(1);
	}

	if (argc > 3)
		usage();

	while ((opt = getopt(argc, argv, "dSvh")) != -1) {
		switch (opt) {
		case 'S':
			bsizeorder = 1;
		case 'd':
			/* Open in detail mode, if set */
			showdetail = 1;
			printptr = &printent_long;
			break;
		case 'v':
			fprintf(stdout, "%s\n", VERSION);
			return 0;
		case 'h':
		default:
			usage();
		}
	}

	if (argc == optind) {
		/* Start in the current directory */
		ipath = getcwd(cwd, sizeof(cwd));
		if (ipath == NULL)
			ipath = "/";
	} else {
		ipath = realpath(argv[optind], cwd);
		if (!ipath) {
			fprintf(stderr, "%s: no such dir\n", argv[optind]);
			exit(1);
		}
	}

	open_max = max_openfds();

	if (getuid() == 0)
		showhidden = 1;
	initfilter(showhidden, &ifilter);

	/* Get the default desktop mime opener, if set */
	opener = getenv("NNN_OPENER");

	/* Get the fallback desktop mime opener, if set */
	fallback_opener = getenv("NNN_FALLBACK_OPENER");

	/* Get the desktop file browser, if set */
	desktop_manager = getenv("NNN_DE_FILE_MANAGER");

	/* Get the default copier, if set */
	copier = getenv("NNN_COPIER");

	signal(SIGINT, SIG_IGN);

	/* Test initial path */
	if (canopendir(ipath) == 0) {
		fprintf(stderr, "%s: %s\n", ipath, strerror(errno));
		exit(1);
	}

	/* Set locale */
	setlocale(LC_ALL, "");

	initcurses();
	browse(ipath, ifilter);
	exitcurses();
	exit(0);
}
