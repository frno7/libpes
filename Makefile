# Define V=1 to compile or test verbosely.

ifeq ($(ARCH),)
   MACHINE = $(shell uname -m)
   ARCH = $(MACHINE)
endif

BUILD_DIR = build/$(ARCH)

vpath %.o $(BUILD_DIR)

CFLAGS = -Wall -O2 -g
BASIC_CFLAGS = -Iinclude
LDFLAGS = -lm

ALL_CFLAGS = $(CFLAGS) $(BASIC_CFLAGS)

.PHONY: all

include src/Makefile
include test/Makefile
include tools/Makefile

ifneq "$(MAKECMDGOALS)" "clean"
   create-output-directories := \
	  $(shell for f in $(OBJ); do mkdir -p $(BUILD_DIR)/$$(dirname $$f); done)
endif

$(OBJ): %.o : %.c
	$(QUIET_CC)$(CC) -o $(BUILD_DIR)/$@ -c $(ALL_CFLAGS) $<

.PHONY: clean
clean:
	$(Q)rm -rf build

# Pretty print
V             = @
Q             = $(V:1=)
QUIET_CC      = $(Q:@=@echo    '     CC       '$@;)
QUIET_AR      = $(Q:@=@echo    '     AR       '$@;)
QUIET_LINK    = $(Q:@=@echo    '     LINK     '$@;)
QUIET_TEST    = $(Q:@=@echo    '     TEST     '$@;)

# Header dependencies
HAVE_GCC_DEP:=$(shell touch .gcc-test.c && 				\
		$(CC) -c -Wp,-MD,.gcc-test.d .gcc-test.c 2>/dev/null && \
		echo 'yes'; rm -f .gcc-test.d .gcc-test.o .gcc-test.c)
ifeq ($(HAVE_GCC_DEP),yes)
   BASIC_CFLAGS += -Wp,-MMD,$(BUILD_DIR)/$(@D)/$(@F).d -MT $(@D)/$(@F)
endif
ifneq "$(MAKECMDGOALS)" "clean"
   DEP_FILES := $(addprefix $(BUILD_DIR)/, $(addsuffix .d, $(OBJ)))
   $(if $(DEP_FILES),$(eval -include $(DEP_FILES)))
endif
