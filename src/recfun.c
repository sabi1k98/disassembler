#include <stdlib.h>
#include <stdio.h>
#include <elfutils.h>
#include <symtab.h>
#include <decoder.h>

typedef struct {
    char** strInstr;
    int capacity;
} StringInstructions;


int initStrInstr(StringInstructions* instr) {
    instr->strInstr = malloc(sizeof(char*) * 1024);
    if ( !instr->strInstr ) {
        perror("Malloc\n");
        return 1;
    }
    instr->capacity = 1024;
    memset(instr->strInstr, 0, 1024 * sizeof(char*) );
    return 0;
}

int resizeStrInstr(StringInstructions* instr) {
    char** tmp = realloc(instr->strInstr, sizeof(char*) * instr->capacity * 2);
    if ( !tmp ) {
        perror("Malloc\n");
        return 1;
    }
    memset(tmp + instr->capacity, 0, sizeof(char*) * instr->capacity);
    instr->capacity *= 2;
    instr->strInstr = tmp;
    return 0;
}


int findEntryPoint(void* mapping) {
    Elf64_Ehdr* header = mapping;
    return header->e_entry;
}

int step(instructionData* data, StringInstructions* result, Elf64_Shdr* header) {
    //printf("index: %d, size: %lu\n", data->index, header->sh_size);
    while ((unsigned long) data->index < header->sh_size ) {
        //printf("index: %d, size: %lu\n", data->index, header->sh_size);
        if ( data->index >= result->capacity && resizeStrInstr(result) ) {
            return 1;
        } 
        if (result->strInstr[data->index]) {
            return 0;
        }
        result->strInstr[data->index] = malloc(60);
        memset(result->strInstr[data->index], 0, 60);
       if ( !result->strInstr[data->index] ) {
            perror("Malloc\n");
            return 1;
       }
        int current = data->index;
        if ( !decodeSingleInstruction(header->sh_size, data, result->strInstr[data->index], false) || !strncmp(result->strInstr[current], "ret", 3) ) {
            return 0;
        }
        if ( !strncmp(result->strInstr[current], "Unknown", 7) ) {
            for ( int i = current; i < data->index; i++ ) {
                if ( !result->strInstr[i] ) {
                    result->strInstr[i] = malloc(60);
                    memset(result->strInstr[i], 0, 60);
                }
                sprintf(result->strInstr[i], "Unknown: 0x%x", data->instruction[i]);
            }
        }
        clearInstructionData(data);
        if ( !strncmp("jmp", result->strInstr[current], 3) ) {
            data->index = strtol(result->strInstr[current] + 4, NULL, 16) - data->offset;
            continue;
        }
        if ( !strncmp("j", result->strInstr[current], 1) || !strncmp("call", result->strInstr[current], 3)) {
            int save = data->index;
            data->index = strtol(result->strInstr[current] + 4, NULL, 16);
            step(data, result, header);
            data->index = save;
        }

    }
    return 0;
}


void fillLabels(SymbolTable* symtab, StringInstructions* result, instructionData* data) {
    Symbol* s;
    char buffer[40];
    for ( int i = 0; i < result->capacity; i++ ) {
        if ( !result->strInstr[i] ) {
            continue;
        }
        if ( result->strInstr[i][0] == 'j' || !strncmp(result->strInstr[i], "call", 4) ) {
            uint32_t jumpAddr = strtol(result->strInstr[i] + 4, NULL, 16); 
            if ( jumpAddr - data->offset < (unsigned) result->capacity && !result->strInstr[jumpAddr - data->offset] ) {
                strcat(result->strInstr[i], " # [broken] ");
                continue;
            }
            s = searchValue(symtab, jumpAddr);
            strcat(result->strInstr[i], " #");
            if ( !s ) {
                sprintf(buffer, "sub_%x", i + data->offset);
                strcat(result->strInstr[i], buffer);
            } else {
                strcat(result->strInstr[i], s->name);
            }
        }
    }
}

int recursiveDissasembly(void* mapping, SymbolTable* symtab ) {
    uint32_t entry = findEntryPoint(mapping);
    printf("Entry: %x\n", entry);
    Elf64_Shdr* section = findSection(mapping, ".text");
    instructionData data = {0, { 0 }, false, 0, NULL, mapping, { {0, 0, false} }, entry};
    data.instruction += section->sh_offset;
    memset(data.transitions, 0xff, 2048 * sizeof(transition));
    data.transitions[0].to = section->sh_offset;
    printf("Sectionoffset: %lx\n", section->sh_offset);
    StringInstructions result;
    Symbol* s = searchValue(symtab, data.index);
    if ( s ) {
        printf("<%s>_<entry>\n", s->name);
    }
    if ( initStrInstr(&result) ) {
        return 1;
    }
    step(&data, &result, section);
    fillLabels(symtab, &result, &data);
    for ( int i = 0; i < result.capacity; i++ ) {
        if ( !result.strInstr[i] ) {
            continue;
        }
        int labelIndex = searchLabelIndex(&data, i + entry);
        if ( labelIndex != -1 ) {
            s = searchValue(symtab, i + entry);
            if ( !s ) {
                printf("sub_%x:\n", i + entry);
            } else {
                printf("<%s>_<%x>\n", s->name, i + entry);
            }
        }
        printf("%x:\t", i + entry);
        printf("%s\n", result.strInstr[i]);
        free(result.strInstr[i]);
    }
    free(result.strInstr);
    return 0;
}



int main(int argc, char** argv) {
    if ( argc != 2 ) {
        fprintf(stderr, "No input file provided\n");
        return EXIT_FAILURE;
    }
    int size;
    char* mapping = mapElf(argv[1], &size);
    if ( !mapping ) {
        return EXIT_FAILURE;
    }
    SymbolTable symtab = parseFunctionNames(mapping); 
    recursiveDissasembly(mapping, &symtab);
    munmap(mapping, size);
    free(symtab.content);
    return EXIT_SUCCESS; 
}
