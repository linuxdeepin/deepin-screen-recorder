#include "bit_writing_block.h"

int pushBack(BitWritingBlock * bitWritingBlock, uint8_t* currnet) {
	++(bitWritingBlock->datasSize);
	bitWritingBlock->datas = (uint8_t**)realloc(bitWritingBlock->datas, bitWritingBlock->datasSize * sizeof(uint8_t*));
	bitWritingBlock->datas[(bitWritingBlock->datasSize) - 1] = currnet;
}


BitWritingBlock * initBitWritingBlock(BitWritingBlock * bitWritingBlock) {

	bitWritingBlock = (BitWritingBlock *)malloc(sizeof(BitWritingBlock));

	bitWritingBlock->currnet = (uint8_t*)malloc(BLOCK_SIZE * sizeof(uint8_t));
	memset(bitWritingBlock->currnet, 0, BLOCK_SIZE);

	bitWritingBlock->datasSize = 1;
	bitWritingBlock->datas = (uint8_t**)malloc(bitWritingBlock->datasSize * sizeof(uint8_t*));
	bitWritingBlock->datas[(bitWritingBlock->datasSize) - 1] = bitWritingBlock->currnet;

	bitWritingBlock->pos = 0;
	bitWritingBlock->remain = 8;

	return bitWritingBlock;
}

int releaseBitWritingBlock(BitWritingBlock * bitWritingBlock) {
	return 0;
}

void writeBits(BitWritingBlock * bitWritingBlock, uint32_t src, int32_t bitNum)
{
	while (0 < bitNum) {
		if (bitWritingBlock->remain <= bitNum) {
			bitWritingBlock->currnet[bitWritingBlock->pos] = bitWritingBlock->currnet[bitWritingBlock->pos] | (src << (8 - (bitWritingBlock->remain)));
			src >>= bitWritingBlock->remain;
			bitNum -= bitWritingBlock->remain;
			bitWritingBlock->remain = 8;
			++(bitWritingBlock->pos);
			if (bitWritingBlock->pos == BLOCK_SIZE) {
				bitWritingBlock->currnet = (uint8_t*)malloc(BLOCK_SIZE * sizeof(uint8_t));
				memset(bitWritingBlock->currnet, 0, BLOCK_SIZE);
				pushBack(bitWritingBlock, bitWritingBlock->currnet);
				bitWritingBlock->pos = 0;
			}
		}
		else {
			bitWritingBlock->currnet[bitWritingBlock->pos] = (bitWritingBlock->currnet[bitWritingBlock->pos] << bitNum) | (((1 << bitNum) - 1) & src);
			bitWritingBlock->remain -= bitNum;
			bitNum = 0;
		}
	}
}

void writeByte(BitWritingBlock * bitWritingBlock, uint8_t b)
{
	writeBits(bitWritingBlock, b, 8);
}

int toFile(BitWritingBlock * bitWritingBlock, FILE* dst)
{
	uint8_t size;
	int i = 0;
	for (i = 0; i<bitWritingBlock->datasSize; ++i) {
		uint8_t* block = bitWritingBlock->datas[i];
		size = block == bitWritingBlock->currnet ? (bitWritingBlock->remain == 0 ? bitWritingBlock->pos : bitWritingBlock->pos + 1) : BLOCK_SIZE;
		fwrite(&size, 1, 1, dst);
		fwrite(block, size, 1, dst);
	}

	return 0;
}
