#include <Adafruit_NeoPixel.h>

#define LED_PIN 4         // for LED strip data
#define NUM_LEDS 6        // length of strip
#define BUTTON_PIN 2      // for momentary switch
#define DEBOUNCE_DELAY 5000 // 5-second buffer to prevent multiple triggers

Adafruit_NeoPixel strip(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);

enum Mode { OFF, GREEN_BREATH, YELLOW_BREATH, RED_BREATH, DYING };
Mode currentMode = OFF;

unsigned long lastPressTime = 0;  // to track last button press

void setColor(int r, int g, int b);
void cycleMode();
void breathingEffect(int r, int g, int b, int cycleTime, bool ragged = false);
void dyingEffect();
int randomizeCycleTime(int cycleTime);

void setup() {
    pinMode(BUTTON_PIN, INPUT_PULLUP); // internal pull-up resistor for button
    strip.begin();
    strip.show(); // LEDs off initially
    randomSeed(analogRead(0));

    // startup sequence: blink between white and yellow 3 times
    for (int i = 0; i < 3; i++) {
        setColor(255, 255, 255);  // white
        delay(200);
        setColor(255, 255, 0);    // yellow
        delay(200);
    }
    setColor(0, 0, 0); // LEDs off
}

void loop() {
    // check if button is pressed and respect the debounce time
    if (digitalRead(BUTTON_PIN) == LOW && millis() - lastPressTime > DEBOUNCE_DELAY) {
        lastPressTime = millis();  // update last press time
        cycleMode(); // change to next mode
    }

    // execute the current mode
    switch (currentMode) {
        case GREEN_BREATH:
            breathingEffect(0, 255, 0, randomizeCycleTime(4000));  // Green, slow breathing (15 BPM)
            break;
        case YELLOW_BREATH:
            breathingEffect(255, 255, 0, randomizeCycleTime(3000)); // Yellow, slightly faster (20 BPM)
            break;
        case RED_BREATH:
            breathingEffect(255, 0, 0, randomizeCycleTime(1700), true); // Red, ragged breathing (35 BPM)
            break;
        case DYING:
            dyingEffect(); // final erratic flicker and fade out
            currentMode = OFF; // ensure lights stay off after dying
            break;
        default:
            break;
    }
}

// hardcoded order
void cycleMode() {
    switch (currentMode) {
        case OFF: currentMode = GREEN_BREATH; break;
        case GREEN_BREATH: currentMode = YELLOW_BREATH; break;
        case YELLOW_BREATH: currentMode = RED_BREATH; break;
        case RED_BREATH: currentMode = DYING; break;
        case DYING: currentMode = OFF; break;
    }
}

// randomize Cycle Time by ±10%
int randomizeCycleTime(int cycleTime) {
    return cycleTime + random(-cycleTime / 10, cycleTime / 10 + 1);
}

void breathingEffect(int r, int g, int b, int cycleTime, bool ragged = false) {
    int step = ragged ? 30 : 5; // controls smoothness vs. roughness
    int halfCycle = cycleTime / 2; // inhale or exhale duration
    
    int numSteps = 256 / step; // number of steps for inhale/exhale
    int stepDelay = halfCycle / numSteps; // delay per brightness step

    for (int i = 0; i < 256; i += step) {
        setColor((r * i) / 255, (g * i) / 255, (b * i) / 255);
        delay(stepDelay);
    }
    for (int i = 255; i >= 0; i -= step) {
        setColor((r * i) / 255, (g * i) / 255, (b * i) / 255);
        delay(stepDelay);
    }
}

void dyingEffect() {
    unsigned long startTime = millis();
    while (millis() - startTime < 15000) {  // 15-second sequence
        int flicker = random(50, 255);
        setColor(flicker, 0, 0); // random flickering red
        delay(random(100, 400)); // random flicker timing

        // occasionally do a very slow breath
        if (random(0, 10) > 7) {
            breathingEffect(100, 0, 0, randomizeCycleTime(2500), true);
        }
    }

    // final slow fade out
    for (int i = 255; i >= 0; i -= 5) {
        setColor(i, 0, 0);
        delay(50);
    }
    setColor(0, 0, 0); // LEDs off
}

void setColor(int r, int g, int b) {
    for (int i = 0; i < NUM_LEDS; i++) {
        strip.setPixelColor(i, strip.Color(r, g, b));
    }
    strip.show();
}
