#ifndef TEMEPR_PRIO_H_
#define TEMEPR_PRIO_H_

#include <stdbool.h>

// Attempt to set a the priority schedulling for the running program
bool try_set_prio(int pri);

#endif