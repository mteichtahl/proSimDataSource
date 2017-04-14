

CC = gcc 
DEBUG = -g3 -O0 
CFLAGS = -I . $(DEBUG)
CDFLAGS = -arch x86_64

LDFLAGS = -lPoKeys -L./src -lusb-1.0 -L/usr/lib/ -L/usr/local/include \-lconfig -luv -lzlog -lpthread
SOURCES = ./src/main.c \
		  
		  
OBJECTS = $(SOURCES:.c=.o)


pokey: $(OBJECTS)
	$(CC) $(OBJECTS) -o app $(CFLAGS) $(LDFLAGS) $(CDFLAGS) 
	dsymutil ./app

clean:
	-rm -rf src/pokey.dSYM/

all: clean pokey

install: clean all
	mkdir -p logs
	touch logs/log
	brew install --HEAD libuv