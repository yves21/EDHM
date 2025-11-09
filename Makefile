CC ?= gcc
CFLAGS := -Wall -Wextra -std=c11 -g -Iinclude -Ikernel/include
USER_CFLAGS := -Wall -Wextra -std=c11 -fPIC -Iinclude -Iuser/libc/include
LDFLAGS := -ldl

BUILD_DIR := build
KERNEL_BUILD_DIR := $(BUILD_DIR)/kernel
USER_LIBC_BUILD_DIR := $(BUILD_DIR)/user/libc
USER_APPS_BUILD_DIR := $(BUILD_DIR)/user/apps

KERNEL_OBJS := \
$(KERNEL_BUILD_DIR)/main.o \
$(KERNEL_BUILD_DIR)/process.o \
$(KERNEL_BUILD_DIR)/syscalls.o \
$(KERNEL_BUILD_DIR)/exec/elf_loader.o

USER_LIBC_OBJS := \
$(USER_LIBC_BUILD_DIR)/crt0.o \
$(USER_LIBC_BUILD_DIR)/libc.o

USER_APPS := shell echo cat
USER_APP_TARGETS := $(addprefix $(USER_APPS_BUILD_DIR)/,$(addsuffix .so,$(USER_APPS)))

all: $(KERNEL_BUILD_DIR)/kernel $(USER_APP_TARGETS)

$(KERNEL_BUILD_DIR)/kernel: $(KERNEL_OBJS)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $^ $(LDFLAGS) -o $@

$(KERNEL_BUILD_DIR)/%.o: kernel/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(KERNEL_BUILD_DIR)/exec/%.o: kernel/exec/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(USER_LIBC_BUILD_DIR)/%.o: user/libc/%.c
	@mkdir -p $(dir $@)
	$(CC) $(USER_CFLAGS) -c $< -o $@

$(USER_APPS_BUILD_DIR)/%.o: user/apps/%.c
	@mkdir -p $(dir $@)
	$(CC) $(USER_CFLAGS) -c $< -o $@

$(USER_APPS_BUILD_DIR)/%.so: $(USER_APPS_BUILD_DIR)/%.o $(USER_LIBC_OBJS)
	@mkdir -p $(dir $@)
	$(CC) -shared -fPIC -o $@ $^

clean:
	rm -rf $(BUILD_DIR)

.PHONY: all clean
