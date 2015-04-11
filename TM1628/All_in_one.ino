// Testing sketch for DVD LED module with TM1628 IC
// Written by Vasyl Yudin, oct 2012, public domain
// www.blockduino.org
#include "TM1628.h"
// define - data pin D9, clock pin D8 and strobe pin D7
// обьявление портов: DIO - порт D9, CLK - D8, STB - D7
TM1628 dvdLED(4, 2, 3);

#include <stdarg.h>
void p(char *fmt, ... ){
        char buf[128]; // resulting string limited to 128 chars
        va_list args;
        va_start (args, fmt );
        vsnprintf(buf, 128, fmt, args);
        va_end (args);
        Serial.print(buf);
}

void p(const __FlashStringHelper *fmt, ... ){
  char buf[128]; // resulting string limited to 128 chars
  va_list args;
  va_start (args, fmt);
#ifdef __AVR__
  vsnprintf_P(buf, sizeof(buf), (const char *)fmt, args); // progmem for AVR
#else
  vsnprintf(buf, sizeof(buf), (const char *)fmt, args); // for the rest of the world
#endif
  va_end(args);
  Serial.print(buf);
}

int z=0;
int button;
int state=0;

void setup() {
  dvdLED.begin(ON, 1);
  Serial.begin(9600);
  //setTime(2, 12, 13, 24, 10, 12);
}

void disc_animate(uint8_t direction)
{
  static uint8_t i = 0;
  static uint8_t toggle = 0;
  static uint8_t direction_previous;

  if (direction_previous != direction)
  {
    i = 0;
  }

  if (direction)
    toggle ? dvdLED.setLEDon(i) : dvdLED.setLEDoff(i);
  else
    toggle ? dvdLED.setLEDon(5 - i) : dvdLED.setLEDoff(5 - i);

  dvdLED.update();

    if (i >= 5)
        toggle = !toggle;
    i = (i+1) % 6;   // 6 slices for the disc icon
    
    direction_previous = direction;

}

void loop()
{
  static int i = 0;
  static int toggle = 0;
  static long time = 0;
  
  static int segment = 0;

  button = dvdLED.getButtons();
  switch(button)
  {
    case 2:
      segment--;
      break;
    case 8:
      i++;
      break;
    case 1:
      segment++;
      break;
    default:
      break;
  }
  if (i > 9)
  {
    segment++;
    i = 0;
  }
  
  time+=100;
  
//  delay(500);
  i++;
//   if (button != 0){
//       p("%d\n", button);
//      i++;
//   }
   
//   dvdLED.clear();
//    disc_animate(0);

//delay(200);
 // 1, 3, 4, OK
 
//dvdLED.setSeg(segment, i);
//dvdLED.setTime(time / 3600,(time / 60) % 60,time % 3600 % 60);
dvdLED.print("CULOne");
//dvdLED.setLEDon(LED_CL1);
//dvdLED.setLEDon(LED_CL2);
//dvdLED.update(); 
//dvdLED.setLEDon(LED_CL1);
//dvdLED.setLEDon(LED_CL2);
  

//    dvdLED.clear();

//    dvdLED.setSeg(i, 8);
//    dvdLED.update();
//    delay(1000);
//    i = (i+1) % 7;



//dvdLED.setDisc(1);
/*


dvdLED.setLEDon(LED_DVD);
dvdLED.setLEDon(LED_VCD);
dvdLED.setLEDon(LED_MP3);
dvdLED.setLEDon(LED_PLY);
dvdLED.setLEDon(LED_PAU);


dvdLED.setLEDon(LED_RET); //MP4

dvdLED.setLEDon(LED_DTS);
dvdLED.setLEDon(LED_DDD);
*/
}

void loop2() {
  button = dvdLED.getButtons();
   if (button != 0){
     state++;
     dvdLED.clear();
   }
   if (state == 0){
     delay(500);
   } else 
   if (state == 1){
     ShowLEDs();
   } else 
   if (state == 2){
     for(int i =0; i<16; i++)
       ;
       //ShowTime();
   } else 
   if (state == 3){
     ShowPrint();
   }  else state = 0;
}

void ShowTime(){
//  dvdLED.setTime(hour(), minute(), second());
  
  delay(50);
  dvdLED.setLED(z++%8);
  
  if (millis()/500 & 0x01){
      dvdLED.setLEDon(LED_CL1);
      dvdLED.setLEDon(LED_CL2);
    } else {
      dvdLED.setLEDoff(LED_CL1);
      dvdLED.setLEDoff(LED_CL2);
    }
}

void ShowLEDs(){
  for(int i=0;i<19;i++){
    dvdLED.setLEDon(i);
    delay(100);    
  }
  
  for(int i=0;i<19;i++){
    dvdLED.setLEDoff(i);
    delay(25);    
  }
}

void ShowPrint(){
int z=0;
    dvdLED.clear();
    dvdLED.print("b_duino");
    dvdLED.setLED(z++);
    delay(500);
    
// пример взят из reference/Serial_Print.html
    dvdLED.clear();
    dvdLED.print(78);//DEC default
    dvdLED.setLED(z++);
    delay(500);

    dvdLED.clear();
    dvdLED.print(78, 0);
    dvdLED.setLED(z++);
    delay(500);

    dvdLED.clear();
    dvdLED.print(78, BIN);
    dvdLED.setLED(z++);
    delay(500);
    
    dvdLED.clear();
    dvdLED.print(78, OCT);
    dvdLED.setLED(z++);
    delay(500);
    
    dvdLED.clear();
    dvdLED.print(78, HEX);
    dvdLED.setLED(z++);
    delay(500);
    
    dvdLED.clear();
    dvdLED.print(-1.2345678);
    dvdLED.setLED(z++);
    delay(500);
    
    dvdLED.clear();
    dvdLED.print(-1.2345678, 4);
    dvdLED.setLED(z++);
    delay(500);
}
