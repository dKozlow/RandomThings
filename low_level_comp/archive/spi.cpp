#include <avr/io.h>
#include <util/atomic.h>

void initSPI() {
    // Configure SPI pins: SCLK (PB2), MOSI (PB0), MISO (PB1)
    DDRB |= (1 << DDB2); // Set SCLK pin (PB2) as output
    DDRB |= (1 << DDB0); // Set MOSI pin (PB0) as output
    DDRB &= ~(1 << DDB1); // Set MISO pin (PB1) as input
    // Set up SPI control register (USICR)
    USICR = (1 << USIWM0) |   // Enable 3-wire mode
            (1 << USICS1) | (1 << USICS0) |   // External clock, positive edge
            (1 << USICLK);  // Clock source is external
    
}
uint8_t transferSPI(uint8_t data) {
    // Zet de te verzenden data in USI data register
    USIDR = data;

    // Genereer klokpulsen om data te versturen en ontvangen
    for (int i = 0; i < 8; ++i) {
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
            USICR |= (1 << USITC); // Toggle klok (16 keer in totaal)
        }
    }

    // Return de ontvangen data
    return USIDR;
}

int main() {
    initSPI();

    // Voorbeeld: verstuur 0xAA via SPI en ontvang de ontvangen data
    uint8_t receivedData = transferSPI(0xFA);

    // Blijf in een oneindige lus
    while (1) {}

    return 0;
}
