## OS-SimpleLoader

OS-SimpleLoader is a minimalistic ELF executable loader written in C, designed for educational purposes and small-scale projects. This loader provides basic functionality to load ELF executables, map program segments into memory, and execute the loaded program.

### Features

- **Basic ELF Loading:** Supports loading ELF executables with minimal dependencies.
- **Memory Mapping:** Maps program segments into memory, handling both code and data sections.
- **Executable Execution:** Executes loaded ELF binaries, starting from the entry point specified in the ELF header.
- **Lazy Loading:** Utilizes lazy loading to load segments into memory only when needed.
- **Page Fault Handling:** Handles segmentation faults (page faults) during execution by dynamically allocating memory for faulted segments.
- **Error Handling:** Provides robust error handling for file operations, memory mapping, and execution.
- **Statistics Reporting:** Reports statistics on page faults, page allocations, and internal fragmentation after program execution.

### Usage

1. Clone the repository to your local machine.
2. Compile the loader source code using a C compiler (e.g., `gcc loader.c -o loader -lm`).
3. Run the compiled binary with the path to the ELF executable as an argument (e.g., `./loader <ELF Executable>`).
4. The loader will load the ELF executable, map its segments into memory, and execute the program.
5. After execution, the loader will report statistics on page faults, page allocations, and internal fragmentation.

### Contributions

- Parth Sandeep Rastogi
- Aryan Jain
