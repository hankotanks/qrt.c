CC = gcc
LIBS = -lm
CFLAGS = -fopenmp -Wall -Wextra

OBJDIR = obj

OBJFILES = rt.o
OBJ = $(patsubst %,$(OBJDIR)/%,$(OBJFILES))

DEPS = lalg.h geom.h in.h buffer.h scene.h

$(OBJDIR)/%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

rt: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)
