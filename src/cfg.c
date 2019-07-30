#include "cfg.h"


void openBasicBlock(int bbNum, bool enclosure) {
    if ( enclosure ) {
        printf("\" ]\n");
    }
    printf("%d [ shape=rectangle label=\"", bbNum); 
}

void makeTransition(uint32_t from, uint32_t to, bool fallthrought) {
    printf("%d -> %d", from, to);
    if ( fallthrought ) {
        printf(" [ label=\"fallthrough\" ]");
    }
    putchar('\n');
}


uint32_t findBB(instructionData* data, uint32_t addr) {
    uint32_t max = 0;
    for ( uint32_t i = 0; data->transitions[i].to < 0xffffff; i++ ) {
        if ( data->transitions[i].to > max && data->transitions[i].to < addr ) {
            max = data->transitions[i].to;
        }
    }
    return max;
}

void addrToBB(instructionData* data) {
    for ( uint32_t i = 0; data->transitions[i].to < 0xffffff; i++ ) {
        if ( data->transitions[i].from == 0xffffffff ) {
            continue;
        }
        data->transitions[i].from = findBB(data, data->transitions[i].from); 
    }
}

void makeTransitions(instructionData* data) {
    for ( uint32_t i = 0; data->transitions[i].to < 0xffffff; i++ ) {
        if ( data->transitions[i].from == 0xffffffff ) {
           continue; 
        }
        makeTransition(data->transitions[i].from, data->transitions[i].to, data->transitions[i].fallthrough);
    }
}

void makeGraph(int length, const uint8_t* instruction) {
    printf("digraph G {\n");
    instructionData data = {0, { 0 }, false, 0, NULL, instruction, { {0, 0, false} }};
    memset(data.transitions, 0xff, 2048 * sizeof(transition));
    data.transitions[0].to = 0x0;
    char strInstr[2048][20] = { { 0 } };
    int prev = 0;
    while ( data.index < length ) {
        if ( !decodeSingleInstruction(length, &data, strInstr[data.index]) ) {
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
                if ( i && data.transitions[labelIndex].to <= (uint32_t)length ) {
                    data.index = findBB(&data, prev);
                    writeLabelIndex(&data, data.transitions[labelIndex].to, true);
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

    printf("\" ]\n");
    makeTransitions(&data);
    putchar('}');
}
