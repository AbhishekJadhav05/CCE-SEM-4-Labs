#include <LPC17xx.h>
#include <stdio.h> // Required for sprintf

// --- Pin Definitions for LCD (Moved to CNA Connector) ---
#define RS_CTRL (1 << 10) // P0.10
#define EN_CTRL (1 << 9)  // P0.9
#define D4_PIN  (1 << 8)  // P0.8
#define D5_PIN  (1 << 7)  // P0.7
#define D6_PIN  (1 << 6)  // P0.6
#define D7_PIN  (1 << 4)  // P0.4
// Mask for all LCD data pins
#define DT_CTRL (D4_PIN | D5_PIN | D6_PIN | D7_PIN)

// --- Pin Definitions for Sensors and LEDs ---
#define PIR_PIN     (1 << 15) // P0.15 (Input)
#define PIR_LED_PIN (1 << 5)  // P0.5  (Output)
#define ADC_LED_PIN (1 << 11) // P0.11 (Output)

// --- ADC Configuration ---
#define ADC_CHANNEL 0      // AD0.0 on P0.23
#define TEMP_THRESHOLD 372 // Threshold from your ADC code (~30°C)

// --- Global Variables ---
unsigned long int temp1, temp2;
unsigned int i;
unsigned char flag1; // 0 for command, 1 for data
unsigned long int init_command[] = {0x33, 0x32, 0x28, 0x0C, 0x06, 0x01, 0x80};
char line1_str[17];
char line2_str[17];

// --- Function Prototypes ---
void lcd_write(void);
void port_write(void);
void delay_lcd(unsigned int);
void ADC_Init(void);
uint16_t ADC_Read(void);
void lcd_display_status(int pir_state, uint16_t adc_val);

int main(void)
{
    unsigned long int pir_status;
    uint16_t adc_value;
	
    // Variables to track state and prevent LCD flicker
    int last_pir_status = -1; // Use -1 to force initial update
    uint16_t last_adc_value = 0;

    SystemInit();
    SystemCoreClockUpdate();

    // --- 1. Configure Pin Functions (PINSEL) ---
    LPC_PINCON->PINSEL0 = 0; // Set all P0.0-P0.15 to GPIO
    LPC_PINCON->PINSEL1 = 0; // Set all P0.16-P0.31 to GPIO
    
    // Configure P0.23 as AD0.0
    LPC_PINCON->PINSEL1 |= (1 << 14);

    // --- 2. Configure Pin Direction (FIODIR) ---
    // Set outputs for LEDs and LCD
    LPC_GPIO0->FIODIR |= (DT_CTRL | RS_CTRL | EN_CTRL); // New LCD pins
    LPC_GPIO0->FIODIR |= PIR_LED_PIN;   // P0.5
    LPC_GPIO0->FIODIR |= ADC_LED_PIN;   // P0.11
    
    // Set input for PIR sensor
    LPC_GPIO0->FIODIR &= ~PIR_PIN;      // P0.15

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

        // Logic for PIR Sensor -> PIR LED
        if (pir_status) {
            LPC_GPIO0->FIOSET = PIR_LED_PIN;
        } else {
            LPC_GPIO0->FIOCLR = PIR_LED_PIN;
        }

        // Logic for ADC Sensor -> ADC LED
        if (adc_value > TEMP_THRESHOLD) {
            LPC_GPIO0->FIOSET = ADC_LED_PIN;
        } else {
            LPC_GPIO0->FIOCLR = ADC_LED_PIN;
        }

        // Update LCD only if a value has changed
        if (pir_status != last_pir_status || adc_value != last_adc_value) 
        {
            lcd_display_status(pir_status, adc_value);
            last_pir_status = pir_status;
            last_adc_value = adc_value;
        }
        
        delay_lcd(50000); // Poll sensors at a reasonable rate
    }
}

/**
 * @brief Updates both lines of the LCD with current sensor status
 * @param pir_state The status of the PIR sensor (non-zero for motion)
 * @param adc_val The 12-bit value from the ADC
 */
void lcd_display_status(int pir_state, uint16_t adc_val)
{
    // --- Line 1: PIR Status ---
    sprintf(line1_str, "PIR: %-3s", pir_state ? "YES" : "NO ");
    
    flag1 = 0; temp1 = 0x80; lcd_write(); // Set cursor to line 1
    flag1 = 1; // Data mode
    for (i = 0; line1_str[i] != '\0'; i++) {
        temp1 = line1_str[i];
        lcd_write();
    }

    // --- Line 2: ADC Status ---
    const char* temp_status = (adc_val > TEMP_THRESHOLD) ? "HOT" : "OK ";
    sprintf(line2_str, "ADC:%4d T:%-3s", adc_val, temp_status);

    flag1 = 0; temp1 = 0xC0; lcd_write(); // Set cursor to line 2
    flag1 = 1; // Data mode
    for (i = 0; line2_str[i] != '\0'; i++) {
        temp1 = line2_str[i];
        lcd_write();
    }
}


/**
 * @brief Writes a command/data byte to the LCD in 4-bit mode.
 * This function is modified to use the new non-contiguous pins.
 */
void lcd_write(void)
{
    // --- Send High Nibble (bits 4-7) ---
    temp2 = 0; // Clear temp register
    if (temp1 & 0x10) temp2 |= D4_PIN; // P0.8
    if (temp1 & 0x20) temp2 |= D5_PIN; // P0.7
    if (temp1 & 0x40) temp2 |= D6_PIN; // P0.6
    if (temp1 & 0x80) temp2 |= D7_PIN; // P0.4
    port_write();

    // --- Send Low Nibble (bits 0-3) ---
    temp2 = 0; // Clear temp register
    if (temp1 & 0x01) temp2 |= D4_PIN; // P0.8
    if (temp1 & 0x02) temp2 |= D5_PIN; // P0.7
    if (temp1 & 0x04) temp2 |= D6_PIN; // P0.6
    if (temp1 & 0x08) temp2 |= D7_PIN; // P0.4
    port_write();
}

/**
 * @brief Pulses the Enable (EN) pin to latch data.
 * This function is modified to use the new control pin definitions.
 */
void port_write(void)
{
    LPC_GPIO0->FIOCLR = DT_CTRL; // Clear all data pins
    LPC_GPIO0->FIOSET = temp2;   // Set the required data bits

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
 * @brief Initializes the ADC peripheral (from your ADC code)
 */
void ADC_Init(void) {
    LPC_SC->PCONP |= (1 << 12);      // Power up ADC
    LPC_ADC->ADCR = (1 << ADC_CHANNEL) | // Select AD0.0
                    (4 << 8) |       // ADC clock = PCLK/5
                    (1 << 21);       // Enable ADC
}

/**
 * @brief Reads a 12-bit value from the ADC (from your ADC code)
 * @return 12-bit ADC result
 */
uint16_t ADC_Read(void) {
    LPC_ADC->ADCR |= (1 << 24);      // Start conversion
    while ((LPC_ADC->ADGDR & (1U << 31)) == 0); // Wait for DONE bit
    return (LPC_ADC->ADGDR >> 4) & 0xFFF; // 12-bit result
}

/**
 * @brief Simple busy-wait delay
 */
void delay_lcd(unsigned int r1)
{
    unsigned int r;
    for (r = 0; r < r1; r++);
}
