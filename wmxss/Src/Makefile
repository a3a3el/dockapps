CC     = gcc
CFLAGS = -O2 -Wall
INCDIR = -I/usr/X11R6/include/X11 -I/usr/X11R6/include
DESTDIR= /usr/X11R6
LIBDIR = -L/usr/X11R6/lib

# for Linux
LIBS   = -lXpm -lX11 -lXext

# for Solaris
#LIBS   = -lXpm -lX11 -lXext -lsocket -lnsl

OBJS   = wmxss.o \
         xutils.o


.c.o:
	$(CC) $(CFLAGS) -D$(shell echo `uname -s`) -c $< -o $*.o $(INCDIR)


all:	wmxss.o wmxss

wmxss.o: wmxss_master.xpm wmxss_mask.xbm
wmxss:	$(OBJS)
	$(CC) $(COPTS) $(SYSTEM) -o wmxss $^ $(INCDIR) $(LIBDIR) $(LIBS)

clean:
	for i in $(OBJS) ; do \
		rm -f $$i; \
	done
	rm -f wmxss

install:: wmxss
	install -c -s -m 0755 wmxss $(DESTDIR)/bin
	install -c -m 0644 wmxss.1 $(DESTDIR)/man/man1

