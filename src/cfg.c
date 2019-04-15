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
    memset(data.basicBlocks, 0xff, 2048 * sizeof(transition));
    data.basicBlocks[0].to = 0x0;
    char strInstr[2048][20] = { { 0 } };
    int bbIndex;
    while ( data.index < length ) {
        if ( (bbIndex = searchLabelIndex(&data, data.index)) != -1 ) {
            if ( isNonJumpInstr(data.opcode) && data.index ) {
                data.index--;
                writeLabelIndex(&data, ++data.index);
            }
            data.currentBB++;
            //assignFromBB(&data, data.index, data.currentBB);
        }
        if ( !decodeSingleInstruction(length, instruction, &data, strInstr[data.index]) ) {
            return;
        }
        //printf("%s\\l", strInstr);
        clearInstructionData(&data);
    }
    data.currentBB = 0;
    for ( int i = 0; i < 2048; i++ ) {
        if ( !strInstr[i][0] ) {
            continue;
        }
        int labelIndex = searchLabelIndex(&data, i);
        if ( labelIndex != -1 ) {
            openBasicBlock(data.currentBB, data.currentBB != 0);
            printf("BB-0x%x:\\l", i);
            assignFromBB(&data, i, data.currentBB);
            data.currentBB++;
        }
        printf("%x:\t", i);
        printf("%s\\l", strInstr[i]);
    }

    printf("\\l\" ]\n");
    makeTransitions(&data);
    putchar('}');
    /* 
    for (int i = 0; data.basicBlocks[i].to <= 0xffffff; i++) {
        printf("%d\nFrom: %d\nTo: %d\nFromBB: %d\nToBB: %d\n", i, data.basicBlocks[i].from, data.basicBlocks[i].to, data.basicBlocks[i].fromBB, data.basicBlocks[i].toBB);
    }
    */
}
