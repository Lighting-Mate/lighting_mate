//1207デモ用　Bluetooth接続確立時LEDON (LED3個用)　
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

static BLEAddress *pServerAddress;
static boolean doConnect = false;
static boolean connected = false;

Adafruit_NeoPixel strip = Adafruit_NeoPixel(LED_NUM, PIN_BUTTON, NEO_RGB + NEO_KHZ800);
uint32_t c = strip.Color(0, 0, 0);
uint8_t ledOn = false, add = 10, color = 0;

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
}

class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice advertisedDevice) {
    Serial.print("BLE Advertised Device found: ");
    Serial.println(advertisedDevice.toString().c_str());
    
    if (advertisedDevice.haveServiceUUID() && advertisedDevice.getServiceUUID().equals(BLEUUID(SERVICE_UUID))) {
      Serial.print("Found our device!  address: "); 
      advertisedDevice.getScan()->stop();
      
      delay(100);
      pServerAddress = new BLEAddress(advertisedDevice.getAddress());
      delay(100);
      
      Serial.print(pServerAddress->toString().c_str());
      doConnect = true;
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

  BLEScan* pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true);
  pBLEScan->start(30);

  Serial.println("Ready");
}

void loop() {

  if (doConnect == true) {
    if (connectToServer(*pServerAddress)) {
      Serial.println("We are now connected to the BLE Server.");
      connected = true;
    } else {
      Serial.println("We have failed to connect to the server; there is nothin more we will do.");
    }
    doConnect = false;
  }
  
  delay(1000);
}
