TARGET=dinic
CC=gcc
CFLAGS+= -O3 -Wall -Wextra -std=c99 -Wdeclaration-after-statement -Wbad-function-cast -Wstrict-prototypes -Wmissing-declarations -Wmissing-prototypes -Wno-unused-parameter -Werror -g
SOURCES=$(shell echo ../apifiles/*.c)
BSTRING_O=../apifiles/auxlibs/bstring/bstrlib.o
STACK_O=../apifiles/auxlibs/stack/stack.o
QUEUE_O=../apifiles/auxlibs/queue/queue.o
LEXER_O=../apifiles/auxlibs/lexer/lexer.o
OBJECTS=$(SOURCES:.c=.o)
MAIN=main.o

all: $(TARGET)

# BString requiere flags distintos:
$(BSTRING_O):CFLAGS=-ansi -Wall -Werror -pedantic -g

$(TARGET): $(MAIN) $(OBJECTS) $(BSTRING_O) $(STACK_O) $(QUEUE_O) $(LEXER_O) 
	$(CC) $^ -o $@

clean:
	rm -f $(TARGET) $(OBJECTS) $(BSTRING_O) $(STACK_O) $(QUEUE_O) $(LEXER_O) .depend *~

.depend: *.[ch]
	$(CC) -MM $(SOURCES) >.depend

-include .depend

.PHONY: clean all
