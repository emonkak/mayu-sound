# Makefile for mayu-sound

NAME = sound
VERSION = 0.0.0
CC = gcc
CFLAGS = 
TARGETS = sound.dll
SRC_DIST = sound.c
BIN_DIST = $(TARGETS)




all: build


build: $(TARGETS)

sound.dll: sound.o
	$(CC) $(CFLAGS) -O2 -shared -mno-cygwin -o $@ \
		-Wl,--whole-archive sound.o \
		-Wl,--no-whole-archive -lwinmm \
		-Wl,--kill-at
	strip -s $@

sound.o: sound.c
	$(CC) $(CFLAGS) -c sound.c


install: build
	if [ "x$(DEST_DIR)" = "x" ]; then \
		echo "Please run: make DEST_DIR=MAYU_PLUGINS_DIR install"; \
		false; \
	fi
	cp -vp $(TARGETS) $(DEST_DIR)


clean:
	rm -f *.o $(TARGETS)


any-dist:
	rm -rf $(NAME)-$(VERSION) $(NAME)-$(VERSION)$(DIST_SUFFIX).tar.bz2
	mkdir $(NAME)-$(VERSION)
	cp $(DIST) $(NAME)-$(VERSION)
	tar jcf $(NAME)-$(VERSION)$(DIST_SUFFIX).tar.bz2 $(NAME)-$(VERSION)
	rm -rf $(NAME)-$(VERSION)

src-dist:
	make DIST="$(SRC_DIST)" DIST_SUFFIX=-src any-dist
bin-dist:
	make DIST="$(BIN_DIST)" DIST_SUFFIX=-bin any-dist




# __END__
