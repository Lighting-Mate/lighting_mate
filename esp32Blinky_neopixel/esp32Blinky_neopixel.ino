#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

#define SERVICE_UUID        "7F9B5867-6E4B-4EF8-923F-D32903E1E43C"
#define BLINK_UUID          "ED8EC9CC-D2CF-4327-AB97-DDA66E03385C"
#define TEXT_UUID           "E4025514-0A8D-4C0B-B173-5D5535DCF29E"
#define DEVICE_NAME         "ESP_Blinky"

BLECharacteristic *pCharBlink;
BLECharacteristic *pCharText;


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
    if( touchCallback() ) return;
    strip.show();
    delay(30*seed);
  }
  for(uint16_t i=255; i>0; i--){
    c = strip.Color(i, i, i);
    for(uint16_t i=0; i<strip.numPixels(); i++) {
      strip.setPixelColor(i, c);
    }
    delay(1);
    if( touchCallback() ) return;
    strip.show();
    delay(30*seed);
  }
}

bool touchCallback() {
  int in = analogRead(33);
//  Serial.println("Analog in:" + String(in) );
  if(in < 500) return false;

  for(int j=0; j<3; j++){
    for(uint16_t i=0; i<255; i++){
      c = strip.Color(i, i, i);
      for(uint16_t i=0; i<strip.numPixels(); i++) {
        strip.setPixelColor(i, c);
      }
      delay(1);
      strip.show();
      delay(0.01);
    }
    for(uint16_t i=255; i>0; i--){
      c = strip.Color(i, i, i);
      for(uint16_t i=0; i<strip.numPixels(); i++) {
        strip.setPixelColor(i, c);
      }
      delay(1);
      strip.show();
      delay(0.01);
    }
  }
  return true;
}

class BlinkCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
    }
};


class TextCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      std::string value = pCharacteristic->getValue();
      Serial.print("Got text value: \"");
      for (int i = 0; i < value.length(); i++) {
        Serial.print(value[i]);
      }

      if(value == "lighton"){
        for(int j=0; j<3; j++){
          for(uint16_t i=0; i<255; i++){
            c = strip.Color(i, i, i);
            for(uint16_t i=0; i<strip.numPixels(); i++) {
              strip.setPixelColor(i, c);
            }
            delay(1);
            strip.show();
            delay(0.01);
          }
          for(uint16_t i=255; i>0; i--){
            c = strip.Color(i, i, i);
            for(uint16_t i=0; i<strip.numPixels(); i++) {
              strip.setPixelColor(i, c);
            }
            delay(1);
            strip.show();
            delay(0.01);
          }
        }
      }
      Serial.println("\"");
    }
};

class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      Serial.println("Connected");
    };

    void onDisconnect(BLEServer* pServer) {
      Serial.println("Disconnected");
    }
};


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

  BLEDevice::init(DEVICE_NAME);
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());


  BLEService *pService = pServer->createService(SERVICE_UUID);

  pCharBlink = pService->createCharacteristic(BLINK_UUID, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY | BLECharacteristic::PROPERTY_WRITE);
  pCharBlink->setCallbacks(new BlinkCallbacks());
  pCharBlink->addDescriptor(new BLE2902());

  pCharText = pService->createCharacteristic(TEXT_UUID, BLECharacteristic::PROPERTY_WRITE);
  pCharText->setCallbacks(new TextCallbacks());

  pService->start();

  // ----- Advertising

  BLEAdvertising *pAdvertising = pServer->getAdvertising();

  BLEAdvertisementData adv;
  adv.setName(DEVICE_NAME);
  adv.setCompleteServices(BLEUUID(SERVICE_UUID));
  pAdvertising->setAdvertisementData(adv);

  BLEAdvertisementData adv2;
  adv2.setName(DEVICE_NAME);
  //  adv.setCompleteServices(BLEUUID(SERVICE_UUID));  // uncomment this if iOS has problems discovering the service
  pAdvertising->setScanResponseData(adv2);

  pAdvertising->start();

  Serial.println("Ready");
}

void loop() {
  chaosBlink();
  delay(50);
}
