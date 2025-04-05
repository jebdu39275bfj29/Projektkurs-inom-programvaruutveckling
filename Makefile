# Makefile for Triangle Passing Combination (Cross-platform)

CC = gcc
SRCDIR = ./source

### Nedan är kod för Windows
#INCLUDE = C:\msys64\mingw64\include\SDL2
#CFLAGS = -g -I$(INCLUDE) -c
#LDFLAGS = -lmingw32 -lSDL2main -lSDL2_image -lSDL2 -mwindows
#EXECUTABLE = triangle_sim


### Nedan är kod för MacOs
INCLUDE = /opt/homebrew/include
LIBDIR = /opt/homebrew/lib
CFLAGS = -g -I$(INCLUDE) -D_THREAD_SAFE -c
LDFLAGS = -L$(LIBDIR) -lSDL2main -lSDL2_image -lSDL2
EXECUTABLE = triangle_sim


vpath %.h $(SRCDIR)
vpath %.c $(SRCDIR)

# OS check (works in Git Bash / MSYS2 / Linux / macOS / Windows CMD)
UNAME_S := $(shell uname 2>/dev/null)
ifeq ($(UNAME_S),)
    # Probably Windows CMD or PowerShell
    RM = del /Q
    RM_EXEC = del /Q
    RM_SUFFIX = .exe
else
    # Bash-based (MSYS2, Git Bash, Linux, macOS)
    RM = rm -f
    RM_EXEC = rm -f
    RM_SUFFIX =
endif

all: $(EXECUTABLE)

$(EXECUTABLE): $(SRCDIR)/main.o $(SRCDIR)/GameModel.o $(SRCDIR)/GameView.o $(SRCDIR)/GameController.o
	$(CC) $^ -o $@ $(LDFLAGS)

$(SRCDIR)/main.o: main.c GameModel.h GameView.h GameController.h
	$(CC) $(CFLAGS) $< -o $@

$(SRCDIR)/GameModel.o: GameModel.c GameModel.h
	$(CC) $(CFLAGS) $< -o $@

$(SRCDIR)/GameView.o: GameView.c GameView.h GameModel.h
	$(CC) $(CFLAGS) $< -o $@

$(SRCDIR)/GameController.o: GameController.c GameController.h GameModel.h
	$(CC) $(CFLAGS) $< -o $@

clean:
	@echo Cleaning...
	$(RM) $(SRCDIR)/*.o
	$(RM_EXEC) $(EXECUTABLE)$(RM_SUFFIX)
	@echo Done.