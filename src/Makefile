all: main
CC = g++
CFLAGS = -g

MEMOBJS = memory.cpp test_memory.cpp
BUFOBJS = memory.cpp buffer.cpp test_buffer.cpp
MACOBJS = memory.cpp buffer.cpp mac.cpp test_mac.cpp

main: main.cpp memory.cpp memory.hpp buffer.cpp buffer.hpp mac.cpp mac.hpp
	$(CC) $(CFLAGS) -o main $(OBJS)

memtest: test_memory.cpp memory.cpp memory.hpp
	$(CC) -o memtest $(MEMOBJS)

buftest: test_buffer.cpp buffer.cpp buffer.hpp memory.cpp memory.hpp
	$(CC) -o buftest $(BUFOBJS)

mactest: test_mac.cpp mac.cpp mac.hpp buffer.cpp buffer.hpp memory.cpp memory.hpp
	$(CC) $(CFLAGS) -o mactest $(MACOBJS)

clean:
	rm -f *~ *.o memtest buftest mactest main
