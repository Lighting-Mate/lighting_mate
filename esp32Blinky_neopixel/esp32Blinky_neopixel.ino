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

static std::vector<BLEAddress*> pServerAddresses;
static std::vector<BLEClient*> pClients;

static boolean doConnect = false;

Adafruit_NeoPixel strip = Adafruit_NeoPixel(LED_NUM, PIN_BUTTON, NEO_RGB + NEO_KHZ800);
uint32_t c = strip.Color(0, 0, 0);
uint8_t ledOn = false, add = 10, color = 0;

bool connectToServer(BLEAddress pAddress) {
    Serial.println(pAddress.toString().c_str());
    
    BLEClient*  pClient  = BLEDevice::createClient();
    Serial.println(" - Created client");
    
    pClient->connect(pAddress);
    Serial.println(" - Connected to server");

    pClients.push_back(pClient);
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
      
      Serial.println(pServerAddress->toString().c_str());
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

  BLEDevice::init(DEVICE_NAME);
  
  BLEScan* pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(false);
  pBLEScan->start(15);

  Serial.println("-x- scan over -x-");
  doConnect = true;
}

void loop() {

  if (doConnect == true) {
    for(int i=0; i < pServerAddresses.size(); i++){
      if( connectToServer( *pServerAddresses[i] ) ){
        Serial.println("- Connect Server Done;");
      }else{
        Serial.println("-- Connect Something wrong...;");
      }
    }
    doConnect = false;
  }

  if ( !pServerAddresses.empty() ) {
    delay(3000);
    for( int i=0; i < pClients.size(); i++ ){
      BLEClient* pClient = pClients[i];
      
      BLERemoteService* pRemoteService = pClient->getService(BLEUUID(SERVICE_UUID));
      if (pRemoteService == nullptr) continue;
  
      BLERemoteCharacteristic* pRemoteChara = pRemoteService->getCharacteristic(BLEUUID(TEXT_UUID));
      if (pRemoteChara == nullptr) continue;
      
      pRemoteChara->writeValue("hello, world?");
    }
  }
  
  delay(1000);
}
