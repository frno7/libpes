# Part of ../Makefile

LIBPES_SRC   = src/pec.c src/pec-decoder.c src/pec-encoder.c \
	       src/pes.c src/pes-decoder.c src/pes-encoder.c \
	       src/sax.c src/svg-emb-decoder.c \
	                 src/svg-emb-encoder.c \
	       src/svg-emb-pes-transcoder.c \
	       src/pes-svg-emb-transcoder.c

LIBPES_OBJ   = $(sort $(patsubst %.c, %.o, $(LIBPES_SRC)))

OBJ += $(sort $(LIBPES_OBJ))

all: lib

.PHONY: lib
lib: $(BUILD_DIR)/libpes.a

$(BUILD_DIR)/libpes.a: ${LIBPES_OBJ}
	$(QUIET_AR)$(AR) rcs $@ $(addprefix $(BUILD_DIR)/, ${LIBPES_OBJ})
