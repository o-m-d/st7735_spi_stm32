#include "st7735.h"

uint16_t scr_width;
uint16_t scr_height;

void ST7735_write(uint8_t data)
{
#if _SPI_TYPE == SOFT
  uint8_t i;

  for (i = 0; i < 8; i++)
  {
    if (data & 0x80) SDA_H(); else SDA_L();
    data = data << 1;
    SCK_L();
    SCK_H();
  }
#else
  while (SPI_I2S_GetFlagStatus(SPI_PORT,SPI_I2S_FLAG_TXE) == RESET);
  SPI_I2S_SendData(SPI_PORT,data);
#endif
}

void ST7735_cmd(uint8_t cmd)
{
  A0_L();
  ST7735_write(cmd);
#if _SPI_TYPE != SOFT
  while (SPI_I2S_GetFlagStatus(SPI_PORT,SPI_I2S_FLAG_BSY) == SET);
#endif
}

void ST7735_data(uint8_t data)
{
  A0_H();
  ST7735_write(data);
#if _SPI_TYPE != SOFT
  while (SPI_I2S_GetFlagStatus(SPI_PORT,SPI_I2S_FLAG_BSY) == SET);
#endif
}

uint16_t RGB565(uint8_t R,uint8_t G,uint8_t B)
{
  return ((R >> 3) << 11) | ((G >> 2) << 5) | (B >> 3);
}

void ST7735_Init(void) {
#if _SPI_TYPE == SOFT
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO,ENABLE);
  GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE); // Disable JTAG for use PB3
#elif _SPI_TYPE == 1
  // SPI1
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1 | RCC_APB2Periph_GPIOA,ENABLE);
#elif _SPI_TYPE == 2
  // SPI2
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO,ENABLE);
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2,ENABLE);
#elif _SPI_TYPE == 3
  // SPI3
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO,ENABLE);
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI3,ENABLE);
  GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE); // Disable JTAG for use PB3
#endif

#if _SPI_TYPE != SOFT
  // Configure and enable SPI
  SPI_InitTypeDef SPI;
  SPI.SPI_Mode = SPI_Mode_Master;
  SPI.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_2;
  SPI.SPI_Direction = SPI_Direction_1Line_Tx;
  SPI.SPI_CPOL = SPI_CPOL_Low;
  SPI.SPI_CPHA = SPI_CPHA_1Edge;
  SPI.SPI_CRCPolynomial = 7;
  SPI.SPI_DataSize = SPI_DataSize_8b;
  SPI.SPI_FirstBit = SPI_FirstBit_MSB;
  SPI.SPI_NSS = SPI_NSS_Soft;
  SPI_Init(SPI_PORT,&SPI);
  // NSS must be set to '1' due to NSS_Soft settings (otherwise it will be Multimaster mode).
  SPI_NSSInternalSoftwareConfig(SPI_PORT,SPI_NSSInternalSoft_Set);
  SPI_Cmd(SPI_PORT,ENABLE);
#endif

  GPIO_InitTypeDef PORT;
  PORT.GPIO_Mode = GPIO_Mode_Out_PP;
  PORT.GPIO_Speed = GPIO_Speed_50MHz;
#if _SPI_TYPE == SOFT
  PORT.GPIO_Pin = ST7735_SDA_PIN;
  GPIO_Init(ST7735_SDA_PORT,&PORT);
  PORT.GPIO_Pin = ST7735_SCK_PIN;
  GPIO_Init(ST7735_SCK_PORT,&PORT);
#else
  // Configure SPI pins
  PORT.GPIO_Pin = SPI_SCK_PIN | SPI_MOSI_PIN;
  PORT.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_Init(SPI_GPIO_PORT,&PORT);
  PORT.GPIO_Mode = GPIO_Mode_Out_PP;
#endif

  RCC_APB2PeriphClockCmd(ST7735_A0_PORT_CLK | ST7735_RST_PORT_CLK | ST7735_CS_PORT_CLK , ENABLE);
  PORT.GPIO_Pin = ST7735_CS_PIN;
  GPIO_Init(ST7735_CS_PORT,&PORT);
  PORT.GPIO_Pin = ST7735_RST_PIN;
  GPIO_Init(ST7735_RST_PORT,&PORT);
  PORT.GPIO_Pin = ST7735_A0_PIN;
  GPIO_Init(ST7735_A0_PORT,&PORT);

  // Reset display
  CS_H();
  RST_H();
  Delay_US(5);
  RST_L();
  Delay_US(5);
  RST_H();
  CS_H();
  Delay_US(5);
  CS_L();

  ST7735_cmd(0x11);     // Sleep out, booster on
  Delay_US(20);

  ST7735_cmd(0xb1);     // In normal mode (full colors):
  A0_H();
  ST7735_write(0x05);   //   RTNA set 1-line period: RTNA2, RTNA0
  ST7735_write(0x3c);   //   Front porch: FPA5,FPA4,FPA3,FPA2
  ST7735_write(0x3c);   //   Back porch: BPA5,BPA4,BPA3,BPA2

  ST7735_cmd(0xb2);     // In idle mode (8-colors):
  A0_H();
  ST7735_write(0x05);   //   RTNB set 1-line period: RTNAB, RTNB0
  ST7735_write(0x3c);   //   Front porch: FPB5,FPB4,FPB3,FPB2
  ST7735_write(0x3c);   //   Back porch: BPB5,BPB4,BPB3,BPB2

  ST7735_cmd(0xb3);     // In partial mode + full colors:
  A0_H();
  ST7735_write(0x05);   //   RTNC set 1-line period: RTNC2, RTNC0
  ST7735_write(0x3c);   //   Front porch: FPC5,FPC4,FPC3,FPC2
  ST7735_write(0x3c);   //   Back porch: BPC5,BPC4,BPC3,BPC2
  ST7735_write(0x05);   //   RTND set 1-line period: RTND2, RTND0
  ST7735_write(0x3c);   //   Front porch: FPD5,FPD4,FPD3,FPD2
  ST7735_write(0x3c);   //   Back porch: BPD5,BPD4,BPD3,BPD2

  ST7735_cmd(0xB4);     // Display dot inversion control:
  ST7735_data(0x03);    //   NLB,NLC

  ST7735_cmd(0x3a);     // Interface pixel format
//ST7735_data(0x03);    // 12-bit/pixel RGB 4-4-4 (4k colors)
  ST7735_data(0x05);    // 16-bit/pixel RGB 5-6-5 (65k colors)
//ST7735_data(0x06);    // 18-bit/pixel RGB 6-6-6 (256k colors)

//ST7735_cmd(0x36);     // Memory data access control:
                        // MY MX MV ML RGB MH - -
//ST7735_data(0x00);    // Normal: Top to Bottom; Left to Right; RGB
//ST7735_data(0x80);    // Y-Mirror: Bottom to top; Left to Right; RGB
//ST7735_data(0x40);    // X-Mirror: Top to Bottom; Right to Left; RGB
//ST7735_data(0xc0);    // X-Mirror,Y-Mirror: Bottom to top; Right to left; RGB
//ST7735_data(0x20);    // X-Y Exchange: X and Y changed positions
//ST7735_data(0xA0);    // X-Y Exchange,Y-Mirror
//ST7735_data(0x60);    // X-Y Exchange,X-Mirror
//ST7735_data(0xE0);    // X-Y Exchange,X-Mirror,Y-Mirror

  ST7735_cmd(0x20);     // Display inversion off
//ST7735_cmd(0x21);     // Display inversion on

  ST7735_cmd(0x13);     // Partial mode off

  ST7735_cmd(0x26);     // Gamma curve set:
  ST7735_data(0x01);    // Gamma curve 1 (G2.2) or (G1.0)
//ST7735_data(0x02);    // Gamma curve 2 (G1.8) or (G2.5)
//ST7735_data(0x04);    // Gamma curve 3 (G2.5) or (G2.2)
//ST7735_data(0x08);    // Gamma curve 4 (G1.0) or (G1.8)

  ST7735_cmd(0x29);     // Display on

  CS_H();

  ST7735_Orientation(scr_normal);
}

void ST7735_Orientation(ScrOrientation_TypeDef orientation)
{
  CS_L();
  ST7735_cmd(0x36); // Memory data access control:
  switch (orientation)
  {
  case scr_CW:
    scr_width  = scr_h;
    scr_height = scr_w;
    ST7735_data(0xA0); // X-Y Exchange,Y-Mirror
    break;
  case scr_CCW:
    scr_width  = scr_h;
    scr_height = scr_w;
    ST7735_data(0x60); // X-Y Exchange,X-Mirror
    break;
  case scr_180:
    scr_width  = scr_w;
    scr_height = scr_h;
    ST7735_data(0xc0); // X-Mirror,Y-Mirror: Bottom to top; Right to left; RGB
    break;
  default:
    scr_width  = scr_w;
    scr_height = scr_h;
    ST7735_data(0x00); // Normal: Top to Bottom; Left to Right; RGB
    break;
  }
  CS_H();
}

void ST7735_AddrSet(uint16_t XS, uint16_t YS, uint16_t XE, uint16_t YE)
{
  ST7735_cmd(0x2a); // Column address set
  A0_H();
  ST7735_write(XS >> 8);
  ST7735_write(XS);
  ST7735_write(XE >> 8);
  ST7735_write(XE);

  ST7735_cmd(0x2b); // Row address set
  A0_H();
  ST7735_write(YS >> 8);
  ST7735_write(YS);
  ST7735_write(YE >> 8);
  ST7735_write(YE);

  ST7735_cmd(0x2c); // Memory write
}

void ST7735_Clear(uint16_t color)
{
  uint16_t i;
  uint8_t  CH,CL;

  CH = color >> 8;
  CL = (uint8_t)color;

  CS_L();
  ST7735_AddrSet(0,0,scr_width - 1,scr_height - 1);
  A0_H();
  for (i = 0; i < scr_width * scr_height; i++)
  {
    ST7735_write(CH);
    ST7735_write(CL);
  }
  CS_H();
}

void ST7735_Pixel(uint16_t X, uint16_t Y, uint16_t color)
{
  CS_L();
  ST7735_AddrSet(X,Y,X,Y);
  A0_H();
  ST7735_write(color >> 8);
  ST7735_write((uint8_t)color);
  CS_H();
}

void ST7735_HLine(uint16_t X1, uint16_t X2, uint16_t Y, uint16_t color)
{
  uint16_t i;
  uint8_t CH = color >> 8;
  uint8_t CL = (uint8_t)color;

  CS_L();
  ST7735_AddrSet(X1,Y,X2,Y);
  A0_H();
  for (i = 0; i <= (X2 - X1); i++)
  {
    ST7735_write(CH);
    ST7735_write(CL);
  }
  CS_H();
}

void ST7735_VLine(uint16_t X, uint16_t Y1, uint16_t Y2, uint16_t color)
{
  uint16_t i;
  uint8_t CH = color >> 8;
  uint8_t CL = (uint8_t)color;

  CS_L();
  ST7735_AddrSet(X,Y1,X,Y2);
  A0_H();
  for (i = 0; i <= (Y2 - Y1); i++)
  {
    ST7735_write(CH);
    ST7735_write(CL);
  }
  CS_H();
}

void ST7735_Line(int16_t X1, int16_t Y1, int16_t X2, int16_t Y2, uint16_t color)
{
  int16_t dX = X2-X1;
  int16_t dY = Y2-Y1;
  int16_t dXsym = (dX > 0) ? 1 : -1;
  int16_t dYsym = (dY > 0) ? 1 : -1;

  if (dX == 0)
  {
    if (Y2>Y1) ST7735_VLine(X1,Y1,Y2,color); else ST7735_VLine(X1,Y2,Y1,color);
    return;
  }
  if (dY == 0)
  {
    if (X2>X1) ST7735_HLine(X1,X2,Y1,color); else ST7735_HLine(X2,X1,Y1,color);
    return;
  }

  dX *= dXsym;
  dY *= dYsym;
  int16_t dX2 = dX << 1;
  int16_t dY2 = dY << 1;
  int16_t di;

  if (dX >= dY)
  {
    di = dY2 - dX;
    while (X1 != X2)
    {
      ST7735_Pixel(X1,Y1,color);
      X1 += dXsym;
      if (di < 0)
      {
        di += dY2;
      }
      else
      {
        di += dY2 - dX2;
        Y1 += dYsym;
      }
    }
  }
  else
  {
    di = dX2 - dY;
    while (Y1 != Y2)
    {
      ST7735_Pixel(X1,Y1,color);
      Y1 += dYsym;
      if (di < 0)
      {
        di += dX2;
      }
      else
      {
        di += dX2 - dY2;
        X1 += dXsym;
      }
    }
  }
  ST7735_Pixel(X1,Y1,color);
}

void ST7735_Rect(uint16_t X1, uint16_t Y1, uint16_t X2, uint16_t Y2, uint16_t color)
{
  ST7735_HLine(X1,X2,Y1,color);
  ST7735_HLine(X1,X2,Y2,color);
  ST7735_VLine(X1,Y1,Y2,color);
  ST7735_VLine(X2,Y1,Y2,color);
}

void ST7735_FillRect(uint16_t X1, uint16_t Y1, uint16_t X2, uint16_t Y2, uint16_t color)
{
  uint16_t i;
  uint16_t FS = (X2 - X1 + 1) * (Y2 - Y1 + 1);
  uint8_t CH = color >> 8;
  uint8_t CL = (uint8_t)color;

  CS_L();
  ST7735_AddrSet(X1,Y1,X2,Y2);
  A0_H();
  for (i = 0; i < FS; i++)
  {
    ST7735_write(CH);
    ST7735_write(CL);
  }
  CS_H();
}

void ST7735_PutChar5x7(uint8_t scale, uint16_t X, uint16_t Y, uint8_t chr, uint16_t color, uint16_t bgcolor)
{
  uint16_t i,j;
  uint8_t buffer[5];
  uint8_t CH = color >> 8;
  uint8_t CL = (uint8_t)color;
  uint8_t BCH = bgcolor >> 8;
  uint8_t BCL = (uint8_t)bgcolor;

  if ((chr >= 0x20) && (chr <= 0x7F))
  {
    // ASCII[0x20-0x7F]
    memcpy(buffer,&Font5x7[(chr - 32) * 5], 5);
  }
  else if (chr >= 0xA0)
  {
    // CP1251[0xA0-0xFF]
    memcpy(buffer,&Font5x7[(chr - 64) * 5], 5);
  }
  else
  {
    // unsupported symbol
    memcpy(buffer,&Font5x7[160], 5);
  }

  CS_L();

  // scale equals 1 drawing faster
  if (scale == 1)
  {
    ST7735_AddrSet(X, Y, X + 5, Y + 7);
    A0_H();
    for (j = 0; j < 7; j++)
    {
      for (i = 0; i < 5; i++)
      {
        if ((buffer[i] >> j) & 0x01)
        {
          ST7735_write(CH);
          ST7735_write(CL);
        }
        else
        {
          ST7735_write(BCH);
          ST7735_write(BCL);
        }
      }
      // vertical spacing
      ST7735_write(BCH);
      ST7735_write(BCL);
    }

    // horizontal spacing
    for (i = 0; i < 6; i++)
    {
      ST7735_write(BCH);
      ST7735_write(BCL);
    }
  }
  else
  {
    A0_H();
    for (j = 0; j < 7; j++)
    {
      for (i = 0; i < 5; i++)
      {
        // pixel group
        ST7735_FillRect(X + (i * scale), Y + (j * scale), X + (i * scale) + scale - 1, Y + (j * scale) + scale - 1, ((buffer[i] >> j) & 0x01) ? color : bgcolor);
      }
      // vertical spacing
//      ST7735_FillRect(X + (i * scale), Y + (j * scale), X + (i * scale) + scale - 1, Y + (j * scale) + scale - 1, V_SEP);
      ST7735_FillRect(X + (i * scale), Y + (j * scale), X + (i * scale) + scale - 1, Y + (j * scale) + scale - 1, bgcolor);
    }
    // horizontal spacing
//    ST7735_FillRect(X, Y + (j * scale), X + (i * scale) + scale - 1, Y + (j * scale) + scale - 1, H_SEP);
    ST7735_FillRect(X, Y + (j * scale), X + (i * scale) + scale - 1, Y + (j * scale) + scale - 1, bgcolor);
  }
  CS_H();
}

void ST7735_PutStr5x7(uint8_t scale, uint8_t X, uint8_t Y, char *str, uint16_t color, uint16_t bgcolor)
{
  // scale equals 1 drawing faster
  if (scale == 1)
  {
    while (*str)
    {
      ST7735_PutChar5x7(scale, X,Y,*str++,color,bgcolor);
      if (X < scr_width - 6) { X += 6; } else if (Y < scr_height - 8) { X = 0; Y += 8; } else { X = 0; Y = 0; }
    };
  }
  else
  {
    while (*str)
    {
      ST7735_PutChar5x7(scale, X,Y,*str++,color,bgcolor);
      if (X < scr_width - (scale*5) + scale) { X += (scale * 5) + scale; } else if (Y < scr_height - (scale * 7) + scale) { X = 0; Y += (scale * 7) + scale; } else { X = 0; Y = 0; }
    };
  }
}

void ST7735_PutChar7x11(uint16_t X, uint16_t Y, uint8_t chr, uint16_t color, uint16_t bgcolor)
{
  uint16_t i,j;
  uint8_t buffer[11];
  uint8_t CH = color >> 8;
  uint8_t CL = (uint8_t)color;
  uint8_t BCH = bgcolor >> 8;
  uint8_t BCL = (uint8_t)bgcolor;

  if ((chr >= 0x20) && (chr <= 0x7F))
  {
    // ASCII[0x20-0x7F]
    memcpy(buffer,&Font7x11[(chr - 32) * 11], 11);
  }
  else if (chr >= 0xA0)
  {
    // CP1251[0xA0-0xFF]
    memcpy(buffer,&Font7x11[(chr - 64) * 11], 11);
  }
  else
  {
    // unsupported symbol
    memcpy(buffer,&Font7x11[160], 11);
  }

  CS_L();
  ST7735_AddrSet(X, Y, X + 7, Y + 11);
  A0_H();
  for (i = 0; i < 11; i++)
  {
    for (j = 0; j < 7; j++)
    {
      if ((buffer[i] >> j) & 0x01)
      {
        ST7735_write(CH);
        ST7735_write(CL);
      }
      else
      {
        ST7735_write(BCH);
        ST7735_write(BCL);
      }
    }
    // vertical spacing
    ST7735_write(BCH);
    ST7735_write(BCL);
  }

  // horizontal spacing
  for (i = 0; i < 8; i++)
  {
    ST7735_write(BCH);
    ST7735_write(BCL);
  }

  CS_H();
}

void ST7735_PutStr7x11(uint8_t X, uint8_t Y, char *str, uint16_t color, uint16_t bgcolor)
{
  while (*str)
  {
    ST7735_PutChar7x11(X,Y,*str++,color,bgcolor);
    if (X < scr_width - 8) { X += 8; } else if (Y < scr_height - 12) { X = 0; Y += 12; } else { X = 0; Y = 0; }
  };
}
