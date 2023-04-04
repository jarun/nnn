#include "mach_gettime.h"
#include <mach/mach_time.h>

#define MT_NANO (+1.0E-9)
#define MT_GIGA UINT64_C(1000000000)

// TODO create a list of timers,
static double mt_timebase = 0.0;
static uint64_t mt_timestart = 0;

int clock_gettime(clockid_t clk_id, struct timespec *tp)
{
	kern_return_t retval = KERN_SUCCESS;

	if (clk_id == TIMER_ABSTIME) {
		if (!mt_timestart) { // only one timer, initialized on the first call to the TIMER
			mach_timebase_info_data_t tb;
			mach_timebase_info(&tb);
			mt_timebase = tb.numer;
			mt_timebase /= tb.denom;
			mt_timestart = mach_absolute_time();
		}

		double diff = (mach_absolute_time() - mt_timestart) * mt_timebase;
		tp->tv_sec = diff * MT_NANO;
		tp->tv_nsec = diff - (tp->tv_sec * MT_GIGA);
	} else { // other clk_ids are mapped to the corresponding mach clock_service
		clock_serv_t cclock;
		mach_timespec_t mts;

		host_get_clock_service(mach_host_self(), clk_id, &cclock);
		retval = clock_get_time(cclock, &mts);
		mach_port_deallocate(mach_task_self(), cclock);

		tp->tv_sec = mts.tv_sec;
		tp->tv_nsec = mts.tv_nsec;
	}

	return retval;
}

/*  Copyright (c) 2015-2018 Alf Watt - Open Source - https://opensource.org/licenses/MIT */
