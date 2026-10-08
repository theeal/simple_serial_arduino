#include <Arduino.h>
//#include "src/SimpleSerial/src/SimpleSerial.h" // FOR TESTING ADD LIBRARY UNDER YOUR SKETCH
#include "SimpleSerial.h" // NORMAL LIBRAYY IN IDE

const uint8_t NUM_LEDS = 32;
const uint8_t PACKET_ID_BUTTONS = 10;

// Example array of 32 output pins driving LEDs (Adjust pins for your target MCU)
const uint8_t LED_PINS[NUM_LEDS] = {
    2, 3, 4, 5, 6, 7, 8, 9,
    10, 11, 12, 13, 14, 15, 18, 19,
    21, 22, 23, 25, 26, 27, 32, 33,
    34, 35, 36, 39, 40, 41, 42, 43
};

// Auto-reset timeout set to 1000ms (1 second)
SimpleSerial serial_link(&Serial1, 1000);

void turn_off_all_leds() {
    for (uint8_t i = 0; i < NUM_LEDS; i++) {
        digitalWrite(LED_PINS[i], LOW);
    }
}

void setup() {
    Serial.begin(115200); //USB

    Serial1.begin(115200);


    serial_link.begin();

    // Set pin modes for 32 LEDs
    for (uint8_t i = 0; i < NUM_LEDS; i++) {
        pinMode(LED_PINS[i], OUTPUT);
        digitalWrite(LED_PINS[i], LOW);
    }

    Serial.println("Receiver Ready for 32 LEDs with Timeout Auto-Reset.");
}

void loop() {
    serial_link.loop();

    // Safety handling: If no valid stream received within 1 second, turn off all 32 LEDs
    if (serial_link.hasTimedOut()) {
        turn_off_all_leds();
    }

    // Process arriving serial frames
    if (serial_link.available()) {
        SimpleSerial::Packet pkt = serial_link.read();

        if (pkt.id == PACKET_ID_BUTTONS) {
            bool leds_state[NUM_LEDS] = {false};

            // Unpack up to 4 bytes back into 32 boolean pin states
            byte_conversion::bytes_to_bool_array(pkt.payload, leds_state, NUM_LEDS);

            // Drive all 32 LED outputs
            for (uint8_t i = 0; i < NUM_LEDS; i++) {
                digitalWrite(LED_PINS[i], leds_state[i] ? HIGH : LOW);
            }
        }
    }
}
