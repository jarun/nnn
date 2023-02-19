/*
 * BSD 2-Clause License
 *
 * Copyright (C) 2014-2016, Lazaros Koromilas <lostd@2f30.org>
 * Copyright (C) 2014-2016, Dimitris Papastamos <sin@2f30.org>
 * Copyright (C) 2016-2023, Arun Prakash Jana <engineerarun@gmail.com>
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

#ifdef DEBUG
static int DEBUG_FD;

static int xprintf(int fd, const char *fmt, ...)
{
	char buf[BUFSIZ];
	int r;
	va_list ap;

	va_start(ap, fmt);
	r = vsnprintf(buf, sizeof(buf), fmt, ap);
	if (r > 0 && (unsigned int)r < sizeof(buf))
		r = write(fd, buf, r);
	va_end(ap);
	return r;
}

static int enabledbg(void)
{
	FILE *fp = fopen("/tmp/nnndbg", "w");

	if (!fp) {
		perror("dbg(1)");

		fp = fopen("./nnndbg", "w");
		if (!fp) {
			perror("dbg(2)");
			return -1;
		}
	}

	DEBUG_FD = dup(fileno(fp));
	fclose(fp);
	if (DEBUG_FD == -1) {
		perror("dbg(3)");
		return -1;
	}

	return 0;
}

static void disabledbg(void)
{
	close(DEBUG_FD);
}

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

#define DPRINTF_D(x) xprintf(DEBUG_FD, "ln " TOSTRING(__LINE__) ": " #x "=%d\n", x)
#define DPRINTF_U(x) xprintf(DEBUG_FD, "ln " TOSTRING(__LINE__) ": " #x "=%u\n", x)
#define DPRINTF_S(x) xprintf(DEBUG_FD, "ln " TOSTRING(__LINE__) ": " #x "=%s\n", x)
#define DPRINTF_P(x) xprintf(DEBUG_FD, "ln " TOSTRING(__LINE__) ": " #x "=%p\n", x)
#else
#define DPRINTF_D(x)
#define DPRINTF_U(x)
#define DPRINTF_S(x)
#define DPRINTF_P(x)
#endif /* DEBUG */
