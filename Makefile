

CC = gcc 
DEBUG = -g3 -O0 
CFLAGS += -luv -lpthread -lzlog -I./elements/

LDFLAGS =  
SOURCES = ./src/main.c \
		  ./src/elements/elements.c
		  
OBJECTS = $(SOURCES:.c=.o)

lib: $(OBJECTS) 
	$(CC) -o proSimDataSource.o $(LDFLAGS) $(CDFLAGS) $(OBJECTS) $(CFLAGS) 
	$(CC) $(CFLAGS) -shared -W1,-soname, -o libProSimDataSource.so $(OBJECTS)

clean:
	-rm -rf src/pokey.dSYM/
	-rm -rf *.o
	-rm -f src/*.o
	-rm -f *.so
	-rm -f ./src/elements/*.o

all: clean lib
	 cp libProSimDataSource.so ../simPokey/libs/.
	
install: clean all
	#brew install --HEAD libuv

