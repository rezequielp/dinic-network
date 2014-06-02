TARGET=dinic
CC=gcc
CFLAGS+= -ansi -Wall -Wextra -Wdeclaration-after-statement -Wbad-function-cast -Wstrict-prototypes -Wmissing-declarations -Wmissing-prototypes -Wno-unused-parameter -Werror -pedantic -g
SOURCES=$(shell echo *.c)
BSTRING_OBJECTS=bstring/bstrlib.o
STACK_OBJECTS=auxlibs/queue&stack/stack.o
LEXER_OBJECTS=auxlibs/lexer/lexer.o
OBJECTS=$(SOURCES:.c=.o)

all: $(TARGET)

# BString requiere flags distintos:
$(BSTRING_OBJECTS):CFLAGS=-ansi -Wall -Werror -pedantic -g

$(TARGET): $(OBJECTS) $(BSTRING_OBJECTS) $(STACK_OBJECTS) $(LEXER_OBJECTS)
	$(CC) $^ -o $@

clean:
	rm -f $(TARGET) $(OBJECTS) $(BSTRING_OBJECTS) $(STACK_OBJECTS) $(LEXER_OBJECTS) .depend *~

.depend: *.[ch]
	$(CC) -MM $(SOURCES) >.depend

-include .depend

.PHONY: clean all
