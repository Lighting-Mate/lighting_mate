#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

#define SERVICE_UUID        "7F9B5867-6E4B-4EF8-923F-D32903E1E43C"
#define BLINK_UUID          "ED8EC9CC-D2CF-4327-AB97-DDA66E03385C"
#define SHARE_UUID          "200E8A07-A7DF-4969-93E4-17C9E9FE76E1"
#define TEXT_UUID           "E4025514-0A8D-4C0B-B173-5D5535DCF29E"
#define SMART_SERVICE_UUID  "06E17ABD-F5EB-4980-BBED-2E67F1664628"
#define SMART_CHARA_UUID    "33AD1DB5-C067-4E8C-95C6-6804EB95BE96"
#define DEVICE_NAME         "ESP_Blinky"


#include <string>
#include <Adafruit_NeoPixel.h>
#include <vector>
#ifdef __AVR__
  #include <avr/power.h>
#endif

#define LED_PIN 32
#define LED_NUM 5

class Colors {
  public:
  uint8_t color[3]; 
  Colors(String value){
    for (int i = 0; i < 3; i++) {
      String tmp = String(value[i*2]) + String(value[i*2+1]);
      color[i] = atof( ("0x"+tmp).c_str());
    }  
  };
  
  Colors(){
    randomSeed( analogRead(0) ); // 未使用ピンのノイズでシード値を設定
    int randIndex = random(3);   // 0 ~ 2
    Serial.println( randIndex );
    for (int i = 0; i < 3; i++) {
      int randNumber = i == randIndex ? random(127, 256) : random(50, 150);
      color[i] = randNumber;
    } 
  };
  
  uint8_t getRed(){ return color[0]; }
  uint8_t getGreen(){ return color[1]; }
  uint8_t getBlue(){ return color[2]; }
};

static std::vector<BLEAddress*> pServerAddresses;
static std::vector<BLEClient*> pClients;

static boolean doConnect = false;
static boolean doSmartInterrupt = false;
std::string state = "FFFFFF";
static Colors stateColor = Colors(); // 固有色
static Colors otherColor = stateColor; // 相手の固有色(初期化のみ自分の色を格納)
static boolean turn = false; // 発光回数の偶奇を通知 


Adafruit_NeoPixel strip = Adafruit_NeoPixel(LED_NUM, LED_PIN, NEO_RGB + NEO_KHZ800);
uint32_t c = strip.Color(0, 0, 0);
float seed = 0.5;

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



bool touchCallback() {
  int in = analogRead(33);
//  Serial.println("Analog in:" + String(in) );
  if(in < 500) return false;
  
  touchLighting();

  if ( !pClients.empty() ) {
    for( int i=0; i < pClients.size(); i++ ){
      BLEClient* pClient = pClients[i];
      
      BLERemoteService* pRemoteService = pClient->getService(BLEUUID(SERVICE_UUID));
      if (pRemoteService == nullptr) continue;
  
      BLERemoteCharacteristic* pRemoteChara = pRemoteService->getCharacteristic(BLEUUID(TEXT_UUID));
      if (pRemoteChara == nullptr) continue;
      
      pRemoteChara->writeValue("lighton");
    }
  }

  return true;
}


bool readCallback() {
  if ( !pClients.empty() ) {
    for( int i=0; i < pClients.size(); i++ ){
      BLEClient* pClient = pClients[i];
      
      BLERemoteService* pRemoteService = pClient->getService(BLEUUID(SERVICE_UUID));
      if (pRemoteService == nullptr) continue;
  
      BLERemoteCharacteristic* pRemoteChara = pRemoteService->getCharacteristic(BLEUUID(BLINK_UUID));
      if (pRemoteChara == nullptr) continue;
      
      std::string value = pRemoteChara->readValue();
      if( value == "checker"){
        touchLighting();
        return true;
      }
    }
  }
  return false;
}

class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      Serial.println("Connected");
    };
    void onDisconnect(BLEServer* pServer) {
      Serial.println("Disconnected");
    }
};


class smartCallbacks: public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pCharacteristic) {
    std::string value = pCharacteristic->getValue();
    state = value;
    Serial.print("Get smart value: \"");
    for (int i = 0; i < value.length(); i++) {
      Serial.print(value[i]);
    }
    Serial.println("\"");
    doSmartInterrupt = true;
  }

  void onRead(BLECharacteristic *pCharacteristic) {
  }
};


bool smartInterruptCallback(){
  if( !doSmartInterrupt )return false;
  
  Colors colors = Colors( String(state.c_str()) );
  for(int j=0; j<3; j++){
    for(uint16_t i=0; i<255; i++){
      c = strip.Color(colors.getRed()/255.0*i, colors.getGreen()/255.0*i, colors.getBlue()/255.0*i);
      for(uint16_t i=0; i<strip.numPixels(); i++) {
        strip.setPixelColor(i, c);
      }
      strip.setBrightness( i );
      delay(1);
      strip.show();
      delay(0.01);
    }
    for(uint16_t i=255; i>0; i--){
      c = strip.Color(colors.getRed()/255.0*i, colors.getGreen()/255.0*i, colors.getBlue()/255.0*i);
      for(uint16_t i=0; i<strip.numPixels(); i++) {
        strip.setPixelColor(i, c);
      }
      strip.setBrightness( i );
      delay(1);
      strip.show();
      delay(0.01);
    }
  }
  doSmartInterrupt = false;
  return true;
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



////////// This is server code //////////

  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());
  BLEService *pService = pServer->createService(SMART_SERVICE_UUID);

  BLECharacteristic *pCharSmart;
  pCharSmart = pService->createCharacteristic(SMART_CHARA_UUID, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY | BLECharacteristic::PROPERTY_WRITE);
  pCharSmart->setCallbacks(new smartCallbacks());
  pCharSmart->addDescriptor(new BLE2902());

  pService->start();

  BLEAdvertising *pAdvertising = pServer->getAdvertising();

  BLEAdvertisementData adv;
  adv.setName(DEVICE_NAME);
  adv.setCompleteServices(BLEUUID(SMART_SERVICE_UUID));
  pAdvertising->setAdvertisementData(adv);

  BLEAdvertisementData adv2;
  adv2.setName(DEVICE_NAME);
  //  adv.setCompleteServices(BLEUUID(SERVICE_UUID));  // uncomment this if iOS has problems discovering the service
  pAdvertising->setScanResponseData(adv2);

  pAdvertising->start();

  Serial.println("Ready");
  
////////// Server code end //////////
  
  doConnect = true;
}

void loop() {
  delay(100);
  
  if (doConnect == true) {
    for(int i=0; i < pServerAddresses.size(); i++){
      BLEAddress* temp = pServerAddresses[i];
      if( connectToServer( *temp ) ){
        Serial.println("- Connect Server Done;");
      }else{
        Serial.println("-- Connect Something wrong...;");
      }
      delete temp;
    }
    pServerAddresses.clear();
    doConnect = false;
  }
  
  turn ? chaosBlink() : twoColorGradation();
  turn = !turn;
}
