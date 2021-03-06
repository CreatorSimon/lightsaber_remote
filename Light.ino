#include <FastLED.h>
#include <EEPROMex.h>

#define NUM_LEDS 124

#define DATA_PIN 3
#define BUTTON 5
#define COLOR Red

#define EEADIN 0
#define EEADBR 1
#define EEADHEX 2

uint8_t buttonflag = 0;
uint8_t on = 0;
uint16_t current_led = 0;
uint16_t delay_time = 10000;

//buffer
char buffer[10];

//input variables
uint8_t receivedInput;
uint8_t input = 3;
uint8_t last_input = 3;
uint8_t hue_factor = 4;

//hex color
uint32_t hex_color = 0xFF0000;
boolean hex_color_set = false;

//brightness 
uint8_t simple_brightness = 100;

// Define the array of leds
CRGB leds[NUM_LEDS];

void setup() {
  FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS);

  pinMode(BUTTON, OUTPUT);
  attachInterrupt(digitalPinToInterrupt(BUTTON), turn_on_off, CHANGE);

  //Read Eeprom
  simple_brightness = EEPROM.read(EEADBR);
  input = EEPROM.read(EEADIN);
  hex_color = EEPROM.readLong(EEADHEX);

  Serial.begin(9600);

}

void loop() {
  recvOneChar();

  //check if input value is in the range 
  if(receivedInput > 0 && receivedInput < 6)
  {
    hex_color_set = true;

    last_input = input;
    input = receivedInput;

    EEPROM.write(EEADIN, input);
  }

  switch(input)
  {
    case 1:
    set_hex_color();
    break;

    case 2:
    set_simple_brightness();
    break;

    case 3:
    lightsaber_mode();
    break;

    case 4:
    rainbow_circle(50);
    break;

    case 5:
    rainbow(50);
    break;
  }
}

/*
* Rainbow circle mode.
*/
void rainbow_circle(int s)
{
  for (int j = 0; j < NUM_LEDS / 2; j++)
  {
    for (int i = 0; i < NUM_LEDS / 2; i++)
    {
      leds[i] = CHSV(0 + (i * hue_factor) - (j * hue_factor), 255, getSimpleBrightness());
      leds[NUM_LEDS - i] = CHSV(0 + (i * hue_factor) - (j * hue_factor), 255, getSimpleBrightness());
    }
    FastLED.show();
    delay(s);
  }
}

/*
* Moving rainbow effect for all Fans
*/
void rainbow(int s)
{
  for (int j = 0; j < 255; j++) {
    for (int i = 0; i < NUM_LEDS / 2; i++) {
      leds[i] = CHSV(i - (j * 2), 255, getSimpleBrightness()); /* The higher the value 4 the less fade there is and vice versa */ 
      leds[NUM_LEDS - i] = CHSV(i - (j * 2), 255, getSimpleBrightness()); /* The higher the value 4 the less fade there is and vice versa */ 
    }
    FastLED.show();
    delay(s); /* Change this to your hearts desire, the lower the value the faster your colors move (and vice versa) */
  }
}

void lightsaber_mode(void)
{
  if(digitalRead(BUTTON) == LOW)
  {
    buttonflag = 1;
  }
  else
  {
    buttonflag = 0;
  }

  if(buttonflag && !on)
  {
    for(uint16_t i = current_led; i < NUM_LEDS/2; i++)
    {
      if(!buttonflag)
      {
        break; 
      }
      leds[i] = getHexColor();
      leds[NUM_LEDS-i] = getHexColor();;
      leds[i].fadeToBlackBy(255 - getSimpleBrightness());
      leds[NUM_LEDS-i].fadeToBlackBy(255 - getSimpleBrightness());
      current_led = i;
      FastLED.show();
      delay(2); 
    }
    on = 1;
  }
  if(!buttonflag && on)
  {
    for(int16_t i = current_led; i >= 0; i--)
    {
      if(buttonflag)
      {
        break; 
      }
      leds[i] = CRGB::Black;
      leds[NUM_LEDS-i] = CRGB::Black;
      current_led = i;
      FastLED.show();
      delay(2); 
    }
    on = 0;
  }
}


void turn_on_off()
{
  buttonflag ^= 1;
  delay(delay_time);
}

void set_last_input()
{
  if(last_input > 0 && last_input < 3)
  {
    last_input = 3;
  }

  input = last_input;
  EEPROM.write(EEADIN, input);
}

void setSimpleBrightness(uint8_t SimpleBrightness)
{
  simple_brightness = SimpleBrightness;
}

uint8_t getSimpleBrightness(void)
{
  return simple_brightness;
}

/*
* This function is for the communication. Simply gets the integer given over the serial bus.
*
*/
void recvOneChar() {
    if (Serial.available() > 0) {
        receivedInput = Serial.parseInt();   
    }else{
      receivedInput = 0;
    }
}

/*
* Function to clean the the serial buffer.
*/
void serialFlush(){
  while(Serial.available() > 0) {
    char t = Serial.read();
  }
}

/*
* Hex to long
*/
uint32_t x2i(char *s)
{
 uint32_t x = 0;
 for(;;) {
   char c = *s;
   if (c >= '0' && c <= '9') {
      x *= 16;
      x += c - '0';
   }
   else if (c >= 'a' && c <= 'f') {
      x *= 16;
      x += (c - 'a') + 10;
   }
   else break;
   s++;
 }
 return x;
}

/*
* Set the Brightness for the LEDs
*/
void set_simple_brightness(void)
{
  while(!Serial.available());

  delay(100);

  Serial.println("Set Brightness:");
  serialFlush();
  while(!Serial.available());
  setSimpleBrightness(Serial.parseInt());
  EEPROM.write(EEADBR, getSimpleBrightness());

  set_last_input();
  Serial.println(input);
  on = 0;
  current_led = 0;
}

/*
* Set hex color 
*/

void set_hex_color(void)
{
  if(hex_color_set)
  {
    while(!Serial.available());

    delay(100);

    Serial.println("Set Hex Color:");
    serialFlush();
    while(!Serial.available());
    Serial.readString().toCharArray(buffer, 10);
    
    setHexColor(x2i(buffer));
    EEPROM.writeLong(EEADHEX, getHexColor());

    hex_color_set = false;
    set_last_input();
    Serial.println(input);
    on = 0;
    current_led = 0;
  }
}

void setHexColor(uint32_t HexColor)
{
  hex_color = HexColor;
}

uint32_t getHexColor(void)
{
  return hex_color;
}
