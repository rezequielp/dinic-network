TARGET=dinic
CC=gcc
CFLAGS+= -O3 -Wall -Wextra -std=c99 -Wdeclaration-after-statement -Wbad-function-cast -Wstrict-prototypes -Wmissing-declarations -Wmissing-prototypes -Wno-unused-parameter -Werror -g
SOURCES=$(shell echo ../apifiles/*.c)
OBJECTS=$(SOURCES:.c=.o)
MAIN=main.o

all: $(TARGET)

$(TARGET): $(MAIN) $(OBJECTS)
	$(CC) $^ -o $@

clean:
	rm -f $(TARGET) $(OBJECTS) .depend *~

.depend: *.[ch]
	$(CC) -MM $(SOURCES) >.depend

-include .depend

.PHONY: clean all
