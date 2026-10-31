CC      := arm-none-eabi-gcc
OBJCOPY := arm-none-eabi-objcopy
SIZE    := arm-none-eabi-size

TARGET  := thumbv7m-none-eabi
RUSTLIB := rustfmt/target/$(TARGET)/release/librustfmt_ffi.a

CFLAGS  := -mcpu=cortex-m3 -mthumb -Os -g -std=c11 -Wall -Wextra \
           -ffreestanding -ffunction-sections -fdata-sections \
           -fno-tree-loop-distribute-patterns -Iinclude
# --gc-sections drops the parts of Rust's core library that the firmware never calls.
# -z noexecstack: Rust's objects carry no GNU-stack note; the concept does not apply to a Cortex-M
LDFLAGS := -nostdlib -T link.ld -Wl,--gc-sections -Wl,-z,noexecstack

BUILD := build
ELF   := $(BUILD)/fw-demo.elf
BIN   := $(BUILD)/fw-demo.bin
OBJS  := $(BUILD)/startup.o $(BUILD)/main.o

QEMU := qemu-system-arm -cpu cortex-m3 -machine lm3s6965evb -nographic \
        -semihosting-config enable=on,target=native -kernel

.PHONY: all run size test clean $(RUSTLIB)

all: $(ELF) $(BIN)

$(BUILD):
	mkdir -p $@

$(BUILD)/%.o: src/%.c src/semihosting.h include/rustfmt.h | $(BUILD)
	$(CC) $(CFLAGS) -c $< -o $@

# Always ask cargo; it rebuilds only when the Rust sources changed
$(RUSTLIB):
	cargo build --release --manifest-path rustfmt/Cargo.toml --target $(TARGET)

$(ELF): $(OBJS) $(RUSTLIB) link.ld
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBJS) $(RUSTLIB) -o $@

$(BIN): $(ELF)
	$(OBJCOPY) -O binary $< $@

run: $(ELF)
	$(QEMU) $<

size: $(ELF)
	$(SIZE) $<

# The Rust functions are pure, so they can be unit-tested on the Mac itself
test:
	cargo test --lib --manifest-path rustfmt/Cargo.toml

clean:
	rm -rf $(BUILD)
	cargo clean --manifest-path rustfmt/Cargo.toml
