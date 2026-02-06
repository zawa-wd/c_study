CC = gcc

CFLAGS = -Wall
LIBS = -lm

TARGET = sensor_fusion
CSVFILE = sensor_log.csv

SRCS = sensor_fusion.c

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRCS) $(LIBS)

clean:
	rm -f $(TARGET) $(CSVFILE)
