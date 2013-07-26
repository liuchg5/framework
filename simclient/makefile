TARGET = ./simclient

CC = gcc
XX = g++
CFLAGS = -Wall -O -pg -g
LFLAGS = -Wall -O -pg -g -lpthread 

SOURCES = $(wildcard *.c *.cpp)	
OBJS = $(patsubst %.c,%.o,$(patsubst %.cpp,%.o,$(SOURCES)))

$(TARGET): $(OBJS)
	$(XX) *.o $(LFLAGS) -o $(TARGET)
	chmod a+x  $(TARGET)
	
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.cpp
	$(XX) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(TARGET)
	rm -rf $(OBJS)






	
	
