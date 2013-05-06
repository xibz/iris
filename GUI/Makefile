LIBS=-lgtk-3 -lgdk-3 -latk-1.0 -lgio-2.0 -lpangocairo-1.0 -lgdk_pixbuf-2.0 -lcairo-gobject -lpango-1.0 -lcairo -lgobject-2.0 -lglib-2.0  -I/usr/include/gtk-3.0 -I/usr/include/pango-1.0 -I/usr/include/gio-unix-2.0/ -I/usr/include/atk-1.0 -I/usr/include/cairo -I/usr/include/gdk-pixbuf-2.0 -I/usr/include/freetype2 -I/usr/include/glib-2.0 -I/usr/lib/x86_64-linux-gnu/glib-2.0/include -I/usr/include/pixman-1 -I/usr/include/libpng12
CFLAGS=-c
CC=gcc

all: iRis

iRis: Menubar.o chatBox.o About.o irisGui.o
	$(CC) Menubar.o chatBox.o About.o irisGui.o -o iRis $(LIBS)

Menubar.o: Menubar.c
	$(CC) $(CFLAGS) Menubar.c $(LIBS)
	
chatBox.o: chatBox.c
	$(CC) $(CFLAGS) chatBox.c $(LIBS)
	
About.o: About.c
	$(CC) $(CFLAGS) About.c $(LIBS)

irisGui.o: irisGui.c
	$(CC) $(CFLAGS) irisGui.c $(LIBS)
clean:
	rm -rf *o iRis
