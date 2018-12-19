CC=gcc
CXXFLAGS=-g -Wall -I./include
SRC=src
OBJDIR=obj
BINDIR=bin
INC_DIR=include

SRCFILES = y86-parser
SRCFILES_TSRC = $(addsuffix .c, $(SRCFILES))
SRCFILES_SRC = $(addprefix $(SRC)/, $(SRCFILES_TSRC))

SRCFILES_TOBJ = $(addsuffix .o, $(SRCFILES))
SRCFILES_OBJ = $(addprefix $(OBJDIR)/, $(SRCFILES_TOBJ))

DEP = $(SRCFILES_OBJ:.o=.d)

LIBS=
LINKS=$(addprefix -l, $(LIBS))

all: setup y86

setup:
	mkdir -p $(OBJDIR) $(BINDIR)

$(OBJDIR)/%.o: $(SRC)/%.c
	$(CC) $(CXXFLAGS) -c $< -o $@ $(LINKS)

y86: $(SRCFILES_OBJ) $(OBJDIR)/*.o 
	$(CC) $(CXXFLAGS) $? -o $(BINDIR)/y86 $(LINKS)

clean:
	rm -f $(OBJDIR)/*.o $(BINDIR)/*
	rm -r obj bin
