CC=g++
CFLAGS=-pedantic -Wall -Wextra -std=c++11 -g
FILES = ipk-client.cpp md5.cpp
	
all: ipk-client

ipk-client: $(FILES)
	$(CC) $^ $(CFLAGS) -o $@
