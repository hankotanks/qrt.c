CC = gcc
LIBS = -lm
CFLAGS = -fopenmp

OBJDIR = obj

OBJFILES = rt.o
OBJ = $(patsubst %,$(OBJDIR)/%,$(OBJFILES))

DEPS = lalg.h

$(OBJDIR)/%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

rt: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)
