/*
  CPSC 599.88 | A0: Hello World | Idea
*/

#include <Arduino.h>

//Pin constants for buttons
const int p1button = 2;
const int p2button = 3;  

//Pin constants for shift register/IC
const int latchPin = 11;
const int dataPin = 12;
const int clockPin = 13;

//Pin constants for speaker
const int buzzerPin = 9;

// debounce tracker to avoid single presses triggering multiple times
unsigned long p1lastDebounceTime = 0;
unsigned long p2lastDebounceTime = 0;
const unsigned long debounceTime = 150;

// Volatile variables for interrupt button pushes;
volatile int p1halt = 0;
volatile int p2halt = 0;

// int array that holds the values for 8 led pins in the IC
int ledPinsIC[8];

//ints to hold score
int p1score = 0;
int p2score = 0;

int currentLED = 0b00000000;

int toggleVictory = 1;

void setLedOn(int byteToTurnOn);
void p1Press();
void p2Press();
void VictoryTheme();
void failSound();
void jackSound();

/// @brief  The setup
void setup() {
  // initialize the p1 button pin as a input:
  pinMode(p1button, INPUT);
  // initialize the p2 button pin as a input:
  pinMode(p2button, INPUT);
  // initialize the latch pin as an output:
  pinMode(latchPin, OUTPUT);
  // initialize the data pin as an output:
  pinMode(dataPin, OUTPUT);
  // initialize the clock pin as an output:
  pinMode(clockPin, OUTPUT);

  // initialize the buzzer pin as output:
  pinMode(buzzerPin, OUTPUT);

  //Setting up LED registry values to light up individual LEDS
  ledPinsIC[0] = 0b00000001;
  ledPinsIC[1] = 0b00000010;
  ledPinsIC[2] = 0b00000100;
  ledPinsIC[3] = 0b00001000;
  ledPinsIC[4] = 0b00010000;
  ledPinsIC[5] = 0b00100000;
  ledPinsIC[6] = 0b01000000;
  ledPinsIC[7] = 0b10000000;

  //Attaching interrupts to the buttons
  attachInterrupt(digitalPinToInterrupt(p1button), p1Press, FALLING );
  attachInterrupt(digitalPinToInterrupt(p2button), p2Press, FALLING );

  // initialize serial communication:
  Serial.begin(9600);
  setLedOn(0x00);

}

/// @brief The Game Loop
void loop() {

  // for(int i = 1; i < 7; i++) {
  //   setLedOn(ledPinsIC[i]);
  //   delay(300);
  // }

  if(p1halt) {
    p1halt = 0;
    currentLED = currentLED ^ ledPinsIC[1];
    setLedOn(currentLED);
  } else if(p2halt) {
    p2halt = 0;
    currentLED = currentLED ^ ledPinsIC[2];
    setLedOn(currentLED);
  }

  // tone(buzzerPin, 1000);
  if(currentLED == (ledPinsIC[1] | ledPinsIC[2])) {
    if(toggleVictory) {
      // VictoryTheme();
      jackSound();
      // failSound();
    }
    toggleVictory = 0;
  } else {
    toggleVictory = 1;
    noTone(buzzerPin);
  }
}

void failSound() {
    tone(buzzerPin, 587.33, 400);
    delay(400);
    tone(buzzerPin, 554.37 , 400);
    delay(400);
    tone(buzzerPin, 523.25, 400);
    delay(400);
    for(int i = 0; i < 7; i++) {
    tone(buzzerPin, 493.88, 100);
      delay(100);
      tone(buzzerPin, 987.77, 100);
      delay(100);
    }
}

void playSound(unsigned int freq, unsigned long duration, unsigned long del) {
    tone(buzzerPin, freq, duration);
    delay(del);
}

void jackSound() {
  unsigned int C = 1046.5;
  unsigned int B = 987.77;
  unsigned int A = 880.00;
  unsigned int G = 783.99;
  unsigned int F = 698.46;
  unsigned int E = 659.25;
  unsigned int D = 587.33;
  unsigned int C_ = 523.25; 
  unsigned int B_ = 493.88;
  unsigned int A_ = 440.00;
  unsigned int G_ = 392.00; 
  unsigned int F_ = 349.23;
  unsigned int E_ = 329.63;
  unsigned int D_ = 293.66;

  playSound(C_, 300, 400);

  playSound(F, 300, 400);
  playSound(F, 100, 200);
  playSound(G, 300, 400);
  playSound(G, 300, 400);

  playSound(A, 300, 400);
  playSound(C, 300, 400);
  playSound(A, 100, 200);
  playSound(F, 300, 400);
  playSound(C_, 300, 400);

    /**
     * C
     * 
     * F
     * F
     * G
     * G
     * 
     * A
     * C^
     * A
     * F
     * C
     * 
     * F
     * F
     * G
     * G
     * 
     * A
     * F
     * C
     * 
     * F
     * F
     * G
     * G
     * 
     * A
     * C^
     * A
     * F
     * 
     * FFF
     * G
     * B
     * 
     * A
     * 
     * F
     */
}

//From https://www.youtube.com/watch?v=iY98jcuKh5E
void VictoryTheme() {
// Victory theme
// D - 587.33Hz
// C - 523.25Hz 
// B - 493.88Hz
// A - 440.00Hz
// G - 392.00Hz 
// F - 349.23Hz
// E - 329.63Hz
// D - 293.66Hz
// Db - 554.37 

// Ab - 415.30Hz / 830.6Hz
// Bb - 466.16Hz / 932.33Hz
// 1600ms/bar in 4/4
    tone(buzzerPin, 523.25, 133);
    delay(133);
    tone(buzzerPin, 523.25, 133);
    delay(133);
    tone(buzzerPin, 523.25, 133);
    delay(133);
    tone(buzzerPin, 523.25, 400);
    delay(400);
    tone(buzzerPin, 415.30, 400);
    delay(400);
    tone(buzzerPin, 466.16, 400);
    delay(400);
    tone(buzzerPin, 523.25, 133);
    delay(133);
    delay(133);
    tone(buzzerPin, 466.16, 133);
    delay(133);
    tone(buzzerPin, 523.25, 1200);
    delay(1200);
}

/// @brief Function to turn on an LED
/// @param byteToTurnOn the int corresponding to LED pins in the IC
void setLedOn(int byteToTurnOn) {
    digitalWrite(latchPin, LOW);
    shiftOut(dataPin, clockPin, MSBFIRST, byteToTurnOn);
    digitalWrite(latchPin, HIGH);
}

/// @brief Handles player 1 button press.
void p1Press() {
  // if(!p2halt) {
  if(true) {
    if(p1lastDebounceTime + debounceTime <= millis()) {
      p1halt = 1;
      p1lastDebounceTime = millis();
    }
  }
}

/// @brief Handles player 2 button press.
void p2Press() {
  // if(!p1halt) {
  if(true) {
    if(p2lastDebounceTime + debounceTime <= millis()) {
      p2halt = 1;
      p2lastDebounceTime = millis();
    }
  }
}
