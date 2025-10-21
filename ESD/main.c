#include <LPC17xx.h>
#include <stdio.h> // Required for sprintf

// --- Pin Definitions for LCD (Moved to CNA Connector) ---
#define RS_CTRL (1 << 10) // P0.10
#define EN_CTRL (1 << 9)  // P0.9
#define D4_PIN  (1 << 8)  // P0.8
#define D5_PIN  (1 << 7)  // P0.7
#define D6_PIN  (1 << 6)  // P0.6
#define D7_PIN  (1 << 4)  // P0.4
// Mask for all LCD data and control pins
#define LCD_CTRL (DT_CTRL | RS_CTRL | EN_CTRL)
#define DT_CTRL (D4_PIN | D5_PIN | D6_PIN | D7_PIN)

// --- Pin Definitions for Sensors ---
#define PIR_PIN     (1 << 15) // P0.15 (Input)

// --- ADC Configuration ---
#define ADC_CHANNEL 0      // AD0.0 on P0.23
#define TEMP_THRESHOLD 372 // Threshold for temperature sensor (~30°C)

// --- Global Variables ---
unsigned long int temp1, temp2;
unsigned int i;
unsigned char flag1; // 0 for command, 1 for data
unsigned long int init_command[] = {0x33, 0x32, 0x28, 0x0C, 0x06, 0x01, 0x80};

// --- Function Prototypes ---
void lcd_puts(const char *str);
void lcd_write(void);
void port_write(void);
void delay_lcd(unsigned int r1);
void ADC_Init(void);
uint16_t ADC_Read(void);
void lcd_set_cursor(int row, int col);


int main(void)
{
    unsigned long int pir_status;
    uint16_t adc_value;
    int is_intrusion = 0;
    int last_intrusion_state = -1; // Use -1 to force initial display update
    char line1_buffer[17];
    char line2_buffer[17];

    SystemInit();
    SystemCoreClockUpdate();

    // --- 1. Configure Pin Functions (PINSEL) ---
    // Set all relevant Port 0 pins to GPIO function
    LPC_PINCON->PINSEL0 = 0; 
    LPC_PINCON->PINSEL1 = 0; 
    
    // Configure P0.23 as AD0.0 for the ADC sensor
    LPC_PINCON->PINSEL1 |= (1 << 14);

    // --- 2. Configure Pin Direction (FIODIR) ---
    // Set LCD pins as outputs
    LPC_GPIO0->FIODIR |= LCD_CTRL;
    
    // Set PIR sensor pin as input
    LPC_GPIO0->FIODIR &= ~PIR_PIN;

    // --- 3. Initialize Hardware ---
    ADC_Init(); // Initialize ADC peripheral

    // Initialize LCD
    flag1 = 0; // Command mode
    for (i = 0; i < 7; i++) {
        temp1 = init_command[i];
        lcd_write();
        delay_lcd(30000);
    }

    // --- 4. Main Loop ---
    while (1)
    {
        // Read sensor values
        pir_status = (LPC_GPIO0->FIOPIN & PIR_PIN);
        adc_value = ADC_Read();
			
        // Determine the current state
        is_intrusion = (pir_status && (adc_value > TEMP_THRESHOLD));

        if (is_intrusion) {
            // Only update the display if it's not already showing the intrusion message
            if (last_intrusion_state != 1) {
                last_intrusion_state = 1;
                lcd_set_cursor(0, 0);
                lcd_puts("Intrusion       "); // Pad with spaces to clear old text
                lcd_set_cursor(1, 0);
                lcd_puts("Detected!       ");
            }
        } else {
            // --- NORMAL: Display live sensor data ---
            last_intrusion_state = 0;
            
            // Format the strings with padding to clear the line
            sprintf(line1_buffer, "PIR: %d         ", pir_status ? 1 : 0);
            sprintf(line2_buffer, "TEMP: %-4d      ", adc_value);

            lcd_set_cursor(0, 0);
            lcd_puts(line1_buffer);
            
            lcd_set_cursor(1, 0);
            lcd_puts(line2_buffer);
        }
        
        delay_lcd(50000); // Small delay to prevent rapid polling
    }
}

/**
 * @brief Sets the LCD cursor to a specific row and column.
 * @param row The row (0 or 1).
 * @param col The column (0 to 15).
 */
void lcd_set_cursor(int row, int col) {
    unsigned char address;
    if (row == 0) {
        address = 0x80 + col;
    } else {
        address = 0xC0 + col;
    }
    flag1 = 0; // Command mode
    temp1 = address;
    lcd_write();
}

/**
 * @brief Writes a null-terminated string to the current LCD cursor position.
 * @param str The string to write.
 */
void lcd_puts(const char *str) {
    flag1 = 1; // Data mode
    for (i = 0; str[i] != '\0'; i++) {
        temp1 = str[i];
        lcd_write();
    }
}

/**
 * @brief Writes a command/data byte to the LCD in 4-bit mode.
 * This function is modified to use the new non-contiguous pins.
 */
void lcd_write(void)
{
    // Send High Nibble (bits 4-7)
    temp2 = 0;
    if (temp1 & 0x10) temp2 |= D4_PIN;
    if (temp1 & 0x20) temp2 |= D5_PIN;
    if (temp1 & 0x40) temp2 |= D6_PIN;
    if (temp1 & 0x80) temp2 |= D7_PIN;
    port_write();

    // Send Low Nibble (bits 0-3)
    temp2 = 0;
    if (temp1 & 0x01) temp2 |= D4_PIN;
    if (temp1 & 0x02) temp2 |= D5_PIN;
    if (temp1 & 0x04) temp2 |= D6_PIN;
    if (temp1 & 0x08) temp2 |= D7_PIN;
    port_write();
}

/**
 * @brief Pulses the Enable (EN) pin to latch data to the LCD.
 */
void port_write(void)
{
    LPC_GPIO0->FIOCLR = DT_CTRL;
    LPC_GPIO0->FIOSET = temp2;

    if (flag1 == 0)
        LPC_GPIO0->FIOCLR = RS_CTRL; // RS = 0 (Command)
    else
        LPC_GPIO0->FIOSET = RS_CTRL; // RS = 1 (Data)

    LPC_GPIO0->FIOSET = EN_CTRL; // EN = 1
    delay_lcd(1000);
    LPC_GPIO0->FIOCLR = EN_CTRL; // EN = 0 (Falling edge latches data)
    delay_lcd(1000);
}

/**
 * @brief Initializes the ADC peripheral.
 */
void ADC_Init(void) {
    LPC_SC->PCONP |= (1 << 12);      // Power up ADC
    LPC_ADC->ADCR = (1 << ADC_CHANNEL) | // Select AD0.0
                    (4 << 8) |       // ADC clock = PCLK/5
                    (1 << 21);       // Enable ADC
}

/**
 * @brief Reads a 12-bit value from the ADC.
 * @return 12-bit ADC result.
 */
uint16_t ADC_Read(void) {
    LPC_ADC->ADCR |= (1 << 24);      // Start conversion
    while ((LPC_ADC->ADGDR & (1U << 31)) == 0); // Wait for DONE bit
    return (LPC_ADC->ADGDR >> 4) & 0xFFF;
}

/**
 * @brief Simple busy-wait delay.
 */
void delay_lcd(unsigned int r1)
{
    unsigned int r;
    for (r = 0; r < r1; r++);
}
