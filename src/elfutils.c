#include "elfutils.h"


void* mapElf(const char* path, int* const size) {
    int fd = open(path, O_RDONLY);
    if ( fd < 0 ) {
        perror("open");
        return NULL;
    }
    struct stat statBuffer;
    if ( fstat(fd, &statBuffer) == -1 ) {
        perror("fstat");
        close(fd);
        return NULL;
    }
    void* mapping = mmap(NULL, statBuffer.st_size, PROT_READ,
            MAP_PRIVATE, fd, 0);
    if ( mapping == MAP_FAILED ) {
        perror("mmap");
        close(fd);
        return NULL;
    }
    *size = statBuffer.st_size;
    close(fd);
    return mapping;
}


int isValidElf(void* elf) {
    const char* valid = "\x7f\x45\x4c\x46"; //valid first 4 ELF bytes
    Elf64_Ehdr* headerPtr = elf;
    return !strncmp((char *) headerPtr->e_ident, valid, 4) &&
    headerPtr->e_ident[EI_CLASS] == ELFCLASS64 &&
    headerPtr->e_ident[EI_DATA] == ELFDATA2LSB;
}

char* getStrTableString(Elf64_Ehdr* header, uint16_t strOffset) {
    uint16_t offset =  header->e_shstrndx;
    Elf64_Shdr* shstr = (Elf64_Shdr *)((char *)header + header->e_shoff) + offset;
    return (char *) header + shstr->sh_offset + strOffset; 
}

void* findSection(void* elf, const char* name) {
    Elf64_Ehdr* headerPtr = elf;
    Elf64_Shdr* sectionPtr = (Elf64_Shdr *)((char *) elf + headerPtr->e_shoff);
    for ( int i = 0; i < headerPtr->e_shnum; i++ ) {
        uint16_t strOffset = sectionPtr->sh_name; 
        if ( !strcmp(getStrTableString(elf, strOffset), name) ) {
            return sectionPtr; 
        }
        sectionPtr++;
    }
    return NULL;
}
