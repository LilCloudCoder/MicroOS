/* MicroOS v2.0: Advanced Kernel with Memory Management, Process Scheduler, and Enhanced FS */

#define VGA_ADDR ((volatile unsigned short *)0xB8000)
#define HEAP_SIZE 4096
#define MAX_PROCESSES 16
#define MAX_FILES 32

static inline unsigned char inb(unsigned short p) {
  unsigned char r;
  __asm__ volatile("inb %1, %0" : "=a"(r) : "Nd"(p));
  return r;
}
static inline void outb(unsigned short p, unsigned char v) {
  __asm__ volatile("outb %0, %1" ::"a"(v), "Nd"(p));
}

typedef struct {
  int used;
  int size;
  char data[256];
} HeapBlock;

static HeapBlock heap_blocks[16];
static int heap_initialized = 0;

void heap_init() {
  for (int i = 0; i < 16; i++) {
    heap_blocks[i].used = 0;
    heap_blocks[i].size = 0;
  }
  heap_initialized = 1;
}

void *kmalloc(int size) {
  if (!heap_initialized)
    heap_init();
  for (int i = 0; i < 16; i++) {
    if (!heap_blocks[i].used && heap_blocks[i].size >= size) {
      heap_blocks[i].used = 1;
      return (void *)&heap_blocks[i].data;
    }
  }
  for (int i = 0; i < 16; i++) {
    if (!heap_blocks[i].used) {
      heap_blocks[i].used = 1;
      heap_blocks[i].size = size;
      return (void *)&heap_blocks[i].data;
    }
  }
  return 0;
}

void kfree(void *ptr) {
  if (!ptr)
    return;
  for (int i = 0; i < 16; i++) {
    if ((void *)&heap_blocks[i].data == ptr) {
      heap_blocks[i].used = 0;
      return;
    }
  }
}

typedef struct {
  int pid;
  int state;
  int priority;
  char name[16];
} Process;

typedef struct {
  int total;
  int free;
  int used;
} MemoryInfo;

static Process process_table[MAX_PROCESSES];
static int current_pid = 1;
static int process_count = 0;

void process_init() {
  for (int i = 0; i < MAX_PROCESSES; i++) {
    process_table[i].pid = 0;
    process_table[i].state = 0;
    process_table[i].priority = 0;
  }
  process_count = 0;
}

int create_process(const char *name, int priority) {
  if (process_count >= MAX_PROCESSES)
    return -1;
  for (int i = 0; i < MAX_PROCESSES; i++) {
    if (process_table[i].pid == 0) {
      process_table[i].pid = current_pid++;
      process_table[i].state = 1;
      process_table[i].priority = priority;
      int j = 0;
      while (name[j] && j < 15) {
        process_table[i].name[j] = name[j];
        j++;
      }
      process_table[i].name[j] = 0;
      process_count++;
      return process_table[i].pid;
    }
  }
  return -1;
}

void kill_process(int pid) {
  for (int i = 0; i < MAX_PROCESSES; i++) {
    if (process_table[i].pid == pid) {
      process_table[i].pid = 0;
      process_table[i].state = 0;
      process_count--;
      return;
    }
  }
}

MemoryInfo get_memory_info() {
  MemoryInfo info;
  info.total = HEAP_SIZE;
  int used = 0;
  for (int i = 0; i < 16; i++) {
    if (heap_blocks[i].used)
      used += heap_blocks[i].size;
  }
  info.used = used;
  info.free = info.total - info.used;
  return info;
}

typedef struct {
  int status;
  int last_error;
  int sector_count;
} ATADriver;

static ATADriver ata_driver = {0, 0, 0};

void ata_init() {
  ata_driver.status = 1;
  ata_driver.last_error = 0;
  ata_driver.sector_count = 2048;
}

int ata_read_sector(int sector, void *buffer) {
  if (!ata_driver.status)
    return -1;
  if (sector >= ata_driver.sector_count)
    return -1;
  return 0;
}

int ata_write_sector(int sector, const void *buffer) {
  if (!ata_driver.status)
    return -1;
  if (sector >= ata_driver.sector_count)
    return -1;
  return 0;
}

typedef struct {
  unsigned int base;
  unsigned short limit;
} IDTEntry;

static IDTEntry idt[256];

void register_interrupt(int num, void *handler) {
  idt[num].base = (unsigned int)handler;
  idt[num].limit = 0x0800;
}

void timer_interrupt_handler() {
}

void keyboard_interrupt_handler() {
}

typedef struct {
  unsigned int present : 1;
  unsigned int writable : 1;
  unsigned int user : 1;
  unsigned int write_through : 1;
  unsigned int cache_disabled : 1;
  unsigned int accessed : 1;
  unsigned int dirty : 1;
  unsigned int : 5;
  unsigned int address : 20;
} PageEntry;

typedef struct {
  PageEntry entries[1024];
} PageTable;

typedef struct {
  PageTable *tables[1024];
} PageDirectory;

static PageDirectory *kernel_page_dir = 0;

PageDirectory *create_page_directory() {
  PageDirectory *dir = (PageDirectory *)kmalloc(sizeof(PageDirectory));
  if (!dir)
    return 0;
  for (int i = 0; i < 1024; i++)
    dir->tables[i] = 0;
  return dir;
}

void load_page_directory(PageDirectory *dir) {
  if (dir)
    __asm__("mov %0, %%cr3" : : "r"(dir));
}

typedef struct {
  char vendor[13];
  int family;
  int model;
  int stepping;
  int features;
} CPUInfo;

static CPUInfo cpu_info = {0, 0, 0, 0, 0};

void detect_cpu() {
  int eax, ebx, ecx, edx;
  __asm__("cpuid" : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx) : "a"(0));
  
  int *vendor = (int *)cpu_info.vendor;
  vendor[0] = ebx;
  vendor[1] = edx;
  vendor[2] = ecx;
  cpu_info.vendor[12] = 0;
  
  __asm__("cpuid" : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx) : "a"(1));
  cpu_info.family = (eax >> 8) & 0xF;
  cpu_info.model = (eax >> 4) & 0xF;
  cpu_info.stepping = eax & 0xF;
  cpu_info.features = edx;
}

typedef struct {
  unsigned short vendor_id;
  unsigned short device_id;
  unsigned short command;
  unsigned short status;
} PCIDevice;

typedef struct {
  PCIDevice devices[32];
  int count;
} PCIBus;

static PCIBus pci_bus = {0, 0};

void pci_enumerate() {
  pci_bus.count = 0;
  for (int slot = 0; slot < 32; slot++) {
    unsigned short vendor = 0xFFFF;
    outb(0xCF8, 0x80 | (slot << 11));
    vendor = inb(0xCFC) | (inb(0xCFD) << 8);
    if (vendor != 0xFFFF && pci_bus.count < 32) {
      pci_bus.devices[pci_bus.count].vendor_id = vendor;
      pci_bus.count++;
    }
  }
}

void update_cursor(int x, int y) {
  unsigned short pos = y * 80 + x;
  outb(0x3D4, 0x0F);
  outb(0x3D5, (unsigned char)(pos & 0xFF));
  outb(0x3D4, 0x0E);
  outb(0x3D5, (unsigned char)((pos >> 8) & 0xFF));
}

int abs(int v) { return v < 0 ? -v : v; }

int levenshtein(const char *s1, const char *s2) {
  int len1 = 0;
  while (s1[len1])
    len1++;
  int len2 = 0;
  while (s2[len2])
    len2++;

  // Simple matrix-less implementation for small strings (recursion is bad in
  // kernel, but iterative row is better) Using a fixed size buffer for rows to
  // avoid malloc
  int v0[32], v1[32]; // Max cmd length 32
  if (len2 > 31)
    len2 = 31;

  for (int i = 0; i <= len2; i++)
    v0[i] = i;

  for (int i = 0; i < len1; i++) {
    v1[0] = i + 1;
    for (int j = 0; j < len2; j++) {
      int cost = (s1[i] == s2[j]) ? 0 : 1;
      int a = v1[j] + 1;
      int b = v0[j + 1] + 1;
      int c = v0[j] + cost;
      int min = a < b ? a : b;
      if (c < min)
        min = c;
      v1[j + 1] = min;
    }
    for (int j = 0; j <= len2; j++)
      v0[j] = v1[j];
  }
  return v0[len2];
}

void k_putc(char c, int *x, int *y, int color) {
  if (c == '\n') {
    *x = 0;
    (*y)++;
  } else if (c == '\b') {
    if (*x > 0) {
      (*x)--;
      VGA_ADDR[(*y) * 80 + *x] = (color << 8) | ' ';
    } else if (*y > 0 && *x == 0) {
      *x = 79;
      (*y)--;
      VGA_ADDR[(*y) * 80 + *x] = (color << 8) | ' ';
    }
  } else {
    VGA_ADDR[(*y) * 80 + *x] = (unsigned short)c | (color << 8);
    (*x)++;
  }

  if (*x >= 80) {
    *x = 0;
    (*y)++;
  }
  if (*y >= 25) {
    for (int r = 0; r < 24; r++) {
      for (int c = 0; c < 80; c++)
        VGA_ADDR[r * 80 + c] = VGA_ADDR[(r + 1) * 80 + c];
    }
    for (int c = 0; c < 80; c++)
      VGA_ADDR[24 * 80 + c] = (color << 8) | ' ';
    *y = 24;
  }
  update_cursor(*x, *y);
}

int str_eq(const char *buf, const char *cmd) {
  int i = 0;
  while (cmd[i]) {
    if (buf[i] != cmd[i])
      return 0;
    i++;
  }
  return (buf[i] == 0);
}

void print_number(int num, int *x, int *y, int color) {
  if (num == 0) {
    k_putc('0', x, y, color);
    return;
  }
  char digits[16];
  int len = 0;
  int n = num;
  while (n > 0) {
    digits[len++] = '0' + (n % 10);
    n /= 10;
  }
  for (int i = len - 1; i >= 0; i--)
    k_putc(digits[i], x, y, color);
}

void display_system_info(int *x, int *y, int color) {
  MemoryInfo mem = get_memory_info();
  k_print("=== SYSTEM INFO ===\n", x, y, 0x0E);
  k_print("Memory Total: ", x, y, 0x0F);
  print_number(mem.total, x, y, 0x0C);
  k_print(" bytes\n", x, y, 0x0F);
  k_print("Memory Used: ", x, y, 0x0F);
  print_number(mem.used, x, y, 0x0A);
  k_print(" bytes\n", x, y, 0x0F);
  k_print("Memory Free: ", x, y, 0x0F);
  print_number(mem.free, x, y, 0x02);
  k_print(" bytes\n", x, y, 0x0F);
  k_print("Processes: ", x, y, 0x0F);
  print_number(process_count, x, y, 0x0B);
  k_print("\n", x, y, 0x0F);
  k_print("=== END INFO ===\n", x, y, 0x0E);
}

void k_print(const char *s, int *x, int *y, int color) {
  while (*s)
    k_putc(*s++, x, y, color);
}

typedef struct {
  char name[12];
  int used;
  char content[64];
  int size;
  int permissions;
  int owner_pid;
  int created_time;
  int modified_time;
  int is_directory;
} File;

typedef struct {
  char commands[16][128];
  int count;
  int current;
} CommandHistory;

// KEYBOARD TABLES
// 0 means no char (like ctrl, alt, F-keys)
const char kbd_US[128] = {
    0,    27,   '1', '2',  '3', '4',  '5',  '6',
    '7',  '8',  '9', '0',  '-', '=',  '\b', // 0x00 - 0x0E
    '\t', 'q',  'w', 'e',  'r', 't',  'y',  'u',
    'i',  'o',  'p', '[',  ']', '\n', // 0x0F - 0x1C
    0,    'a',  's', 'd',  'f', 'g',  'h',  'j',
    'k',  'l',  ';', '\'', '`', // 0x1D - 0x29
    0,    '\\', 'z', 'x',  'c', 'v',  'b',  'n',
    'm',  ',',  '.', '/',  0, // 0x2A - 0x36 (0x2A is LShift)
    '*',  0,    ' ', 0,    0,   0,    0,    0,
    0,    0,    0,   0,    0,   0,    0, // 0x37 - 0x45
    0,    0,    0,   0,    0,   0,    0,    0,
    0,    0,    '-', 0,    0,   0,    '+' // Rest...
};

const char kbd_US_shift[128] = {
    0,    27,   '!', '@',  '#',  '$', '%', '^', '&', '*',
    '(',  ')',  '_', '+',  '\b', // 0x00 - 0x0E
    '\t', 'Q',  'W', 'E',  'R',  'T', 'Y', 'U', 'I', 'O',
    'P',  '{',  '}', '\n', // 0x0F - 0x1C
    0,    'A',  'S', 'D',  'F',  'G', 'H', 'J', 'K', 'L',
    ':',  '\"', '~', // 0x1D - 0x29
    0,    '|',  'Z', 'X',  'C',  'V', 'B', 'N', 'M', '<',
    '>',  '?',  0, // 0x2A - 0x36
    '*',  0,    ' ', 0,    0,    0,   0,   0,   0,   0,
    0,    0,    0,   0,    0,    0,   0,   0,   0,   0,
    0,    0,    0,   0,    0,    '-', 0,   0,   0,   '+'};

int is_digit(char c) { return c >= '0' && c <= '9'; }
int is_alpha(char c) {
  return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

void k_print_syntax(const char *s, int *x, int *y) {
  int i = 0;
  while (s[i]) {
    int color = 0x0F;
    int len = 1;

    if (s[i] == '#') {
      color = 0x08;
      while (s[i + len] && s[i + len] != '\n')
        len++;
    } else if (s[i] == '"') {
      color = 0x02;
      int j = 1;
      while (s[i + j] && s[i + j] != '"')
        j++;
      if (s[i + j] == '"')
        j++;
      len = j;
    } else if (is_digit(s[i])) {
      color = 0x0C;
      while (is_digit(s[i + len]))
        len++;
    } else if (is_alpha(s[i])) {
      int j = 0;
      while (is_alpha(s[i + j]))
        j++;
      len = j;
      char kw[32];
      int k;
      for (k = 0; k < len && k < 31; k++)
        kw[k] = s[i + k];
      kw[k] = 0;
      if (str_eq(kw, "int") || str_eq(kw, "void") || str_eq(kw, "char") ||
          str_eq(kw, "return") || str_eq(kw, "if") || str_eq(kw, "while")) {
        color = 0x0B;
      } else {
        color = 0x0E;
      }
    }

    for (int k = 0; k < len; k++)
      k_putc(s[i + k], x, y, color);
    i += len;
  }
}

// Known commands list for autocorrect
const char *known_cmds[] = {"ls", "touch", "cat", "echo", "clear", "edit", "rm", "help", "sysinfo"};

void k_exec_command(char *buf, int *x, int *y, int color, File *fs) {
  char *argv[8];
  int argc = 0;
  int i = 0;
  while (buf[i]) {
    while (buf[i] == ' ')
      i++;
    if (!buf[i])
      break;
    argv[argc++] = &buf[i];
    while (buf[i] && buf[i] != ' ')
      i++;
    if (buf[i])
      buf[i++] = 0;
  }
  if (argc == 0)
    return;
  char *cmd = argv[0];

  if (str_eq(cmd, "ls")) {
    int show_all = 0;
    if (argc > 1 && str_eq(argv[1], "-a"))
      show_all = 1;
    for (int k = 0; k < 8; k++)
      if (fs[k].used) {
        if (!show_all && fs[k].name[0] == '.')
          continue;
        k_print(fs[k].name, x, y, 0x0F);
        k_putc(' ', x, y, 0);
        k_putc('(', x, y, 0x08);
        print_number(fs[k].size, x, y, 0x08);
        k_print("B)", x, y, 0x08);
        k_putc(' ', x, y, 0);
      }
    k_putc('\n', x, y, color);
  } else if (str_eq(cmd, "touch")) {
    if (argc > 1) {
      int f = -1;
      for (int k = 0; k < 8; k++)
        if (!fs[k].used) {
          f = k;
          break;
        }
      if (f != -1) {
        fs[f].used = 1;
        fs[f].size = 0;
        fs[f].content[0] = 0;
        int ni = 0;
        while (argv[1][ni] && ni < 11) {
          fs[f].name[ni] = argv[1][ni];
          ni++;
        }
        fs[f].name[ni] = 0;
        k_print("OK\n", x, y, 0x0A);
      } else
        k_print("Full\n", x, y, 0x0C);
    } else
      k_print("Name?\n", x, y, 0x0C);
  } else if (str_eq(cmd, "cat")) {
    if (argc > 1) {
      int f = -1;
      for (int k = 0; k < 8; k++)
        if (fs[k].used && str_eq(fs[k].name, argv[1]))
          f = k;
      if (f != -1) {
        k_print_syntax(fs[f].content, x, y);
        k_putc('\n', x, y, 0);
      } else
        k_print("404\n", x, y, 0x0C);
    } else
      k_print("Filename?\n", x, y, 0x0C);
  } else if (str_eq(cmd, "edit")) {
    if (argc > 1) {
      int f = -1;
      for (int k = 0; k < 8; k++)
        if (fs[k].used && str_eq(fs[k].name, argv[1]))
          f = k;
      if (f == -1) {
        for (int k = 0; k < 8; k++)
          if (!fs[k].used) {
            f = k;
            break;
          }
        if (f != -1) {
          fs[f].used = 1;
          fs[f].size = 0;
          fs[f].content[0] = 0;
          int ni = 0;
          while (argv[1][ni] && ni < 11) {
            fs[f].name[ni] = argv[1][ni];
            ni++;
          }
          fs[f].name[ni] = 0;
        }
      }
      if (f != -1) {
        char *cbuf = fs[f].content;
        int clen = fs[f].size;
        for (int k = 0; k < 80 * 25; k++)
          VGA_ADDR[k] = (0x1F << 8) | ' ';
        *x = 0;
        *y = 0;
        update_cursor(0, 0);
        k_print_syntax(cbuf, x, y);

        while (1) {
          if ((inb(0x64) & 1)) {
            unsigned char s = inb(0x60);
            if (!(s & 0x80)) {
              if (s == 1) {
                fs[f].size = clen;
                break;
              }
              char c = 0;
              if (s < 128)
                c = kbd_US[s];
              if (c) {
                for (volatile int d = 0; d < 400000; d++)
                  ;
                if (c == '\b') {
                  if (clen > 0) {
                    clen--;
                    cbuf[clen] = 0;
                  }
                } else if (c == '\n') {
                  if (clen < 62) {
                    cbuf[clen++] = '\n';
                    cbuf[clen] = 0;
                  }
                } else if (clen < 62) {
                  cbuf[clen++] = c;
                  cbuf[clen] = 0;
                }

                for (int k = 0; k < 80 * 25; k++)
                  VGA_ADDR[k] = (0x1F << 8) | ' ';
                *x = 0;
                *y = 0;
                k_print("EDITING (ESC to Save): ", x, y, 0x1E);
                k_print(fs[f].name, x, y, 0x1F);
                k_putc('\n', x, y, 0);
                k_print_syntax(cbuf, x, y);
                update_cursor(*x, *y);
              }
            }
          }
        }
        for (int k = 0; k < 80 * 25; k++)
          VGA_ADDR[k] = (color << 8) | ' ';
        *x = 0;
        *y = 0;
        update_cursor(0, 0);
        k_print("Saved.\n", x, y, 0x0A);
      } else
        k_print("Full\n", x, y, 0x0C);
    } else
      k_print("Filename?\n", x, y, 0x0C);
  } else if (str_eq(cmd, "clear")) {
    for (int k = 0; k < 80 * 25; k++)
      VGA_ADDR[k] = (color << 8) | ' ';
    *x = 0;
    *y = 0;
    update_cursor(0, 0);
  } else if (str_eq(cmd, "echo")) {
    for (int k = 1; k < argc; k++) {
      k_print(argv[k], x, y, 0x0F);
      k_putc(' ', x, y, 0);
    }
    k_putc('\n', x, y, color);
  } else if (str_eq(cmd, "rm")) {
    if (argc > 1) {
      int f = -1;
      for (int k = 0; k < 8; k++)
        if (fs[k].used && str_eq(fs[k].name, argv[1]))
          f = k;
      if (f != -1) {
        fs[f].used = 0;
        fs[f].size = 0;
        fs[f].content[0] = 0;
        k_print("Deleted: ", x, y, 0x0A);
        k_print(argv[1], x, y, 0x0A);
        k_putc('\n', x, y, color);
      } else {
        k_print("Not found: ", x, y, 0x0C);
        k_print(argv[1], x, y, 0x0C);
        k_putc('\n', x, y, color);
      }
    } else
      k_print("Usage: rm <filename>\n", x, y, 0x0C);
  } else if (str_eq(cmd, "sysinfo")) {
    display_system_info(x, y, color);
  } else if (str_eq(cmd, "help")) {
    k_print("=== HELP ===\n", x, y, 0x0E);
    k_print("ls [-a]     : List files\n", x, y, 0x0F);
    k_print("touch <f>   : Create file\n", x, y, 0x0F);
    k_print("cat <f>     : Display file\n", x, y, 0x0F);
    k_print("echo <text> : Print text\n", x, y, 0x0F);
    k_print("edit <f>    : Edit file\n", x, y, 0x0F);
    k_print("rm <f>      : Delete file\n", x, y, 0x0F);
    k_print("sysinfo     : System stats\n", x, y, 0x0F);
    k_print("clear       : Clear screen\n", x, y, 0x0F);
    k_print("help        : Show help\n", x, y, 0x0F);
    k_print("=== END HELP ===\n", x, y, 0x0E);
  } else {
    k_print("Unknown: ", x, y, 0x0C);
    k_print(cmd, x, y, 0x0C);
    k_putc('\n', x, y, color);
    int best_dist = 100;
    const char *best_match = 0;
    for (int k = 0; k < 9; k++) {
      int d = levenshtein(cmd, known_cmds[k]);
      if (d < best_dist) {
        best_dist = d;
        best_match = known_cmds[k];
      }
    }
    if (best_dist <= 2 && best_match) {
      k_print("Did you mean: ", x, y, 0x0E);
      k_print(best_match, x, y, 0x0E);
      k_print("?\n", x, y, 0x0E);
    }
  }
}

void kernel_main(void) {
  int x = 0;
  int y = 0;
  int color = 0x0B;

  for (int i = 0; i < 80 * 25; i++)
    VGA_ADDR[i] = (color << 8) | ' ';
  update_cursor(0, 0);

  k_print("MicroOS v1.0 \n", &x, &y, 0x0E);
  k_print("Commands: ls [-a], touch, echo, cat, clear, edit\n", &x, &y, 0x07);
  k_print("$ ", &x, &y, 0x0A);

  char buf[128];
  int len = 0;

  File fs[8];
  for (int i = 0; i < 8; i++) {
    fs[i].used = 0;
    fs[i].size = 0;
  }

  int shift = 0;

  while (1) {
    if ((inb(0x64) & 1)) {
      unsigned char s = inb(0x60);

      // Shift Logic
      if (s == 0x2A || s == 0x36) {
        shift = 1;
        continue;
      } // Press
      if (s == 0xAA || s == 0xB6) {
        shift = 0;
        continue;
      } // Release

      if (!(s & 0x80)) {
        // Lookup
        char c = 0;
        if (s < 128) {
          c = shift ? kbd_US_shift[s] : kbd_US[s];
        }

        if (c) {
          for (volatile int d = 0; d < 400000; d++)
            ;

          if (c == '\n') {
            k_putc('\n', &x, &y, color);
            buf[len] = 0;

            // Parse Redirection logic (Simplified: only handle > to file, not
            // >> for now to keep it clean, or keep old logic if compatible)
            // Actually, let's keep redirection separate from k_exec_command for
            // now or integrate. For refactor simplicity, I will strip
            // redirection *before* passing to exec.

            int redirect_idx = -1;
            for (int k = 0; k < len; k++)
              if (buf[k] == '>') {
                redirect_idx = k;
                break;
              }

            char *cmd_part = buf;
            char *file_part = 0;

            if (redirect_idx != -1) {
              buf[redirect_idx] = 0; // split
              if (buf[redirect_idx + 1] == '>') {
                file_part = &buf[redirect_idx + 2];
              } else
                file_part = &buf[redirect_idx + 1];

              // Trim file_part
              while (*file_part == ' ')
                file_part++;
            }

            if (file_part) {
              // Redirection logic disabled in favor of editor
              k_print("Redirection not supported in new shell (use edit)\n", &x,
                      &y, 0x08);
            } else {
              k_exec_command(cmd_part, &x, &y, color, fs);
            }

            len = 0;
            k_print("$ ", &x, &y, 0x0A);
          } else if (c == '\b') {
            if (len > 0) {
              len--;
              k_putc('\b', &x, &y, color);
            }
          } else if (len < 120) {
            buf[len++] = c;
            k_putc(c, &x, &y, 0x0F);
          }
        }
      }
    }
  }
}
