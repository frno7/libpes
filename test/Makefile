# Part of ../Makefile

TEST_SRC = $(sort $(wildcard test/*.c))
TEST_OBJ = $(sort $(patsubst %.c, %.o, $(TEST_SRC)))

OBJ += $(TEST_OBJ)

all: $(BUILD_DIR)/run-tests

.PHONY: test
test: $(BUILD_DIR)/run-tests
	$(QUIET_TEST)$(BUILD_DIR)/run-tests $(if $(V:@=),-v)

$(BUILD_DIR)/run-tests: ${TEST_OBJ} lib
	$(QUIET_LINK)$(CC) ${ALL_CFLAGS} -o $@ \
		$(addprefix $(BUILD_DIR)/, ${TEST_OBJ}) \
		$(BUILD_DIR)/libpes.a ${LDFLAGS}
