#include <LPC17xx.h>
#include <stdio.h> // Required for sprintf

// ======================== LCD PIN DEFINITIONS (CNA CONNECTOR) ========================
// CNA pin mapping: P0.4–P0.11
#define RS_CTRL (1 << 10) // P0.10 - RS
#define EN_CTRL (1 << 9)  // P0.9  - EN

#define D4_PIN  (1 << 8)  // P0.8
#define D5_PIN  (1 << 7)  // P0.7
#define D6_PIN  (1 << 6)  // P0.6
#define D7_PIN  (1 << 5)  // P0.5

#define DT_CTRL (D4_PIN | D5_PIN | D6_PIN | D7_PIN)
#define LCD_CTRL (RS_CTRL | EN_CTRL | DT_CTRL)

// ======================== SENSOR PIN DEFINITIONS ========================
#define PIR_PIN     (1 << 15) // P0.15 (Input for PIR sensor)

// ======================== ADC CONFIGURATION ========================
#define ADC_CHANNEL 0      // AD0.0 (P0.23)
#define TEMP_THRESHOLD 372 // ~30°C threshold

// ======================== GLOBAL VARIABLES ========================
unsigned long int temp1, temp2;
unsigned int i;
unsigned char flag1; // 0 for command, 1 for data
unsigned long int init_command[] = {0x33, 0x32, 0x28, 0x0C, 0x06, 0x01, 0x80};

// ======================== FUNCTION PROTOTYPES ========================
void lcd_puts(const char *str);
void lcd_write(void);
void port_write(void);
void delay_lcd(unsigned int r1);
void ADC_Init(void);
uint16_t ADC_Read(void);
void lcd_set_cursor(int row, int col);

// =====================================================================
int main(void)
{
    unsigned long int pir_status;
    uint16_t adc_value;
    int is_intrusion = 0;
    int last_intrusion_state = -1; // For state tracking
    char line1_buffer[17];
    char line2_buffer[17];

    SystemInit();
    SystemCoreClockUpdate();

    // ======================== PIN CONFIGURATION ========================
    LPC_PINCON->PINSEL0 = 0; // P0.0 - P0.15 as GPIO
    LPC_PINCON->PINSEL1 = 0; // P0.16 - P0.23 as GPIO by default

    // P0.23 as AD0.0
    LPC_PINCON->PINSEL1 |= (1 << 14);

    // ======================== DIRECTION SETUP ========================
    LPC_GPIO0->FIODIR |= LCD_CTRL; // LCD as output
    LPC_GPIO0->FIODIR &= ~PIR_PIN; // PIR as input

    // ======================== INIT ADC + LCD ========================
    ADC_Init();

    flag1 = 0; // Command mode
    for (i = 0; i < 7; i++) {
        temp1 = init_command[i];
        lcd_write();
        delay_lcd(30000);
    }

    // ======================== MAIN LOOP ========================
    while (1)
    {
        pir_status = (LPC_GPIO0->FIOPIN & PIR_PIN);
        adc_value = ADC_Read();

        is_intrusion = (pir_status && (adc_value > TEMP_THRESHOLD));

        if (is_intrusion) {
            if (last_intrusion_state != 1) {
                last_intrusion_state = 1;
                lcd_set_cursor(0, 0);
                lcd_puts("Intrusion       ");
                lcd_set_cursor(1, 0);
                lcd_puts("Detected!       ");
            }
        } else {
            last_intrusion_state = 0;
            sprintf(line1_buffer, "PIR: %d         ", pir_status ? 1 : 0);
            sprintf(line2_buffer, "TEMP: %-4d      ", adc_value);

            lcd_set_cursor(0, 0);
            lcd_puts(line1_buffer);
            lcd_set_cursor(1, 0);
            lcd_puts(line2_buffer);
        }

        delay_lcd(50000);
    }
}

// =====================================================================
// LCD Helper Functions
// =====================================================================
void lcd_set_cursor(int row, int col) {
    unsigned char address;
    if (row == 0) address = 0x80 + col;
    else address = 0xC0 + col;
    flag1 = 0;
    temp1 = address;
    lcd_write();
}

void lcd_puts(const char *str) {
    flag1 = 1;
    for (i = 0; str[i] != '\0'; i++) {
        temp1 = str[i];
        lcd_write();
    }
}

void lcd_write(void)
{
    // Send high nibble
    temp2 = 0;
    if (temp1 & 0x10) temp2 |= D4_PIN;
    if (temp1 & 0x20) temp2 |= D5_PIN;
    if (temp1 & 0x40) temp2 |= D6_PIN;
    if (temp1 & 0x80) temp2 |= D7_PIN;
    port_write();

    // Send low nibble
    temp2 = 0;
    if (temp1 & 0x01) temp2 |= D4_PIN;
    if (temp1 & 0x02) temp2 |= D5_PIN;
    if (temp1 & 0x04) temp2 |= D6_PIN;
    if (temp1 & 0x08) temp2 |= D7_PIN;
    port_write();
}

void port_write(void)
{
    LPC_GPIO0->FIOCLR = DT_CTRL;
    LPC_GPIO0->FIOSET = temp2;

    if (flag1 == 0)
        LPC_GPIO0->FIOCLR = RS_CTRL; // Command
    else
        LPC_GPIO0->FIOSET = RS_CTRL; // Data

    LPC_GPIO0->FIOSET = EN_CTRL; // EN = 1
    delay_lcd(5000);
    LPC_GPIO0->FIOCLR = EN_CTRL; // EN = 0
    delay_lcd(5000);
}

// =====================================================================
// ADC Functions
// =====================================================================
void ADC_Init(void) {
    LPC_SC->PCONP |= (1 << 12); // Power ADC
    LPC_ADC->ADCR = (1 << ADC_CHANNEL) | (4 << 8) | (1 << 21);
}

uint16_t ADC_Read(void) {
    LPC_ADC->ADCR |= (1 << 24);
    while ((LPC_ADC->ADGDR & (1U << 31)) == 0);
    return (LPC_ADC->ADGDR >> 4) & 0xFFF;
}

// =====================================================================
// Delay Function
// =====================================================================
void delay_lcd(unsigned int r1)
{
    unsigned int r;
    for (r = 0; r < r1; r++);
}
