### Edit here as you like :)
BINDIR = /usr/local/bin
###

DEST = wmomikuzi

# Programs used to move the binary to the proper place.
INSTALL = /usr/bin/install
INSTALL_PROGRAM = ${INSTALL} -m 755
UNINSTALL_PROGRAM = rm -rf

CC = gcc
OPTIMIZE = -O2
XLIBDIR = /usr/X11R6/lib
XLIB = -ldockapp -lXpm -lXext -lX11
OBJ = wmomikuzi.o

$(DEST) : $(OBJ)
	$(CC) $(OBJ) -L$(XLIBDIR) $(XLIB) -o $(DEST)

wmomikuzi.o: wmomikuzi.c
	$(CC) $(OPTIMIZE) $< -c

install:
	$(INSTALL_PROGRAM) ./$(DEST) $(BINDIR)/$(DEST)

uninstall:
	$(UNINSTALL_PROGRAM) $(BINDIR)/$(DEST)

clean:
	rm -rf $(DEST) $(OBJ) *~
