#include "SPI.h"
#include "Adafruit_WS2801.h"

//If defined, then the x,y coordinate space is compatiable with the ascii characters '0'..'9'
//to represent the indicies 0..9. Limits the matrix size to a max of 16 in either dimension. To
//disable, simply comment out.
#define ASCII_COMPAT

//If defined, assigned slowly changing colors to each enabled pixel. If disabled (by commenting
//out), uses white (still modified by brightness).
#define COLOR_ENABLED

//Width of the matrix in pixels
#define MATRIX_WIDTH 5

//Height of the matrix in pixels
#define MATRIX_HEIGHT 5

//How many ticks (delimeted by the command 3) for the hue to cycle
#define TICKS_PER_CYCLE 100.0

//LED matrix control and clock pin

#define DATAPIN 2
#define CLOCKPIN 3

//Brightness of the LEDs (in the range 0.0 .. 1.0)
#define LED_BRIGHTNESS 0.5

//Total number of pixels in the matrix (computed)
#define PIXELS ((MATRIX_WIDTH)*(MATRIX_HEIGHT))

//Library for NeoPixel
Adafruit_WS2801 strip = Adafruit_WS2801(PIXELS, DATAPIN, CLOCKPIN);

#ifdef COLOR_ENABLED
//Number of ticks since this GoL sequence began.
int time = 0;
#endif

void setup() {
  //Init LEDs
  strip.begin();
  strip.show();
  
  //Init Serial
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }
}

void loop() {
  if(Serial.available()) {
    switch(Serial.read()) { //read command
#ifdef ASCII_COMPAT
      case '0':
#endif
      case 0: { //clear LED display
        for(int i=0; i<(PIXELS); i++) {
          strip.setPixelColor(i, 0);
        }
        strip.show();

#ifdef COLOR_ENABLED
        time = 0;
#endif

        break;
      }
      
#ifdef ASCII_COMPAT
      case '1':
#endif
      case 1: { //enable pixel
        // wait for 2 bytes over the serial bus for coordinates
        while(Serial.available()<2);
        //read coordinates
        int x = Serial.read();
        int y = Serial.read();
#ifdef ASCII_COMPAT
        // only use the lower 4 bits (allows for compatiability with ASCII chars 0-9)
        x &= 0xF; y&= 0xF;
#endif
        //if valid coordinates, enable pixel
        if(x<(MATRIX_WIDTH) || y<(MATRIX_HEIGHT)) {
          strip.setPixelColor(coordToPixel(x,y), XYTimeColor(x,y));
        }
        break;
      }
      
#ifdef ASCII_COMPAT
      case '2':
#endif
      case 2: { //disable pixel
        // wait for 2 bytes over the serial bus for coordinates
        while(Serial.available()<2);
        //read coordinates
        int x = Serial.read();
        int y = Serial.read();
#ifdef ASCII_COMPAT
        // only use the lower 4 bits (allows for compatiability with ASCII chars 0-9)
        x &= 0xF; y&= 0xF;
#endif
        //if valid coordinates, clear pixel
        if(x<(MATRIX_WIDTH) || y<(MATRIX_HEIGHT)) {
          strip.setPixelColor(coordToPixel(x,y), 0);
        }
        break;
      }
      
#ifdef ASCII_COMPAT
      case '3':
#endif
      case 3: { //refresh screen
        strip.show();
        
#ifdef COLOR_ENABLED
        time++;
#endif
      }
      default:; //ignore any other byte
    }
  }
}

#ifdef COLOR_ENABLED
/*
Convert a hue from the HSL colour space to the RGB colour space
assuming that saturation is 100%. Brightness is specified by the 
define statement at the top.
*/
uint32_t hueToColor(int hue) {
  while(hue>=360) hue-=360;
  while(hue<0) hue+=360;
  
  float bright_f = 255.0 * (LED_BRIGHTNESS);
        
  uint32_t r, g, b;
  
  if (hue < 120) {
    float h = float(hue) / 120.0;
    r = bright_f * (1.0 - h);
    g = bright_f * h;
    b = 0;
  } else if (hue < 240) {
    float h = float(hue-120) / 120.0;
    r = 0;  
    g = bright_f * (1.0 - h);
    b = bright_f * h;
  } else { // hue>=240
    float h = float(hue-240) / 120.0;
    r = bright_f * h;
    g = 0;  
    b = bright_f * (1.0 - h);
  }

//  Serial.print(hue);Serial.print(" => ");Serial.print(r); Serial.print(','); Serial.print(g); Serial.print(','); Serial.println(b);
  
  return (r<<16) | (g<<8) | b;
}

// Generats a color for the pixel based on its mtrix position.
uint32_t XYTimeColor(int x, int y) {
  float _x = x/float(MATRIX_WIDTH);
  float _y = y/float(MATRIX_HEIGHT);
  
  uint32_t hue = 360.0*(time/(TICKS_PER_CYCLE)+_x*(1.5)+_y); //magic!!
  
  return hueToColor(hue);
}

#else

uint32_t XYTimeColor(int x, int y) {
  uint32_t b = 255.0 * (LED_BRIGHTNESS);
  return (b<<16) | (b<<8) | b;
}

#endif

/**
coordinate (0,0) is defined top-left and (WIDTH-1,HEIGHT-1) is bottom right.

pixel 0 is bottom left, snaking up. A 5x5 matrix would map as the following:

           ,==20|21|22|23|24=]         0,0|0,1|0,2|0,3|0,4
           || --+--+--+--+--           ---+---+---+---+---
           `==19|18|17|16|15==.        1,0|1,1|1,2|1,3|1,4
              --+--+--+--+-- |}        ---+---+---+---+---
           ,==10|11|12|13|14=='  from  2,0|2,1|2,2|2,3|2,4
           || --+--+--+--+--           ---+---+---+---+---
           `== 9| 8| 7| 6| 5==.        3,0|3,1|3,2|3,3|3,4
              --+--+--+--+-- ||        ---+---+---+---+---
To arduino <== 0| 1| 2| 3| 4=='        4,0|4,1|4,2|4,3|4,4
  
*/
int coordToPixel(byte x, byte y) {
  y = ((MATRIX_HEIGHT) - 1) - y;
  if(y&1) {
    x = ((MATRIX_WIDTH) - 1) - x;
  }
  return x + y*(MATRIX_WIDTH);
}
