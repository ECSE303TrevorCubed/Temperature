CC ?= gcc
CFLAGS ?= -Wall -Wextra -Icommon
LIBS = -lwiringPi

COMMON_SRCS = common/log.c common/prio.c
COMMON_HDRS = common/constants.h common/log.h common/data.h common/prio.h

POLL_SRCS = poll/main.c poll/poll.c
INTERR_SRCS = interrupt/main.c
THRESH_SRCS = threshold/main.c

TARGETS = temper_poll temper_interr temper_thresh

all: $(TARGETS)

temper_poll: poll/main.c $(COMMON_SRCS) $(COMMON_HDRS)
	$(CC) $(CFLAGS) -o $@ $(POLL_SRCS) $(COMMON_SRCS) $(LIBS)

temper_interr: interrupt/main.c $(COMMON_SRCS) $(COMMON_HDRS)
	$(CC) $(CFLAGS) -o $@ $(INTERR_SRCS) $(COMMON_SRCS) $(LIBS)

temper_thresh: threshold/main.c $(COMMON_SRCS) $(COMMON_HDRS)
	$(CC) $(CFLAGS) -o $@ $(THRESH_SRCS) $(COMMON_SRCS) $(LIBS)

install: $(TARGETS)
	install -d $(DESTDIR)$(out)/bin
	install -m755 temper_poll $(DESTDIR)$(out)/bin/temper_poll
	install -m755 temper_interr $(DESTDIR)$(out)/bin/temper_interr
	install -m755 temper_thresh $(DESTDIR)$(out)/bin/temper_thresh

clean:
	rm -f $(TARGETS)

.PHONY: all install clean
