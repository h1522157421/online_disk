CC=g++
FLAG=-c
OUTPUT=-o
SRCLIST=$(wildcard *.cpp)
OBJLIST=$(SRCLIST:%.cpp=%.o)
OBJ=online_disk_server

$(OBJ):$(OBJLIST)
	$(CC) $(OBJLIST) $(OUTPUT) $@ -L/usr/lib64/mysql -lmysqlclient

%.o:%.cpp
	$(CC) $(FLAG) $< $(OUTPUT) $@

.PHONY:clean
clean:
	rm -rf *.o $(OBJ)

