#include <string>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif

#define SERVICE_UUID        "7F9B5867-6E4B-4EF8-923F-D32903E1E43C"
#define BLINK_UUID          "ED8EC9CC-D2CF-4327-AB97-DDA66E03385C"
#define TEXT_UUID           "E4025514-0A8D-4C0B-B173-5D5535DCF29E"
#define DEVICE_NAME         "ESP_Blinky"

#define PIN_BUTTON 32
#define LED_NUM 3

static std::vector<BLEAddress*> pServerAddresses;

static boolean doConnect = false;

Adafruit_NeoPixel strip = Adafruit_NeoPixel(LED_NUM, PIN_BUTTON, NEO_RGB + NEO_KHZ800);
uint32_t c = strip.Color(0, 0, 0);
uint8_t ledOn = false, add = 10, color = 0;

float x = 0.5;
int cValue = 0;

BLECharacteristic *pCharBlink;
BLECharacteristic *pCharText;

bool connectToServer(BLEAddress pAddress) {
    Serial.println(pAddress.toString().c_str());
    
    BLEClient*  pClient  = BLEDevice::createClient();
    Serial.println(" - Created client");
    
    pClient->connect(pAddress);
    Serial.println(" - Connected to server");

    BLERemoteService* pRemoteService = pClient->getService(BLEUUID(SERVICE_UUID));
    if (pRemoteService == nullptr) {
      Serial.print("Failed to find our service UUID: ");
      Serial.println(BLEUUID(SERVICE_UUID).toString().c_str());
      return false;
    }
    Serial.println(" - Found our service");

    BLERemoteCharacteristic* pRemoteChara = pRemoteService->getCharacteristic(BLEUUID(TEXT_UUID));
    if (pRemoteChara == nullptr) {
      Serial.print("Failed to find our service UUID: ");
      Serial.println(BLEUUID(TEXT_UUID).toString().c_str());
      return false;
    }
    Serial.println(" - Found our Characteristic");
}

class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice advertisedDevice) {
    Serial.print("BLE Advertised Device found: ");
    Serial.println(advertisedDevice.toString().c_str());
    
    if (advertisedDevice.haveServiceUUID() && advertisedDevice.getServiceUUID().equals(BLEUUID(SERVICE_UUID))) {
      Serial.print("Found our device!  address: "); 
      
      delay(100);
      BLEAddress *pServerAddress = new BLEAddress(advertisedDevice.getAddress());
      delay(100);
      
      Serial.print(pServerAddress->toString().c_str());
      pServerAddresses.push_back(pServerAddress);
    }
  }
}; 


void setLed(bool on) {
  if (ledOn == on) {
    strip.show();
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

float light(float x) {
  if(x < 0.5){
    x = x + 2*x*x;
  } else {
    x = x - 2*(1.0-x)*(1.0-x);
  }
  if(x < 0.005) x = 0.005;
  if(x > 0.9) x = 0.9;
  return(x);
}

class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      Serial.println("Connected");
      ledOn = true;
      for(uint16_t i=0; i<strip.numPixels(); i++) {
        strip.setPixelColor(i, strip.Color(128, 128, 128));
      }
      delay(1);
      strip.show();
    };

    void onDisconnect(BLEServer* pServer) {
      Serial.println("Disconnected");
      ledOn = false;
      strip.clear();
      delay(1);
      strip.show();
    }
};

class BlinkCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
//      std::string vtext = pCharacteristic->getValue();
//      uint8_t value0 = vtext[0];
//      
//      strip.setBrightness(value0);
//      setLed(true);
//      Serial.print("Got blink value: ");
//      Serial.println(value0);
   }
};

class TextCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
//      std::string value = pCharacteristic->getValue();
//      Serial.print("Got text value: \"");
//      for (int i = 0; i < value.length(); i++) {
//        Serial.print(value[i]);
//      }
//      Serial.println("\"");
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

//  BLEDevice::init(DEVICE_NAME);
//  BLEServer *pServer = BLEDevice::createServer();
//  pServer->setCallbacks(new MyServerCallbacks());
//
//
//  BLEService *pService = pServer->createService(SERVICE_UUID);
//
//  pCharBlink = pService->createCharacteristic(BLINK_UUID, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY | BLECharacteristic::PROPERTY_WRITE);
//  pCharBlink->setCallbacks(new BlinkCallbacks());
//  pCharBlink->addDescriptor(new BLE2902());
//
//  pCharText = pService->createCharacteristic(TEXT_UUID, BLECharacteristic::PROPERTY_WRITE);
//  pCharText->setCallbacks(new TextCallbacks());
//
//  pService->start();
//
//  // ----- Advertising
//
//  BLEAdvertising *pAdvertising = pServer->getAdvertising();
//
//  BLEAdvertisementData adv;
//  adv.setName(DEVICE_NAME);
//  adv.setCompleteServices(BLEUUID(SERVICE_UUID));
//  pAdvertising->setAdvertisementData(adv);
//
//  BLEAdvertisementData adv2;
//  adv2.setName(DEVICE_NAME);
//  //  adv.setCompleteServices(BLEUUID(SERVICE_UUID));  // uncomment this if iOS has problems discovering the service
//  pAdvertising->setScanResponseData(adv2);
//
//  pAdvertising->start();
//
//  BLEScan* pBLEScan = BLEDevice::getScan();
//  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
//  pBLEScan->setActiveScan(false);
//  pBLEScan->start(20);
//
//  Serial.println("Ready");
//  doConnect = true;
}

void loop() {

//  if (doConnect == true) {
//    for(int i=0; i < pServerAddresses.size(); i++){
//      if( connectToServer( *pServerAddresses[i] ) ){
//        Serial.println("- Connect Server Done;");
//      }else{
//        Serial.println("-- Connect Something wrong...;");
//      }
//    }
//    doConnect = false;
//  }
  
//  delay(1000);

    x = light(x);
    cValue = (int)(x*255);
    Serial.println(x);
    Serial.println(cValue);
    delay(10);
    c = strip.Color(cValue, cValue, cValue);
    for(uint16_t i=0; i<strip.numPixels(); i++) {
      strip.setPixelColor(i, c);
    }
    delay(1);
    strip.show();
    delay((int)random(10, 100));
    
}
