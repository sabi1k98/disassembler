
/**
 * @brief   decoder for subset of assembly instructions
 * @file    decoder.h
 * @author  xsabol
 */

#ifndef DECODER_H
#define DECODER_H

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

enum constants {
    MAX_BYTES = 15 /**< Maximum length of encoded instruction */
};


/**
 * @brief converts input arguments into numeric form
 * @param argc  ammount of bytes
 * @param argv  array of strings, each string should be one byte in hex
 * @param result    array of 1-byte where allocated size must be at least argc
 * @param return true on success, false if conversion fails
 */
bool strHex2Bin(int argc, const char** argv, uint8_t* result);



#endif
