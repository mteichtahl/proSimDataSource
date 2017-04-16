

CC = gcc 
DEBUG = -g3 -O0 
CFLAGS += -luv -lpthread -I./elements/

LDFLAGS =  
SOURCES = ./src/main.c \
		  ./src/elements/elements.c
		  
OBJECTS = $(SOURCES:.c=.o)


lib: $(OBJECTS)
	$(CC) -o proSimDataSource.o $(LDFLAGS) $(CDFLAGS) $(OBJECTS) $(CFLAGS) 
	$(CC) $(CFLAGS) -shared -W1,-soname, -o libProSimDataSource.so $(OBJECTS)

clean:
	-rm -rf src/pokey.dSYM/

all: clean lib
	 cp libProSimDataSource.so ../simPokey/bin/.
	
install: clean all
	brew install --HEAD libuv