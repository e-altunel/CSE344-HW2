CC = gcc
TESTCC = g++
RELEASE_FLAGS = -O2 -Wall -Wextra -Werror -std=c99 -pedantic -DNDEBUG
AR = ar
ARFLAGS = rcs

SRCDIR = src
OBJDIR = obj
INCDIR = inc
LIBDIR = lib
BINDIR = bin

SRC = $(wildcard $(SRCDIR)/*.c)
OBJ = $(patsubst $(SRCDIR)/%.c, $(OBJDIR)/%.o, $(SRC))
INC = $(wildcard $(INCDIR)/*.h)

TARGET_NAME = cse344
TARGET = lib$(TARGET_NAME).a
NAME = main

TARGET_PATH = $(LIBDIR)/$(TARGET)
NAME_PATH = $(BINDIR)/$(NAME).out

all: $(TARGET_PATH) $(NAME_PATH)

run: all
	@echo "\033[1;32mRunning...\033[0m"
	@./$(NAME_PATH)

$(TARGET_PATH): $(OBJ)
	@mkdir -p $(LIBDIR)
	@echo "\033[1;33mCreating Library\033[0m $@"
	@$(AR) $(ARFLAGS) $@ $^

$(NAME_PATH): $(TARGET_PATH) $(OBJDIR)/$(NAME).o
	@mkdir -p $(BINDIR)
	@echo "\033[1;33mLinking\033[0m $<"
	@$(CC) $(RELEASE_FLAGS) -I$(INCDIR) -L$(LIBDIR) -o $@ $(OBJDIR)/$(NAME).o -l$(TARGET_NAME)

$(OBJDIR)/$(NAME).o: $(NAME).c $(INC)
	@mkdir -p $(OBJDIR)
	@echo "\033[1;33mCompiling\033[0m $<"
	@$(CC) $(RELEASE_FLAGS) -I$(INCDIR) -c $< -o $@

$(OBJDIR)/%.o: $(SRCDIR)/%.c $(INC)
	@mkdir -p $(OBJDIR)
	@echo "\033[1;33mCompiling\033[0m $<"
	@$(CC) $(RELEASE_FLAGS) -I$(INCDIR) -c $< -o $@

clean:
	@echo "\033[1;31mCleaning...\033[0m"
	@rm -rf $(OBJDIR) $(DOBJDIR) $(LIBDIR) $(BINDIR) $(TESTBINDIR) $(TESTOBJDIR) $(OBJDIR)/$(NAME).o *.out *.i .vscode fifo1 fifo2

re: clean all

.PHONY: all clean re debug test
.PRECIOUS: $(OBJ) $(DOBJ) $(TESTOBJ)