/*
    Copyright (C) 2017 Alexey Dynda

    This file is part of SSD1306 library.

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
/**
 *   Attiny85 PINS
 *             ____
 *   RESET   -|_|  |- 3V
 *   SCL (3) -|    |- (2)
 *   SDA (4) -|    |- (1)
 *   GND     -|____|- (0)
 *
 *   Atmega328 PINS: connect LCD to A4/A5
 */

#include "ssd1306.h"
#include "ssd1306_i2c.h"
#include "nano_gfx.h"

/* Do not include wire.h for Attiny controllers */
#ifndef SSD1306_EMBEDDED_I2C
    #include <Wire.h>
#endif

/* 
 * Heart image below is defined directly in flash memory.
 * This reduces SRAM consumption.
 * The image is define from bottom to top (bits), from left to
 * right (bytes).
 */
const PROGMEM uint8_t heartImage[8] =
{
    B00001110,
    B00011111,
    B00111111,
    B01111110,
    B01111110,
    B00111101,
    B00011001,
    B00001110
};

/*
 * Define sprite width. The width can be of any size.
 * But sprite height is always assumed to be 8 pixels
 * (number of bits in single byte).
 */
const int spriteWidth = sizeof(heartImage);

/* Lets show 4 hearts on the display */
const int spritesCount = 4;

/* Declare variable that represents our 4 objects */
struct
{
    SPRITE sprite;
    int8_t speedX;
    int8_t speedY;
} objects[ spritesCount ];

/*
 * Each pixel in SSD1306 display takes 1 bit of the memory. So, full resolution
 * of 128x64 LCD display will require 128*64/8 = 1024 bytes of SRAM for the buffer.
 * To let this example to run on Attiny devices (they have 256/512 byte SRAM), we
 * will use small canvas buffer: 32x32 (requires 128 bytes of SRAM), so the example
 * would run even on Attiny45.
 */
const int canvasWidth = 32; // Width must be power of 2, i.e. 16, 32, 64, 128...
const int canvasHeight = 32; // Height must be divided on 8, i.e. 8, 16, 24, 32...
uint8_t canvasData[canvasWidth*(canvasHeight/8)];
/* Create canvas object */
NanoCanvas canvas(canvasWidth, canvasHeight, canvasData);

void setup()
{
    /* Do not init Wire library for Attiny controllers */
#ifndef SSD1306_EMBEDDED_I2C
    Wire.begin();
#endif
    /* Initialize and clear display */
    /* Replace the line below with ssd1306_128x32_i2c_init() if you need to use 128x32 display */
    ssd1306_128x64_i2c_init();
    ssd1306_fillScreen(0x00);
    /* Create 4 "hearts", and place them at different positions and give different movement direction */
    for(uint8_t i = 0; i < spritesCount; i++)
    {
        objects[i].speedX = (i & 1) ? -1:  1;
        objects[i].speedY = (i & 2) ? -1:  1;
        objects[i].sprite = ssd1306_createSprite( i*4, 2 + i*4, spriteWidth, heartImage );
    }
}


void loop()
{
    delay(40);

    /* Recalculate position and movement direction of all 4 "hearts" */
    for (uint8_t i = 0; i < spritesCount; i++)
    {
        objects[i].sprite.x += objects[i].speedX;
        objects[i].sprite.y += objects[i].speedY;
        /* If right boundary is reached, reverse X direction */
        if (objects[i].sprite.x == (canvasWidth - spriteWidth)) objects[i].speedX = -objects[i].speedX;
        /* If left boundary is reached, reverse X direction */ 
        if (objects[i].sprite.x == 0) objects[i].speedX = -objects[i].speedX;
        /* Sprite height is always 8 pixels. Reverse Y direction if bottom boundary is reached. */
        if (objects[i].sprite.y == (canvasHeight - 8)) objects[i].speedY = -objects[i].speedY;
        /* If top boundary is reached, reverse Y direction */
        if (objects[i].sprite.y == 0) objects[i].speedY = -objects[i].speedY;
    }

    /* Clear canvas surface */
    canvas.clear();
    /* Draw rectangle around our canvas. It will show the range of the canvas on the display */
    canvas.drawRect(0, 0, canvasWidth-1, canvasHeight-1);
    /* Draw all 4 sprites on the canvas */
    for (uint8_t i = 0; i < spritesCount; i++)
    {
        canvas.drawSprite( &objects[i].sprite );
    }
    /* Now, draw canvas on the display */
    ssd1306_drawCanvas(48, 0, canvasWidth, canvasHeight, canvasData);
}
