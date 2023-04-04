#ifndef mach_time_h
#define mach_time_h

#include <sys/types.h>
#include <sys/_types/_timespec.h>
#include <mach/mach.h>
#include <mach/clock.h>

/* The opengroup spec isn't clear on the mapping from REALTIME to CALENDAR
 being appropriate or not.
 http://pubs.opengroup.org/onlinepubs/009695299/basedefs/time.h.html */

// XXX only supports a single timer
#define TIMER_ABSTIME -1
#define CLOCK_REALTIME CALENDAR_CLOCK
#define CLOCK_MONOTONIC SYSTEM_CLOCK

typedef int clockid_t;

/* the mach kernel uses struct mach_timespec, so struct timespec
	is loaded from <sys/_types/_timespec.h> for compatibility */
// struct timespec { time_t tv_sec; long tv_nsec; };

int clock_gettime(clockid_t clk_id, struct timespec *tp);

#endif

/*  Copyright (c) 2015-2018 Alf Watt - Open Source - https://opensource.org/licenses/MIT */
