TARGET := app_wdt

SRCS	:= app_wdt.c

CROSS_COMPILE ?= arm-linux-

RM	:= rm -rf

CC	:= $(CROSS_COMPILE)gcc

all:
	$(CC) $(SRCS) -o $(TARGET)
clean:
	$(RM) $(TARGET) *.o
