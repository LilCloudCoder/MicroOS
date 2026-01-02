# MicroOS v1.0

A stable, monolithic, 32-bit operating system kernel written in C and Assembly.




https://github.com/user-attachments/assets/d90169d0-d8de-4fa8-a32f-3d9a137a0e3c



## Features
- **Monolithic Kernel**: All drivers (VGA, Keyboard, FS) are embedded for maximum stability.
- **Stack-Based Memory**: Uses stack memory for filesystem and buffers to prevent memory corruption.
- **Interactive Shell**: Robust command-line interface with autocorrect.
- **RamFS**: In-memory filesystem for temporary file storage.

## Commands

| Command | Usage | Description |
| :--- | :--- | :--- |
| `ls` | `ls [-a]` | List files. Use `-a` to show hidden files. |
| `edit` | `edit <filename>` | Open text editor (Esc to save/exit). |
| `touch` | `touch <filename>` | Create a new empty file. |
| `cat` | `cat <filename>` | Display file (with syntax highlighting). |
| `echo` | `echo <text>` | Print text to output. |
| `clear` | `clear` | Clear the terminal screen. |

## Build & Run

**Requirements**:
- `qemu-system-i386`
- `x86_64-elf-gcc` / `binutils`

**Run**:
```bash
make run
```
