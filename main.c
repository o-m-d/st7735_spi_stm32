/*
 * STM32 driver for SPI ST7735
 *
 * Example usage with STM32F103C8T6
 *
 * ST7735 wiring example:
 * 1 VCC    - 3.3V Vcc
 * 2 GND    - GND
 * 3 CS     - PB5
 * 4 RESET  - PA12
 * 5 A0     - PA11
 * 6 SDA    - PB15 (SPI2 MOSI)
 * 7 SCK    - PB13 (SPI2 SCK)
 * 8 LED    - via resistor 68 Ohm to 3.3V Vcc
 *
 * Using hardware SPI2
 */

#include "stm32f10x.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"

#include "st7735.h"

static __IO uint32_t TimingDelay = 0;

GPIO_InitTypeDef  GPIO_InitStructure;

void SysTick_Handler(void);
void Delay_US(__IO uint32_t nTime);

int main(void)
{
  SysTick_Config(SystemCoreClock / 1000);

  // Configure all unused GPIO port pins in Analog Input mode
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOD | RCC_APB2Periph_GPIOE, ENABLE);
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_All; GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
  GPIO_Init(GPIOA, &GPIO_InitStructure); GPIO_Init(GPIOB, &GPIO_InitStructure); GPIO_Init(GPIOC, &GPIO_InitStructure); GPIO_Init(GPIOD, &GPIO_InitStructure); GPIO_Init(GPIOE, &GPIO_InitStructure);
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOD | RCC_APB2Periph_GPIOE, DISABLE);

  ST7735_Init();
  ST7735_AddrSet(0,0,159,127);
  ST7735_Clear(COLOR565_WHITE);
  ST7735_Orientation(scr_CCW);

  while(1)
  {
    ST7735_Clear(COLOR565_WHITE);
    ST7735_PutStr7x11(0,  0, " !\"#$%&'()*+,-./0123", COLOR565_RED, COLOR565_LIME);
    ST7735_PutStr7x11(0, 12, "456789:;<=>?@ABCDEFG", COLOR565_RED, COLOR565_LIME);
    ST7735_PutStr7x11(0, 24, "HIJKLMNOPQRSTUVWXYZ[", COLOR565_RED, COLOR565_LIME);
    ST7735_PutStr7x11(0, 36, "\\]^_`abcdefghijklmno", COLOR565_RED, COLOR565_LIME);
    ST7735_PutStr7x11(0, 48, "pqrstuvwxyz{|}~¡¢£¤", COLOR565_RED, COLOR565_LIME);
    ST7735_PutStr7x11(0, 60, "¥¦§¨©ª«¬­®¯°±²³´µ¶·¸", COLOR565_RED, COLOR565_LIME);
    ST7735_PutStr7x11(0, 72, "¹º»¼½¾¿ÀÁÂÃÄÅÆÇÈÉÊËÌ", COLOR565_RED, COLOR565_LIME);
    ST7735_PutStr7x11(0, 84, "ÍÎÏÐÑÒÓÔÕÖ×ØÙÚÛÜÝÞßà", COLOR565_RED, COLOR565_LIME);
    ST7735_PutStr7x11(0, 96, "áâãäåæçèéêëìíîïðñòóô", COLOR565_RED, COLOR565_LIME);
    ST7735_PutStr7x11(0, 108, "õö÷øùúûüýþÿ", COLOR565_RED, COLOR565_LIME);
    Delay_US(5000);
    ST7735_Clear(COLOR565_WHITE);
    ST7735_PutStr5x7(1, 0, 0, " !\"#$%&'()*+,-./0123456789", COLOR565_RED, COLOR565_LIME);
    ST7735_PutStr5x7(1, 0, 8, ":;<=>?@ABCDEFGHIJKLMNOPQRS", COLOR565_RED, COLOR565_LIME);
    ST7735_PutStr5x7(1, 0, 16, "TUVWXYZ[\\]^_`abcdefghijklm", COLOR565_RED, COLOR565_LIME);
    ST7735_PutStr5x7(1, 0, 24, "nopqrstuvwxyz{|}~¡¢£¤¥¦§¨", COLOR565_RED, COLOR565_LIME);
    ST7735_PutStr5x7(1, 0, 32, "©ª«¬­®¯°±²³´µ¶·¸¹º»¼½¾¿ÀÁÂ", COLOR565_RED, COLOR565_LIME);
    ST7735_PutStr5x7(1, 0, 40, "ÃÄÅÆÇÈÉÊËÌÍÎÏÐÑÒÓÔÕÖ×ØÙÚÛÜ", COLOR565_RED, COLOR565_LIME);
    ST7735_PutStr5x7(1, 0, 48, "ÝÞßàáâãäåæçèéêëìíîïðñòóôõö", COLOR565_RED, COLOR565_LIME);
    ST7735_PutStr5x7(1, 0, 56, "÷øùúûüýþÿ", COLOR565_RED, COLOR565_LIME);
    Delay_US(5000);
    ST7735_Clear(COLOR565_WHITE);
    ST7735_PutStr5x7(2, 0, 0, "2x scale", COLOR565_RED, COLOR565_LIME);
    ST7735_PutStr5x7(2, 0, 16, "5x7 font", COLOR565_RED, COLOR565_LIME);
    Delay_US(5000);
    ST7735_Clear(COLOR565_WHITE);
    ST7735_PutStr5x7(3, 0, 0, "3x scale", COLOR565_RED, COLOR565_LIME);
    ST7735_PutStr5x7(3, 0, 24, "5x7 font", COLOR565_RED, COLOR565_LIME);
    Delay_US(5000);
  }
}

void Delay_US(__IO uint32_t nTime)
{
  TimingDelay = nTime;
  while(TimingDelay != 0);
}

void SysTick_Handler()
{
  if (TimingDelay != 0) TimingDelay--;
}
