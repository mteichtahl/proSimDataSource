

CC = gcc 
DEBUG = -g3 -O0 
CFLAGS = 
CDFLAGS = -arch x86_64 

LDFLAGS = -c 
SOURCES = ./src/main.c \
		  
OBJECTS = $(SOURCES:.c=.o)


lib: $(OBJECTS)
	$(CC) $(LDFLAGS) $(CDFLAGS) $(OBJECTS) -o proSimDataSource.o $(CFLAGS) 
	$(CC) -shared -W1,-soname, -o libProSimDataSource.so src/main.o -luv

clean:
	-rm -rf src/pokey.dSYM/

all: clean pokey

install: clean all
	brew install --HEAD libuv