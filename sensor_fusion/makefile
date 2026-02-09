CC = gcc

CFLAGS = -Iinclude -Wall
LIBS = -lm

TARGET = navi_sim
CSVFILE = log/*.csv

SRCS = src/main.c src/sensor_fusion.c

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) $(SRCS) -o $(TARGET) $(LIBS)

clean:
	rm -f $(TARGET) $(CSVFILE)
