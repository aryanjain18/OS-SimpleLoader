#include "loader.h"
#include <setjmp.h>
#include <signal.h>

Elf32_Ehdr *ehdr; // ELF Header
Elf32_Phdr *phdr; // Program Header
int fd;           // File Descriptor

// Define variables to track page fault and allocation counts
int page_faults = 0;
int page_allocations = 0;
int internal_fragmentation = 0;

// Define a jump buffer for handling page faults
sigjmp_buf page_fault_jmp;

void *map_memory(void *addr, size_t size, int prot, int flags) {
    void *mem = mmap(addr, size, prot, flags, -1, 0);
    if (mem == MAP_FAILED) {
        perror("Error mapping memory");
        exit(1);
    }
    return mem;
}

void handle_page_fault(int signo, siginfo_t *si, void *context) {
    // Point 2: Handle segmentation faults (page faults)
    // Determine the address that caused the page fault
    page_faults++;
    void *fault_addr = si->si_addr;

    // Check if the fault address is within a valid segment
    int i;
    for (i = 0; i < ehdr->e_phnum; i++) {
        void *start_addr = (void *)phdr[i].p_vaddr;
        void *end_addr = start_addr + phdr[i].p_memsz;

        if (fault_addr >= start_addr && fault_addr <= end_addr) {
            // Allocate memory for the segment that caused the fault
            int k=0;
            while((phdr[i].p_memsz -(k*4096))>4096)
{
            
            void *mem = map_memory(phdr[i].p_vaddr+(k*4096), 4096, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE | MAP_ANONYMOUS);

            // Copy the segment data into the allocated memory
            setting_seek(phdr[i].p_offset);
            ssize_t bytes_read = read(fd, mem, phdr[i].p_filesz);
            if (bytes_read == -1) {
                perror("Error reading data into memory");
                munmap(mem, phdr[i].p_memsz);
                exit(1);
            }
            page_faults++;
            k++;
        }
        void *mem = map_memory(phdr[i].p_vaddr+(k*4096),phdr[i].p_memsz -(k*4096) , PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE | MAP_ANONYMOUS);

            // Copy the segment data into the allocated memory
            setting_seek(phdr[i].p_offset);
            ssize_t bytes_read = read(fd, mem, phdr[i].p_filesz);
            if (bytes_read == -1) {
                perror("Error reading data into memory");
                munmap(mem, phdr[i].p_memsz);
                exit(1);
            }
            k++;
            
            internal_fragmentation += k * 4096 - phdr[i].p_memsz;
            page_allocations += k;
            break;

        }
    }
    return;
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
        exit(1);
    }
}

void reading(void *dest, size_t size) {
    ssize_t bytes_read = read(fd, dest, size);
    if (bytes_read != size) {
        perror("read error");
        loader_cleanup();
        exit(1);
    }
}

void load_and_run_elf(char **exe) {
    struct sigaction sa;

    sa.sa_flags = SA_SIGINFO;
    sa.sa_sigaction = handle_page_fault;
    sigaction(SIGSEGV, &sa, NULL);

    fd = open(exe[1], O_RDONLY);
    if (fd == -1) {
        perror("Error opening file");
        exit(1);
    }

    ehdr = malloc(sizeof(Elf32_Ehdr));
    if (ehdr == NULL) {
        perror("Corrupt ehdr");
        exit(1);
    }

    reading(ehdr, sizeof(Elf32_Ehdr));

    phdr = malloc(sizeof(Elf32_Phdr) * ehdr->e_phnum);
    if (phdr == NULL) {
        perror("Corrupt phdr");
        exit(1);
    }

    setting_seek(ehdr->e_phoff);
    reading(phdr, sizeof(Elf32_Phdr) * ehdr->e_phnum);

    int (*_start)(void) = (int (*)(void))(ehdr->e_entry);
    int result;
    // Point 1: Lazy Loading
    result = _start();
    printf("User _start return value = %d\n", result);
   

    // Point 4: Reporting Page Faults and Page Allocations
    printf("Total page faults: %d\n", page_faults);
    printf("Total page allocations: %d\n", page_allocations);
    printf("Total internal fragmentation: %f KB\n", (float)internal_fragmentation / 1024.0);

    // Handle cleanup
    loader_cleanup();
}

int main(int argc, char **argv) {
    if (argc != 2) {
        printf("Usage: %s <ELF Executable>\n", argv[0]);
        exit(1);
    }

    load_and_run_elf(argv);

    return 0;
}