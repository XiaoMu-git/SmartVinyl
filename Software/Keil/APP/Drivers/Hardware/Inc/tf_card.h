#ifndef _TF_CARD_H_
#define _TF_CARD_H_

#include "sdio.h"

void tfInit(SD_HandleTypeDef *hsd);
uint8_t tfWriteData(const char *path, uint8_t data, uint32_t length);
uint8_t tfReadData(const char *path, uint8_t buff, uint32_t length);

#endif // !_TF_CARD_H_
