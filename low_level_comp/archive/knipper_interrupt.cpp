#include <avr/io.h>
#include <avr/interrupt.h>

#define LED_PIN PB3

void setupTimer() {
    cli(); // Disable interrupts

    // Set prescaler to 1024 (CS12 = 1, CS11 = 0, CS10 = 1)
    TCCR1 |= (1 << CS13) | (1 << CS12) | (1 << CS11) | (1 << CS10);;

    // Enable CTC mode (CTC1 = 1)
    TCCR1 |= (1 << CTC1);
    int freq = 1;
    // Calculate OCR1A value to achieve an interrupt frequency of 1 Hz
    OCR1A = F_CPU / 16384.0 / freq - 1;
    
    OCR1C = OCR1A ;
    // Enable Output Compare Interrupt Enable bit OCIE1A in TIMSK
    TIMSK |= (1 << OCIE1A);

    sei(); // Enable interrupts
}

ISR(TIMER1_COMPA_vect) {
    // Toggle the LED pin by toggling the corresponding bit in the PORTB register
    PORTB ^= (1 << PORTB3);
}

int main() {
    // Set the bit for PB3 as output in the DDRB register
    DDRB |= (1 << LED_PIN);

    // Setup timer and interrupts
    setupTimer();

    // Stay in an infinite loop
    while (true) {}

    return 0;
}
