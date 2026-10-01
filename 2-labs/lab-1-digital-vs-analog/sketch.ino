/*
  Lab 1 - Digital vs Analog
  ESP32 + LED (digital out) + push button (digital in) + potentiometer (analog in)

  What this shows:
    - Digital signals only have two states: HIGH (on) or LOW (off).
      The LED and the button are both digital.
    - Analog signals are a range of values.
      The potentiometer (a dial/knob) is analog: turning it gives every
      value in between, not just "on" or "off".
*/

const int LED_PIN = 2;    // digital output
const int BUTTON_PIN = 15; // digital input
const int POT_PIN = 34;    // analog input (ADC pin)

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);

  // INPUT_PULLUP means the pin reads HIGH by default, and LOW when the
  // button is pressed and connects the pin to GND. This is the standard,
  // reliable way to wire a button.
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  Serial.println("Lab 1 started. Try:");
  Serial.println(" - Watching the LED blink on its own (digital output)");
  Serial.println(" - Pressing the button (digital input)");
  Serial.println(" - Dragging the potentiometer dial (analog input)");
}

void loop() {
  // --- 1. Digital OUTPUT: blink the LED ---
  digitalWrite(LED_PIN, HIGH);
  delay(300);
  digitalWrite(LED_PIN, LOW);
  delay(300);

  // --- 2. Digital INPUT: read the button ---
  // Because of INPUT_PULLUP, "pressed" reads as LOW, not HIGH.
  int buttonState = digitalRead(BUTTON_PIN);
  if (buttonState == LOW) {
    Serial.println("Button: PRESSED");
  } else {
    Serial.println("Button: released");
  }

  // --- 3. Analog INPUT: read the potentiometer ---
  // analogRead() on the ESP32 returns a number from 0 to 4095
  // (that's a 12-bit ADC: 2^12 = 4096 possible values).
  int potRaw = analogRead(POT_PIN);

  // Convert the raw number into something meaningful: a percentage.
  float potPercent = (potRaw / 4095.0) * 100.0;

  Serial.print("Potentiometer raw: ");
  Serial.print(potRaw);
  Serial.print("   ->   ");
  Serial.print(potPercent, 1);
  Serial.println(" %");

  Serial.println("---");
}
