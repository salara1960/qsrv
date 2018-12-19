
NAME=vdev

CFLAG =-std=gnu99 -O0 -Wall -g -pipe -D_GNU_SOURCE -D_REENTERANT

MACHINE := $(shell uname -m)

ifeq ($(MACHINE), x86_64)
	LIB_DIR = /usr/lib64
else
	LIB_DIR = /usr/lib
endif


$(NAME): vdev.o vlib.o
	gcc -o $(NAME) vlib.o vdev.o -L$(LIB_DIR) -lpthread -ldl
	strip $(NAME)
vlib.o: vlib.c
	gcc -c vlib.c $(CFLAG)
vdev.o: vdev.c
	gcc -c vdev.c $(CFLAG)
clean:
	rm -f *.o $(NAME)


