CC=gcc
CXXFLAGS=-g -Wall -I./include
SRC=src
OBJDIR=obj
BINDIR=bin
INC_DIR=include
LEX=flex
YACC=bison
LEXLIB=-lfl
LCFLAGS=-02


P_SRCFILES = y86-parser isa yas-grammar yas
P_SRCFILES_TSRC = $(addsuffix .c, $(P_SRCFILES))
P_SRCFILES_SRC = $(addprefix $(SRC)/parser/, $(P_SRCFILES_TSRC))
P_OBJDIR = $(OBJDIR)/parser

S_SRCFILES = y86-exe y86-sim 
S_SRCFILES_TSRC = $(addsuffix .c, $(S_SRCFILES))
S_SRCFILES_SRC = $(addprefix $(SRC)/simulator/, $(S_SRCFILES_TSRC))
S_OBJDIR = $(OBJDIR)/simulator

P_SRCFILES_TOBJ = $(addsuffix .o, $(P_SRCFILES))
S_SRCFILES_TOBJ = $(addsuffix .o, $(S_SRCFILES))
P_SRCFILES_OBJ = $(addprefix $(P_OBJDIR)/, $(P_SRCFILES_TOBJ))
S_SRCFILES_OBJ = $(addprefix $(S_OBJDIR)/, $(S_SRCFILES_TOBJ))
# SOURCES := $(shell find $(SRC)/ -name '*.c')

DEP = $(P_SRCFILES_OBJ:.o=.d)

LIBS=
LINKS=$(addprefix -l, $(LIBS))

all: setup y86-parser y86-sim

show:
	echo $(S_SRCFILES_SRC)
	echo $(S_SRCFILES_OBJ)

setup:
	mkdir -p $(OBJDIR) $(BINDIR) $(P_OBJDIR) $(S_OBJDIR)

$(P_OBJDIR)/%.o: $(SRC)/parser/%.c
	$(CC) $(CXXFLAGS) -c $< -o $@ $(LINKS)

$(S_OBJDIR)/%.o: $(SRC)/simulator/%.c
	$(CC) $(CXXFLAGS) -c $< -o $@ $(LINKS)

y86-parser: $(P_SRCFILES_OBJ) 
	$(CC) $(CXXFLAGS) $? -o $(BINDIR)/y86-parser $(LINKS)

y86-sim: $(S_SRCFILES_OBJ) 
	$(CC) $(CXXFLAGS) $? -o $(BINDIR)/y86-sim $(LINKS)

clean:
	rm -r $(OBJDIR) $(BINDIR)
