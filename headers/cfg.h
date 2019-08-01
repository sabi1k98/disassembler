
/**
 * @brief   library for creating cfg's in Graphwiz containing basic blocks
 * @file    cfg.h
 * @author  xsabol
 */

#ifndef CFG_H
#define CFG_H
#include "decoder.h"

uint32_t findBB(instructionData* data, uint32_t addr);

void addrToBB(instructionData* data);

void openBasicBlock(int bbNum, bool enclosure);

void makeTransitions(instructionData* data);

void makeTransition(uint32_t from, uint32_t to, bool fallthrough);

void makeGraph(int length, const uint8_t* instructions, int offset);

#endif
