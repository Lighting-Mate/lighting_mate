#include <string>
#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif

#define LED_PIN 32
#define LED_NUM 3

Adafruit_NeoPixel strip = Adafruit_NeoPixel(LED_NUM, LED_PIN, NEO_RGB + NEO_KHZ800);
uint32_t c = strip.Color(0, 0, 0);

float seed = 0.5;



float chaos(float seed) {
  if(seed < 0.5){
    seed = seed + 2*seed*seed;
  } else {
    seed = seed - 2*(1.0-seed)*(1.0-seed);
  }
  if(seed < 0.05) seed = 0.05;
  if(seed > 0.95) seed = 0.95;
  return seed;
}

void chaosBlink() {
  seed = chaos(seed); // seed値の更新
  Serial.println("Seed value:" + String(seed) );
  
  for(uint16_t i=0; i<255; i++){
    c = strip.Color(i, i, i);
    for(uint16_t i=0; i<strip.numPixels(); i++) {
      strip.setPixelColor(i, c);
    }
    delay(1);
    touchCallback();
    strip.show();
    delay(30*seed);
  }
  for(uint16_t i=255; i>0; i--){
    c = strip.Color(i, i, i);
    for(uint16_t i=0; i<strip.numPixels(); i++) {
      strip.setPixelColor(i, c);
    }
    delay(1);
    touchCallback();
    strip.show();
    delay(30*seed);
  }
}

void touchCallback() {
  int in = analogRead(25);
  Serial.println("Analog in:" + String(in) );
  if(in < 4000) {
    for(uint16_t i=0; i<strip.numPixels(); i++) {
      strip.setPixelColor(i, strip.Color(255, 255*0.5, 255*0.8));
    }
  } 
}



void setup() {  
  Serial.begin(115200);
  delay(500);
  
  Serial.println("Starting...");

  #if defined (__AVR_ATtiny85__)
    if (F_CPU == 16000000) clock_prescale_set(clock_div_1);
  #endif
  strip.begin();
  delay(1);
  strip.show();
}

void loop() {
  chaosBlink();
  delay(1000);
}
