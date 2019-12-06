/* ---- Headers ---- */
#include <intrinsics.h>
#include <string.h>

#include "driverlib/driverlib.h"
#include "hal_LCD.h" /* TODO put this in driverlib */
#include "main.h"    /* TODO get rid of main.h by grabbing what you need from it */
/* ---- End of Headers ---- */

/* ---- Definitions and Globals ---- */
#define GREEN_LED 0
#define YELLOW_LED 1
#define ORANGE_LED 2
#define RED_LED 3
#define LED_OFF 4

/* TODO make a type def for all ports and pins (like in main.h) */

#define HIGH_BEEP 900
#define LOW_BEEP 200
#define BEEP_DUR 500

#define DATA_BOUND 4

#define F_DANGER 0
#define F_WARNING 1

#define B_DANGER 0
#define B_WARNING 1
#define B_CAUTION 2

int pos[6] = {pos1, pos2, pos3, pos4, pos5, pos6};

unsigned int back_sensor_data[4];
unsigned int front_sensor_data[4];
int F_FRONT = -1;
int F_REAR = -1;
int B_FRONT = -1;
int B_REAR = -1;

unsigned int front_threshold[2];
unsigned int back_threshold[3];
/* ---- End of Definitions and Globals ---- */

/* ---- Ultrasonic Sensor API ---- */
void Init_Sensor_Data()
{
    int i;
    for (i = 0; i < DATA_BOUND; i++)
    {
        back_sensor_data[i] = 0;
        front_sensor_data[i] = 0;
    }

    front_threshold[F_DANGER] = 25;
    front_threshold[F_WARNING] = 50;

    back_threshold[B_DANGER] = 25;
    back_threshold[B_WARNING] = 50;
    back_threshold[B_CAUTION] = 100;
}

void Send_Front_Trig()
{
    GPIO_setOutputHighOnPin(GPIO_PORT_P8, GPIO_PIN1);
    __delay_cycles(16);
    GPIO_setOutputLowOnPin(GPIO_PORT_P8, GPIO_PIN1);
}

unsigned int Read_Front_Echo()
{
    unsigned int value;
    while (GPIO_getInputPinValue(GPIO_PORT_P2, GPIO_PIN7) == 0);
    while (GPIO_getInputPinValue(GPIO_PORT_P2, GPIO_PIN7) != 0)
    {
        value += 1;
        __delay_cycles(16);
    }
    return (value - 6);
}

void Send_Back_Trig()
{
    GPIO_setOutputHighOnPin(GPIO_PORT_P1, GPIO_PIN1);
    __delay_cycles(16);
    GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN1);
}

unsigned int Read_Back_Echo()
{
    unsigned int value;
    while (GPIO_getInputPinValue(GPIO_PORT_P2, GPIO_PIN5) == 0);
    while (GPIO_getInputPinValue(GPIO_PORT_P2, GPIO_PIN5) != 0)
    {
        value += 1;
        __delay_cycles(16);
    }
    return (value - 6);
}
/* ---- End of Ultrasonic Sensor API ---- */

/* ---- LED API ---- */
void Activate_LED(int LED)
{
    /* set all leds to low */
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
    default:
        break;
    }
    return;
}
/* ---- End of LED API ---- */

/* ---- Buzzer API ---- */
void Delay_MS(unsigned int ms)
{
    unsigned int i;
    for (i = 0; i <= ms; i++)
        __delay_cycles(500); /* suspend execution for 500 cycles */
}

void Delay_US(unsigned int us)
{
    unsigned int i;
    for (i = 0; i <= us / 2; i++)
        __delay_cycles(1); /* delay for one cycle, used for generating square wave */
}

/* this function generates the square wave that makes the piezo speaker sound at a determinated frequency */
void Beep(unsigned int note, unsigned int duration)
{
    long delay = (long)(10000 / note);                  /* determine the period for each note */
    long time = (long)((duration * 100) / (delay * 2)); /* time the note is held for */
    int i;
    for (i = 0; i < time; i++)
    {
        GPIO_setOutputHighOnPin(GPIO_PORT_P1, GPIO_PIN5); /* start pulse */
        Delay_US(delay);                                  /* for a semiperiod */
        GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN5);  /* stop pulse */
        Delay_US(delay);                                  /* for the other semiperiod */
    }
}

void Play_Low_Warning()
{
    int i;
    for (i = 0; i < 2; i++) /* play 2 beeps */
    {
        Delay_MS(80);
        Beep(LOW_BEEP, BEEP_DUR);
    }
    Delay_MS(300);
}

void Play_High_Warning()
{
    int i;
    for (i = 0; i < 4; i++) /* play 4 beeps */
    {
        Delay_MS(80);
        Beep(HIGH_BEEP, BEEP_DUR / 5);
    }
    Delay_MS(300);
}
/* End of Buzzer API */

void Display_User()
{
    GPIO_setOutputHighOnPin(LED2_PORT, LED2_PIN);
    GPIO_setOutputLowOnPin(LED1_PORT, LED1_PIN);

    clearLCD();
    showChar('U', pos3);
    showChar('S', pos4);
    showChar('E', pos5);
    showChar('R', pos6);
    Delay_MS(2000);
    clearLCD();
}

void Setup_Mode()
{
    Activate_LED(LED_OFF);
    clearLCD();

    GPIO_setOutputHighOnPin(LED1_PORT, LED1_PIN);
    GPIO_setOutputLowOnPin(LED2_PORT, LED2_PIN);

    showChar('S', pos2);
    showChar('E', pos3);
    showChar('T', pos4);
    showChar('U', pos5);
    showChar('P', pos6);
    Delay_MS(2000);
    clearLCD();

    showChar('B', pos3);
    showChar('A', pos4);
    showChar('C', pos5);
    showChar('K', pos6);
    Delay_MS(2000);
    clearLCD();

    int b_s = 0;
    while (1)
    {
        if (GPIO_getInputPinValue(SW2_PORT, SW2_PIN) == 0)
        {
            return;
        }
        Send_Back_Trig();
        b_s = Read_Back_Echo();

        unsigned int x;
        char s_d[6];
        unsigned int dummy_t1 = b_s;
        for (x = 0; x < 6; x++)
        {
            s_d[x] = dummy_t1 % 10;
            dummy_t1 /= 10;
            showChar('0' + (s_d[x]), pos[5 - x]);
        }
        
        if (GPIO_getInputPinValue(SW1_PORT, SW1_PIN) == 0)
        {
            back_threshold[B_DANGER] = b_s;
            clearLCD();
            showChar('T', pos1);
            showChar('1', pos2);
            showChar(' ', pos3);
            showChar('S', pos4);
            showChar('E', pos5);
            showChar('T', pos6);
            Delay_MS(2000);
            clearLCD();

            while (1)
            {
                if (GPIO_getInputPinValue(SW2_PORT, SW2_PIN) == 0)
                {
                    return;
                }
                Send_Back_Trig();
                b_s = Read_Back_Echo();

                unsigned int y;
                char s_d1[6];
                unsigned int dummy_t2 = b_s;
                for (y = 0; y < 6; y++)
                {
                    s_d1[y] = dummy_t2 % 10;
                    dummy_t2 /= 10;
                    showChar('0' + (s_d1[y]), pos[5 - y]);
                }

                if (GPIO_getInputPinValue(SW1_PORT, SW1_PIN) == 0)
                {
                    back_threshold[B_WARNING] = b_s;
                    clearLCD();
                    showChar('T', pos1);
                    showChar('2', pos2);
                    showChar(' ', pos3);
                    showChar('S', pos4);
                    showChar('E', pos5);
                    showChar('T', pos6);
                    Delay_MS(2000);
                    clearLCD();

                    while (1)
                    {
                        if (GPIO_getInputPinValue(SW2_PORT, SW2_PIN) == 0)
                        {
                            return;
                        }
                        Send_Back_Trig();
                        b_s = Read_Back_Echo();

                        unsigned int z;
                        char s_d2[6];
                        unsigned int dummy_t3 = b_s;
                        for (z = 0; z < 6; z++)
                        {
                            s_d2[z] = dummy_t3 % 10;
                            dummy_t3 /= 10;
                            showChar('0' + (s_d2[z]), pos[5 - z]);
                        }

                        if (GPIO_getInputPinValue(SW1_PORT, SW1_PIN) == 0)
                        {
                            back_threshold[B_CAUTION] = b_s;
                            clearLCD();
                            showChar('T', pos1);
                            showChar('3', pos2);
                            showChar(' ', pos3);
                            showChar('S', pos4);
                            showChar('E', pos5);
                            showChar('T', pos6);
                            Delay_MS(2000);
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

    showChar('F', pos2);
    showChar('R', pos3);
    showChar('O', pos4);
    showChar('N', pos5);
    showChar('T', pos6);
    Delay_MS(2000);
    clearLCD();

    int f_s = 0;
    while (1)
    {
        if (GPIO_getInputPinValue(SW2_PORT, SW2_PIN) == 0)
        {
            return;
        }
        Send_Front_Trig();
        f_s = Read_Front_Echo();
            
        unsigned int a;
        char s_d[6];
        unsigned int dummy_t1 = f_s;
        for (a = 0; a < 6; a++)
        {
            s_d[a] = dummy_t1 % 10;
            dummy_t1 /= 10;
            showChar('0' + (s_d[a]), pos[5 - a]);
        }
        if (GPIO_getInputPinValue(SW1_PORT, SW1_PIN) == 0)
        {
            front_threshold[F_DANGER] = f_s;
            clearLCD();
            showChar('T', pos1);
            showChar('1', pos2);
            showChar(' ', pos3);
            showChar('S', pos4);
            showChar('E', pos5);
            showChar('T', pos6);
            Delay_MS(2000);
            clearLCD();
            while (1)
            {
                if (GPIO_getInputPinValue(SW2_PORT, SW2_PIN) == 0)
                {
                    return;
                }

                Send_Front_Trig();
                f_s = Read_Front_Echo();

                unsigned int b;
                char s_d1[6];
                unsigned int dummyb = f_s;
                for (b = 0; b < 6; b++)
                {
                    s_d1[b] = dummyb % 10;
                    dummyb /= 10;
                    showChar('0' + (s_d1[b]), pos[5 - b]);
                }

                if (GPIO_getInputPinValue(SW1_PORT, SW1_PIN) == 0)
                {
                    front_threshold[F_WARNING] = f_s;
                    clearLCD();
                    showChar('T', pos1);
                    showChar('2', pos2);
                    showChar(' ', pos3);
                    showChar('S', pos4);
                    showChar('E', pos5);
                    showChar('T', pos6);
                    Delay_MS(2000);
                    clearLCD();
                    break;
                }
            }
            break;
        }
    }

    showChar('D', pos3);
    showChar('O', pos4);
    showChar('N', pos5);
    showChar('E', pos6);
    Delay_MS(2000);
    clearLCD();
}

void Init_GPIO(void)
{
    /* set all GPIO pins to output low to prevent floating input and reduce power consumption */
    GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN0 | GPIO_PIN1 | GPIO_PIN2 | GPIO_PIN3 | GPIO_PIN4 | GPIO_PIN5 | GPIO_PIN6 | GPIO_PIN7);
    GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN0 | GPIO_PIN1 | GPIO_PIN2 | GPIO_PIN3 | GPIO_PIN4 | GPIO_PIN5 | GPIO_PIN6 | GPIO_PIN7);
    GPIO_setOutputLowOnPin(GPIO_PORT_P3, GPIO_PIN0 | GPIO_PIN1 | GPIO_PIN2 | GPIO_PIN3 | GPIO_PIN4 | GPIO_PIN5 | GPIO_PIN6 | GPIO_PIN7);
    GPIO_setOutputLowOnPin(GPIO_PORT_P4, GPIO_PIN0 | GPIO_PIN1 | GPIO_PIN2 | GPIO_PIN3 | GPIO_PIN4 | GPIO_PIN5 | GPIO_PIN6 | GPIO_PIN7);
    GPIO_setOutputLowOnPin(GPIO_PORT_P5, GPIO_PIN0 | GPIO_PIN1 | GPIO_PIN2 | GPIO_PIN3 | GPIO_PIN4 | GPIO_PIN5 | GPIO_PIN6 | GPIO_PIN7);
    GPIO_setOutputLowOnPin(GPIO_PORT_P6, GPIO_PIN0 | GPIO_PIN1 | GPIO_PIN2 | GPIO_PIN3 | GPIO_PIN4 | GPIO_PIN5 | GPIO_PIN6 | GPIO_PIN7);
    GPIO_setOutputLowOnPin(GPIO_PORT_P7, GPIO_PIN0 | GPIO_PIN1 | GPIO_PIN2 | GPIO_PIN3 | GPIO_PIN4 | GPIO_PIN5 | GPIO_PIN6 | GPIO_PIN7);
    GPIO_setOutputLowOnPin(GPIO_PORT_P8, GPIO_PIN0 | GPIO_PIN1 | GPIO_PIN2 | GPIO_PIN3 | GPIO_PIN4 | GPIO_PIN5 | GPIO_PIN6 | GPIO_PIN7);

    /* outputs */
    GPIO_setAsOutputPin(GPIO_PORT_P5, GPIO_PIN2); /* green LED */
    GPIO_setAsOutputPin(GPIO_PORT_P5, GPIO_PIN3); /* yellow LED */
    GPIO_setAsOutputPin(GPIO_PORT_P1, GPIO_PIN3); /* orange LED */
    GPIO_setAsOutputPin(GPIO_PORT_P1, GPIO_PIN4); /* red LED */
    GPIO_setAsOutputPin(GPIO_PORT_P1, GPIO_PIN5); /* buzzer */
    GPIO_setAsOutputPin(GPIO_PORT_P1, GPIO_PIN1); /* back ultrasonic trig pin */
    GPIO_setAsOutputPin(GPIO_PORT_P8, GPIO_PIN1); /* front ultrasonic trig pin */
    GPIO_setAsOutputPin(LED1_PORT, LED1_PIN);     /* on-board led 1 (comment if using UART) */
    GPIO_setAsOutputPin(LED2_PORT, LED2_PIN);     /* on-board led 2 (comment if using UART) */

    /* inputs */
    GPIO_setAsInputPinWithPullUpResistor(SW1_PORT, SW1_PIN); /* on-board pb 1 */
    GPIO_setAsInputPinWithPullUpResistor(SW2_PORT, SW2_PIN); /* on-board pb 2 */
    GPIO_setAsInputPin(GPIO_PORT_P2, GPIO_PIN7);             /* front ultrasonic echo pin */
    GPIO_setAsInputPin(GPIO_PORT_P2, GPIO_PIN5);             /* back ultrasonic echo pin */

    /* set LaunchPad switches as inputs - they are active low, meaning '1' until pressed */
    GPIO_setAsInputPinWithPullUpResistor(SW1_PORT, SW1_PIN);
    GPIO_setAsInputPinWithPullUpResistor(SW2_PORT, SW2_PIN);
}

void main(void)
{
    __disable_interrupt();  /* turns off interrupts during init */
    WDT_A_hold(WDT_A_BASE); /* stop watchdog timer */

    Init_GPIO(); /* sets all pins to output low as a default */
    Init_LCD();  /* sets up the LaunchPad LCD display */
    Init_Sensor_Data();
    Display_User();

    PMM_unlockLPM5(); /* disable the GPIO power-on default high-impedance mode to activate previously configured port settings */
    __enable_interrupt();

    unsigned int front_sensor_val = 0;
    unsigned int back_sensor_val = 0;
    int beeped = 0;
    int state = LED_OFF;

    while (1)
    {
        if (GPIO_getInputPinValue(SW1_PORT, SW1_PIN) == 0) /* if setup mode PB is pressed */
        {
            Setup_Mode();
            Display_User();
        }

//        if (GPIO_getInputPinValue(SW2_PORT, SW2_PIN) == 0)
//        {
//            Display_User();
//        }

        /* send and receive front sensor */
        Send_Front_Trig();
        front_sensor_val = Read_Front_Echo();

//        unsigned int i;
//        char sensor_data[6];
//        unsigned int dummy = front_sensor_val;
//        for (i = 0; i < 6; i++)
//        {
//            sensor_data[i] = dummy % 10;
//            dummy /= 10;
//            showChar('0' + (sensor_data[i]), pos[5 - i]);
//        }

        if (front_sensor_val <= front_threshold[F_DANGER])
        {
            if (beeped != HIGH_BEEP)
            {
                Play_High_Warning();
                beeped = HIGH_BEEP;
            }
        }
        else if (front_sensor_val <= front_threshold[F_WARNING])
        {
            if (beeped != LOW_BEEP)
            {
                Play_Low_Warning();
                beeped = LOW_BEEP;
            }
        }
        else
        {
            beeped = 0;
        }
        
        /* send and receive back sensor */
        Send_Back_Trig();
        back_sensor_val = Read_Back_Echo();

        unsigned int i;
        char sensor_data[6];
        unsigned int dummy = back_sensor_val;
        for (i = 0; i < 6; i++)
        {
            sensor_data[i] = dummy % 10;
            dummy /= 10;
            showChar('0' + (sensor_data[i]), pos[5 - i]);
        }

        if (back_sensor_val <= back_threshold[B_DANGER])
        {
            if (state != RED_LED)
            {
                back_threshold[B_DANGER] += 3;
                Activate_LED(RED_LED);
                state = RED_LED;
            }
        }
        else if (back_sensor_val <= back_threshold[B_WARNING])
        {
            if (state != ORANGE_LED)
            {
                back_threshold[B_DANGER] -= 3;
                back_threshold[B_WARNING] += 3;
                Activate_LED(ORANGE_LED);
                state = ORANGE_LED;
            }
        }
        else if (back_sensor_val <= back_threshold[B_CAUTION])
        {
            if (state != YELLOW_LED)
            {
                back_threshold[B_WARNING] -= 3;
                back_threshold[B_CAUTION] += 3;
                Activate_LED(YELLOW_LED);
                state = YELLOW_LED;
            }
        }
        else
        {
            if (state != GREEN_LED)
            {
                back_threshold[B_CAUTION] -= 3;
                Activate_LED(GREEN_LED);
                state = GREEN_LED;
            }
        }
    }
}
