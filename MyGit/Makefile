TAR = mygit
GIT_SRC = main.cpp $(wildcard src/*.cpp)
SERVER_SRC= $(wildcard ../MyGitServer/src/*.cpp)

GIT_OBJ = $(patsubst %.cpp, %.o, $(GIT_SRC))
SERVER_OBJ = $(patsubst %.cpp, %.o, $(SERVER_SRC))
SRC= $(GIT_SRC) $(SERVER_SRC)
OBJ= $(GIT_OBJ) $(SERVER_OBJ)
INCLUDE= -I./include -I../MyGitServer/include
CFLAGS = -std=c++17

$(TAR) : $(OBJ)
	g++ $(OBJ) -o $(TAR)


%.o : %.cpp
	g++ $(CFLAGS) $(INCLUDE) -c $< -o $@

.PHONY: clean

clean:
	rm -rf .mygit mygit $(GIT_OBJ)
