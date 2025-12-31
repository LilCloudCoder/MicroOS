CC := x86_64-elf-gcc
AS := x86_64-elf-as
LD := x86_64-elf-ld
CFLAGS := -m32 -ffreestanding -fno-pie -Wall -Wextra -O2 -mno-sse -mno-mmx -mno-avx
ASFLAGS := --32
LDFLAGS := -m elf_i386 -T kernel.ld

# Only kernel.c and boot.s
SRC := kernel.c
OBJ := $(SRC:.c=.o) boot.o
TARGET := kernel.elf

.PHONY: all clean run qemu help

all: $(TARGET)

# Link boot.o FIRST to ensure Multiboot header is at the front
$(TARGET): boot.o $(OBJ)
	$(LD) $(LDFLAGS) -o $@ boot.o kernel.o

boot.o: boot.s
	$(AS) $(ASFLAGS) -o $@ $<

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

run: $(TARGET)
	qemu-system-i386 -kernel $(TARGET)

qemu: run

clean:
	rm -f *.o kernel.elf

