#include "tb6612.h"

TIM_HandleTypeDef *tb6612_htim;
uint32_t tb6612_channel;
uint32_t tb6612_pulse;
GPIO_TypeDef *tb6612_in1_gpio;
uint16_t tb6612_in1_pin;
GPIO_TypeDef *tb6612_in2_gpio;
uint16_t tb6612_in2_pin;

/// @brief TB6612 初始化
/// @param  
void tb6612Init(TIM_HandleTypeDef *htim, uint32_t channel, uint32_t pulse, GPIO_TypeDef *in1_gpio, uint16_t in1_pin, GPIO_TypeDef *in2_gpio, uint16_t in2_pin) {
    if (htim == NULL || in1_gpio == NULL || in2_gpio == NULL || tb6612_htim == htim) return;
    GPIO_InitTypeDef GPIO_InitStruct = { 0 };
    __HAL_RCC_GPIOA_CLK_ENABLE();
    tb6612_htim = htim;
    tb6612_channel = channel;
    tb6612_pulse = pulse;
    tb6612_in1_gpio = in1_gpio;
    tb6612_in1_pin = in1_pin;
    tb6612_in2_gpio = in2_gpio;
    tb6612_in2_pin = in2_pin;

    // 初始化 TB6612 引脚
    GPIO_InitStruct.Pin = tb6612_in1_pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(tb6612_in1_gpio, &GPIO_InitStruct);
    HAL_GPIO_WritePin(tb6612_in1_gpio, tb6612_in1_pin, GPIO_PIN_RESET);

    GPIO_InitStruct.Pin = tb6612_in2_pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(tb6612_in2_gpio, &GPIO_InitStruct);
    HAL_GPIO_WritePin(tb6612_in2_gpio, tb6612_in2_pin, GPIO_PIN_RESET);
}

/// @brief 设置电机方向
/// @param dir 
uint8_t tb6612SetDirection(int8_t dirction) {
    if (dirction > 0) {
        HAL_GPIO_WritePin(tb6612_in1_gpio, tb6612_in1_pin, GPIO_PIN_SET);
        HAL_GPIO_WritePin(tb6612_in2_gpio, tb6612_in2_pin, GPIO_PIN_RESET);
    }
    else if (dirction < 0) {
        HAL_GPIO_WritePin(tb6612_in1_gpio, tb6612_in1_pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(tb6612_in2_gpio, tb6612_in2_pin, GPIO_PIN_SET);
    }
    else {
        HAL_GPIO_WritePin(tb6612_in1_gpio, tb6612_in1_pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(tb6612_in2_gpio, tb6612_in2_pin, GPIO_PIN_RESET);
    }
    return RET_DONE;
}

/// @brief 设置电机速度
/// @param speed 0 ~ 100
/// @return 
uint8_t tb6612SetSpeed(uint8_t speed) {
    if(speed > 100) speed = 100;
    uint32_t new_pulse = ((uint32_t)speed * tb6612_pulse) / 100;
    __HAL_TIM_SET_COMPARE(tb6612_htim, tb6612_channel, new_pulse);
    return RET_DONE;
}
