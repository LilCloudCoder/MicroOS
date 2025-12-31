# MicroOS v1.0

A stable, monolithic, 32-bit operating system kernel written in C and Assembly.

## Features
- **Monolithic Kernel**: All drivers (VGA, Keyboard, FS) are embedded for maximum stability.
- **Stack-Based Memory**: Uses stack memory for filesystem and buffers to prevent memory corruption.
- **Interactive Shell**: Robust command-line interface.
- **RamFS**: In-memory filesystem for temporary file storage.

## Commands

| Command | Usage | Description |
| :--- | :--- | :--- |
| `ls` | `ls` | List all files in the filesystem. |
| `touch` | `touch <filename>` | Create a new empty file. |
| `echo` | `echo "text" > <file>` | Write text to a file (overwrites). |
| `echo` | `echo "text" >> <file>` | Append text to a file. |
| `cat` | `cat <filename>` | Display the contents of a file. |
| `clear` | `clear` | Clear the terminal screen. |
| `color` | `color` | Cycle through different color themes. |

## Build & Run

**Requirements**:
- `qemu-system-i386`
- `x86_64-elf-gcc` / `binutils`

**Run**:
```bash
make run
```
