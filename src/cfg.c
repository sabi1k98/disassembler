#include "cfg.h"



void openBasicBlock(int bbNum, bool enclosure) {
    if ( enclosure ) {
        printf("\\l\" ]\n");
    }
    printf("%d [ shape=rectangle label=\"", bbNum); 
}

void makeTransition(int from, int to, int address) {
    printf("%d -> %d [ label = \"%d\" ]\n", from, to, address);
}

void makeTransitions(instructionData* data) {
    for ( int i = 0; data->basicBlocks[i] < 0xff; i++ ) {
        makeTransition(i, i, data->basicBlocks[i]);
    }
}

void makeGraph(int length, uint8_t* instruction) {
    printf("digraph G {\n");
    instructionData data = {0, { 0 }, false, 0, NULL, instruction, { 0 }};
    memset(data.basicBlocks, 0xff, 2048 * 4);
    data.basicBlocks[0] = 0x0;
    openBasicBlock(0, false);
    char strInstr[20];
    int bbIndex;
    while ( data.index < length ) {
        if ( (bbIndex = searchLabelIndex(&data, data.index)) != -1 ) {
            openBasicBlock(bbIndex, true);
            bbIndex++;
        }
        int labelIndex = searchLabelIndex(&data, data.index);
        if ( labelIndex != -1 ) {
            printf("BB%d:\\l", labelIndex);
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
