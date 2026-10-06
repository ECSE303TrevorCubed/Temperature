CC ?= gcc
CFLAGS ?= -Wall -Wextra -Icommon
LIBS = -lwiringPi

COMMON_SRCS = common/log.c common/prio.c
COMMON_HDRS = common/constants.h common/log.h common/data.h common/prio.h

POLL_SRCS = poll/main.c poll/poll.c
POLL_HDRS = poll/poll.h
INTERR_SRCS = interrupt/main.c interrupt/interrupt.c
INTERR_HDRS = interrupt/interrupt.h
THRESH_SRCS = threshold/main.c poll/poll.c threshold/threshold.c
THRESH_HDRS = poll/poll.h threshold/threshold.h
DISPLAY_HDRS = poll/poll.h display/display.h
DISPLAY_SRCS = display/display.c display/main.c

TARGETS = temper_poll temper_interr temper_thresh temper_display

all: $(TARGETS)

temper_poll: $(POLL_SRCS) $(COMMON_SRCS) $(COMMON_HDRS) $(POLL_HDRS)
	$(CC) $(CFLAGS) -o $@ $(POLL_SRCS) $(COMMON_SRCS) $(LIBS)

temper_interr: $(INTERR_SRCS) $(COMMON_SRCS) $(COMMON_HDRS) $(INTERR_HDRS)
	$(CC) $(CFLAGS) -o $@ $(INTERR_SRCS) $(COMMON_SRCS) $(LIBS)

temper_thresh: $(THRESH_SRCS) $(COMMON_SRCS) $(COMMON_HDRS) $(THRESH_HDRS)
	$(CC) $(CFLAGS) -Ipoll -o $@ $(THRESH_SRCS) $(COMMON_SRCS) $(LIBS)

temper_display: $(DISPLAY_SRCS) $(COMMON_SRCS) $(COMMON_HDRS) $(DISPLAY_HDRS)
	$(CC) $(CFLAGS) -Idisplay -o $@ $(DISPLAY_SRCS) $(COMMON_SRCS) $(LIBS)

install: $(TARGETS)
	install -d $(DESTDIR)$(out)/bin
	install -m755 temper_poll $(DESTDIR)$(out)/bin/temper_poll
	install -m755 temper_interr $(DESTDIR)$(out)/bin/temper_interr
	install -m755 temper_thresh $(DESTDIR)$(out)/bin/temper_thresh
	install -m755 temper_display $(DESTDIR)$(out)/bin/temper_display

clean:
	rm -f $(TARGETS)

.PHONY: all install clean
