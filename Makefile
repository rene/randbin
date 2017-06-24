CC = gcc

CPP_FLAGS =         
CFLAGS    = -Wall -Wunused -fno-stack-protector -D_POSIX -D_GNU_SOURCE
LD_FLAGS  = 

executable = randbin
sources = randbin.c
#############################################################

objects = $(sources:.c=.o)

%.o: %.c
	$(CC) $(CFLAGS) $(CPP_FLAGS) -c $<

$(executable) : $(objects)
	$(CC) $(LD_FLAGS) $(LDFLAGS) $(objects) -o $(executable)

.Makefile.dep: *.c
	@$(CC) $(CFLAGS) $(CPP_FLAGS) -MM *.c > $@

-include .Makefile.dep

##
# clean
#
.PHONY clean:
	@rm -f *.o \#* *~  .Makefile.dep
	@rm -f $(executable)

