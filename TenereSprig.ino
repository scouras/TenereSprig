/*
 * 
 * The ATmega32U4 has 32 KB (with 4 KB used for the bootloader). It also has 2.5 KB of SRAM and 1 KB of EEPROM
 * 
 *** No LEDS_OFF
 * Sketch uses 11,766 bytes (41%) of program storage space. Maximum is 28,672 bytes.
 * Global variables use 769 bytes (30%) of dynamic memory, leaving 1,791 bytes for local variables. Maximum is 2,560 bytes.
 * 
 *** With LEDS_OFF
 * Sketch uses 12,018 bytes (41%) of program storage space. Maximum is 28,672 bytes.
 * Global variables use 1,069 bytes (41%) of dynamic memory, leaving 1,491 bytes for local variables. Maximum is 2,560 bytes.
 * 
 *** Board
 * Arduino Genuino/Micro
 * 
 * 
 *** KEYCHAIN REMOTE WIRING
 * brown - GND
 * white - DATA
 * black - CLK
 * blue - VCC
 * 
 */

#include <avr/sleep.h>
//#include <RF24.h>
#include <FastLED.h>
#include <EEPROM.h>
#include "btn.h"
#include "timer.h"
#include "sparkle_receiver.h"

//----------------------------------------------------------------------------------
// General stuff
//----------------------------------------------------------------------------------

#define NUM_LEDS                 300
#define FPS                       60
#define INITIAL_BRIGHTNESS       128
#define BRIGHTNESS_INCREMENT       1.2

#define PATTERN_CYCLE_TIME     12000
#define CHIPSET               APA102
#define COLOR_ORDER              BGR

#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))

#define DEBUG_SERIAL               0
#define DEBUG_BAUD              9600
//#define DEBUG_BAUD            115200
#define DEBUG_INTERVAL          1000

//----------------------------------------------------------------------------------
// Controller specific definitions
//----------------------------------------------------------------------------------

#define MAX_VOLTS      5
//#define MAX_mAMPS  10000
#define MAX_mAMPS   2000

#define LED1_SCK      15 // PB1
#define LED1_MOSI     16 // PB2

#define LED_EN_PIN     5  // PC6

#define BUTTON1_PIN    3
#define BUTTON2_PIN    2
#define BUTTON3_PIN    0
#define BUTTON4_PIN    1
#define MODE_PIN      BUTTON4_PIN


//----------------------------------------------------------------------------------
// Globals
//----------------------------------------------------------------------------------

CRGB leds[NUM_LEDS];
CRGB * leds2 = (leds + (NUM_LEDS/2));

Btn btn_mode(MODE_PIN);
Btn btn_brightness_up(BUTTON2_PIN);
Btn btn_brightness_down(BUTTON3_PIN);

uint8_t g_brightness = INITIAL_BRIGHTNESS;
uint32_t g_now = 0;

//----------------------------------------------------------------------------------
// LED Keychain
//----------------------------------------------------------------------------------
void leds_wake_up() { }
void leds_sleep() {

    //=================== SLEEP

    // fade LEDs
    for (int j = g_brightness; j > 0; j-=2) {
        FastLED.setBrightness(j);
        FastLED.show();
    }
    FastLED.clear();

    // disable I/O
    pinMode(LED1_SCK, INPUT);
    pinMode(LED1_MOSI, INPUT);
    digitalWrite(LED_EN_PIN, LOW);
    USBCON |= _BV(FRZCLK);  //freeze USB clock
    PLLCSR &= ~_BV(PLLE);   // turn off USB PLL
    USBCON &= ~_BV(USBE);   // disable USB
    delay(500);

    // go to sleep
    set_sleep_mode(SLEEP_MODE_PWR_DOWN);
    sleep_enable();
    attachInterrupt(0, leds_wake_up, LOW);
    sleep_mode();


    //=================== WAKE ON INTERRUPT
    sleep_disable();
    detachInterrupt(0);

    // reenable I/O
    sei();
    USBDevice.attach();
    delay(100);
    FastLED.clear();
    pinMode(LED1_SCK, OUTPUT);
    pinMode(LED1_MOSI, OUTPUT);
    digitalWrite(LED_EN_PIN, HIGH);
}

//----------------------------------------------------------------------------------
// Patterns
//----------------------------------------------------------------------------------

#include "blinkypants_patterns.h"
#include "fastled_patterns.h"
#include "tinybee_patterns.h"
#include "artemiid_patterns.h"
#include "patterns.h"
//#include "Fire2012.h"

//----------------------------------------------------------------------------------
// Mode / state
//----------------------------------------------------------------------------------

typedef int (*SimplePatternList[])();
uint8_t g_current_pattern = 0;
MillisTimer pattern_timer;

SimplePatternList patterns = {
  //Fire2012WithPalette,
  //plasma,
  test_leaf_registration,
  test_mimsy_registration,
  collision,
  confetti,
  moving_palette,
  rainbow,
  //sinelon,
  sinelonN,
  mode_yalda,
  fader_loop1,
  //fader_loop2,
  //fader_loop3,
};


void next_pattern()
{
    g_current_pattern = (g_current_pattern + 1) % ARRAY_SIZE( patterns);
    write_state();
    FastLED.clear();
}

void enable_autocycle() {
    if (pattern_timer.running()) return;    
    pattern_timer.start(PATTERN_CYCLE_TIME, true);
    next_pattern();  
}

void disable_autocycle() {
    if (!pattern_timer.running()) return;
    pattern_timer.stop();
    write_state();
}

void brightness_up() {
 switch (g_brightness) {
      case 0 ... 15: g_brightness += 1; break;
      case 16 ... 100: g_brightness += 15; break;
      case 101 ... (0xff - 30): g_brightness += 30; break;
      case (0xff - 30 + 1) ... 254: g_brightness = 255; break;
      case 255: break;
  } 
}

void brightness_down() {
  switch (g_brightness) {
      case 0: break;
      case 1 ... 15: g_brightness -= 1; break;
      case 16 ... 100: g_brightness -= 15; break;
      case 101 ... 255: g_brightness -= 30; break;
  }            
}

void mode_button() {
    if (pattern_timer.running()) {
      disable_autocycle();
    } else {
      next_pattern();
    }
}

void read_state() {
    uint8_t buffer = 0;
    // autocycle
    EEPROM.get(0, buffer);
    if (buffer) {
        pattern_timer.start(PATTERN_CYCLE_TIME, true);
    }
    
    // current pattern
    EEPROM.get(1, buffer);
    if (buffer == 255) {
      g_current_pattern = 0;
    } else {
      g_current_pattern = buffer % ARRAY_SIZE( patterns);
    }

    // brightness
    EEPROM.get(2, buffer);
    if (buffer == 255) {
      g_brightness = INITIAL_BRIGHTNESS;
    } else {
      g_brightness = buffer;
    }
    
}

void write_state() {
    EEPROM.update(0, pattern_timer.running());
    EEPROM.update(1, g_current_pattern);
    EEPROM.update(2, g_brightness);
}

//----------------------------------------------------------------------------------
//                                                                             SETUP
//----------------------------------------------------------------------------------

void setup() {
  delay(1000);
  pinMode(LED_EN_PIN, OUTPUT);
  digitalWrite(LED_EN_PIN, LOW);
  if (DEBUG_SERIAL) {
    Serial.begin(DEBUG_BAUD);
    delay(3000);
  }

  pinMode(BUTTON1_PIN, INPUT_PULLUP);
  pinMode(BUTTON2_PIN, INPUT_PULLUP);
  pinMode(BUTTON3_PIN, INPUT_PULLUP);
  pinMode(BUTTON4_PIN, INPUT_PULLUP);
  pinMode(LED_EN_PIN, OUTPUT);
  digitalWrite(LED_EN_PIN, LOW);
  delay(500);
  digitalWrite(LED_EN_PIN, HIGH);

  pinMode(LED1_SCK, OUTPUT);
  pinMode(LED1_MOSI, OUTPUT);
  
  // Leds
  FastLED.addLeds<CHIPSET, LED1_MOSI, LED1_SCK, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  //FastLED.setMaxPowerInVoltsAndMilliamps(MAX_VOLTS, MAX_mAMPS);

  read_state();
  delay(3000);
}


//----------------------------------------------------------------------------------
//                                                                              LOOP
//----------------------------------------------------------------------------------

uint32_t NOW = 0;
uint32_t STEPS = 0;
//uint32_t PATTERN_CYCLES = 0;
//uint32_t PATTERN_CYCLE_STEPS = 100000;
uint32_t fps = 0;
uint32_t LAST_TIME = NOW;
uint32_t LAST_STEPS = STEPS;

void loop() {
  g_now = millis();

  STEPS++;
  NOW = millis();

  if (DEBUG_SERIAL && ((NOW-LAST_TIME) > DEBUG_INTERVAL)) {

    fps = (STEPS-LAST_STEPS) * 1000 / (NOW-LAST_TIME);
    LAST_TIME = NOW;
    LAST_STEPS = STEPS;
      
    Serial.print("Millies: ");
    Serial.print(NOW);
    Serial.print("   Steps: ");
    Serial.print(STEPS);
    Serial.println();

    /*
    int b1 = digitalRead(BUTTON1_PIN);
    int b2 = digitalRead(BUTTON2_PIN);
    int b3 = digitalRead(BUTTON3_PIN);
    int b4 = digitalRead(BUTTON4_PIN);
    Serial.print("Buttons: ");
    Serial.print(b1); Serial.print(" ");
    Serial.print(b2); Serial.print(" ");
    Serial.print(b3); Serial.print(" ");
    Serial.print(b4); Serial.print(" ");
    Serial.println();

    Serial.print("Power (Actual/Max): ");
    Serial.print(calculate_unscaled_power_mW( leds, NUM_LEDS ) * g_brightness / 256);
    Serial.print(" / ");
    Serial.print(MAX_mAMPS);
    Serial.println();
    */
    
    Serial.print("Brightness (Actual/Max): ");
    Serial.print(g_brightness); 
    //Serial.print(" / ");
    //Serial.print(calculate_max_brightness_for_power_mW( 255, 1900 ));
    Serial.println();

    /*
    Serial.print("FPS (Target/Actual): ");
    Serial.print(FPS);
    Serial.print(" / ");
    Serial.print(fps);
    Serial.println();
    
    Serial.print("--------------------");
    Serial.println();
    */
  }


  
  // mode button. long press enables auto_cycle, short press changes to next pattern
  btn_mode.poll(
    []() {
      mode_button();
    },
    []() {
       enable_autocycle();
    }
  );

  random16_add_entropy( random());

  if (pattern_timer.fired()) {
    next_pattern();
  }



  // LED Keychain only buttons
  btn_brightness_up.poll(
        /* Brightness UP pressed */
        brightness_up,
        /* Brightness UP held */
        []() {
            if (g_brightness < 0xff) {
                g_brightness++;
                
            }
        }
    );

   btn_brightness_down.poll(
        /* Brightness DOWN pressed */
        brightness_down,
        /* Brightness DOWN held */
        []() {
            if (g_brightness > 0) {
                g_brightness--;
                
            }
        }
    );
  
  // Check for power off
  if (!digitalRead(BUTTON1_PIN)) {
     delay(10);
     if (!digitalRead(BUTTON1_PIN)) {
       leds_sleep();
     }
  }
 

  uint8_t brightness = g_brightness;
  /*
  if (pattern_timer.running()) {
    if (pattern_timer.sinceStart() < 300) {
        brightness = scale8(pattern_timer.sinceStart() * 256 / 300, g_brightness);
    }
    if (pattern_timer.untilDone() < 300) {
        brightness = scale8(pattern_timer.untilDone() * 256 / 300, g_brightness);
    }
  }
  */
  FastLED.setBrightness(brightness);

  //========================================================== RUN PATTERN
  uint32_t d = (patterns)[g_current_pattern]();
  FastLED.show();
  if (d == 0) d = 1000/FPS;
  FastLED.delay(d);
}
