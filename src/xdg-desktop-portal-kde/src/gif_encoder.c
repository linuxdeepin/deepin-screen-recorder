#include "gif_encoder.h"
#include<stdio.h>

GifInfo * init(GifInfo * gifInfo, uint16_t width, uint16_t height, const char* gifTitle) {

	gifInfo = (GifInfo *)malloc(sizeof(GifInfo));

	gifInfo->width = width;
	gifInfo->height = height;
	gifInfo->left = 0;
	gifInfo->top = 0;
	gifInfo->gifTitle = gifTitle;
	gifInfo->useDither = 0;
/*
    errno_t err;
    err = fopen_s(&gifInfo->gifFile, gifInfo->gifTitle, "wb");
    if (err != 0) {
        return NULL;
    }
*/
    gifInfo->gifFile = fopen(gifInfo->gifTitle, "wb");
    if(gifInfo->gifFile == NULL){
        return NULL;
    }

	header(gifInfo);
	logicalScreenDescriptor(gifInfo);
	basicGlobalColorTable(gifInfo);

	return gifInfo;
}


int header(GifInfo * gifInfo) {
	fwrite("GIF89a", 6, 1, gifInfo->gifFile);
	return 0;
}

int logicalScreenDescriptor(GifInfo * gifInfo) {

	fwrite(&gifInfo->width, 2, 1, gifInfo->gifFile);
	fwrite(&gifInfo->height, 2, 1, gifInfo->gifFile);

	uint8_t globalColorTableFlag = 1;
	uint8_t colorResolution = 7;
	uint8_t sortFlag = 0;
	uint8_t sizeOfGlobalColorTable = 7;

	uint8_t packedField = (globalColorTableFlag << 7) | (colorResolution << 4) | (sortFlag << 3) | sizeOfGlobalColorTable;

	fwrite(&packedField, 1, 1, gifInfo->gifFile);


	uint8_t backgroundColorIndex = 0xFF;

	fwrite(&backgroundColorIndex, 1, 1, gifInfo->gifFile);

	uint8_t pixelAspectRatio = 0;

	fwrite(&pixelAspectRatio, 1, 1, gifInfo->gifFile);

	return 0;
}


int basicGlobalColorTable(GifInfo * gifInfo) {
	const int R_RANGE = 6;
	const int G_RANGE = 7;
	const int B_RANGE = 6;

	uint8_t colorTable[256][3];
    int idx = 0;
    int r = 0;
    int g = 0;
    int b = 0;
    for (r = 0; r < R_RANGE; ++r) {
        for (g = 0; g < G_RANGE; ++g) {
            for (b = 0; b < B_RANGE; ++b) {
				colorTable[idx][0] = 255 * r / (R_RANGE - 1);
				colorTable[idx][1] = 255 * g / (G_RANGE - 1);
				colorTable[idx][2] = 255 * b / (B_RANGE - 1);
				++idx;
			}
		}
	}
	for (; idx < 256; ++idx) {
		colorTable[idx][0] = 0;
		colorTable[idx][1] = 0;
		colorTable[idx][2] = 0;
	}

	fwrite(colorTable, 256 * 3, 1, gifInfo->gifFile);

	return 0;
}


int graphicsControlExtension(GifInfo * gifInfo, uint16_t delay) {

    uint8_t extensionIntroducer = 0x21;
    uint8_t grapicControlLabel = 0xF9;
    uint8_t byteSize = 0x04;

	fwrite(&extensionIntroducer, 1, 1, gifInfo->gifFile);
	fwrite(&grapicControlLabel, 1, 1, gifInfo->gifFile);
	fwrite(&byteSize, 1, 1, gifInfo->gifFile);

	uint8_t reservedForFutureUse = 0;
	uint8_t disposalMethod = 2;
	uint8_t userInputFlag = 0;
	uint8_t transparentColorFlag = 1;
	uint8_t packedField = (reservedForFutureUse << 5) | (disposalMethod << 2) | (userInputFlag << 1) | transparentColorFlag;
	fwrite(&packedField, 1, 1, gifInfo->gifFile);

	uint16_t delayTime = delay;
	fwrite(&delayTime, 2, 1, gifInfo->gifFile);

	uint8_t transparentColorIndex = 0xFF;	
	fwrite(&transparentColorIndex, 1, 1, gifInfo->gifFile);

    uint8_t blockTerminator = 0;
	fwrite(&blockTerminator, 1, 1, gifInfo->gifFile);

	return 0;
}

int imageDescriptor(GifInfo * gifInfo) {

    uint8_t imageSeperator = 0x2C;
    uint16_t imageLeft = gifInfo->left;
    uint16_t imageTop = gifInfo->top;
	uint16_t imageWidth = gifInfo->width;
	uint16_t imageHeight = gifInfo->height;

	fwrite(&imageSeperator, 1, 1, gifInfo->gifFile);
	fwrite(&imageLeft, 2, 1, gifInfo->gifFile);
	fwrite(&imageTop, 2, 1, gifInfo->gifFile);
	fwrite(&imageWidth, 2, 1, gifInfo->gifFile);
	fwrite(&imageHeight, 2, 1, gifInfo->gifFile);

	uint8_t localColorTableFlag = 0;
	uint8_t interlaceFlag = 0;
	uint8_t sortFlag = 0;
	uint8_t reservedForFutureUse = 0;
	uint8_t sizeOfLocalColorTable = 0;

	uint8_t packedField = (localColorTableFlag << 7) | (interlaceFlag << 6) | (sortFlag << 5) | (reservedForFutureUse << 3) | sizeOfLocalColorTable;

	fwrite(&packedField, 1, 1, gifInfo->gifFile);

	return 0;
}

int imageData(GifInfo * gifInfo, uint8_t * indexStream) {

	int32_t MAX_STACK_SIZE = 4096;
	int32_t BYTE_NUM = 256;

	uint32_t pixelNum = gifInfo->width * gifInfo->height;
	uint8_t* endPixels = indexStream + (0 + gifInfo->height - 1) * gifInfo->width + 0 + gifInfo->width;
	uint8_t dataSize = 8;
	uint32_t codeSize = dataSize + 1;
	uint32_t codeMask = (1 << codeSize) - 1;
	BitWritingBlock * writingBlock = NULL;
	writingBlock = initBitWritingBlock(writingBlock);
	fwrite(&dataSize, 1, 1, gifInfo->gifFile);

	uint16_t * lzwInfoHolder;
	lzwInfoHolder = (uint16_t*)malloc((MAX_STACK_SIZE * BYTE_NUM) * sizeof(uint16_t));

	uint16_t* lzwInfos = &lzwInfoHolder[0];

	indexStream = indexStream + gifInfo->width * 0 + 0;
	uint8_t* rowStart = indexStream;
	uint32_t clearCode = 1 << dataSize;
	writeBits(writingBlock, clearCode, codeSize);
	uint32_t infoNum = clearCode + 2;
	uint16_t current = *indexStream;
	uint8_t endOfImageData = 0;

	++indexStream;
	if (gifInfo->width <= indexStream - rowStart) {
		rowStart = rowStart + gifInfo->width;
		indexStream = rowStart;
	}

	uint16_t* next;
	while (endPixels > indexStream) {
		next = &lzwInfos[current * BYTE_NUM + *indexStream];
		if (0 == *next || *next >= MAX_STACK_SIZE) {
			writeBits(writingBlock, current, codeSize);
			*next = infoNum;
			if (infoNum < MAX_STACK_SIZE) {
				++infoNum;
			}
			else {
				writeBits(writingBlock, clearCode, codeSize);
				infoNum = clearCode + 2;
				codeSize = dataSize + 1;
				codeMask = (1 << codeSize) - 1;
				memset(lzwInfos, 0, MAX_STACK_SIZE * BYTE_NUM * sizeof(uint16_t));
			}
			if (codeMask < infoNum - 1 && infoNum < MAX_STACK_SIZE) {
				++codeSize;
				codeMask = (1 << codeSize) - 1;
			}
			if (endPixels <= indexStream) {
				break;
			}
			current = *indexStream;
		}
		else {
			current = *next;
		}
		++indexStream;
		if (gifInfo->width <= indexStream - rowStart) {
			rowStart = rowStart + gifInfo->width;
			indexStream = rowStart;
		}
	}
	writeBits(writingBlock, current, codeSize);
	toFile(writingBlock, gifInfo->gifFile);
	fwrite(&endOfImageData, 1, 1, gifInfo->gifFile);

	return 0;
}


int finish(GifInfo * gifInfo) {
	uint8_t trailer = 0x3B;

	fwrite(&trailer, 1, 1, gifInfo->gifFile);
	fclose(gifInfo->gifFile);
	free(gifInfo);

	return 0;
}

int basicReduceColor(GifInfo * gifInfo, uint32_t* pixels)
{
	const int R_RANGE = 6;
	const int G_RANGE = 7;
	const int B_RANGE = 6;

	const int32_t ERROR_PROPAGATION_DIRECTION_NUM = 4;
	const int32_t ERROR_PROPAGATION_DIRECTION_X[] = { 1, -1, 0, 1 };
	const int32_t ERROR_PROPAGATION_DIRECTION_Y[] = { 0, 1, 1, 1 };
	const int32_t ERROR_PROPAGATION_DIRECTION_WEIGHT[] = { 7, 3, 5, 1 };
	
	uint32_t pixelNum = (gifInfo->width) * (gifInfo->height);
	uint8_t* dst = (uint8_t*)pixels;
	uint32_t* src = pixels;
	uint32_t* last = src + pixelNum;
    uint32_t y = 0;
    uint32_t x = 0;
    for (y = 0; y < gifInfo->height; ++y) {
        for (x = 0; x < gifInfo->width; ++x) {
			uint32_t color = *src;
			if (0 == (color >> 24)) {
                *dst = 255;
			}
			else {
				int16_t r = color & 0xFF;
				int16_t g = (color >> 8) & 0xFF;
				int16_t b = (color >> 16) & 0xFF;
				uint16_t rIdx = (r * (R_RANGE - 1) + 127) / 255;
				uint16_t gIdx = (g * (G_RANGE - 1) + 127) / 255;
				uint16_t bIdx = (b * (B_RANGE - 1) + 127) / 255;
				*dst = (uint8_t)(rIdx * (G_RANGE * B_RANGE) + gIdx * B_RANGE + bIdx);

				if (gifInfo->useDither == 0) {
					int16_t diffR = r - (255 * rIdx / (R_RANGE - 1));
					int16_t diffG = g - (255 * gIdx / (G_RANGE - 1));
					int16_t diffB = b - (255 * bIdx / (B_RANGE - 1));
                    int directionId;
                    for (directionId = 0; directionId < ERROR_PROPAGATION_DIRECTION_NUM; ++directionId) {
						uint32_t* pixel = src + ERROR_PROPAGATION_DIRECTION_X[directionId] + ERROR_PROPAGATION_DIRECTION_Y[directionId] * gifInfo->width;
						if (x + ERROR_PROPAGATION_DIRECTION_X[directionId] >= gifInfo->width ||
							y + ERROR_PROPAGATION_DIRECTION_Y[directionId] >= gifInfo->height || 0 == (*src >> 24)) {
							continue;
						}
						int32_t weight = ERROR_PROPAGATION_DIRECTION_WEIGHT[directionId];
						int32_t dstR = ((int32_t)((*pixel) & 0xFF) + (diffR * weight + 8) / 16);
						int32_t dstG = (((int32_t)((*pixel) >> 8) & 0xFF) + (diffG * weight + 8) / 16);
						int32_t dstB = (((int32_t)((*pixel) >> 16) & 0xFF) + (diffB * weight + 8) / 16);
						int32_t dstA = (int32_t)(*pixel >> 24);
						int32_t newR = MIN(255, MAX(0, dstR));
						int32_t newG = MIN(255, MAX(0, dstG));
						int32_t newB = MIN(255, MAX(0, dstB));
						*pixel = (dstA << 24) | (newB << 16) | (newG << 8) | newR;
					}
				}
			}
			++dst;
			++src;
		}
	}

	return 0;
}

int writeNetscapeExt(GifInfo * gifInfo)
{
    const uint8_t netscapeExt[] = { 0x21, 0xFF, 0x0B, 'N', 'E', 'T', 'S', 'C', 'A', 'P', 'E', '2', '.', '0', 0x03, 0x01, 0x00, 0x00, 0x00 };
	fwrite(netscapeExt, sizeof(netscapeExt), 1, gifInfo->gifFile);
	return 0;
}

