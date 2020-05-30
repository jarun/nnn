/*
 * BSD 2-Clause License
 *
 * Copyright (C) 2014-2016, Lazaros Koromilas <lostd@2f30.org>
 * Copyright (C) 2014-2016, Dimitris Papastamos <sin@2f30.org>
 * Copyright (C) 2016-2020, Arun Prakash Jana <engineerarun@gmail.com>
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

/* Handle variable CTX_MAX */
#ifndef CTX_MAX
#    define CTX_MAX 4
#elif (CTX_MAX < 1) || (CTX_MAX > 8)
#    error "CTX_MAX should be in range 1..8"
#endif

#define REPEAT(num, macro) REPEAT_START(num, macro)
#define REPEAT_START(num, macro) REPEAT_##num(macro)
#define REPEAT_1(macro) macro(1)
#define REPEAT_2(macro) REPEAT_1(macro) macro(2)
#define REPEAT_3(macro) REPEAT_2(macro) macro(3)
#define REPEAT_4(macro) REPEAT_3(macro) macro(4)
#define REPEAT_5(macro) REPEAT_4(macro) macro(5)
#define REPEAT_6(macro) REPEAT_5(macro) macro(6)
#define REPEAT_7(macro) REPEAT_6(macro) macro(7)
#define REPEAT_8(macro) REPEAT_7(macro) macro(8)

#define SEL_CTX(num) SEL_CTX##num,
#define CTX_KEYDEF(num) {'0' + (num), SEL_CTX##num},

#define CONTROL(c) ((c) & 0x1f)

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
	SEL_BOOKMARK,
	SEL_REMOTE,
	SEL_CYCLE,
	SEL_CYCLER,

	REPEAT(CTX_MAX, SEL_CTX)

	SEL_PIN,
	SEL_FLTR,
	SEL_MFLTR,
	SEL_HIDDEN,
	SEL_DETAIL,
	SEL_STATS,
	SEL_CHMODX,
	SEL_ARCHIVE,
	SEL_SORT,
	SEL_REDRAW,
	SEL_SEL,
	SEL_SELMUL,
	SEL_SELALL,
	SEL_SELEDIT,
	SEL_CP,
	SEL_MV,
	SEL_CPMVAS,
	SEL_RM,
	SEL_OPENWITH,
	SEL_NEW,
	SEL_RENAME,
	SEL_RENAMEMUL,
	SEL_UMOUNT,
	SEL_HELP,
	SEL_AUTONEXT,
	SEL_EDIT,
	SEL_PLUGIN,
	SEL_SHELL,
	SEL_LAUNCH,
	SEL_RUNCMD,
	SEL_LOCK,
	SEL_SESSIONS,
	SEL_EXPORT,
	SEL_TIMETYPE,
	SEL_QUITCTX,
	SEL_QUITCD,
	SEL_QUIT,
	SEL_QUITFAIL,
#ifndef NOMOUSE
	SEL_CLICK,
#endif
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
	/* Leader key */
	{ 'b',            SEL_BOOKMARK },
	{ CONTROL('_'),   SEL_BOOKMARK },
	/* Connect to server over SSHFS */
	{ 'c',            SEL_REMOTE },
	/* Cycle contexts in forward direction */
	{ '\t',           SEL_CYCLE },
	/* Cycle contexts in reverse direction */
	{ KEY_BTAB,       SEL_CYCLER },
	/* Go to/create context N */
	REPEAT(CTX_MAX, CTX_KEYDEF)
	/* Mark a path to visit later */
	{ ',',            SEL_PIN },
	/* Filter */
	{ '/',            SEL_FLTR },
	/* Toggle filter mode */
	{ CONTROL('N'),   SEL_MFLTR },
	/* Toggle hide .dot files */
	{ '.',            SEL_HIDDEN },
	/* Detailed listing */
	{ 'd',            SEL_DETAIL },
	/* File details */
	{ 'f',            SEL_STATS },
	{ CONTROL('F'),   SEL_STATS },
	/* Toggle executable status */
	{ '*',            SEL_CHMODX },
	/* Create archive */
	{ 'z',            SEL_ARCHIVE },
	/* Sort toggles */
	{ 't',            SEL_SORT },
	{ CONTROL('T'),   SEL_SORT },
	/* Redraw window */
	{ CONTROL('L'),   SEL_REDRAW },
	/* Select current file path */
	{ CONTROL('J'),   SEL_SEL },
	{ ' ',            SEL_SEL },
	/* Toggle select multiple files */
	{ 'm',            SEL_SELMUL },
	{ CONTROL('K'),   SEL_SELMUL },
	/* Select all files in current dir */
	{ 'a',            SEL_SELALL },
	/* List, edit selection */
	{ 'E',            SEL_SELEDIT },
	/* Copy from selection buffer */
	{ 'p',            SEL_CP },
	{ CONTROL('P'),   SEL_CP },
	/* Move from selection buffer */
	{ 'v',            SEL_MV },
	{ CONTROL('V'),   SEL_MV },
	/* Copy/move from selection buffer and rename */
	{ 'w',            SEL_CPMVAS },
	{ CONTROL('W'),   SEL_CPMVAS },
	/* Delete from selection buffer */
	{ 'x',            SEL_RM },
	{ CONTROL('X'),   SEL_RM },
	/* Open in a custom application */
	{ 'o',            SEL_OPENWITH },
	{ CONTROL('O'),   SEL_OPENWITH },
	/* Create a new file */
	{ 'n',            SEL_NEW },
	/* Show rename prompt */
	{ CONTROL('R'),   SEL_RENAME },
	/* Rename contents of current dir */
	{ 'r',            SEL_RENAMEMUL },
	/* Disconnect a SSHFS mount point */
	{ 'u',            SEL_UMOUNT },
	/* Show help */
	{ '?',            SEL_HELP },
	/* Quit a context */
	{ '+',            SEL_AUTONEXT },
	/* Edit in EDITOR */
	{ 'e',            SEL_EDIT },
	/* Run a plugin */
	{ ';',            SEL_PLUGIN },
	{ CONTROL('S'),   SEL_PLUGIN },
	/* Run command */
	{ '!',            SEL_SHELL },
	{ CONTROL(']'),   SEL_SHELL },
	/* Launcher */
	{ '=',            SEL_LAUNCH },
	/* Run a command */
	{ ']',            SEL_RUNCMD },
	/* Lock screen */
	{ '0',            SEL_LOCK },
	/* Manage sessions */
	{ 's',            SEL_SESSIONS },
	/* Export list */
	{ '>',            SEL_EXPORT },
	/* Set time type */
	{ 'T',            SEL_TIMETYPE },
	/* Quit a context */
	{ 'q',            SEL_QUITCTX },
	/* Change dir on quit */
	{ CONTROL('G'),   SEL_QUITCD },
	/* Quit */
	{ CONTROL('Q'),   SEL_QUIT },
	/* Quit with an error code */
	{ 'Q',            SEL_QUITFAIL },
#ifndef NOMOUSE
	{ KEY_MOUSE,      SEL_CLICK },
#endif
};
