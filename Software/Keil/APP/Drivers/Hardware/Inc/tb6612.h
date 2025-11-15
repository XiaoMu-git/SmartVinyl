#ifndef _TB6612_H_
#define _TB6612_H_

#include "tim.h"

void tb6612Init(TIM_HandleTypeDef *htim, uint32_t channel, uint32_t pulse, GPIO_TypeDef *in1_gpio, uint16_t in1_pin, GPIO_TypeDef *in2_gpio, uint16_t in2_pin);
uint8_t tb6612SetDirection(int8_t dirction);
uint8_t tb6612SetSpeed(uint8_t speed);

#endif // !_TB6612_H_
