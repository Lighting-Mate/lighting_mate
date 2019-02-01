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
//  Serial.println("Seed value:" + String(seed) );

  //  選択した色をそのまま反映させ続けたい場合は下記のコードに変更。
  //  Colors colors = Colors( String(state.c_str()) );
  
  for(uint16_t i=0; i<255; i++){
    c = strip.Color(colors.getRed()/255.0*i, colors.getGreen()/255.0*i, colors.getBlue()/255.0*i);
    for(uint16_t i=0; i<strip.numPixels(); i++) {
      strip.setPixelColor(i, c);
    }
    strip.setBrightness( i );
    delay(1);
    if( touchCallback() ) return;
    if( readCallback() ) return;
    if( smartInterruptCallback() ) return;
    strip.show();
    delay(30*seed);
  }
  for(uint16_t i=255; i>0; i--){
    c = strip.Color(colors.getRed()/255.0*i, colors.getGreen()/255.0*i, colors.getBlue()/255.0*i);
    for(uint16_t i=0; i<strip.numPixels(); i++) {
      strip.setPixelColor(i, c);
    }
    strip.setBrightness( i );
    delay(1);
    if( touchCallback() ) return;
    if( readCallback() ) return;
    if( smartInterruptCallback() ) return;
    strip.show();
    delay(30*seed);
  }
  
}


// タッチされた時の光り方を制御している関数
void touchLighting() {
  for(int j=0; j<3; j++){
    for(uint16_t i=0; i<255; i++){
      c = strip.Color(stateColor.getRed()/255.0*i, stateColor.getGreen()/255.0*i, stateColor.getBlue()/255.0*i);
      for(uint16_t i=0; i<strip.numPixels(); i++) {
        strip.setPixelColor(i, c);
      }
      strip.setBrightness( i );
      delay(1);
      strip.show();
      delay(0.01);
    }
    for(uint16_t i=255; i>0; i--){
      c = strip.Color(stateColor.getRed()/255.0*i, stateColor.getGreen()/255.0*i, stateColor.getBlue()/255.0*i);
      for(uint16_t i=0; i<strip.numPixels(); i++) {
        strip.setPixelColor(i, c);
      }
      strip.setBrightness( i );
      delay(1);
      strip.show();
      delay(0.01);
    }
  }
}


// 2色与えるとグラデーション発光する
void twoColorGradation() {
  seed = chaos(seed);
  if ( !pClients.empty() ) {
    do {
      int randChild = random( pClients.size() );
      BLEClient* pClient = pClients[randChild];  
      BLERemoteService* pRemoteService = pClient->getService(BLEUUID(SERVICE_UUID));
      if (pRemoteService == nullptr) continue;
      BLERemoteCharacteristic* pRemoteShareChara = pRemoteService->getCharacteristic(BLEUUID(SHARE_UUID));
      if (pRemoteShareChara == nullptr) continue;
      
      std::string otherValue = pRemoteShareChara->readValue();
      otherColor = Colors( String(otherValue.c_str()) );
    }while(false);
  }
  uint8_t myr = stateColor.getRed();
  uint8_t myg = stateColor.getGreen();
  uint8_t myb = stateColor.getBlue();
  
  int r, g, b;

  r = (otherColor.getRed() - myr );
  g = (otherColor.getGreen() - myg );
  b = (otherColor.getBlue() - myb );

  for(uint16_t i=0; i<255; i++) {
    uint32_t c = strip.Color((uint8_t)myr + r*i/255.0, (uint8_t)myg + g*i/255.0,  (uint8_t)myb + b*i/255.0);
    for(uint16_t j=0; j<strip.numPixels(); j++) {
        strip.setPixelColor(j, c);
    }
    strip.setBrightness( i );
    delay(1);
    if( touchCallback() ) return;
    if( readCallback() ) return;
    if( smartInterruptCallback() ) return;
    strip.show();
    delay(30*seed);
  }
  for(uint16_t i=255; i>0; i--) {
    uint32_t c = strip.Color((uint8_t)myr + r*i/255.0, (uint8_t)myg + g*i/255.0,  (uint8_t)myb + b*i/255.0);
    for(uint16_t j=0; j<strip.numPixels(); j++) {
        strip.setPixelColor(j, c);
    }
    strip.setBrightness( i );
    delay(1);
    if( touchCallback() ) return;
    if( readCallback() ) return;
    if( smartInterruptCallback() ) return;
    strip.show();
    delay(30*seed);
  }
}
