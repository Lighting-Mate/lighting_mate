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
    c = RgbColor(i, i, i);
    for(uint16_t i=0; i<PixelCount; i++) {
      strip.SetPixelColor(i, c);
    }
    delay(1);
    if( touchCallback() ) return;
    strip.Show();
    delay(30*seed);
  }
  for(uint16_t i=255; i>0; i--){
    c = RgbColor(i, i, i);
    for(uint16_t i=0; i<PixelCount; i++) {
      strip.SetPixelColor(i, c);
    }
    delay(1);
    if( touchCallback() ) return;
    strip.Show();
    delay(30*seed);
  }
}

// タッチされた時の光り方を制御している関数
void touchLighting() {
  for(int j=0; j<3; j++){
    for(uint16_t i=0; i<255; i++){
      c = RgbColor(i, i, i);
      for(uint16_t i=0; i<PixelCount; i++) {
        strip.SetPixelColor(i, c);
      }
      strip.Show();
    }
    for(uint16_t i=255; i>0; i--){
      c = RgbColor(i, i, i);
      for(uint16_t i=0; i<PixelCount; i++) {
        strip.SetPixelColor(i, c);
      }
      strip.Show();
    }
  }
}
