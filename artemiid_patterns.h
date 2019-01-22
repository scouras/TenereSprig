
const TProgmemRGBPalette16 Artemiid_Colors_p FL_PROGMEM =
{
    CRGB::Black,
    CRGB::Indigo,
    CRGB::Aquamarine,
    CRGB::DarkSlateBlue,    
    
    CRGB::Amethyst,
    CRGB::SeaGreen,
    CRGB::Teal,
    CRGB::Black,

    CRGB::Black,
    CRGB::BlueViolet,
    CRGB::DarkMagenta,
    CRGB::DarkOrchid,
    
    CRGB::Indigo,
    CRGB::LightSeaGreen,
    CRGB::LightSkyBlue,
    CRGB::Black,
};

CRGBPalette16 current_palette = Artemiid_Colors_p;

uint16_t distance(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2) {
    uint16_t dx = x2 - x1;
    uint16_t dy = y2 - y1;
    return sqrt16((dx * dx) + (dy * dy));
}


int plasma() {
  // loosely based on http://www.mennovanslooten.nl/blog/post/72
  
   uint8_t offset1 = beat8(3);
   uint8_t offset2 = beat8(7); 

   uint8_t y=60;
   
   for (uint8_t x=0; x<NUM_LEDS; x++) {
      uint8_t i1 = (x * -4) + offset1;
      uint8_t i2 = distance(x * 2, y * 2, sin8(-offset2), cos8(-offset2));
      
      nblend(leds[x], ColorFromPalette(current_palette, avg8(i1, i2), 255, LINEARBLEND), 128);
   }
   
   fadeLightBy(leds, NUM_LEDS, 10);

   return 0;
}
