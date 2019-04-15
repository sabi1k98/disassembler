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

int addrToBB(instructionData* data, uint32_t addr) {
    for ( uint32_t i = 0; data->basicBlocks[i].to < 0xffffff; i++ ) {
        if ( addr == data->basicBlocks[i].to && data->basicBlocks[i].toBB != -1 ) {
            return data->basicBlocks[i].toBB;
        }
        if ( addr == data->basicBlocks[i].from && data->basicBlocks[i].fromBB != -1 ) {
            return data->basicBlocks[i].fromBB;
        }
    }
    return -1;
}

void makeTransitions(instructionData* data) {
    for ( uint32_t i = 0; data->basicBlocks[i].to < 0xffffff; i++ ) {
        if ( data->basicBlocks[i].from == 0xffffff ) {
           continue; 
        }
        if ( data->basicBlocks[i].toBB == -1 ) {
            assignFromBB(data, data->basicBlocks[i].to, addrToBB(data, data->basicBlocks[i].to));
        }
        if ( data->basicBlocks[i].toBB == -1 || data->basicBlocks[i].fromBB == -1 ) {
            continue;
        }
        makeTransition(data->basicBlocks[i].fromBB, data->basicBlocks[i].toBB);
    }
}

void makeGraph(int length, uint8_t* instruction) {
    printf("digraph G {\n");
    instructionData data = {0, { 0 }, false, 0, NULL, instruction, { {0, 0, 0, 0} }, -1};
    memset(data.basicBlocks, 0xff, 2048 * 4);
    data.basicBlocks[0].to = 0x0;
    openBasicBlock(0, false);
    char strInstr[20];
    int bbIndex;
    while ( data.index < length ) {
        if ( (bbIndex = searchLabelIndex(&data, data.index)) != -1 ) {
            if (isNonJumpInstr(data.opcode)) {
                data.index--;
                writeLabelIndex(&data, ++data.index);
            }
            data.currentBB++;
            openBasicBlock(data.currentBB, true);
            printf("BB-0x%x:\\l", data.index);
            assignFromBB(&data, data.index, data.currentBB);
        }
        if ( !decodeSingleInstruction(length, instruction, &data, strInstr) ) {
            return;
        }
        printf("%s\\l", strInstr);
        clearInstructionData(&data);
    }
    printf("\\l\" ]\n");
    makeTransitions(&data);
    putchar('}');
}
