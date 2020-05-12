CC = gcc
CFLAGS = -pthread -lrt
main: main_iris.c iris.c button_MAQnow_interruption.c button_onoff_interruption.c ../fsm.c ../tmr.c ../task.c iris.h
	$(CC) main_iris.c iris.c button_MAQnow_interruption.c button_onoff_interruption.c ../fsm.c ../tmr.c ../task.c -o iris $(CFLAGS)
