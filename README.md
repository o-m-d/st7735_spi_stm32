# STM32 library for SPI ST7735 LCD

Inspired by a (../../LonelyWolf/stm32/tree/master/ST7735)

Developed and tested on STM32F103C8T6 and 160x128 screen in RGB 565 mode.

[Demo video of supported fonts](https://youtu.be/faLqhA-05B0)

Library provides some graph functions and two fonts: 5x7 pixels with scale and 7x11 pixels without scale. Fonts support ASCII and CP1251 encodings.
Library need external delay function, usually defined in main.c .

Standard libraries like CMSIS are not included.