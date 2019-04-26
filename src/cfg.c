#include "cfg.h"


bool isNonJumpInstr(uint8_t opcode) {
    return opcode != JMP_REL8OFF && opcode != JMP_REL32OFF
        && opcode != JE_REL8OFF && opcode != JE_REL32OFF
        && opcode != JNE_REL8OFF && opcode != JNE_REL32OFF
        && opcode != JB_REL8OFF && opcode != JB_REL32OFF;        
}

void openBasicBlock(int bbNum, bool enclosure) {
    if ( enclosure ) {
        printf("\\l\" ]\n");
    }
    printf("%d [ shape=rectangle label=\"", bbNum); 
}

void makeTransition(uint32_t from, uint32_t to) {
    printf("%d -> %d\n", from, to);
}


uint32_t findBB(instructionData* data, uint32_t addr) {
    uint32_t max = 0;
    for ( uint32_t i = 0; data->basicBlocks[i].to < 0xffffff; i++ ) {
        if ( data->basicBlocks[i].to > max && data->basicBlocks[i].to < addr ) {
            max = data->basicBlocks[i].to;
        }
    }
    return max;
}

void addrToBB(instructionData* data) {
    for ( uint32_t i = 0; data->basicBlocks[i].to < 0xffffff; i++ ) {
        if ( data->basicBlocks[i].from == 0xffffffff ) {
            continue;
        }
        //printf("%d %d\n", data->basicBlocks[i].from, data->basicBlocks[i].to);
        data->basicBlocks[i].from = findBB(data, data->basicBlocks[i].from); 
    }
}

void makeTransitions(instructionData* data) {
    for ( uint32_t i = 0; data->basicBlocks[i].to < 0xffffff; i++ ) {
        if ( data->basicBlocks[i].from == 0xffffffff ) {
           continue; 
        }
        makeTransition(data->basicBlocks[i].from, data->basicBlocks[i].to);
    }
}

void makeGraph(int length, uint8_t* instruction) {
    printf("digraph G {\n");
    instructionData data = {0, { 0 }, false, 0, NULL, instruction, { {0, 0, 0, 0} }, -1};
    memset(data.basicBlocks, 0xff, 2048 * sizeof(transition));
    data.basicBlocks[0].to = 0x0;
    char strInstr[2048][20] = { { 0 } };
    int prev = 0;
    while ( data.index < length ) {
        if ( !decodeSingleInstruction(length, instruction, &data, strInstr[data.index]) ) {
            return;
        }
        clearInstructionData(&data);
    }
    addrToBB(&data);
    for ( int i = 0; i < 2048; i++ ) {
        if ( !strInstr[i][0] ) {
            continue;
        }
        int labelIndex = searchLabelIndex(&data, i);
        if ( labelIndex != -1 ) { 
            if ( strInstr[prev][0] != 'j' ) {
                if ( i && data.basicBlocks[labelIndex].to <= (uint32_t)length ) {
                    data.index = findBB(&data, prev);
                    writeLabelIndex(&data, data.basicBlocks[labelIndex].to);
                }
                //addrToBB(&data);
            }
            openBasicBlock(i, i != 0);
            printf("BB-0x%x:\\l", i);
        }
        printf("%x:\t", i);
        printf("%s\\l", strInstr[i]);
        prev = i;
    }

    printf("\\l\" ]\n");
    makeTransitions(&data);
    putchar('}');
    /*
    for ( uint32_t i = 0; data.basicBlocks[i].to < 0xffffff; i++ ) {
        printf("%d %d\n", data.basicBlocks[i].from, data.basicBlocks[i].to);
    }
    */
}
