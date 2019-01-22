

int NUM_LEAVES = 15;
int LEDS_PER_LEAF = 7;
int LEAF_TIP = (LEDS_PER_LEAF-1)/2;

int test_leaf_registration() {
    int hue = 0;
    int dHue = 255 / NUM_LEAVES;
    int pos;
    for (int i = 0; i < NUM_LEDS; i++) {
      leds[i] = CRGB(0,0,0);
    }

    for (int l = 0; l < NUM_LEAVES; l++) {
      pos = LEAF_TIP + l * LEDS_PER_LEAF;
      leds[pos] = CHSV(hue,255,255);
      hue += dHue;
    }

    leds[0] = CRGB(255,255,255);
    leds[NUM_LEDS-1] = CRGB(255,255,255);

    hue %= 255;
    return 0;
}



int MIMSY_REGISTRATIONS[] = {25, 50, 54, 72, 158, 171};
//int MIMSY_REGISTRATIONS[] = {10, 15, 20, 25, 30};

int test_mimsy_registration() {
  leds[0] = CRGB(255,0,0); 
  leds[1] = CRGB(0,255,0);
  leds[2] = CRGB(0,255,0);
  leds[3] = CRGB(0,0,255);
  leds[4] = CRGB(0,0,255);
  leds[5] = CRGB(0,0,255);

 
  int hue =   0;
  int dhue = 32;
  int sat = 255;
  int brt = 255;  
  
  for (int i = 0; i < 6; i++) {
    uint8_t l = MIMSY_REGISTRATIONS[i];
    if (l >= NUM_LEDS) { continue; }
    leds[l-1] = CHSV(hue, sat, brt);
    leds[l-0] = CHSV(hue, sat, brt);
    hue += dhue;
  }

   return 0;
}
