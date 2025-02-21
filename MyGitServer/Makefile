GITSERVER_SRC= $(wildcard src/*.cpp)
GIT_SRC= $(wildcard ../MyGit/src/*.cpp)
SERVER_SRC= $(GITSERVER_SRC) $(GIT_SRC) test/server.cpp
CLIENT_SRC=$(GITSERVER_SRC) $(GIT_SRC) test/client.cpp
TEST_SRC= $(GITSERVER_SRC) test/test.cpp
SERVER_OBJ = $(patsubst %.cpp, %.o, $(SERVER_SRC))
CLIENT_OBJ = $(patsubst %.cpp, %.o, $(CLIENT_SRC))
INCLUDE = -I./include -I../MyGit/include
BIN=bin

all: server client

server: $(SERVER_OBJ)
	@mkdir -p $(BIN)
	g++ $(SERVER_OBJ) -o $(BIN)/server

client: $(CLIENT_OBJ)
	g++ $(CLIENT_OBJ) -o $(BIN)/client

%.o: %.cpp
	g++ $(INCLUDE) -c $< -o $@

.PHONY:clean

clean:
	rm -f $(BIN)/* $(SERVER_OBJ) $(CLIENT_OBJ)