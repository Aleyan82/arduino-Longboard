/*
 * Copyright (c) 2016 Thomas Roell.  All rights reserved.
 * Copyright (c) 2019 Fredrik Hubinette.  All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal with the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 *  1. Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimers.
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimers in the
 *     documentation and/or other materials provided with the distribution.
 *  3. Neither the name of Thomas Roell, nor the names of its contributors
 *     may be used to endorse or promote products derived from this Software
 *     without specific prior written permission.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
 * CONTRIBUTORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * WITH THE SOFTWARE.
 */

#include "Arduino.h"
#include "stm32l4_wiring_private.h"

#define PWM_INSTANCE_TIM1      0
#define PWM_INSTANCE_TIM2      1
#define PWM_INSTANCE_TIM3      2
#define PWM_INSTANCE_TIM15     3
#define PWM_INSTANCE_TIM16     4

/*
 * Pins descriptions
 */
extern const PinDescription g_APinDescription[] =
{
  // 0, Data1 / ID
  { GPIOA, GPIO_PIN_MASK(GPIO_PIN_PA7),  GPIO_PIN_PA7_TIM1_CH1N,   (PIN_ATTR_ADC | PIN_ATTR_EXTI | PIN_ATTR_PWM),   PWM_INSTANCE_TIM1,  PWM_CHANNEL_1, ADC_INPUT_12    },
  // 1, Join PA7 (data1/ID) 
  { GPIOA, GPIO_PIN_MASK(GPIO_PIN_PA6),  GPIO_PIN_PA6,             (PIN_ATTR_ADC | PIN_ATTR_EXTI),                  PWM_INSTANCE_NONE,  PWM_CHANNEL_NONE, ADC_INPUT_11    },
  
  // 2, data2 (joined with PB4), WS2811, uart1 tx
  { GPIOA, GPIO_PIN_MASK(GPIO_PIN_PA9),  GPIO_PIN_PA9_TIM1_CH2,    (PIN_ATTR_EXTI | PIN_ATTR_PWM),                   PWM_INSTANCE_TIM1,  PWM_CHANNEL_2, ADC_INPUT_NONE },
  // 3, Join with PA9 (data2), spi1 miso, uart1 cts
  { GPIOB, GPIO_PIN_MASK(GPIO_PIN_PB4),  GPIO_PIN_PB4,             (PIN_ATTR_EXTI),                                 PWM_INSTANCE_NONE,  PWM_CHANNEL_NONE, ADC_INPUT_NONE },

  // 4, data3 (joined with PB5), WS2811, uart1 rx
  { GPIOA, GPIO_PIN_MASK(GPIO_PIN_PA10), GPIO_PIN_PA10_TIM1_CH3,   (PIN_ATTR_EXTI | PIN_ATTR_PWM),                   PWM_INSTANCE_TIM1,  PWM_CHANNEL_3, ADC_INPUT_NONE },
  // 5, Join PA10 (data3), spi1 mosi, sai sd b
  { GPIOB, GPIO_PIN_MASK(GPIO_PIN_PB5),  GPIO_PIN_PB5,             (PIN_ATTR_EXTI),                                 PWM_INSTANCE_NONE,  PWM_CHANNEL_NONE, ADC_INPUT_NONE },

  // 6, neopixels 4
  { GPIOA, GPIO_PIN_MASK(GPIO_PIN_PA4),  GPIO_PIN_PA4,             (PIN_ATTR_ADC | PIN_ATTR_DAC | PIN_ATTR_EXTI),   PWM_INSTANCE_NONE,  PWM_CHANNEL_NONE, ADC_INPUT_9    },

  // 7, Free 1, WS2811, spi1 sck, sai sck b, uart 1 rts
  { GPIOB, GPIO_PIN_MASK(GPIO_PIN_PB3),  GPIO_PIN_PB3_TIM2_CH2,    (PIN_ATTR_PWM | PIN_ATTR_EXTI),                  PWM_INSTANCE_TIM2,  PWM_CHANNEL_2,    ADC_INPUT_NONE },
  // 8, FREE 2, WS2811, PWM, spi2 sck, uart3 tx, i2c4 scl
  { GPIOB, GPIO_PIN_MASK(GPIO_PIN_PB10), GPIO_PIN_PB10_TIM2_CH3,   (PIN_ATTR_PWM | PIN_ATTR_EXTI),                  PWM_INSTANCE_TIM2,  PWM_CHANNEL_3,    ADC_INPUT_NONE },
  // 9, FREE 3, joined with PC2, WS2811, PWM, uart3 rx, i2c4 scl
  { GPIOB, GPIO_PIN_MASK(GPIO_PIN_PB11), GPIO_PIN_PB11_TIM2_CH4,   (PIN_ATTR_PWM | PIN_ATTR_EXTI),                  PWM_INSTANCE_TIM2,  PWM_CHANNEL_4,    ADC_INPUT_NONE },
  // 10, join pb11 (Free 3), spi2 miso, adc
  { GPIOC, GPIO_PIN_MASK(GPIO_PIN_PC2),  GPIO_PIN_PC2,            (PIN_ATTR_ADC | PIN_ATTR_EXTI),                   PWM_INSTANCE_NONE,  PWM_CHANNEL_NONE, ADC_INPUT_3 },

  // 11, Button 1 (POW), joined with PC5, i2c2 sda
  { GPIOB, GPIO_PIN_MASK(GPIO_PIN_PB14), GPIO_PIN_PB14,            (PIN_ATTR_EXTI),                                 PWM_INSTANCE_NONE,  PWM_CHANNEL_NONE, ADC_INPUT_NONE },
  // 12, Join PB14 (Button 1), adc, wkup
  { GPIOC, GPIO_PIN_MASK(GPIO_PIN_PC5),  GPIO_PIN_PC5,            (PIN_ATTR_ADC | PIN_ATTR_EXTI | PIN_ATTR_WKUP5),  PWM_INSTANCE_NONE,  PWM_CHANNEL_NONE, ADC_INPUT_14 },
  // 13, Button 2 (AUX), joined with PA2, i2c2 scl
  { GPIOB, GPIO_PIN_MASK(GPIO_PIN_PB13), GPIO_PIN_PB13,            (PIN_ATTR_EXTI),                                 PWM_INSTANCE_NONE,  PWM_CHANNEL_NONE, ADC_INPUT_NONE },
  // 14, Joined to PB13 (Button 2), adc, wkup
  { GPIOA, GPIO_PIN_MASK(GPIO_PIN_PA2),  GPIO_PIN_PA2_TIM2_CH3,    (PIN_ATTR_PWM | PIN_ATTR_EXTI | PIN_ATTR_WKUP4), PWM_INSTANCE_TIM2,  PWM_CHANNEL_3,    ADC_INPUT_7 },
  // 15, Button 3 (AUX2), spi2 mosi
  { GPIOB, GPIO_PIN_MASK(GPIO_PIN_PB15), GPIO_PIN_PB15_TIM15_CH2,  (PIN_ATTR_PWM | PIN_ATTR_EXTI),                  PWM_INSTANCE_TIM15, PWM_CHANNEL_2,    ADC_INPUT_NONE },

  // 16, RX, i2c3 scl, adc
  { GPIOC, GPIO_PIN_MASK(GPIO_PIN_PC0),  GPIO_PIN_PC0,            (PIN_ATTR_ADC | PIN_ATTR_EXTI),                   PWM_INSTANCE_NONE,  PWM_CHANNEL_NONE, ADC_INPUT_1 },
  // 17, TX, i2c3 sda, adc
  { GPIOC, GPIO_PIN_MASK(GPIO_PIN_PC1),  GPIO_PIN_PC1,            (PIN_ATTR_ADC | PIN_ATTR_EXTI),                   PWM_INSTANCE_NONE,  PWM_CHANNEL_NONE, ADC_INPUT_2 },

  // 18, I2C SDA
  { GPIOB, GPIO_PIN_MASK(GPIO_PIN_PB7),  GPIO_PIN_PB7,             (PIN_ATTR_EXTI),                                 PWM_INSTANCE_NONE,  PWM_CHANNEL_NONE, ADC_INPUT_NONE },
  // 19, I2C SCL
  { GPIOB, GPIO_PIN_MASK(GPIO_PIN_PB8),  GPIO_PIN_PB8,             PIN_ATTR_EXTI,                                   PWM_INSTANCE_NONE, PWM_CHANNEL_1, ADC_INPUT_NONE    },

  // 20, LED Channel 1
  { GPIOA, GPIO_PIN_MASK(GPIO_PIN_PA1),  GPIO_PIN_PA1_TIM15_CH1N,  (PIN_ATTR_PWM | PIN_ATTR_EXTI),                  PWM_INSTANCE_TIM15, PWM_CHANNEL_1,    ADC_INPUT_NONE },
  // 21, LED Channel 2
  { GPIOB, GPIO_PIN_MASK(GPIO_PIN_PB1),  GPIO_PIN_PB1_TIM3_CH4,    (PIN_ATTR_ADC | PIN_ATTR_PWM | PIN_ATTR_EXTI),   PWM_INSTANCE_TIM3,  PWM_CHANNEL_4,    ADC_INPUT_16   },
  // 22, LED Channel 3
  { GPIOC, GPIO_PIN_MASK(GPIO_PIN_PC7),  GPIO_PIN_PC7_TIM3_CH2,   (PIN_ATTR_PWM | PIN_ATTR_EXTI),                   PWM_INSTANCE_TIM3,  PWM_CHANNEL_2, ADC_INPUT_NONE },
  // 23, LED Channel 4
  { GPIOB, GPIO_PIN_MASK(GPIO_PIN_PB6),  GPIO_PIN_PB6_TIM16_CH1N,  (PIN_ATTR_PWM | PIN_ATTR_EXTI),                  PWM_INSTANCE_TIM16, PWM_CHANNEL_1, ADC_INPUT_NONE },
  // 24, LED Channel 5
  { GPIOC, GPIO_PIN_MASK(GPIO_PIN_PC6),  GPIO_PIN_PC6_TIM3_CH1,   (PIN_ATTR_PWM | PIN_ATTR_EXTI),                   PWM_INSTANCE_TIM3,  PWM_CHANNEL_1, ADC_INPUT_NONE },
  // 25, LED Channel 6
  { GPIOB, GPIO_PIN_MASK(GPIO_PIN_PB0),  GPIO_PIN_PB0_TIM3_CH3,   (PIN_ATTR_ADC | PIN_ATTR_PWM | PIN_ATTR_EXTI),    PWM_INSTANCE_TIM3,  PWM_CHANNEL_3,    ADC_INPUT_15   },

  // 26, Onboard LED
  { GPIOA, GPIO_PIN_MASK(GPIO_PIN_PA15), GPIO_PIN_PA15_TIM2_CH1,   (PIN_ATTR_EXTI | PIN_ATTR_PWM),                  PWM_INSTANCE_TIM2,  PWM_CHANNEL_1,    ADC_INPUT_NONE },

  // 27, detect charging  (is this really a wakeup pin?)
  { GPIOA, GPIO_PIN_MASK(GPIO_PIN_PA0),  GPIO_PIN_PA0_TIM2_CH1,  (PIN_ATTR_ADC|PIN_ATTR_PWM|PIN_ATTR_EXTI|PIN_ATTR_WKUP1), PWM_INSTANCE_TIM2, PWM_CHANNEL_1, ADC_INPUT_5 },

  // 28, Touch Cap
  { GPIOB, GPIO_PIN_MASK(GPIO_PIN_PB12), GPIO_PIN_PB12,            (PIN_ATTR_EXTI),                                 PWM_INSTANCE_NONE,  PWM_CHANNEL_NONE, ADC_INPUT_NONE },
  
  // 29, vtest
  { GPIOC, GPIO_PIN_MASK(GPIO_PIN_PC4),  GPIO_PIN_PC4,            (PIN_ATTR_ADC | PIN_ATTR_EXTI),                   PWM_INSTANCE_NONE,  PWM_CHANNEL_NONE, ADC_INPUT_13 },

  // 30, Motion Interrupt
  { GPIOC, GPIO_PIN_MASK(GPIO_PIN_PC13), GPIO_PIN_PC13,           (PIN_ATTR_EXTI | PIN_ATTR_WKUP2),                 PWM_INSTANCE_NONE,  PWM_CHANNEL_NONE, ADC_INPUT_NONE },

  // 31, Booster Enable
  { GPIOH, GPIO_PIN_MASK(GPIO_PIN_PH0),  GPIO_PIN_PH0,            (PIN_ATTR_EXTI),                                  PWM_INSTANCE_NONE,  PWM_CHANNEL_NONE, ADC_INPUT_NONE },
  // 32, Amplifier Enable
  { GPIOH, GPIO_PIN_MASK(GPIO_PIN_PH1),  GPIO_PIN_PH1,             0,                                               PWM_INSTANCE_NONE,  PWM_CHANNEL_NONE, ADC_INPUT_NONE },
  // 33, BOOT button
  { GPIOH, GPIO_PIN_MASK(GPIO_PIN_PH3),  GPIO_PIN_PH3,             0,                                               PWM_INSTANCE_NONE,  PWM_CHANNEL_NONE, ADC_INPUT_NONE },
};

stm32l4_ct_assert(STM32L4_NELEM(g_APinDescription) ==  NUM_TOTAL_PINS);

extern const unsigned int g_PWMInstances[] = {
  TIMER_INSTANCE_TIM1,  // WS2811
  TIMER_INSTANCE_TIM2,  // Flex (PWM/Savi/Servo)
  TIMER_INSTANCE_TIM3,  // PWM
  TIMER_INSTANCE_TIM15, // PWM
  TIMER_INSTANCE_TIM16, // PWM/IR
};

stm32l4_ct_assert(STM32L4_NELEM(g_PWMInstances) ==  PWM_INSTANCE_COUNT);

// overlaps with Data2/3
extern const stm32l4_uart_pins_t g_Serial1Pins = { GPIO_PIN_PA10_USART1_RX, GPIO_PIN_PA9_USART1_TX, GPIO_PIN_NONE, GPIO_PIN_NONE };
extern const unsigned int g_Serial1Instance = UART_INSTANCE_USART1;
extern const unsigned int g_Serial1Mode = 0;

// overlaps with Free2/Free3
extern const stm32l4_uart_pins_t g_Serial2Pins = { GPIO_PIN_PB11_USART3_RX, GPIO_PIN_PB10_USART3_TX, GPIO_PIN_NONE, GPIO_PIN_NONE };
extern const unsigned int g_Serial2Instance = UART_INSTANCE_USART3;
extern const unsigned int g_Serial2Mode = 0;

// Main serial port. (Button2 could potentially be used for CTS)
extern const stm32l4_uart_pins_t g_Serial3Pins = { GPIO_PIN_PC0_LPUART1_RX, GPIO_PIN_PC1_LPUART1_TX, GPIO_PIN_NONE, GPIO_PIN_NONE };
extern const unsigned int g_Serial3Instance = UART_INSTANCE_LPUART1;
extern const unsigned int g_Serial3Mode = 0;

// SPI1, overlaps with data2/3 & Free1
extern const stm32l4_spi_pins_t g_SPIPins = { GPIO_PIN_PB5_SPI1_MOSI, GPIO_PIN_PB4_SPI1_MISO, GPIO_PIN_PB3_SPI1_SCK, GPIO_PIN_NONE };
extern const unsigned int g_SPIInstance = SPI_INSTANCE_SPI1;
extern const unsigned int g_SPIMode = SPI_MODE_RX_DMA | SPI_MODE_TX_DMA | SPI_MODE_RX_DMA_SECONDARY | SPI_MODE_TX_DMA_SECONDARY; // TODO: Check best DMA setup

// SPI2, overlaps with Free2, Free3 & Button3 (AUX2)
extern const stm32l4_spi_pins_t g_SPI1Pins = { GPIO_PIN_PB15_SPI2_MOSI, GPIO_PIN_PC2_SPI2_MISO, GPIO_PIN_PB10_SPI2_SCK, GPIO_PIN_NONE };
extern const unsigned int g_SPI1Instance = SPI_INSTANCE_SPI2;
extern const unsigned int g_SPI1Mode = 0;

// SPI3, same pins as SPI1
extern const stm32l4_spi_pins_t g_SPI2Pins = { GPIO_PIN_PB5_SPI3_MOSI, GPIO_PIN_PB4_SPI3_MISO, GPIO_PIN_PB3_SPI3_SCK, GPIO_PIN_NONE };
extern const unsigned int g_SPI2Instance = SPI_INSTANCE_SPI3;
extern const unsigned int g_SPI2Mode = SPI_MODE_RX_DMA | SPI_MODE_TX_DMA | SPI_MODE_RX_DMA_SECONDARY | SPI_MODE_TX_DMA_SECONDARY;

// Main I2C
extern const stm32l4_i2c_pins_t g_WirePins = { GPIO_PIN_PB8_I2C1_SCL, GPIO_PIN_PB7_I2C1_SDA };
extern const unsigned int g_WireInstance = I2C_INSTANCE_I2C1;
extern const unsigned int g_WireMode = I2C_MODE_RX_DMA | I2C_MODE_TX_DMA | I2C_MODE_RX_DMA_SECONDARY | I2C_MODE_TX_DMA_SECONDARY;

// Overlaps with Button1 & Button2
extern const stm32l4_i2c_pins_t g_Wire1Pins = { GPIO_PIN_PB13_I2C2_SCL, GPIO_PIN_PB14_I2C2_SDA };
extern const unsigned int g_Wire1Instance = I2C_INSTANCE_I2C2;
extern const unsigned int g_Wire1Mode = 0;

// Overlaps with RX/TX
extern const stm32l4_i2c_pins_t g_Wire2Pins = { GPIO_PIN_PC0_I2C3_SCL, GPIO_PIN_PC1_I2C3_SDA };
extern const unsigned int g_Wire2Instance = I2C_INSTANCE_I2C3;
extern const unsigned int g_Wire2Mode = 0;

// Overlaps with Free2/3
extern const stm32l4_i2c_pins_t g_Wire3Pins = { GPIO_PIN_PB10_I2C2_SCL, GPIO_PIN_PB11_I2C2_SDA };
extern const unsigned int g_Wire3Instance = I2C_INSTANCE_I2C4;
extern const unsigned int g_Wire3Mode = 0;

// I2S A
extern const stm32l4_sai_pins_t g_SAIPins = { GPIO_PIN_PA8_SAI1_SCK_A, GPIO_PIN_PB9_SAI1_FS_A, GPIO_PIN_PC3_SAI1_SD_A, GPIO_PIN_PB14_SAI1_MCLK_A };
extern const unsigned int g_SAIInstance = SAI_INSTANCE_SAI1A;
extern const unsigned int g_SAIMode = SAI_MODE_DMA | SAI_MODE_DMA_SECONDARY;

// I2S B, Overlaps with Free1 and Data2/4
extern const stm32l4_sai_pins_t g_SAI1Pins = { GPIO_PIN_PB3_SAI1_SCK_B, GPIO_PIN_PA4_SAI1_FS_B, GPIO_PIN_PB5_SAI1_SD_B, GPIO_PIN_PB4_SAI1_MCLK_B };
extern const unsigned int g_SAI1Instance = SAI_INSTANCE_SAI1B;
extern const unsigned int g_SAI1Mode = SAI_MODE_DMA | SAI_MODE_DMA_SECONDARY;

