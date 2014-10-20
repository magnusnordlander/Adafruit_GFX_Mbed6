/*********************************************************************
This is a library for our Monochrome OLEDs based on SSD1306 drivers

  Pick one up today in the adafruit shop!
  ------> http://www.adafruit.com/category/63_98

These displays use SPI to communicate, 4 or 5 pins are required to  
interface

Adafruit invests time and resources providing this open source code, 
please support Adafruit and open-source hardware by purchasing 
products from Adafruit!

Written by Limor Fried/Ladyada  for Adafruit Industries.  
BSD license, check license.txt for more information
All text above, and the splash screen must be included in any redistribution
*********************************************************************/

/*
 *  Modified by Neal Horman 7/14/2012 for use in mbed
 */

#ifndef _ADAFRUIT_SSD1306_H_
#define _ADAFRUIT_SSD1306_H_

#include "mbed.h"
#include "Adafruit_GFX.h"

#include <vector>
#include <algorithm>

// A DigitalOut sub-class that provides a constructed default state
class DigitalOut2 : public DigitalOut
{
public:
	DigitalOut2(PinName pin, bool active = false) : DigitalOut(pin) { write(active); };
	DigitalOut2& operator= (int value) { write(value); return *this; };
	DigitalOut2& operator= (DigitalOut2& rhs) { write(rhs.read()); return *this; };
	operator int() { return read(); };
};

#define SSD1306_EXTERNALVCC 0x1
#define SSD1306_SWITCHCAPVCC 0x2

class Adafruit_SSD1306 : public Adafruit_GFX
{
public:
	Adafruit_SSD1306(PinName RST, uint8_t rawHeight = 32, uint8_t rawWidth = 128)
		: Adafruit_GFX(rawWidth,rawHeight)
		, rst(RST,false)
	{
		buffer.resize(rawHeight * rawWidth / 8);
	};

	void begin(uint8_t switchvcc = SSD1306_SWITCHCAPVCC);
	virtual void command(uint8_t c) = 0;
	virtual void data(uint8_t c) = 0;
    
	void clearDisplay(void);
	virtual void invertDisplay(bool i);
	void display();
	virtual void splash();
    
	virtual void drawPixel(int16_t x, int16_t y, uint16_t color);
    
protected:
	virtual void sendDisplayBuffer() = 0;
	DigitalOut2 rst;

	// the memory buffer for the LCD
	std::vector<uint8_t> buffer;
};

class Adafruit_SSD1306_Spi : public Adafruit_SSD1306
{
public:
	Adafruit_SSD1306_Spi(SPI &spi, PinName DC, PinName RST, PinName CS, uint8_t rawHieght = 32, uint8_t rawWidth = 128)
	    : Adafruit_SSD1306(RST, rawHieght, rawWidth)
	    , cs(CS,true)
	    , dc(DC,false)
	    , mspi(spi)
	    {
		    begin();
		    splash();
		    display();
	    };

	virtual void command(uint8_t c)
	{
	    cs = 1;
	    dc = 0;
	    cs = 0;
	    mspi.write(c);
	    cs = 1;
	};

	virtual void data(uint8_t c)
	{
	    cs = 1;
	    dc = 1;
	    cs = 0;
	    mspi.write(c);
	    cs = 1;
	};

protected:
	virtual void sendDisplayBuffer()
	{
		cs = 1;
		dc = 1;
		cs = 0;

		for(uint16_t i=0, q=buffer.size(); i<q; i++)
			mspi.write(buffer[i]);

		cs = 1;
	};

	DigitalOut2 cs, dc;
	SPI &mspi;
};

class Adafruit_SSD1306_I2c : public Adafruit_SSD1306
{
public:
	#define SSD_I2C_ADDRESS     0x78
	Adafruit_SSD1306_I2c(I2C &i2c, PinName RST, uint8_t i2cAddress = SSD_I2C_ADDRESS, uint8_t rawHeight = 32, uint8_t rawWidth = 128)
	    : Adafruit_SSD1306(RST, rawHeight, rawWidth)
	    , mi2c(i2c)
	    , mi2cAddress(i2cAddress)
	    {
		    begin();
		    splash();
		    display();
	    };

	virtual void command(uint8_t c)
	{
		char buff[2];
		buff[0] = 0; // Command Mode
		buff[1] = c;
		mi2c.write(mi2cAddress, buff, sizeof(buff));
	}

	virtual void data(uint8_t c)
	{
		char buff[2];
		buff[0] = 0x40; // Data Mode
		buff[1] = c;
		mi2c.write(mi2cAddress, buff, sizeof(buff));
	};

protected:
	virtual void sendDisplayBuffer()
	{
		char buff[17];
		buff[0] = 0x40; // Data Mode

		// send display buffer in 16 byte chunks
		for(uint16_t i=0, q=buffer.size(); i<q; i+=16 ) 
		{	uint8_t x ;

			// TODO - this will segfault if buffer.size() % 16 != 0
			for(x=1; x<sizeof(buff); x++) 
				buff[x] = buffer[i+x];
			mi2c.write(mi2cAddress, buff, sizeof(buff));
		}
	};

	I2C &mi2c;
	uint8_t mi2cAddress;
};

#endif