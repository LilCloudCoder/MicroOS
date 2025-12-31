#ifndef KERNEL_H
#define KERNEL_H

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;

void terminal_initialize();
void terminal_putchar(char c);
void kprintf(const char *str, ...);

#endif
