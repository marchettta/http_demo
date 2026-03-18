CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra

SRC_DIR = src
BIN_DIR = bin

all: dirs \/http_server \/http_client

dirs:
	mkdir -p \

\/http_server: \/server.cpp
	\ \ -o \/http_server \/server.cpp

\/http_client: \/client.cpp
	\ \ -o \/http_client \/client.cpp

clean:
	rm -rf \/*

.PHONY: all clean dirs
