#ifndef DEVICES_TIMER_H
#define DEVICES_TIMER_H

#include <round.h>
#include <stdint.h>
#include "lib/kernel/list.h"
#include "threads/thread.h"
#include "threads/synch.h"

/* Number of timer interrupts per second. */
#define TIMER_FREQ 100

struct sleep_elem
{
  struct thread t;
  struct list_elem elem;
  int64_t tickStopSleep; //the tick at which the thread should stop sleeping
  struct semaphore s;
};

void timer_init (void);
void timer_calibrate (void);

int64_t timer_ticks (void);
int64_t timer_elapsed (int64_t);

void timer_sleep (int64_t ticks);
void timer_msleep (int64_t milliseconds);
void timer_usleep (int64_t microseconds);
void timer_nsleep (int64_t nanoseconds);

void timer_print_stats (void);

#endif /* devices/timer.h */
