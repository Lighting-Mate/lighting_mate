#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

#define SERVICE_UUID        "7F9B5867-6E4B-4EF8-923F-D32903E1E43C"
#define BLINK_UUID          "ED8EC9CC-D2CF-4327-AB97-DDA66E03385C"
#define SHARE_UUID          "200E8A07-A7DF-4969-93E4-17C9E9FE76E1"
#define TEXT_UUID           "E4025514-0A8D-4C0B-B173-5D5535DCF29E"
#define DEVICE_NAME         "ESP_Blinky"

BLECharacteristic *pCharBlink;
BLECharacteristic *pCharShare;
BLECharacteristic *pCharText;


#include <string>
#include <NeoPixelBus.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif

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


const uint8_t  PixelPin = 32;
const uint16_t PixelCount = 3;

NeoPixelBus<NeoRgbFeature, Neo800KbpsMethod> strip(PixelCount, PixelPin);
RgbColor c = RgbColor(0, 0, 0);
float seed = 0.5;
static Colors stateColor = Colors(); // 固有色
static Colors otherColor = Colors(); // 相手の固有色
static boolean turn = false; // 発光回数の偶奇を通知 



bool touchCallback() {
  int in = analogRead(33);
//  Serial.println("Analog in:" + String(in) );
  if(in < 500) return false;

  touchLighting();
  
  pCharBlink->setValue("checker");
  return true;
}

class BlinkCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
    }

    void onRead(BLECharacteristic *pCharacteristic) {
      pCharacteristic->setValue("read done");
      Serial.println("\"");
    }
};


class ShareCallbacks: public BLECharacteristicCallbacks {
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
        touchLighting();
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
  strip.Begin();
  delay(1);
  strip.Show();

  BLEDevice::init(DEVICE_NAME);
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());


  BLEService *pService = pServer->createService(SERVICE_UUID);

  pCharBlink = pService->createCharacteristic(BLINK_UUID, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY | BLECharacteristic::PROPERTY_WRITE);
  pCharBlink->setCallbacks(new BlinkCallbacks());
  pCharBlink->addDescriptor(new BLE2902());

  pCharText = pService->createCharacteristic(TEXT_UUID, BLECharacteristic::PROPERTY_WRITE);
  pCharText->setCallbacks(new TextCallbacks());

  pCharShare = pService->createCharacteristic(SHARE_UUID, BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_READ);
  pCharShare->setCallbacks(new ShareCallbacks());

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
  pCharShare->setValue("00FF00");
}

void loop() {
  turn ? chaosBlink() : twoColorGradation();
  turn = !turn;

  delay(50);
}
