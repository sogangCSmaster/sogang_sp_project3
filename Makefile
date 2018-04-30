C = gcc
NAME = 20131575
OBJS = $(NAME).c
TARGET = $(NAME).out

.SUFFIXES : .c .o

all : $(TARGET)
	
$(TARGET) : $(OBJS) $(NAME).h
			$(CC) -g -Wall -o $@ $(OBJS) -lm

clean : 
	rm -f $(TARGET)
