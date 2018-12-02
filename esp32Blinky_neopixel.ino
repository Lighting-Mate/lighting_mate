#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <Adafruit_NeoPixel.h>
#include <TaskScheduler.h>

#ifdef __AVR__
  #include <avr/power.h>
#endif

#define SERVICE_UUID        "7F9B5867-6E4B-4EF8-923F-D32903E1E43C"
#define BLINK_UUID          "ED8EC9CC-D2CF-4327-AB97-DDA66E03385C"
#define TEXT_UUID           "E4025514-0A8D-4C0B-B173-5D5535DCF29E"
#define DEVICE_NAME         "ESP_Blinky"

#define PIN_BUTTON 32

Scheduler scheduler;
Adafruit_NeoPixel strip = Adafruit_NeoPixel(6, PIN_BUTTON, NEO_GBR + NEO_KHZ800);
uint32_t c = strip.Color(0, 0, 0);
uint8_t ledOn = false, add = 10, color = 0;

BLECharacteristic *pCharBlink;
BLECharacteristic *pCharText;

void led(bool on) {
  uint16_t i, bright;
  uint32_t c = strip.Color(255, 255, 255);
  
  for(uint8_t i=0; i<strip.numPixels(); i++) {
    strip.setPixelColor(i, c);
void setLed(bool on) {
  if (ledOn == on) {
    strip.show();
    delay(10);
    return;
  }
  ledOn = on;

  if (ledOn) {
    Serial.println("LED ON");
    c = strip.Color(128, 128, 128);
  } else {
    Serial.println("LED OFF");
    c = strip.Color(0, 0, 0);
  }
  for(uint16_t i=0; i<strip.numPixels(); i++) {
    strip.setPixelColor(i, c);
  }
  delay(1);
  strip.show();
  
  pCharBlink->setValue(&ledOn, 1);
}

class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      Serial.println("Connected");
    };

    void onDisconnect(BLEServer* pServer) {
      Serial.println("Disconnected");
    }
};

class BlinkCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      std::string value = pCharacteristic->getValue();

      if (value.length()  == 1) {
        uint8_t v = value[0];
        Serial.print("Got blink value: ");
        Serial.println(v);
        setBlink(v ? true : false);
      } else {
        Serial.println("Invalid data received");
      }
    }
};

class TextCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      std::string value = pCharacteristic->getValue();

      Serial.print("Got text value: \"");
      for (int i = 0; i < value.length(); i++) {
        Serial.print(value[i]);
      }
      Serial.println("\"");
    }
};

void setup() {
  Serial.begin(9600);
  delay(500);
  Serial.println("Starting...");

  #if defined (__AVR_ATtiny85__)
    if (F_CPU == 16000000) clock_prescale_set(clock_div_1);
  #endif
  // End of trinket special code
  strip.begin();
  delay(1);
  strip.show(); // Initialize all pixels to 'off'

  Serial.println("Starting...");
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
  scheduler.execute();
  setLed(ledOn);
  delay(10);
}
