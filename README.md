# MicroOS v2.0

A stable, monolithic, 32-bit operating system kernel written in C and Assembly with advanced features.






https://github.com/user-attachments/assets/d8dd1a36-470f-492b-b910-d0f7ab49d4aa





## Features
- **Monolithic Kernel**: All drivers (VGA, Keyboard, FS) are embedded for maximum stability.
- **Stack-Based Memory**: Uses stack memory for filesystem and buffers to prevent memory corruption.
- **Interactive Shell**: Robust command-line interface with autocorrect.
- **RamFS**: In-memory filesystem for temporary file storage.
- **Memory Management**: Heap allocator with kmalloc and kfree functions.
- **Process Scheduler**: Process table and management with PID allocation.
- **Advanced I/O**: ATA disk driver with sector-level read/write support.
- **Paging**: Virtual memory with page directory and page table structures.
- **Interrupt Handling**: IDT-based interrupt management.
- **CPU Detection**: CPUID support for CPU feature detection.
- **PCI Enumeration**: Hardware device enumeration via PCI bus.
- **Thread Safety**: Spinlock synchronization primitives.
- **Kernel Logging**: Thread-safe kernel log buffer.
- **Loadable Modules**: Module loader interface for kernel extensions.
- **Boot Parameters**: Multiboot info parsing and memory map enumeration.

## Commands

| Command | Usage | Description |
| :--- | :--- | :--- |
| `ls` | `ls [-a]` | List files with sizes. Use `-a` to show hidden files. |
| `edit` | `edit <filename>` | Open text editor (Esc to save/exit). |
| `touch` | `touch <filename>` | Create a new empty file. |
| `cat` | `cat <filename>` | Display file (with syntax highlighting). |
| `echo` | `echo <text>` | Print text to output. |
| `rm` | `rm <filename>` | Delete a file. |
| `sysinfo` | `sysinfo` | Display system information (memory, processes). |
| `help` | `help` | Show all available commands. |
| `clear` | `clear` | Clear the terminal screen. |

## Build & Run

**Requirements**:
- `qemu-system-i386`
- `x86_64-elf-gcc` / `binutils`

**Run**:
```bash
make run
```
