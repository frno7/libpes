# Part of ../Makefile

PES_INFO_SRC = tools/pes-info.c tools/file.c
PES_SVG_SRC  = tools/pes-to-svg-emb.c tools/file.c
SVG_PES_SRC  = tools/svg-emb-to-pes.c tools/file.c

PES_INFO_OBJ = $(sort $(patsubst %.c, %.o, $(PES_INFO_SRC)))
PES_SVG_OBJ  = $(sort $(patsubst %.c, %.o, $(PES_SVG_SRC)))
SVG_PES_OBJ  = $(sort $(patsubst %.c, %.o, $(SVG_PES_SRC)))

OBJ += $(sort $(PES_INFO_OBJ) $(PES_SVG_OBJ) $(SVG_PES_OBJ))

all: tools

.PHONY: tools
tools: $(BUILD_DIR)/pes-info \
       $(BUILD_DIR)/pes-to-svg-emb \
       $(BUILD_DIR)/svg-emb-to-pes

$(BUILD_DIR)/pes-info: ${PES_INFO_OBJ} lib
	$(QUIET_LINK)$(CC) ${ALL_CFLAGS} -o $@ \
		$(addprefix $(BUILD_DIR)/, ${PES_INFO_OBJ}) \
		$(BUILD_DIR)/libpes.a ${LDFLAGS}

$(BUILD_DIR)/pes-to-svg-emb: ${PES_SVG_OBJ} lib
	$(QUIET_LINK)$(CC) ${ALL_CFLAGS} -o $@ \
		$(addprefix $(BUILD_DIR)/, ${PES_SVG_OBJ}) \
		$(BUILD_DIR)/libpes.a ${LDFLAGS}

$(BUILD_DIR)/svg-emb-to-pes: ${SVG_PES_OBJ} lib
	$(QUIET_LINK)$(CC) ${ALL_CFLAGS} -o $@ \
		$(addprefix $(BUILD_DIR)/, ${SVG_PES_OBJ}) \
		$(BUILD_DIR)/libpes.a ${LDFLAGS}
