C_SRC=src/mongoose.c

CPP_SRC=src/Config.cpp\
	src/Mutex.cpp\
	src/Log.cpp\
	src/Version.cpp\
	src/Util.cpp\
	src/Properties.cpp\
	src/FileHelper.cpp\
	src/Client.cpp\
	src/TitanAdapter.cpp\
	src/Server.cpp
LIBS=

CFLAGS=-static-libstdc++ -c -Wall -O2 -DNO_CGI -DNO_SSL -DNDEBUG -Iinclude $(CROSS_CFLAGS)
LDFLAGS=$(CROSS_LDFLAGS) $(LIBS) -lpthread -ldl
EXEC=e2webserv
OBJ=$(CPP_SRC:.cpp=.o) $(C_SRC:.c=.o)

all: $(EXEC)

$(EXEC): $(OBJ)
	$(CXX) $(LDFLAGS) $(LIBS) $(OBJ) -o $@
	$(STRIP) $(EXEC)
	cp $(EXEC) $(ARCH)

.cpp.o:
	$(CXX) $(CFLAGS) -std=c++0x $< -o $@

.c.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -rf $(OBJ) $(EXEC)

