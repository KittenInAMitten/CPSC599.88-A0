/*
  CPSC 599.88 | A0: Hello World | Idea
*/

#include <Arduino.h>

// Pin constants for buttons
const int p1button = 2;
const int p2button = 3;

// Pin constants for shift register/IC
const int latchPin = 11;
const int dataPin = 12;
const int clockPin = 13;

// Pin constants for LEDs
const int ledBPin = 4;

// Pin constants for speaker
const int buzzerPin = 9;

// Debounce tracker to avoid single presses triggering multiple times
volatile unsigned long p1lastDebounceTime = 0;
volatile unsigned long p2lastDebounceTime = 0;
const unsigned long debounceTime = 250;

// Volatile variables for interrupt button pushes;
volatile int p1halt = 0;
volatile int p2halt = 0;

// Unused ledPinsIC array
// // int array that holds the values for 8 led pins in the IC
// int ledPinsIC[8];

// ints to hold score
int p1score = 0;
int p2score = 0;

// int to hold the current LED configuration for the IC
int currentLED = 0b00000000;

// unsigned longs for sound delay adding
unsigned long sd = 0;
unsigned long sdAdd = 0;

// constant song multiplier to speed up or slow down the song
const float sm = 0.3;

// state flags
int reactGo = 0;
int countingDown = 0;
volatile int fail = 0;

// blink assist toggle tracker
int blinkToggle = 0;

// last note time to track how long the delay is
unsigned long lastNoteTime = 0;

void setLedOn(int byteToTurnOn);
unsigned int getScoreAsByte();
void setScoreLeds();
void victoryFanfare();
void failSound();
void jackSound();
void blinkLedPToggle();
void blinkLedToggle();
void blinkLedOn();
void blinkLedOff();
void p1Press();
void p2Press();

/// @brief  The setup
void setup()
{
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
    // initialize the led blue pin as an output:
    pinMode(ledBPin, OUTPUT);

    // initialize the buzzer pin as output:
    pinMode(buzzerPin, OUTPUT);

    // Unused lendPinsIC array
    // // Setting up LED registry values to light up individual LEDS
    // ledPinsIC[0] = 0b00000001;
    // ledPinsIC[1] = 0b00000010;
    // ledPinsIC[2] = 0b00000100;
    // ledPinsIC[3] = 0b00001000;
    // ledPinsIC[4] = 0b00010000;
    // ledPinsIC[5] = 0b00100000;
    // ledPinsIC[6] = 0b01000000;
    // ledPinsIC[7] = 0b10000000;

    // Attaching interrupts to the buttons
    attachInterrupt(digitalPinToInterrupt(p1button), p1Press, FALLING);
    attachInterrupt(digitalPinToInterrupt(p2button), p2Press, FALLING);

    // initialize serial communication:
    Serial.begin(9600);
    setLedOn(0x00);

    randomSeed(analogRead(0));
}

/// @brief The Game Loop
void loop()
{
    if (reactGo)
    {
        countingDown = 0;
        if (p1halt || p2halt)
        {
            reactGo = 0;
            if (p1halt)
            {
                p1score = p1score + 1;
            }
            else if (p2halt)
            {
                p2score = p2score + 1;
            }
            fail = 0;
            setScoreLeds();
            noTone(buzzerPin);
            victoryFanfare();
            p1halt = 0;
            p2halt = 0;
        }
        blinkLedOn();
        tone(buzzerPin, 698.46);
        lastNoteTime = 0;
    }
    else if (fail)
    {
        countingDown = 0;
        reactGo = 0;
        failSound();
        noTone(buzzerPin);
        fail = 0;
        p1halt = 0;
        p2halt = 0;
        lastNoteTime = 0;
    }
    else
    {
        // tone(buzzerPin, 1000);
        if (p1halt && p2halt)
        {
            fail = 0;
            currentLED = 0x00;
            p1lastDebounceTime = millis();
            p2lastDebounceTime = millis();
            p1halt = 0;
            p2halt = 0;
            reactGo = 0;
            if (p1score >= 3 || p2score >= 3)
            {
                p1score = 0;
                p2score = 0;
                setScoreLeds();
            }
            countingDown = 1;
            jackSound();
        }
        else
        {
            if(millis() >= lastNoteTime + 400) {
                if(p1score >= 3 || p2score >= 3) {    
                    blinkLedToggle();
                }
                if(p1halt) {
                    currentLED = currentLED ^ 0b00001110;
                    setLedOn(currentLED);
                } else if(p2halt) {
                    currentLED = currentLED ^ 0b11100000;
                    setLedOn(currentLED);
                } else {
                    setScoreLeds();
                }
                lastNoteTime = millis();
            }
            noTone(buzzerPin);
            countingDown = 0;
        }
    }
}

unsigned int getScoreAsByte()
{

    unsigned int ledp1 = 0;
    unsigned int ledp2 = 0;

    for (int i = 0; i < min(p1score, 3); i++)
    {
        ledp1 = (unsigned int)1 << 1 | ledp1 << 1;
    }

    for (int i = 0; i < min(p2score, 3); i++)
    {
        ledp2 = ledp2 << 1 | (unsigned int)1;
    }
    ledp2 = ledp2 << (3 - p2score);

    unsigned int ledsToTurnOn = (ledp2 << 5) | ledp1;

    return ledsToTurnOn;
}

void setScoreLeds()
{
    if(p1score < 3 && p2score < 3)
    {
        blinkLedOff();
    }

    setLedOn(getScoreAsByte());
}

void failSound()
{
    setLedOn(0b00001110);
    tone(buzzerPin, 1174.66, 300);
    delay(175);
    setLedOn(0x0);
    delay(175);
    setLedOn(0b00001110);
    tone(buzzerPin, 1108.73, 300);
    delay(175);
    setLedOn(0x0);
    delay(175);
    setLedOn(0b00001110);
    tone(buzzerPin, 1046.50, 300);
    delay(175);
    setLedOn(0x0);
    delay(175);
    setLedOn(0b00001110);
    for (int i = 0; i < 7; i++)
    {
        tone(buzzerPin, 493.88, 75);
        delay(88);
        tone(buzzerPin, 987.77, 75);
        delay(88);
        if (i % 2 == 0)
        {
            setLedOn(0x0);
        }
        else
        {

            setLedOn(0b00001110);
        }
    }
    setLedOn(0x0);
}

void playSound(unsigned int freq, unsigned long duration, unsigned long del)
{
    if (!fail)
    {
        blinkToggle = !blinkToggle;
        if (blinkToggle)
        {
            setLedOn(random(2, 256));
        }
        else
        {
            setLedOn(0);
        }

        tone(buzzerPin, freq, sm * (duration + sd));
        delay(sm * (del + sd));
        sd = sd + sdAdd;
    }
}

void jackSound()
{
    unsigned int D__ = 1174.66;
    unsigned int C = 1046.5;
    // unsigned int B = 987.77;
    unsigned int A = 880.00;
    unsigned int G = 783.99;
    unsigned int F = 698.46;
    // unsigned int E = 659.25;
    // unsigned int D = 587.33;
    unsigned int C_ = 523.25;
    // unsigned int B_ = 493.88;
    // unsigned int A_ = 440.00;
    // unsigned int G_ = 392.00;
    // unsigned int F_ = 349.23;
    // unsigned int E_ = 329.63;
    // unsigned int D_ = 293.66;
    unsigned int Bb = 932.33;

    unsigned long lastNoteDelay = random(1500, 6500);
    sdAdd = random(15, 30);

    sd = 0;

    // Manually converted https://musescore.com/dakook_music/jack-in-the-box/piano-tutorial to frequencies and delays.

    // C_
    playSound(C_, (350), (400));

    // FF GG
    playSound(F, (350), (400));
    playSound(F, (150), (200));
    playSound(G, (350), (400));
    playSound(G, (150), (200));

    // ACA F
    playSound(A, (150), (200));
    playSound(C, (150), (200));
    playSound(A, (150), (200));
    playSound(F, (350), (400));

    // C_
    playSound(C_, (150), (200));

    // FF GG
    playSound(F, (350), (400));
    playSound(F, (150), (200));
    playSound(G, (350), (400));
    playSound(G, (150), (200));

    // A  F
    playSound(A, (550), (600));
    playSound(F, (350), (400));

    // C_
    playSound(C_, (150), (200));

    // FF GG
    playSound(F, (350), (400));
    playSound(F, (150), (200));
    playSound(G, (350), (400));
    playSound(G, (150), (200));

    // ACA F
    playSound(A, (150), (200));
    playSound(C, (150), (200));
    playSound(A, (150), (200));
    playSound(F, (350), (450));

    // FFFFF G Bb (Can't do chord with one buzzer :c)
    playSound(D__, (500), (550));
    playSound(G, (350), (400));
    playSound(Bb, (150), (200));

    // A
    playSound(A, (550), (600));

    digitalWrite(ledBPin, LOW);
    setLedOn(0);
    lastNoteTime = millis();
    while (millis() <= lastNoteTime + lastNoteDelay)
    {
        if (fail)
            break;
    }
    if (!fail)
    {
        reactGo = 1;
    }
    else
    {
        reactGo = 0;
    }
}

// From https://www.youtube.com/watch?v=iY98jcuKh5E
void victoryFanfare()
{
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
    sd = 0;

    delay(150);

    blinkLedPToggle();

    tone(buzzerPin, 523.25, 133);
    delay(133);
    tone(buzzerPin, 523.25, 133);
    delay(133);
    tone(buzzerPin, 523.25, 133);
    delay(133);
    blinkLedPToggle();
    tone(buzzerPin, 523.25, 400);
    delay(400);
    blinkLedPToggle();
    tone(buzzerPin, 415.30, 400);
    delay(400);
    blinkLedPToggle();
    tone(buzzerPin, 466.16, 400);
    delay(400);
    blinkLedPToggle();
    tone(buzzerPin, 523.25, 133);
    delay(133);
    delay(133);
    tone(buzzerPin, 466.16, 133);
    delay(133);
    blinkLedPToggle();
    tone(buzzerPin, 523.25, 1200);
    delay(133);
    delay(266);
    blinkLedPToggle();
    delay(133);
    delay(266);
    blinkLedPToggle();
    delay(133);
    delay(266);
    blinkLedPToggle();
    delay(133);
    delay(266);
    blinkLedPToggle();
}

/// @brief Function to turn on an LED
/// @param byteToTurnOn the int corresponding to LED pins in the IC
void setLedOn(int byteToTurnOn)
{
    digitalWrite(latchPin, LOW);
    shiftOut(dataPin, clockPin, MSBFIRST, byteToTurnOn);
    digitalWrite(latchPin, HIGH);
}

void blinkLedPToggle()
{
    blinkToggle = !blinkToggle;
    if(blinkToggle) {
        blinkLedOn();
        if (p1halt)
        {
            setLedOn(0b11101110 & getScoreAsByte());
        }
        else if (p2halt)
        {
            setLedOn(0b11101110 & getScoreAsByte());
        }
    } else {
        blinkLedOff();
        if(p1halt) {
            setLedOn(0b11100000 & getScoreAsByte());
        } else if(p2halt) {
            setLedOn(0b00001110 & getScoreAsByte());
        }
    }
}

void blinkLedToggle() {
    blinkToggle = !blinkToggle;
    if(blinkToggle) {
        digitalWrite(ledBPin, HIGH);
    } else {
        digitalWrite(ledBPin, LOW);
    }
}

void blinkLedOn()
{
    digitalWrite(ledBPin, HIGH);
}

void blinkLedOff()
{
    digitalWrite(ledBPin, LOW);
}

/// @brief Handles player 1 button press.
void p1Press()
{
    if (!countingDown && !reactGo)
    {
        p1halt = 1;
    }
    else if (!p2halt && (countingDown || reactGo))
    {
        if (p1lastDebounceTime + debounceTime <= millis())
        {
            if (countingDown)
            {
                fail = 1;
            }
            p1halt = 1;
            p1lastDebounceTime = millis();
        }
    }
}

/// @brief Handles player 2 button press.
void p2Press()
{
    if (!countingDown && !reactGo)
    {
        p2halt = 1;
    }
    else if (!p1halt && (countingDown || reactGo))
    {
        if (p2lastDebounceTime + debounceTime <= millis())
        {
            if (countingDown)
            {
                fail = 1;
            }
            p2halt = 1;
            p2lastDebounceTime = millis();
        }
    }
}
