CC = gcc
CFLAGS = -pthread -lrt -g -Wall
main: main_sensor.c sensor_ack.c sensor.c ../fsm.c ../tmr.c ../task.c sensor.h
	$(CC) main_sensor.c sensor_ack.c sensor.c ../fsm.c ../tmr.c ../task.c -o sensor $(CFLAGS)

clean:
	$(RM) *~ sensor
