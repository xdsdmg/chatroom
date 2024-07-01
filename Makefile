FLAGS = -Wall -g -lssh

build:
	gcc main.cpp -lstdc++ -lsshs

ssh_server: ssh_server.c
	gcc -o ssh_server ssh_server.c $(FLAGS)
