C_SRC=src/mongoose.c

CPP_SRC=src/Config.cpp\
	src/Mutex.cpp\
	src/Log.cpp\
	src/Version.cpp\
	src/Util.cpp\
	src/FileHelper.cpp\
	src/Client.cpp\
	src/TitanAdapter.cpp\
	src/Server.cpp
LIBS=

#CC=gcc
CC=/tdt/tdt/tufsbox/devkit/sh4/bin/sh4-linux-gcc

#CXX=g++
CXX=/tdt/tdt/tufsbox/devkit/sh4/bin/sh4-linux-g++

#CROSS_CFLAGS=
CROSS_CFLAGS=-I/tdt/tdt/tufsbox/devkit/sh4/include

CFLAGS=-c -Wall -O2 -DNO_CGI -DNO_SSL -DNDEBUG -Iinclude $(CROSS_CFLAGS)

CROSS_LDFLAGS=-L/tdt/tdt/tufsbox/devkit/sh4/lib
#CROSS_LDFLAGS=

LDFLAGS=$(CROSS_LDFLAGS) $(LIBS) -lpthread -ldl
EXEC=e2webserv
OBJ=$(CPP_SRC:.cpp=.o) $(C_SRC:.c=.o)

all: $(EXEC)

$(EXEC): $(OBJ)
	$(CXX) $(LDFLAGS) $(LIBS) $(OBJ) -o $@
	cp $(EXEC) sh4/v1.1

.cpp.o:
	$(CXX) $(CFLAGS) $< -o $@

.c.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -rf $(OBJ) $(EXEC)

tpk: $(EXEC)
	cp $(EXEC) tpk-raw/flash/mnt/bin
	cp $(EXEC) tpk-raw/swap/var/swap/bin
	$(JAVA_HOME)/bin/java -jar java/tpk.jar tpk-raw/flash tpk/titan-plugin-network-e2webserv_1.1.9_sh4.tpk
	$(JAVA_HOME)/bin/java -jar java/tpk.jar tpk-raw/swap tpk/titan-plugin-swapnetwork-e2webserv_1.1.9_sh4.tpk
	gzip tpk/titan-plugin-network-e2webserv_1.1.9_sh4.tpk
	gzip tpk/titan-plugin-swapnetwork-e2webserv_1.1.9_sh4.tpk
