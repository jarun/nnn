/*
 * BSD 2-Clause License
 *
 * Copyright (C) 2014-2016, Lazaros Koromilas <lostd@2f30.org>
 * Copyright (C) 2014-2016, Dimitris Papastamos <sin@2f30.org>
 * Copyright (C) 2016-2019, Arun Prakash Jana <engineerarun@gmail.com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * * Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 *
 * * Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include <curses.h>

#define CONTROL(c) ((c) ^ 0x40)

/* Supported actions */
enum action {
	SEL_BACK = 1,
	SEL_GOIN,
	SEL_NAV_IN,
	SEL_NEXT,
	SEL_PREV,
	SEL_PGDN,
	SEL_PGUP,
	SEL_CTRL_D,
	SEL_CTRL_U,
	SEL_HOME,
	SEL_END,
	SEL_FIRST,
	SEL_CDHOME,
	SEL_CDBEGIN,
	SEL_CDLAST,
	SEL_CDROOT,
	SEL_VISIT,
	SEL_LEADER,
	SEL_CYCLE,
	SEL_CYCLER,
	SEL_CTX1,
	SEL_CTX2,
	SEL_CTX3,
	SEL_CTX4,
	SEL_PIN,
	SEL_FLTR,
	SEL_MFLTR,
	SEL_TOGGLEDOT,
	SEL_DETAIL,
	SEL_STATS,
	SEL_ARCHIVE,
	SEL_ARCHIVELS,
	SEL_EXTRACT,
	SEL_FSIZE,  /* file size */
	SEL_ASIZE,  /* apparent size */
	SEL_BSIZE,  /* block size */
	SEL_EXTN,   /* order by extension */
	SEL_MTIME,
	SEL_REDRAW,
	SEL_SEL,
	SEL_SELMUL,
	SEL_SELALL,
	SEL_SELLST,
	SEL_SELEDIT,
	SEL_CP,
	SEL_MV,
	SEL_CPMVAS,
	SEL_RMMUL,
	SEL_RM,
	SEL_OPENWITH,
	SEL_NEW,
	SEL_RENAME,
	SEL_RENAMEMUL,
	SEL_ARCHIVEMNT,
	SEL_SSHFS,
	SEL_UMOUNT,
	SEL_HELP,
	SEL_EXEC,
	SEL_SHELL,
	SEL_PLUGKEY,
	SEL_PLUGIN,
	SEL_LAUNCH,
	SEL_RUNCMD,
	SEL_RUNEDIT,
	SEL_RUNPAGE,
	SEL_LOCK,
	SEL_SESSIONS,
	SEL_QUITCTX,
	SEL_QUITCD,
	SEL_QUIT,
	SEL_CLICK,
};

/* Associate a pressed key to an action */
struct key {
	int sym;         /* Key pressed */
	enum action act; /* Action */
};

static struct key bindings[] = {
	/* Back */
	{ KEY_LEFT,       SEL_BACK },
	{ 'h',            SEL_BACK },
	/* Inside or select */
	{ KEY_ENTER,      SEL_GOIN },
	{ '\r',           SEL_GOIN },
	/* Pure navigate inside */
	{ KEY_RIGHT,      SEL_NAV_IN },
	{ 'l',            SEL_NAV_IN },
	/* Next */
	{ 'j',            SEL_NEXT },
	{ KEY_DOWN,       SEL_NEXT },
	/* Previous */
	{ 'k',            SEL_PREV },
	{ KEY_UP,         SEL_PREV },
	/* Page down */
	{ KEY_NPAGE,      SEL_PGDN },
	/* Page up */
	{ KEY_PPAGE,      SEL_PGUP },
	/* Ctrl+D */
	{ CONTROL('D'),   SEL_CTRL_D },
	/* Ctrl+U */
	{ CONTROL('U'),   SEL_CTRL_U },
	/* First entry */
	{ KEY_HOME,       SEL_HOME },
	{ 'g',            SEL_HOME },
	{ CONTROL('A'),   SEL_HOME },
	/* Last entry */
	{ KEY_END,        SEL_END },
	{ 'G',            SEL_END },
	{ CONTROL('E'),   SEL_END },
	/* Go to first file */
	{ '\'',           SEL_FIRST },
	/* HOME */
	{ '~',            SEL_CDHOME },
	/* Initial directory */
	{ '@',            SEL_CDBEGIN },
	/* Last visited dir */
	{ '-',            SEL_CDLAST },
	/* Go to / */
	{ '`',            SEL_CDROOT },
	/* Visit marked directory */
	{ CONTROL('B'),   SEL_VISIT },
	/* Leader key */
	{ CONTROL('_'),   SEL_LEADER },
	{ ',',            SEL_LEADER },
	/* Cycle contexts in forward direction */
	{ '\t',        SEL_CYCLE },
	/* Cycle contexts in reverse direction */
	{ KEY_BTAB,       SEL_CYCLER },
	/* Go to/create context N */
	{ '1',            SEL_CTX1 },
	{ '2',            SEL_CTX2 },
	{ '3',            SEL_CTX3 },
	{ '4',            SEL_CTX4 },
	/* Mark a path to visit later */
	{ 'b',            SEL_PIN },
	/* Filter */
	{ '/',            SEL_FLTR },
	/* Toggle filter mode */
	{ KEY_IC,         SEL_MFLTR },
	{ CONTROL('N'),   SEL_MFLTR },
	/* Toggle hide .dot files */
	{ '.',            SEL_TOGGLEDOT },
	/* Detailed listing */
	{ 'd',            SEL_DETAIL },
	/* File details */
	{ 'D',            SEL_STATS },
	/* Create archive */
	{ 'f',            SEL_ARCHIVE },
	/* List archive */
	{ 'F',            SEL_ARCHIVELS },
	/* Extract archive */
	{ CONTROL('F'),   SEL_EXTRACT },
	/* Toggle sort by size */
	{ 'z',            SEL_FSIZE },
	/* Sort by apparent size including dir contents */
	{ 'A',            SEL_ASIZE },
	/* Sort by total block count including dir contents */
	{ 'S',            SEL_BSIZE },
	/* Sort by file extension */
	{ 'E',            SEL_EXTN },
	/* Toggle sort by time */
	{ 't',            SEL_MTIME },
	/* Redraw window */
	{ CONTROL('L'),   SEL_REDRAW },
	{ KEY_F(5),       SEL_REDRAW },
	/* Select current file path */
	{ CONTROL('J'),   SEL_SEL },
	{ ' ',            SEL_SEL },
	/* Toggle select multiple files */
	{ 'm',            SEL_SELMUL },
	{ CONTROL('K'),   SEL_SELMUL },
	/* Select all files in current dir */
	{ 'a',            SEL_SELALL },
	/* Show list of copied files */
	{ 'M',            SEL_SELLST },
	/* Edit selection buffer */
	{ 'K',            SEL_SELEDIT },
	/* Copy from selection buffer */
	{ 'P',            SEL_CP },
	/* Move from selection buffer */
	{ 'V',            SEL_MV },
	/* Copy/move from selection buffer and rename */
	{ 'w',            SEL_CPMVAS },
	/* Delete from selection buffer */
	{ 'X',            SEL_RMMUL },
	/* Delete currently selected */
	{ CONTROL('X'),   SEL_RM },
	/* Open in a custom application */
	{ CONTROL('O'),   SEL_OPENWITH },
	/* Create a new file */
	{ 'n',            SEL_NEW },
	/* Show rename prompt */
	{ CONTROL('R'),   SEL_RENAME },
	{ KEY_F(2),       SEL_RENAME },
	/* Rename contents of current dir */
	{ 'r',            SEL_RENAMEMUL },
	/* Mount an archive */
	{ 'T',            SEL_ARCHIVEMNT },
	/* Connect to server over SSHFS */
	{ 'c',            SEL_SSHFS },
	/* Disconnect a SSHFS mount point */
	{ 'u',            SEL_UMOUNT },
	/* Show help */
	{ '?',            SEL_HELP },
	/* Execute file */
	{ 'C',            SEL_EXEC },
	/* Run command */
	{ '!',            SEL_SHELL },
	{ CONTROL(']'),   SEL_SHELL },
	/* Plugin key */
	{ 'x',            SEL_PLUGKEY },
	{ ':',            SEL_PLUGKEY },
	{ ';',            SEL_PLUGKEY },
	/* Run a plugin */
	{ 'R',            SEL_PLUGIN },
	{ CONTROL('V'),   SEL_PLUGIN },
	/* Launcher */
	{ '=',            SEL_LAUNCH },
	/* Run a command */
	{ ']',            SEL_RUNCMD },
	{ CONTROL('P'),   SEL_RUNCMD },
	/* Open in EDITOR or PAGER */
	{ 'e',            SEL_RUNEDIT },
	{ 'p',            SEL_RUNPAGE },
	/* Lock screen */
	{ 'L',            SEL_LOCK },
	/* Quit a context */
	{ 'q',            SEL_QUITCTX },
	/* Change dir on quit */
	{ CONTROL('G'),   SEL_QUITCD },
	/* Quit */
	{ 'Q',            SEL_QUIT },
	{ CONTROL('Q'),   SEL_QUIT },
	{ KEY_MOUSE,      SEL_CLICK },
	{ 'U',            SEL_SESSIONS },
};
