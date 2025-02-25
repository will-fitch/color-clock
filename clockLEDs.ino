#include <FastLED.h>

#define NUM_LEDS 57
#define LED_PIN 7
#define LEFT_SWITCH_PIN 2
#define RIGHT_SWITCH_PIN 3

const unsigned long MILLIS_PER_CYCLE = 720000;
const unsigned long MILLIS_PER_HUE = MILLIS_PER_CYCLE / 256;
const uint8_t MIN_VALUE = 30;
const uint8_t MAX_VALUE = 100;

CRGB leds[NUM_LEDS];
unsigned long noonMillis;
unsigned long switchOn;
int prevSwitchState;
int historicState;

void setup() {

  Serial.begin(9600);

  FastLED.addLeds<NEOPIXEL, LED_PIN>(leds, NUM_LEDS);
  pinMode(LEFT_SWITCH_PIN, INPUT_PULLUP);
  pinMode(RIGHT_SWITCH_PIN, INPUT_PULLUP);

  noonMillis = 0;
  switchOn = 0;
  prevSwitchState = getSwitchState();
  historicState = 0;

  long millisPerHour = MILLIS_PER_CYCLE / 12;
  //long millisPer20Minutes = MILLIS_PER_CYCLE / 36;

  for(int i = 0; i < 36; i++) {
    leds[35-i] = getColor((i/3) * millisPerHour);
  }

}

void loop() {

  unsigned long sysMillis = millis();
  unsigned long clockMillis = getClockMillis( sysMillis );

  int currentSwitchState = getSwitchState();

  if(currentSwitchState == 2) {
    if(switchOn == 0) {
      switchOn = sysMillis;
    }
    noonMillis = MILLIS_PER_CYCLE + sysMillis;
    noonMillis -= (MILLIS_PER_CYCLE/12)*(((sysMillis-switchOn)/2000)%12);
    noonMillis %= MILLIS_PER_CYCLE;
  } else if(currentSwitchState == 0) {
    if(switchOn == 0) {
      switchOn = sysMillis;
    }
    setOuterColors((sysMillis-switchOn)/500);
  } else {
    switchOn = 0;
    //setOuterColors(0);
  }

  printTime(clockMillis);
  setInnerColor(clockMillis);
}

// void handleSwitch(unsigned long sysMillis) {
//   uint8_t currentSwitchState = switchState();

//   if(currentSwitchState != prevSwitchState) {
//     switchOn = sysMillis;
//     if(currentSwitchState == 0) {
//       //0 init
//     } else if(currentSwitchState == 1) {
//       //1 init
//     } else if(currentSwitchState == 2) {
//       //2 init
//     }
//   }

//   unsigned long onTime = sysMillis - switchOn;

//   if(currentSwitchState == 0) {
//     //0 loop
//   } else if(currentSwitchState == 1) {
//     //1 loop
//   } else if (currentSwitchState == 2) {
//     //2 loop
//   }

//   prevSwitchState = currentSwitchState;
// }

void setOuterColors(int rotation) {
  long millisPerHour = MILLIS_PER_CYCLE / 12;

  for(int i = 0; i < 36; i++) {
    int index = (35-i+rotation)%36;
    leds[index] = getColor((i/3) * millisPerHour);
  }
}

void setInnerColor(unsigned long clockMillis) {
  CRGB odd = getColorAlternating(clockMillis, false);
  CRGB even = getColorAlternating(clockMillis, true);
  
  for(int i = 36; i <= 56; i++) {
    leds[i] = i%2==0? even : odd;
  }
  
  FastLED.show();
}

uint8_t getSwitchState() {
  int left = digitalRead(LEFT_SWITCH_PIN);
  int right = digitalRead(RIGHT_SWITCH_PIN);
  if(left == 0 && right == 1) {
    return 2;
  }
  if(left == 1 && right == 1) {
    return 1;
  }
  return 0;
}

CRGB getColor(unsigned long currentMillis) {
  return getColor(currentMillis, 70);
}

CRGB getColor(unsigned long currentMillis, uint8_t val) {
  currentMillis %= MILLIS_PER_CYCLE;
  uint8_t hue = (uint8_t)(currentMillis /= MILLIS_PER_HUE);

  return CRGB().setHSV(hue, 255, val);
}

CRGB getColorAlternating(unsigned long currentMillis, bool isEven) {
  currentMillis = currentMillis % MILLIS_PER_CYCLE;
  uint8_t hue = (uint8_t)(currentMillis / MILLIS_PER_HUE);
  
  unsigned long val = ( currentMillis % MILLIS_PER_HUE );
  val = rangeMap(val, 0, MILLIS_PER_HUE, MIN_VALUE, MAX_VALUE);

  if(hue % 2 == isEven) {
    hue += 1;
    hue %= 256;
  } else {
    val = MIN_VALUE+MAX_VALUE-val;
  }

  return CRGB().setHSV(hue, 255, (uint8_t)val);
}

unsigned long getClockMillis(unsigned long systemMillis) {
  systemMillis += MILLIS_PER_CYCLE;
  systemMillis -= noonMillis;
  systemMillis = systemMillis % MILLIS_PER_CYCLE;
  return systemMillis;
}

void printTime(unsigned long clockMillis) {
  clockMillis = clockMillis % MILLIS_PER_CYCLE;
  clockMillis = clockMillis * 12;
  float timeFloat = (1.0*clockMillis) / MILLIS_PER_CYCLE;
  int hours = (int)timeFloat;
  timeFloat = timeFloat - hours;
  int minutes = (int)(timeFloat*60);
  Serial.print(hours);
  Serial.print(":");
  if(minutes < 10) {
    Serial.print("0");
  }
  Serial.println(minutes);
}

unsigned long rangeMap(unsigned long value, unsigned long fromMin, unsigned long fromMax, unsigned long toMin, unsigned long toMax) {
  value -= fromMin;
  value *= (toMax - toMin);
  value /= (fromMax - fromMin);
  value += toMin;
  return value;
}
