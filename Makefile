PROG = lang
CC = g++
FLAGS = -std=c++14 -Wall -O3
OBJS = main.o tokenize.o parse.o execute.o helpers.o

$(PROG) : $(OBJS)
	$(CC) $(FLAGS) -o $(PROG) $(OBJS)
main.o :
	$(CC) $(FLAGS) -c main.cpp
tokenize.o :
	$(CC) $(FLAGS) -c tokenize.cpp
parse.o :
	$(CC) $(FLAGS) -c parse.cpp
execute.o :
	$(CC) $(FLAGS) -c execute.cpp
helpers.o :
	$(CC) $(FLAGS) -c helpers.cpp
clean :
	rm -f $(PROG) $(OBJS)
