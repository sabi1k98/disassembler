#ifndef ELF_UTILS
#define ELF_UTILS
#include <elf.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>

void* mapElf(const char* path, int* const size);

int isValidElf(void* elf);

void* findSection(void* elf, const char* name);

char* getStrTableString(Elf64_Ehdr* elf_header, uint16_t strOffset);


#endif
