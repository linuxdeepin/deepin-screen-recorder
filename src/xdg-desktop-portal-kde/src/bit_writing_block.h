#ifndef BIT_WRITING_BLOCK_H
#define BIT_WRITING_BLOCK_H

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

static const int32_t BLOCK_SIZE = 255;

typedef struct _BitWritingBlock {
	uint8_t** datas;
	int datasSize;
	uint8_t* currnet;
	int pos;
	int remain;
} BitWritingBlock;

int pushBack(BitWritingBlock * bitWritingBlock, uint8_t* currnet);
BitWritingBlock * initBitWritingBlock(BitWritingBlock * bitWritingBlock);
int releaseBitWritingBlock(BitWritingBlock * bitWritingBlock);
void writeBits(BitWritingBlock * bitWritingBlock, uint32_t src, int32_t bitNum);
void writeByte(BitWritingBlock * bitWritingBlock, uint8_t b);
int toFile(BitWritingBlock * bitWritingBlock, FILE* dst);

#endif BIT_WRITING_BLOCK_H