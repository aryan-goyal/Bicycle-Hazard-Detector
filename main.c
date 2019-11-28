#include <intrinsics.h>
#include <stdint.h>
#include <string.h>
#include "driverlib/driverlib.h"
#include "hal_LCD.h"
#include "main.h"

/* Definitions */
#define GREEN_LED 0
#define YELLOW_LED 1
#define ORANGE_LED 2
#define RED_LED 3

#define HIGH_BEEP 900
#define LOW_BEEP 200
#define BEEP_DUR 500

//...

/* Function Declarations */
void send_pulse_front_trig();

void light_LED(int LED);
void print_to_LCD(char *str);
void delay_ms(unsigned int ms);
void delay_us(unsigned int us);
void beep(unsigned int note, unsigned int duration);
void low_beep_warning();
void high_beep_warning();

//...

/* Globals */
char ADCState = 0; //Busy state of the ADC
int16_t ADCResult = 0; //Storage for the ADC conversion result
int thresh1 = 10;
int thresh2 = 20;
int thresh3 = 30;
int f_thresh1 = 10;
int f_thresh2 = 20;
int pos[6] = {pos1, pos2, pos3, pos4, pos5, pos6};

//...

void main(void)
{
    __disable_interrupt(); //Turns off interrupts during init
    WDT_A_hold(WDT_A_BASE); //Stop watchdog timer

    Init_GPIO();    //Sets all pins to output low as a default
    Init_LCD();     //Sets up the LaunchPad LCD display

    PMM_unlockLPM5(); //Disable the GPIO power-on default high-impedance mode to activate previously configured port settings
    __enable_interrupt();

    //GPIO_enableInterrupt(GPIO_PORT_P2, GPIO_PIN5);
    //GPIO_selectInterruptEdge(GPIO_PORT_P2, GPIO_PIN5, GPIO_LOW_TO_HIGH_TRANSITION);
    //GPIO_clearInterrupt(GPIO_PORT_P2, GPIO_PIN5);
    int beeped = 0;

    for(;;) //Infinite loop
    {
        if (GPIO_getInputPinValue(SW1_PORT, SW1_PIN) == 0)
        {
            setup_sequence();
            user_sequence();
        }

        if (GPIO_getInputPinValue(SW2_PORT, SW2_PIN) == 0)
        {
            user_sequence();
        }

        unsigned int front_sensor_val = 0;
        unsigned int back_sensor_val = 0;

        send_pulse_front_trig();
        while(GPIO_getInputPinValue(GPIO_PORT_P2, GPIO_PIN7) == 0);
        while(GPIO_getInputPinValue(GPIO_PORT_P2, GPIO_PIN7) != 0)
        {
            front_sensor_val += 1;
            __delay_cycles(16);
        }

        if (front_sensor_val <= f_thresh1)
        {
            if (beeped != HIGH_BEEP)
            {
                high_beep_warning();
                beeped = HIGH_BEEP;
            }
        }
        else if (front_sensor_val <= f_thresh2)
        {
            if (beeped != LOW_BEEP)
            {
                low_beep_warning();
                beeped = LOW_BEEP;
            }
        }

        send_pulse_back_trig();
        while(GPIO_getInputPinValue(GPIO_PORT_P2, GPIO_PIN5) == 0);
        while(GPIO_getInputPinValue(GPIO_PORT_P2, GPIO_PIN5) != 0)
        {
            back_sensor_val += 1;
           __delay_cycles(16);
        }

        unsigned int i;
        char sensor_data[6];
        unsigned int dummy = back_sensor_val;
        for (i = 0; i < 6; i++)
        {
            sensor_data[i] = dummy % 10;
            dummy /= 10;
            showChar('0' + (sensor_data[i]), pos[5 - i]);
        }

        if (back_sensor_val <= thresh1)
        {
            light_LED(RED_LED);
        }
        else if (back_sensor_val <= thresh2)
        {
            light_LED(ORANGE_LED);
        }
        else if (back_sensor_val <= thresh3)
        {
            light_LED(YELLOW_LED);
        }
        else
        {
            light_LED(GREEN_LED);
        }
    }
}

void user_sequence(){
    GPIO_setOutputLowOnPin(LED1_PORT, LED1_PIN);
    GPIO_setOutputLowOnPin(LED2_PORT, LED2_PIN);
    clearLCD();
    showChar('U', pos1);
    showChar('S', pos2);
    showChar('E', pos3);
    showChar('R', pos4);
    delay_ms(3000);
    clearLCD();
}

void setup_sequence(){
    light_LED(4);

    GPIO_setOutputHighOnPin(LED1_PORT, LED1_PIN);
    GPIO_setOutputHighOnPin(LED2_PORT, LED2_PIN);
    clearLCD();
    showChar('S', pos1);
    showChar('E', pos2);
    showChar('T', pos3);
    showChar('U', pos4);
    showChar('P', pos5);
    delay_ms(2000);
    clearLCD();
    showChar('B', pos1);
    showChar('A', pos2);
    showChar('C', pos3);
    showChar('K', pos4);
    delay_ms(3000);
    clearLCD();
    int b_s = 0;
    for(;;){
        b_s = 0;
        send_pulse_back_trig();
        while(GPIO_getInputPinValue(GPIO_PORT_P2, GPIO_PIN5) == 0);
        while(GPIO_getInputPinValue(GPIO_PORT_P2, GPIO_PIN5) != 0)
        {
            b_s += 1;
            __delay_cycles(16);
        }
        unsigned int x;
        char s_d[6];
        unsigned int dummys = b_s;
        for (x = 0; x < 6; x++)
        {
            s_d[x] = dummys % 10;
            dummys /= 10;
            showChar('0' + (s_d[x]), pos[5 - x]);
        }
        if (GPIO_getInputPinValue(SW1_PORT, SW1_PIN) == 0){
            thresh1 = b_s;
            clearLCD();
            showChar('T', pos1);
            showChar('1', pos2);
            showChar(' ', pos3);
            showChar('S', pos4);
            showChar('E', pos5);
            showChar('T', pos6);
            delay_ms(3000);
            clearLCD();
            for(;;){
                b_s = 0;
                send_pulse_back_trig();
                while(GPIO_getInputPinValue(GPIO_PORT_P2, GPIO_PIN5) == 0);
                while(GPIO_getInputPinValue(GPIO_PORT_P2, GPIO_PIN5) != 0)
                {
                    b_s += 1;
                    __delay_cycles(16);
                }
                unsigned int y;
                char s_d1[6];
                unsigned int dummy1 = b_s;
                for (y = 0; y < 6; y++)
                {
                    s_d1[y] = dummy1 % 10;
                    dummy1 /= 10;
                    showChar('0' + (s_d1[y]), pos[5 - y]);
                }
                if (GPIO_getInputPinValue(SW1_PORT, SW1_PIN) == 0){
                    thresh2 = b_s;
                    clearLCD();
                    showChar('T', pos1);
                    showChar('2', pos2);
                    showChar(' ', pos3);
                    showChar('S', pos4);
                    showChar('E', pos5);
                    showChar('T', pos6);
                    delay_ms(3000);
                    clearLCD();
                    for(;;){
                        b_s = 0;
                        send_pulse_back_trig();
                        while(GPIO_getInputPinValue(GPIO_PORT_P2, GPIO_PIN5) == 0);
                        while(GPIO_getInputPinValue(GPIO_PORT_P2, GPIO_PIN5) != 0)
                        {
                            b_s += 1;
                            __delay_cycles(16);
                        }
                        unsigned int z;
                        char s_d2[6];
                        unsigned int dummy2 = b_s;
                        for (z = 0; z < 6; z++)
                        {
                            s_d2[z] = dummy2 % 10;
                            dummy2 /= 10;
                            showChar('0' + (s_d2[z]), pos[5 - z]);
                        }
                        if (GPIO_getInputPinValue(SW1_PORT, SW1_PIN) == 0){
                            thresh3 = b_s;
                            clearLCD();
                            showChar('T', pos1);
                            showChar('3', pos2);
                            showChar(' ', pos3);
                            showChar('S', pos4);
                            showChar('E', pos5);
                            showChar('T', pos6);
                            delay_ms(3000);
                            clearLCD();
                            break;
                        }
                    }
                    break;
                }
            }
            break;
        }
    }

    showChar('F', pos1);
    showChar('R', pos2);
    showChar('O', pos3);
    showChar('N', pos4);
    showChar('T', pos5);
    delay_ms(3000);
    int f_s = 0;
    for(;;){
        f_s = 0;
        send_pulse_front_trig();
        while(GPIO_getInputPinValue(GPIO_PORT_P2, GPIO_PIN7) == 0);
        while(GPIO_getInputPinValue(GPIO_PORT_P2, GPIO_PIN7) != 0)
        {
            f_s += 1;
            __delay_cycles(16);
        }
        unsigned int a;
        char s_da[6];
        unsigned int dummya = f_s;
        for (a = 0; a < 6; a++)
        {
            s_da[a] = dummya % 10;
            dummya /= 10;
            showChar('0' + (s_da[a]), pos[5 - a]);
        }
        if (GPIO_getInputPinValue(SW1_PORT, SW1_PIN) == 0){
            f_thresh1 =  f_s;
            clearLCD();
            showChar('T', pos1);
            showChar('1', pos2);
            showChar(' ', pos3);
            showChar('S', pos4);
            showChar('E', pos5);
            showChar('T', pos6);
            delay_ms(3000);
            clearLCD();
            for(;;){
                f_s = 0;
                send_pulse_front_trig();
                while(GPIO_getInputPinValue(GPIO_PORT_P2, GPIO_PIN7) == 0);
                while(GPIO_getInputPinValue(GPIO_PORT_P2, GPIO_PIN7) != 0)
                {
                    f_s += 1;
                    __delay_cycles(16);
                }
                unsigned int b;
                char s_db[6];
                unsigned int dummyb = f_s;
                for (b = 0; b < 6; b++)
                {
                    s_db[b] = dummyb % 10;
                    dummyb /= 10;
                    showChar('0' + (s_db[b]), pos[5 - b]);
                }
                if (GPIO_getInputPinValue(SW1_PORT, SW1_PIN) == 0){
                    f_thresh2 = f_s;
                    clearLCD();
                    showChar('T', pos1);
                    showChar('2', pos2);
                    showChar(' ', pos3);
                    showChar('S', pos4);
                    showChar('E', pos5);
                    showChar('T', pos6);
                    delay_ms(3000);
                    clearLCD();
                    break;
                }
            }
            break;
        }
    }

    showChar('D', pos1);
    showChar('O', pos2);
    showChar('N', pos3);
    showChar('E', pos4);
    delay_ms(3000);
    clearLCD();
}

void send_pulse_front_trig()
{
        GPIO_setOutputHighOnPin(GPIO_PORT_P8, GPIO_PIN1); //Send a pulse
        __delay_cycles(16);
        GPIO_setOutputLowOnPin(GPIO_PORT_P8, GPIO_PIN1);
}

void send_pulse_back_trig()
{
        GPIO_setOutputHighOnPin(GPIO_PORT_P1, GPIO_PIN1); //Send a pulse
        __delay_cycles(16);
        GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN1);
}

void light_LED(int LED)
{
    GPIO_setOutputLowOnPin(GPIO_PORT_P5, GPIO_PIN2);
    GPIO_setOutputLowOnPin(GPIO_PORT_P5, GPIO_PIN3);
    GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN3);
    GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN4);

    switch (LED)
    {
        case GREEN_LED:
                GPIO_setOutputHighOnPin(GPIO_PORT_P5, GPIO_PIN2);
                break;
        case YELLOW_LED:
                GPIO_setOutputHighOnPin(GPIO_PORT_P5, GPIO_PIN3);
                break;
        case ORANGE_LED:
                GPIO_setOutputHighOnPin(GPIO_PORT_P1, GPIO_PIN3);
                break;
        case RED_LED:
                GPIO_setOutputHighOnPin(GPIO_PORT_P1, GPIO_PIN4);
                break;
    }
    return;
}

void print_to_LCD(char *str)
{
    int len = strlen(str);
    int pos[6] = {pos1, pos2, pos3, pos4, pos5, pos6};

    if (len > 6) //String is too long for LCD display
    {
        displayScrollText(str);
        return;
    }

    int i;
    for (i = 0; i < len; i++)
    {
        showChar(str[i], pos[i]);
    }
}

void delay_ms(unsigned int ms)
{
    unsigned int i;
    for (i = 0; i<= ms; i++)
       __delay_cycles(500); //Suspend execution for 500 cycles
}

void delay_us(unsigned int us)
{
    unsigned int i;
    for (i = 0; i<= us/2; i++)
       __delay_cycles(1); //Delay for one cycle, used for generating square wave
}

//This function generates the square wave that makes the piezo speaker sound at a determinated frequency.
void beep(unsigned int note, unsigned int duration)
{
    long delay = (long)(10000/note); //Determine the period for each note
    long time = (long)((duration*100)/(delay*2)); //Time the note is held for
    int i;
    for (i=0;i<time;i++)
    {
        GPIO_setOutputHighOnPin(GPIO_PORT_P1, GPIO_PIN5); //Start pulse
        delay_us(delay); //For a semiperiod
        GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN5); //Stop pulse
        delay_us(delay); //For the other semiperiod
    }
}

void low_beep_warning()
{
    int i;
    for (i = 0; i < 2; i++)
    {
        delay_ms(80);
        beep(LOW_BEEP, BEEP_DUR);
    }
    delay_ms(300);
}

void high_beep_warning()
{
    int i;
    for (i = 0; i < 4; i++)
    {
        delay_ms(80);
        beep(HIGH_BEEP, BEEP_DUR/5);
    }
    delay_ms(300);
}

//#pragma vector=PORT2_VECTOR
//__interrupt void Port_2(void)
//{
//
//}

void Init_GPIO(void)
{
    //Set all GPIO pins to output low to prevent floating input and reduce power consumption
    GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN0|GPIO_PIN1|GPIO_PIN2|GPIO_PIN3|GPIO_PIN4|GPIO_PIN5|GPIO_PIN6|GPIO_PIN7);
    GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN0|GPIO_PIN1|GPIO_PIN2|GPIO_PIN3|GPIO_PIN4|GPIO_PIN5|GPIO_PIN6|GPIO_PIN7);
    GPIO_setOutputLowOnPin(GPIO_PORT_P3, GPIO_PIN0|GPIO_PIN1|GPIO_PIN2|GPIO_PIN3|GPIO_PIN4|GPIO_PIN5|GPIO_PIN6|GPIO_PIN7);
    GPIO_setOutputLowOnPin(GPIO_PORT_P4, GPIO_PIN0|GPIO_PIN1|GPIO_PIN2|GPIO_PIN3|GPIO_PIN4|GPIO_PIN5|GPIO_PIN6|GPIO_PIN7);
    GPIO_setOutputLowOnPin(GPIO_PORT_P5, GPIO_PIN0|GPIO_PIN1|GPIO_PIN2|GPIO_PIN3|GPIO_PIN4|GPIO_PIN5|GPIO_PIN6|GPIO_PIN7);
    GPIO_setOutputLowOnPin(GPIO_PORT_P6, GPIO_PIN0|GPIO_PIN1|GPIO_PIN2|GPIO_PIN3|GPIO_PIN4|GPIO_PIN5|GPIO_PIN6|GPIO_PIN7);
    GPIO_setOutputLowOnPin(GPIO_PORT_P7, GPIO_PIN0|GPIO_PIN1|GPIO_PIN2|GPIO_PIN3|GPIO_PIN4|GPIO_PIN5|GPIO_PIN6|GPIO_PIN7);
    GPIO_setOutputLowOnPin(GPIO_PORT_P8, GPIO_PIN0|GPIO_PIN1|GPIO_PIN2|GPIO_PIN3|GPIO_PIN4|GPIO_PIN5|GPIO_PIN6|GPIO_PIN7);

    //GPIO_setAsOutputPin(GPIO_PORT_P1, GPIO_PIN0|GPIO_PIN1|GPIO_PIN2|GPIO_PIN3|GPIO_PIN4|GPIO_PIN5|GPIO_PIN6|GPIO_PIN7);
    //GPIO_setAsOutputPin(GPIO_PORT_P2, GPIO_PIN0|GPIO_PIN1|GPIO_PIN2|GPIO_PIN3|GPIO_PIN4|GPIO_PIN5|GPIO_PIN6|GPIO_PIN7);
    //GPIO_setAsOutputPin(GPIO_PORT_P3, GPIO_PIN0|GPIO_PIN1|GPIO_PIN2|GPIO_PIN3|GPIO_PIN4|GPIO_PIN5|GPIO_PIN6|GPIO_PIN7);
    //GPIO_setAsOutputPin(GPIO_PORT_P4, GPIO_PIN0|GPIO_PIN1|GPIO_PIN2|GPIO_PIN3|GPIO_PIN4|GPIO_PIN5|GPIO_PIN6|GPIO_PIN7);
    //GPIO_setAsOutputPin(GPIO_PORT_P5, GPIO_PIN0|GPIO_PIN1|GPIO_PIN2|GPIO_PIN3|GPIO_PIN4|GPIO_PIN5|GPIO_PIN6|GPIO_PIN7);
    //GPIO_setAsOutputPin(GPIO_PORT_P6, GPIO_PIN0|GPIO_PIN1|GPIO_PIN2|GPIO_PIN3|GPIO_PIN4|GPIO_PIN5|GPIO_PIN6|GPIO_PIN7);
    //GPIO_setAsOutputPin(GPIO_PORT_P7, GPIO_PIN0|GPIO_PIN1|GPIO_PIN2|GPIO_PIN3|GPIO_PIN4|GPIO_PIN5|GPIO_PIN6|GPIO_PIN7);
    //GPIO_setAsOutputPin(GPIO_PORT_P8, GPIO_PIN0|GPIO_PIN1|GPIO_PIN2|GPIO_PIN3|GPIO_PIN4|GPIO_PIN5|GPIO_PIN6|GPIO_PIN7);

    /* Outputs */
    GPIO_setAsOutputPin(LED1_PORT, LED1_PIN); //P1.0 - internal LED 1
    GPIO_setAsOutputPin(LED2_PORT, LED2_PIN); //P4.0 - internal LED 2

    GPIO_setAsOutputPin(GPIO_PORT_P5, GPIO_PIN2); //P5.2 - green LED
    GPIO_setAsOutputPin(GPIO_PORT_P5, GPIO_PIN3); //P5.3 - yellow LED
    GPIO_setAsOutputPin(GPIO_PORT_P1, GPIO_PIN3); //P1.3 - orange LED
    GPIO_setAsOutputPin(GPIO_PORT_P1, GPIO_PIN4); //P1.4 - red LED
    GPIO_setAsOutputPin(GPIO_PORT_P1, GPIO_PIN5); //P1.5 - buzzer

    GPIO_setAsOutputPin(GPIO_PORT_P1, GPIO_PIN1); //P1.1 - back ultrasonic trig pin
    GPIO_setAsOutputPin(GPIO_PORT_P8, GPIO_PIN1); //P8.1 - front ultrasonic trig pin

    /* Inputs */
    GPIO_setAsInputPinWithPullUpResistor(SW1_PORT, SW1_PIN); //P1.2 - built-on pb 1
    GPIO_setAsInputPinWithPullUpResistor(SW2_PORT, SW2_PIN); //P2.6 - built-on pb 2

    GPIO_setAsInputPin(GPIO_PORT_P2, GPIO_PIN5); //P2.5 - back ultrasonic echo pin
    GPIO_setAsInputPin(GPIO_PORT_P2, GPIO_PIN7); //P2.7 - front ultrasonic echo pin

    //Set LaunchPad switches as inputs - they are active low, meaning '1' until pressed
    GPIO_setAsInputPinWithPullUpResistor(SW1_PORT, SW1_PIN);
    GPIO_setAsInputPinWithPullUpResistor(SW2_PORT, SW2_PIN);

    //Set LED1 and LED2 as outputs
    GPIO_setAsOutputPin(LED1_PORT, LED1_PIN); //Comment if using UART
    GPIO_setAsOutputPin(LED2_PORT, LED2_PIN);
}
