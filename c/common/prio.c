#include <sched.h>
#include <stdbool.h>
#include <string.h>

#include "prio.h"

bool try_set_prio(int pri) {
  struct sched_param sched;

  memset(&sched, 0, sizeof(sched));

  if (pri > sched_get_priority_max(SCHED_RR))
    sched.sched_priority = sched_get_priority_max(SCHED_RR);
  else
    sched.sched_priority = pri;

  return sched_setscheduler(0, SCHED_RR, &sched) != -1;
}
