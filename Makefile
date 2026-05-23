CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -pedantic
TARGET = des_elevator

SRCS = main.c simulation.c elevator.c passenger.c floor.c event.c logger.c file_manager.c random_seed.c statistics.c
OBJS = $(SRCS:.c=.o)

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS) -lm

%.o: %.c constants.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET) simulation_log.txt

run: $(TARGET)
	./$(TARGET)
