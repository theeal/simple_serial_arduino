#include <Arduino.h>
//#include "src/SimpleSerial/src/SimpleSerial.h" // FOR TESTING ADD LIBRARY UNDER YOUR SKETCH
#include "SimpleSerial.h" // NORMAL LIBRAYY IN IDE

// Pin Definitions
const uint8_t BUTTON_PINS[] = {2, 3, 4, 5};
const uint8_t NUM_BUTTONS = sizeof(BUTTON_PINS) / sizeof(BUTTON_PINS[0]);

const uint8_t PACKET_ID_BUTTONS = 10;

SimpleSerial serial_link(&Serial1);

void setup() {
    Serial.begin(115200); // Debug Monitor (USB)
  
    Serial1.begin(115200);

    serial_link.begin();

    // Initialize button pins using internal pull-up resistors
    for (uint8_t i = 0; i < NUM_BUTTONS; i++) {
        pinMode(BUTTON_PINS[i], INPUT_PULLDOWN);
    }

    Serial.println("Sender Initialized with 4 Buttons.");
}

void loop() {
    serial_link.loop();

    static uint32_t last_tx = 0;
    if (millis() - last_tx >= 50) { // Transmit button states every 50 ms
        last_tx = millis();

        bool button_states[4];

        // Active LOW logic (Pressed = LOW = true)
        for (uint8_t i = 0; i < NUM_BUTTONS; i++) {
            button_states[i] = (digitalRead(BUTTON_PINS[i]) == HIGH);
        }

        // Pack the 4 button bool states into 1 byte and transmit
        serial_link.send_bool_array(PACKET_ID_BUTTONS, button_states, NUM_BUTTONS);
    }
}
