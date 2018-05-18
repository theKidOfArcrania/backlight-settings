CC=g++
CFLAGS=-Wall -pedantic-errors 
PROG=backlight
OBJS=backlight.o 
DEPS=$(OBJS:%.o=%.d)
PREFIX=/usr/local

all: $(PROG)
.PHONY: all

install:
	@mkdir $(PREFIX)/bin 2> /dev/null || true
	cp $(PROG) $(PREFIX)/bin
	chown root:video $(PREFIX)/bin/$(PROG)
	chmod 2755 $(PREFIX)/bin/$(PROG)
	cp 81-backlight.rules /etc/udev/rules.d


.PHONY: install

$(PROG): $(OBJS)

%.d: %.cc
	@set -e; rm -f $@; \
	 $(CC) -MM $(CFLAGS_ALL) $< > $@.$$$$; \
	 sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	 rm -f $@.$$$$

%.o: %.cc
	$(CC) $(CFLAGS) -o $@ -c $<

.PHONY: clean
clean: 
	rm -fr $(OBJS) $(DEPS)

-include $(DEPS)

