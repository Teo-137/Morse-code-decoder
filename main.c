	
#include "MKL05Z4.h"
#include "ADC.h"
#include "frdm_bsp.h"
#include "lcd1602.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

volatile float adc_volt_coeff = ((float)(((float)2.91) / 4096));
volatile uint8_t space = 0;
volatile uint16_t temp;
volatile float wynik;

volatile uint32_t signal_start = 0;
volatile uint32_t signal_duration = 0;
volatile uint8_t signal_detected = 0;
volatile uint32_t current_time = 0;
volatile uint32_t last_time = 0;
volatile uint32_t delay;

const uint32_t dot_threshold_min = 7;   // Minimalny czas trwania kropki 70 ms
const uint32_t dot_threshold_max = 13;	// Maksymalny czas trwania kropki 130 ms
const uint32_t dash_threshold_min = 17; // Minimalny czas trwania kreski 170 ms
const uint32_t dash_threshold_max = 23;	// Maksymalny czas trwania kreski 230 ms
const uint32_t letter_space_threshold_min =33	; //Minimalna przerwa między literami 270 ms
const uint32_t letter_space_threshold_max =39	;	//Maksymalna przerwa między literami 330 ms
const uint32_t word_space_threshold_min =55	; //Minimalna przerwa między literami 270 ms
const uint32_t word_space_threshold_max =65;	//Maksymalna przerwa między literami 330 ms

char morse_code[100] = "";
char decoded_message[100] = "";

const struct MorseMap {
    char *morse;
    char letter;
} morse_map[] = {
    {".-", 'A'}, {"-...", 'B'}, {"-.-.", 'C'}, {"-..", 'D'},
    {".", 'E'}, {"..-.", 'F'}, {"--.", 'G'}, {"....", 'H'},
    {"..", 'I'}, {".---", 'J'}, {"-.-", 'K'}, {".-..", 'L'},
    {"--", 'M'}, {"-.", 'N'}, {"---", 'O'}, {".--.", 'P'},
    {"--.-", 'Q'}, {".-.", 'R'}, {"...", 'S'}, {"-", 'T'},
    {"..-", 'U'}, {"...-", 'V'}, {".--", 'W'}, {"-..-", 'X'},
    {"-.--", 'Y'}, {"--..", 'Z'},
    {NULL, '\0'}
};

void decode_morse_code(char *code)
{
	
    for (int i = 0; morse_map[i].morse != NULL; i++) {
        if (strcmp(code, morse_map[i].morse) == 0) {
            char letter = morse_map[i].letter;
            size_t len = strlen(decoded_message);
            decoded_message[len] = letter;
            decoded_message[len + 1] = '\0';
            break;
        }
    }
}

void display_message_on_lcd(const char *message) {
    char line1[17] = "                 ";
    char line2[17] = "                 ";
	
    strncpy(line1, message, 16);

    if (strlen(message) > 16) {
     strncpy(line2, message + 16, 16);
    }
    
    LCD1602_SetCursor(0, 0);
    LCD1602_Print(line1);
    LCD1602_SetCursor(0, 1);
    LCD1602_Print(line2);
}

void SysTick_Handler(void)
{
    current_time++;
}

void ADC0_IRQHandler()
{

    temp = ADC0->R[0]; // Odczyt danej i skasowanie flagi COCO
    wynik = temp * adc_volt_coeff;

    if (wynik > 1.5) { // Sygnał wykryty
        if (!signal_detected) {
            signal_start = current_time;
            signal_detected = 1;
						last_time=current_time;

        }
    } else { // Brak sygnału
        if (signal_detected) {
            signal_duration = current_time - signal_start;
            signal_detected = 0;

            if (signal_duration > dot_threshold_min && signal_duration < dot_threshold_max) {
                strcat(morse_code, ".");
							
            } else if (signal_duration > dash_threshold_min && signal_duration < dash_threshold_max) {
                strcat(morse_code, "-");
            }
					
        }
				
        if (current_time - last_time > letter_space_threshold_min && current_time - last_time < letter_space_threshold_max && strlen(morse_code) > 0) {
            decode_morse_code(morse_code);
						space=1;
            morse_code[0] = '\0'; // Reset kodu Morse'a dla nowej litery
        }
				if (current_time - last_time > word_space_threshold_min && current_time - last_time < word_space_threshold_max && strlen(decoded_message) > 0) {
           if(space){
						strcat(decoded_message, " ");
						 space=0;
					 }
        }
    }

    ADC0->SC1[0] |= ADC_SC1_ADCH(8);
}


int main(void)
{
    uint8_t kal_error;
    LCD1602_Init();
    LCD1602_Backlight(TRUE);
    LCD1602_Print("Odbiornik Morsa");

    kal_error = ADC_Init();
    if (kal_error) {
        while (1);
    }
		SysTick_Config(SystemCoreClock / 100); //co 10ms
    ADC0->SC1[0] = ADC_SC1_AIEN_MASK | ADC_SC1_ADCH(8);

    while (1) {
			delay=current_time;
			display_message_on_lcd(decoded_message);
			
			if((strlen(decoded_message)) %32==0 && strlen(decoded_message)>0){
					while(current_time-delay<100){ //Opóźnienie do wyświetlenia 32 znaku przed resetem zmiennej decoded_message
					display_message_on_lcd(decoded_message);
					}
					decoded_message[0]='\0';
					LCD1602_ClearAll();
			}
			
    }
}