#include <stdint.h>
#include <TouchScreen.h>
#include <TFT.h>
 
//Measured ADC values for (0,0) and (240-1,320-1)
//TS_MINX corresponds to ADC value when X = 0
//TS_MINY corresponds to ADC value when Y = 0
//TS_MAXX corresponds to ADC value when X = 240 -1
//TS_MAXY corresponds to ADC value when Y = 320 -1
 
static unsigned int TS_MINX, TS_MAXX, TS_MINY, TS_MAXY;
 
//Touch Screen Co-ordinate mapping register
static unsigned int MapX1, MapX2, MapY1, MapY2;
 
// For better pressure precision, we need to know the resistance
// between X+ and X- Use any multimeter to read it
// The 2.8" TFT Touch shield has 300 ohms across the X plate
 
/* Usage: TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300); 
 Where, XP = X plus, YP = Y plus, XM = X minus and YM = Y minus */
//init TouchScreen port pins. This would be reinitialized in setup() based on the hardware detected.
TouchScreen ts = TouchScreen(57, 54, A1, A2, 300); 
 
void setup(void) 
{
  Serial.begin(9600);
  Tft.init();//init TFT
  initTouchScreenParameters(); //initializes Touch Screen parameters based on the detected TFT Touch Schield hardware
  Tft.drawChar('+', 0, 0, 1, WHITE);
  Tft.drawChar('x', MAX_X-10, 0, 1, GREEN);
  Tft.drawChar('y', 0, MAX_Y-10, 1, BLUE);
}

void loop(void) 
{
  // a point object holds x y and z coordinates
  Point p = ts.getPoint();
 
  p.x = map(p.x, TS_MINX, TS_MAXX, MapX1, MapX2);
  p.y = map(p.y, TS_MINY, TS_MAXY, MapY1, MapY2);
 
 
  // we have some minimum pressure we consider 'valid'
  // pressure of 0 means no pressing!
  if (p.z > ts.pressureThreshhold) {
    Tft.drawCircle(p.x, p.y, 1, WHITE);
  }
}
 
#define __AVR_ATmega2560__
 
void initTouchScreenParameters()
{
  //This function initializes Touch Screen parameters based on the detected TFT Touch Schield hardware

    //Touchscreen parameters for this hardware
    TS_MINX = 120;
    TS_MAXX = 910;
    TS_MINY = 120;
    TS_MAXY = 950;
 
    MapX1 = 239;
    MapX2 = 0;
    MapY1 = 0;
    MapY2 = 319;
}
