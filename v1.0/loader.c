#include "loader.h"

#define error_exit 1
#define runflag 1


Elf32_Ehdr *ehdr; // ELF Header
Elf32_Phdr *phdr; // Program Header
int fd;           // File Descriptor

void *map_memory(void *addr, size_t size, int prot, int flags) {
    void *mem = mmap(addr, size, prot, flags, -1, 0);
    if (mem == MAP_FAILED) {
        perror("Error mapping memory");
        exit(1);
    }
    return mem;
}
void loader_cleanup() {
  if (phdr) {
    free(phdr);
    phdr = NULL;
  } 
  if (ehdr) {
    free(ehdr);
    ehdr = NULL;
  }
  close(fd);
}

void setting_seek(int offset) {
  if (lseek(fd, offset, SEEK_SET) == -1) {
    perror("lseek failed");
    loader_cleanup();
    exit(error_exit);
  }
}

void reading(void *dest, size_t size) {
  ssize_t bytes_read = read(fd, dest, size);
  if (bytes_read != size) {
    perror("read error");
    loader_cleanup();
    exit(error_exit);
  }
}

void load_and_run_elf(char **exe) {
  unsigned int my_type = 0;
  unsigned int memEhd = 0;
  fd = open(exe[1], O_RDONLY);
  if (fd == -1) {
    perror("Error opening file");
    exit(1);
  }

  ehdr = malloc(sizeof(Elf32_Ehdr));
  if (memEhd == 0 && ehdr == NULL) {
    perror("Corrupt ehdr");
  }

  reading(ehdr, sizeof(Elf32_Ehdr));

  phdr = malloc(sizeof(Elf32_Phdr) * ehdr->e_phnum);
  if (memEhd == 0 && phdr == NULL) {
    perror("Corrupt phdr");
  }

  setting_seek(ehdr->e_phoff);
  reading(phdr, sizeof(Elf32_Phdr) * ehdr->e_phnum);

for (int i = 0; i < ehdr->e_phnum; i++) {

    if (phdr[i].p_type == PT_LOAD && runflag) {
        void *memes = map_memory((void *)phdr[i].p_vaddr, phdr[i].p_memsz, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE | MAP_ANONYMOUS);
        if (memes == MAP_FAILED) {
            perror("Error mapping memory");
            exit(1);
        }

        do {
            setting_seek(phdr[i].p_offset);
            ssize_t bytes_read = read(fd, memes, phdr[i].p_filesz);
            if (bytes_read == -1) {
                perror("Error reading data into memory");
                munmap(memes, phdr[i].p_memsz);
                exit(1);
            }
        } while (0);

        // Adjust the memory protection after reading data
        if (mprotect(memes, phdr[i].p_memsz, PROT_READ | PROT_EXEC) == -1) {
            perror("Error adjusting memory protection");
            munmap(memes, phdr[i].p_memsz); // Clean up the mmap
            exit(1);
        }
    }
}

  int (*_start)(void) = (int (*)(void))(ehdr->e_entry);
  int result = _start();
  printf("User _start return value = %d\n", result);

}

int main(int argc, char **argv) {
  if (argc != 2) {
    printf("Usage: %s <ELF Executable> \n", argv[0]);
    exit(1);
  }

  load_and_run_elf(argv);
  loader_cleanup();

  return 0;
}