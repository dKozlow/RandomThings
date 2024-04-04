#include <avr/io.h>

// Definieer het bit voor pin PB3 (fysiek pin 2)
#define LED_PIN PB3

int main() {
    // Zet het bit voor PB3 als output in het DDRB-register
    DDRB |= (1 << LED_PIN);

    // Blijf in een oneindige lus
    while(1) {
        // Toggle de LED-pin door het betreffende bit te togglen in het PORTB-register
        PORTB ^= (1 << LED_PIN);

        // Pauzeer voor een bepaalde tijd (1 seconde in dit geval)
        for (long i = 0; i < F_CPU / 10; i++) {
            asm("nop"); // Doe niets (pauzeer)
        }
    }

    return 0;
}
