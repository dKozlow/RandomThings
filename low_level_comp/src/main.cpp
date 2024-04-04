#define SHIFT_REGISTER_CS_PIN PB3
#define BMP280_CS_PIN PB4

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <Arduino.h>

void displayHexValueAsDecimal(uint8_t hexValue);
void writeDigit(int digit);
void setupBMP280();
void spi_init(void);
void setupTimer();
uint8_t spi_recieve(void);
uint16_t read16(uint8_t reg);
void spi_transfer(uint8_t data);
long read32();
void writeDot();
int16_t spi_receive16S(void);

uint16_t T1;
int16_t T2, T3;
uint16_t P1;
int16_t P2, P3, P4, P5, P6, P7, P8, P9;

int32_t t_fine; // Global variable to store t_fine value

void setupBMP280() {
    // Schakel de sensor in normale modus met oversampling instellingen
    PORTB &= ~(1 << BMP280_CS_PIN); // CS laag om communicatie te starten
    spi_transfer(0x74); // Ctrl_meas register
    spi_transfer(0b01000011); // Normale modus, temperatuur oversampling x2
    PORTB |= (1 << BMP280_CS_PIN); // CS hoog om communicatie te beëindigen
}

void setupPins(void) {
    DDRB |= (1 << PB2) | (1 << PB1) | (1 << SHIFT_REGISTER_CS_PIN) | (1 << BMP280_CS_PIN);// Set SCLK, MOSI, and CS as output
    DDRB &= ~(1 << PB0);// Set MISO as input
    PORTB |= (1 << SHIFT_REGISTER_CS_PIN) | (1 << BMP280_CS_PIN);// Set CS high to disable the shift register and BMP280
    USICR = (1 << USIWM0) | (1 << USICS1) | (1 << USICLK);// Enable 3-wire mode, external clock, positive edge
}
void setupTimer() {
    cli(); // Disable interrupts
    // Set prescaler to 16384 (CS13 = 1, CS12 = 1, CS11 = 1, CS10 = 1)
    TCCR1 |= (1 << CS13) | (1 << CS12) | (1 << CS11) | (1 << CS10);
    // Enable CTC mode (CTC1 = 1)
    TCCR1 |= (1 << CTC1);
    // Calculate OCR1A value to achieve an interrupt frequency of 0.1 Hz (10 seconds)
    OCR1A = 195;
    // Set OCR1C for the same value to match OCR1A, ensuring timer resets correctly
    OCR1C = OCR1A;
    // Enable Output Compare Interrupt Enable bit OCIE1A in TIMSK
    TIMSK |= (1 << OCIE1A);
    sei(); // Enable interrupts
}
void displayTemperature(float temp) {
    // Convert the float temperature to an integer for display purposes
    int tens = (int)(temp / 10);
    int ones = (int)temp % 10;
    int tenths = (int)(temp * 10) % 10;
    int hundredths = (int)(temp * 100) % 10;
    // Display logica
    writeDigit(tens);
    _delay_ms(500);
    writeDigit(ones);
    _delay_ms(500);
    writeDot();
    _delay_ms(500);
    writeDigit(tenths);
    _delay_ms(500);
    writeDigit(hundredths);
    _delay_ms(500);
    writeDigit(11);// Blanco display
}
void spi_transfer(uint8_t data) {
    USIDR = data;
    USISR = (1 << USIOIF); // Clear counter and counter overflow interrupt flag
    while ((USISR & (1 << USIOIF)) == 0) {// Wait for the transfer to complete
        USICR |= (1 << USITC);
    }
}
uint8_t spi_recieve(void) {
    USIDR = 0x00;
    USISR = (1 << USIOIF); // Clear counter and counter overflow interrupt flag
    while ((USISR & (1 << USIOIF)) == 0) {// Wait for the transfer to complete
        USICR |= (1 << USITC);
    }
    return USIDR;// Return the received data
}
uint16_t spi_receive16(void) {// Read a 16-bit value from the sensor
    uint8_t lowByte = spi_recieve();
    uint8_t highByte = spi_recieve();

    return (uint16_t)highByte << 8 | lowByte;// Combine the two bytes into a 16-bit value
}
int16_t spi_receive16S(void) {
    uint16_t received = spi_receive16(); // Use the existing function to get the data
    int16_t signedValue = (int16_t)received;// Convert the unsigned value to a signed value
    return signedValue;
}
// long spi_receive20(void) {
//     uint8_t byte1 = spi_recieve(); // Lees het eerste byte
//     uint8_t byte2 = spi_recieve(); // Lees het tweede byte
//     uint8_t byte3 = spi_recieve(); // Lees het derde byte, gebruik alleen de bovenste 4 bits

//     // Combineer de drie bytes in een 20-bit waarde
//     long result = ((long)byte1 << 12) | ((long)byte2 << 4) | ((long)byte3 >> 4);
//     return result;
// }
uint8_t bmp280_read_register(uint8_t reg) {// Read a register from the BMP280 ID
    PORTB &= ~(1 << BMP280_CS_PIN); 
    spi_transfer(0xD0);
    uint8_t data = spi_recieve();
    PORTB |= (1 << BMP280_CS_PIN);
    if(data == 0x58){
        writeDigit(1);
    }
    else
    {
        writeDigit(0);
    }
}

void readCoeff() {
    PORTB &= ~(1 << BMP280_CS_PIN); // CS low
    spi_transfer(0x88); //
    T1 = spi_receive16();
    T2 = spi_receive16S();
    T3 = spi_receive16S();
    P1 = spi_receive16();
    P2 = spi_receive16S();
    P3 = spi_receive16S();
    P4 = spi_receive16S();
    P5 = spi_receive16S();
    P6 = spi_receive16S();
    P7 = spi_receive16S();
    P8 = spi_receive16S();
    PORTB |= (1 << BMP280_CS_PIN); // CS high
}

float readTemp(){
    long adc_T;// Read the temperature data from the sensor
    PORTB &= ~(1 << BMP280_CS_PIN); // Start communication
    spi_transfer(0xFA); // Send the temperature data register address
    adc_T = (long)spi_recieve() << 12;  // MSB
    adc_T |= (long)spi_recieve() << 4;   // LSB
    adc_T |= ((long)spi_recieve() & 0x0F);  // XLSB
    PORTB |= (1 << BMP280_CS_PIN); // End communication
    long var1 = (((( adc_T >> 3) - (( long ) T1 << 1) ) ) * (( long ) T2 ) ) >> 11;// Calculate the temperature
    long var2 = ((((( adc_T >> 4) - (( long ) T1 ) ) * (( adc_T >> 4) - (( long ) T1 ) ) ) >> 12) * ((long ) T3 ) ) >> 14;//
    t_fine = var1 + var2;

    float T= (t_fine * 5 + 128) >> 8;
    return T/100;// Return the temperature in degrees Celsius
}
float readPressure() {
    readTemp(); // Make sure t_fine is calculated
    long adc_P;
    PORTB &= ~(1 << BMP280_CS_PIN); // Start communication
    spi_transfer(0xF7); // Send the pressure data register address
    adc_P = (long)spi_recieve() << 12; // MSB
    adc_P |= (long)spi_recieve() << 4; // LSB
    adc_P |= (long)spi_recieve() >> 4; // XLSB
    PORTB |= (1 << BMP280_CS_PIN); // End communication

    int64_t var1, var2, p;
    var1 = ((int64_t)t_fine) - 128000;
    var2 = var1 * var1 * (int64_t)P6;
    var2 = var2 + ((var1 * (int64_t)P5) << 17);
    var2 = var2 + (((int64_t)P4) << 35);
    var1 = ((var1 * var1 * (int64_t)P3) >> 8) + ((var1 * (int64_t)P2) << 12);
    var1 = (((((int64_t)1) << 47) + var1)) * (int64_t)P1 >> 33;
    if (var1 == 0) {
        return 0; // avoid exception caused by division by zero
    } 
    p = 1048576 - adc_P;
    p = (((p << 31) - var2) * 3125) / var1;
    var1 = (((int64_t)P9) * (p >> 13) * (p >> 13)) >> 25;
    var2 = (((int64_t)P8) * p) >> 19;

    p = ((p + var1 + var2) >> 8) + (((int64_t)P7) << 4);
    return float(p) / 256; // Pressure in Pascals
}
void displayPressure(float pressure) {
    // Convert float pressure to a long integer for display purposes
    long pressureLong = (long)pressure;
    // Calculate the digits to display
    int tienduizenden = (int)(pressureLong / 10000); // Ten thousands place
    int duizenden = (int)(pressureLong / 1000) % 10; // Thousands place
    int honderden = (int)(pressureLong / 100) % 10; // Hundreds place
    int tientallen = (int)(pressureLong / 10) % 10; // Tens place
    int eenheden = (int)pressureLong % 10; // Ones place

    if (tienduizenden > 0) {
        writeDigit(tienduizenden);
        _delay_ms(500);
    }

    if (tienduizenden > 0 || duizenden > 0) {
        writeDigit(duizenden);
        _delay_ms(500);
    }

    writeDigit(honderden);
    _delay_ms(500);
    writeDigit(tientallen);
    _delay_ms(500);
    writeDigit(eenheden);
    _delay_ms(500);
    // Clear the display section, preparing for the next value.
    writeDigit(11);
}
void writeDot(){// Write a dot to the display
    PORTB &= ~(1 << PB3);
    spi_transfer(0b00000001);
    PORTB |= (1 << PB3);

}
void writeDigit(int digit) {// Write a digit to the display
    static const uint8_t segmentMap[10] = {
        0b01111110, // 0
        0b00001100, // 1
        0b10110110, // 2
        0b10011110, // 3
        0b11001100, // 4
        0b11011010, // 5
        0b11111010, // 6
        0b00001110, // 7
        0b11111110, // 8
        0b11011110  // 9
    };
    if (digit == 11){
        PORTB &= ~(1 << PB3);
        _delay_ms(10);
        spi_transfer(0b00000000);
        PORTB |= (1 << PB3);
        return;
    }
    
    PORTB &= ~(1 << PB3);// Set CS low to enable the shift register
    _delay_ms(10); 
    spi_transfer(segmentMap[digit]);
    PORTB |= (1 << PB3);
}

void displayHexValueAsDecimal(uint8_t hexValue) {//check function because no serial debugger
    uint16_t decimalValue = hexValue; // Ensure it's treated as numeric decimal value
    // Check if the value is a three-digit number
    if (decimalValue >= 100) {
        uint8_t hundreds = decimalValue / 100; // Extract hundreds place
        writeDigit(hundreds);
        _delay_ms(1000); // Visible for 1 second

        decimalValue %= 100; // Remove the hundreds place
    }
    // Check if the value is a two-digit number or we have tens left after removing hundreds
    if (decimalValue >= 10) {
        uint8_t tens = decimalValue / 10; // Extract tens place
        writeDigit(tens);
        _delay_ms(1000); // Visible for 1 second

        decimalValue %= 10; // Remove the tens place
    }
    // Display the remaining ones place
    writeDigit(decimalValue);
    _delay_ms(1000); // Visible for 1 second
}
ISR(TIMER1_COMPA_vect) {
        // Voer elke 5 seconden de gewenste actie uit
        displayTemperature(readTemp());
    }
int main(void) {
    setupPins();
    _delay_ms(1000);
    // bmp280_read_register(0xD0);
    // _delay_ms(1000);
    readCoeff();
    _delay_ms(1000);
    setupBMP280();
    _delay_ms(1000);
    setupTimer();
    while(1){
    }
}
