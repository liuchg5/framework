TARGET = ./midsrv

CC = gcc
XX = g++
CFLAGS = -Wall -O0 -pg -g

SOURCES = $(wildcard *.c *.cpp)	
OBJS = $(patsubst %.c,%.o,$(patsubst %.cpp,%.o,$(SOURCES)))

$(TARGET): $(OBJS)
	$(XX) $(CFLAGS) *.o -o $(TARGET)
	chmod a+x  $(TARGET)
	
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.cpp
	$(XX) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(TARGET)
	rm -rf $(OBJS)






	
	
