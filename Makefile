###############################################################################
#
# Makefile
#
###############################################################################

HEADERS  = glm.h 
SOURCES  = glm.c start.c 
DEPENDS  = $(SOURCES:.c=.d)
OBJECTS  = $(SOURCES:.c=.o)
PROGRAMS = start 

###############################################################################

ifdef DEBUG
OPTFLAGS = -g
else
OPTFLAGS = -O3 -s
endif

CC      = gcc
CFLAGS  = -Wall -Wno-format -m32 $(OPTFLAGS)
LDFLAGS = -lGL -lglut -lGLU -m32 -lopenal -lSDL -lalut

# Mac OS X: OpenGL and GLUT are frameworks, override LDFLAGS above with these
#LDFLAGS = -framework OpenGL -framework GLUT

###############################################################################

all: $(PROGRAMS)

$(PROGRAMS):
	$(CC) $(CFLAGS) $^ $(LDFLAGS) -o $@

define PROGRAM_template
$(1): $(addsuffix .o,$(1)) glm.o
endef
$(foreach t,$(PROGRAMS),$(eval $(call PROGRAM_template,$(t))))

clean:
	$(RM) $(OBJECTS) $(DEPENDS)
	$(RM) $(PROGRAMS)

.PHONY: all clean

###############################################################################

%.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@

%.d: %.c
	$(CC) -MM $(CFLAGS) $< > $@

###############################################################################

ifneq ($(MAKECMDGOALS),clean)
-include $(DEPENDS)
endif

###############################################################################
