# Morse-code-decoder
The project involves a device that decodes Morse code transmitted via light signals. The decoded code is then displayed on an alphanumeric LCD display (2 x 16 characters). The light signal is received using a photoresistor.
______________________
How does it work?
______________________
Using fixed values, we define the duration of each signal (‘.’, ‘-’, ‘ ’). Sending a signal triggers an interrupt from the ADC module. If the signal is strong enough, we start measuring the time using the SysTick timer. When the signal disappears, we record the detected character and decode it by saving it into an array. If the pause between signals is long enough, we recognize whether a whole word has been collected. Then, we display the contents of the array on the LCD display. If the message is longer than 32 characters, we start again from the first line and clear the previous message.

The project was developed in the μVision5 environment and on the KL05Z development board.
