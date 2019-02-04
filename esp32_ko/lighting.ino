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
  
  for(uint16_t i=0; i<255; i++){
    c = RgbColor(stateColor.getRed()/255.0*i, stateColor.getGreen()/255.0*i, stateColor.getBlue()/255.0*i);
    for(uint16_t i=0; i<PixelCount; i++) {
      strip.SetPixelColor(i, c);
    }
    if( touchCallback() ) return;
    if( doTextInterrupt ) {
      touchLighting(true);
      doTextInterrupt = false;
      return;
    }
    if( HomeInterruptCallback() ) return;
    strip.Show();
    delay(30*seed);
  }
  for(uint16_t i=255; i>0; i--){
    c = RgbColor(stateColor.getRed()/255.0*i, stateColor.getGreen()/255.0*i, stateColor.getBlue()/255.0*i);
    for(uint16_t i=0; i<PixelCount; i++) {
      strip.SetPixelColor(i, c);
    }
    if( touchCallback() ) return;
    if( doTextInterrupt ) {
      touchLighting(true);
      doTextInterrupt = false;
      return;
    }
    if( HomeInterruptCallback() ) return;
    strip.Show();
    delay(30*seed);
  }
}

// タッチされた時の光り方を制御している関数
void touchLighting(bool isOther) {
  Colors colors = isOther ? Colors(otherColor) : Colors(stateColor);
  for(int j=0; j<2; j++){
    for(uint16_t i=50; i<200; i++){
      c = RgbColor(colors.getRed()/255.0*i, colors.getGreen()/255.0*i, colors.getBlue()/255.0*i);
      for(uint16_t i=0; i<PixelCount; i++) {
        strip.SetPixelColor(i, c);
      }
      strip.Show();
    }
    for(uint16_t i=200; i>50; i--){
      c = RgbColor(colors.getRed()/255.0*i, colors.getGreen()/255.0*i, colors.getBlue()/255.0*i);
      for(uint16_t i=0; i<PixelCount; i++) {
        strip.SetPixelColor(i, c);
      }
      strip.Show();
    }
  }
  for(uint16_t i=50; i>0; i--){
    c = RgbColor(colors.getRed()/255.0*i, colors.getGreen()/255.0*i, colors.getBlue()/255.0*i);
    for(uint16_t i=0; i<PixelCount; i++) {
      strip.SetPixelColor(i, c);
    }
    strip.Show();
  }
}


// 2色与えるとグラデーション発光する
void twoColorGradation() {
  seed = chaos(seed);
  uint8_t myr = stateColor.getRed();
  uint8_t myg = stateColor.getGreen();
  uint8_t myb = stateColor.getBlue();
  
  int r, g, b;

  r = (otherColor.getRed() - myr );
  g = (otherColor.getGreen() - myg );
  b = (otherColor.getBlue() - myb );
  
  for(uint16_t i=0; i<255; i++) {
    RgbColor c = RgbColor( (myr + r*i/255.0)/255.0*i, (myg + g*i/255.0)/255.0*i,  (myb + b*i/255.0)/255.0*i);  
    for(uint16_t j=0; j<PixelCount; j++) {
        strip.SetPixelColor(j, c);
    }
    if( touchCallback() ) return;
    if( doTextInterrupt ) {
      touchLighting(true);
      doTextInterrupt = false;
      return;
    }
    if( HomeInterruptCallback() ) return;
    strip.Show();
    delay(30*seed);
  }
  for(uint16_t i=255; i>0; i--) {
    RgbColor c = RgbColor( (myr + r*i/255.0)/255.0*i, (myg + g*i/255.0)/255.0*i,  (myb + b*i/255.0)/255.0*i);  
    for(uint16_t j=0; j<PixelCount; j++) {
        strip.SetPixelColor(j, c);
    }
    if( touchCallback() ) return;
    if( doTextInterrupt ) {
      touchLighting(true);
      doTextInterrupt = false;
      return;
    }
    if( HomeInterruptCallback() ) return;
    strip.Show();
    delay(30*seed);
  }
}
