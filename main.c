/*
 * main.c
 */

#define PART_TM4C123GH6PM

#include <stdint.h>
#include <stdbool.h>
#include "stdlib.h"
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_uart.h"
#include "inc/hw_gpio.h"
#include "inc/hw_pwm.h"
#include "inc/hw_types.h"
#include "driverlib/adc.h"
#include "driverlib/timer.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/pin_map.h"
#include "driverlib/rom.h"
#include "driverlib/rom_map.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"
#include "driverlib/udma.h"
#include "driverlib/pwm.h"
#include "driverlib/ssi.h"
#include "driverlib/systick.h"
#include "driverlib/adc.h"
#include "utils/uartstdio.h"
#include "utils/uartstdio.c"
#include <string.h>

#include <stdint.h>
#include <stdbool.h>
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/debug.h"
#include "driverlib/sysctl.h"
#include "driverlib/adc.h"
#include "driverlib/uart.h"

void PWMint();

#include "inc/hw_ints.h"
#include "inc/hw_timer.h"
#include "inc/hw_gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/sysctl.c"
#include "driverlib/sysctl.h"
#include "driverlib/Timer.c"
#include "driverlib/Timer.h"
#include "driverlib/gpio.c"
#include "driverlib/gpio.h"


 //about 2ms at 80Mhz
#define time 56666

//PWM frequency in hz
uint32_t freq = 100000;

void itoa(int n, char s[]);
void reverse(char s[]);
int mystrlen(char s[]);

volatile uint32_t millis=0;
void SycTickInt(){
  millis++;
}
void SysTickbegin(){
  SysTickPeriodSet(80000);
  SysTickIntRegister(SycTickInt);
  SysTickIntEnable();
   SysTickEnable();
}

void
InitConsole(void)
{

    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    GPIOPinConfigure(GPIO_PA0_U0RX);
    GPIOPinConfigure(GPIO_PA1_U0TX);

    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
    UARTClockSourceSet(UART0_BASE, UART_CLOCK_PIOSC);
    GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);

    UARTStdioConfig(0, 9600, 16000000);
}



int main(void) {
    uint32_t ui32ADC0Value[4];
    volatile uint32_t ui32SignalAvg;
    char numberStr[10];
    int i;

    SysCtlClockSet(
    SYSCTL_SYSDIV_5 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ);

    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    GPIOPinConfigure(GPIO_PA0_U0RX);
    GPIOPinConfigure(GPIO_PA1_U0TX);
    GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);
    UARTConfigSetExpClk(UART0_BASE, SysCtlClockGet(), 9600,
            (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE));

    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
    GPIOPinTypeADC(GPIO_PORTE_BASE, GPIO_PIN_0);

    SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);
    ADCSequenceConfigure(ADC0_BASE, 1, ADC_TRIGGER_PROCESSOR, 0);
    ADCSequenceStepConfigure(ADC0_BASE, 1, 0, ADC_CTL_CH3);
    ADCSequenceStepConfigure(ADC0_BASE, 1, 1, ADC_CTL_CH3);
    ADCSequenceStepConfigure(ADC0_BASE, 1, 2, ADC_CTL_CH3);
    ADCSequenceStepConfigure(ADC0_BASE, 1, 3,
            ADC_CTL_CH3 | ADC_CTL_IE | ADC_CTL_END);
    ADCSequenceEnable(ADC0_BASE, 1);



    uint32_t Period, dutyCycle;
      Period = SysCtlClockGet()/freq ;
      dutyCycle = Period-2;

      /*
        Configure PF1 as T0CCP1
        Configure PF2 as T1CCP0
        Configure PF3 as T1CCP1
      */

      SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
      SysCtlDelay(3);
      GPIOPinConfigure(GPIO_PF1_T0CCP1);
      GPIOPinConfigure(GPIO_PF2_T1CCP0);
      GPIOPinConfigure(GPIO_PF3_T1CCP1);
      GPIOPinTypeTimer(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3);
      //

      /*
        Configure timer 0 to split pair and timer B in PWM mode
        Set period and starting duty cycle.
      */
      SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);
      SysCtlDelay(3);
      TimerConfigure(TIMER0_BASE, TIMER_CFG_SPLIT_PAIR|TIMER_CFG_B_PWM);
      TimerLoadSet(TIMER0_BASE, TIMER_B, Period -1);
      TimerMatchSet(TIMER0_BASE, TIMER_B, dutyCycle); // PWM


      /*
        Configure timer 1 to split pair and timer A and B in PWM mode
        Set period and starting duty cycle.
      */
      SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER1);
      SysCtlDelay(3);
      TimerConfigure(TIMER1_BASE, TIMER_CFG_SPLIT_PAIR|TIMER_CFG_A_PWM|TIMER_CFG_B_PWM);
      TimerLoadSet(TIMER1_BASE, TIMER_A, Period -1);
      TimerLoadSet(TIMER1_BASE, TIMER_B, Period -1);
      TimerMatchSet(TIMER1_BASE, TIMER_A, dutyCycle);
      TimerMatchSet(TIMER1_BASE, TIMER_B, dutyCycle);

      //Turn on both timers
      TimerEnable(TIMER0_BASE, TIMER_B);
      TimerEnable(TIMER1_BASE, TIMER_A|TIMER_B);




      //SysCtlClockSet(SYSCTL_SYSDIV_2_5|SYSCTL_USE_PLL|SYSCTL_OSC_MAIN|SYSCTL_XTAL_16MHZ);
      		 SysTickbegin();
      		  InitConsole();

      	    uint32_t ADCValues[1];
      	    uint32_t TempValueC;
      	    uint32_t TempValueF;
      	    UARTprintf("ADC ->\n");
      	    UARTprintf("  Type: Internal Temperature Sensor\n");
      	    UARTprintf("  Samples: One\n");
      	    UARTprintf("  Update Rate: 250ms\n");
      	    UARTprintf("  Input Pin: Internal temperature sensor\n\n");

      	    SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);
      	    //SysCtlDelay(3);

      	    ADCSequenceConfigure(ADC0_BASE, 3, ADC_TRIGGER_PROCESSOR, 0);

      	    ADCSequenceStepConfigure(ADC0_BASE, 3, 0, ADC_CTL_TS | ADC_CTL_IE |
      	                             ADC_CTL_END);

      	    ADCSequenceEnable(ADC0_BASE, 3);
      	    ADCIntClear(ADC0_BASE, 3);




    while (1) {
        ADCIntClear(ADC0_BASE, 1);
        ADCProcessorTrigger(ADC0_BASE, 1);
        while (!ADCIntStatus(ADC0_BASE, 1, false));
        ADCSequenceDataGet(ADC0_BASE, 1, ui32ADC0Value);

        ui32SignalAvg = (ui32ADC0Value[0] + ui32ADC0Value[1] + ui32ADC0Value[2]
                + ui32ADC0Value[3] + 2) / 4;

    //Blue brightness goes up - PF2


        itoa(ui32SignalAvg,numberStr);
        for(i=0; numberStr[i] != '\0' ; ++i){
            UARTCharPut(UART0_BASE,numberStr[i]);
        }
        UARTCharPut(UART0_BASE,'\r');
        UARTCharPut(UART0_BASE,'\n');

        //Blue brightness goes up - PF2
        int number=0;
        sscanf(numberStr, "%d", &number);
        if(number<700){
           for(i=Period-2; i > 0;i--){
             TimerMatchSet(TIMER1_BASE, TIMER_A, i);
             //SysCtlDelay(time);
           }
        }
        if(number>700 && number<1300){
           //Red brightness goes down - PF1
           for(i=1; i < Period-1; i++){
             TimerMatchSet(TIMER0_BASE, TIMER_B, i);
             //SysCtlDelay(time);
           }
        }
           //Green brightness goes up - PF3
        if(number>1300 && number<2000){
           for(i=Period-2; i >  0;i--){
             TimerMatchSet(TIMER1_BASE, TIMER_B, i);
             //SysCtlDelay(time);
           }
        }
           //Blue brightness goes down - PF2
        if(number>2000 && number<2600){
           for(i=1; i < Period-1; i++){
             TimerMatchSet(TIMER1_BASE, TIMER_A, i);
             //SysCtlDelay(time);
           }
        }
           //Red brightness goes up - PF1
        if(number>2600 && number<3100){
           for(i=Period-2; i > 0;i--){
             TimerMatchSet(TIMER0_BASE, TIMER_B, i);
             //SysCtlDelay(time);
           }
        }
           //Green brightness goes down - PF3
        if(number>3100){
           for(i=1; i < Period-1; i++){
             TimerMatchSet(TIMER1_BASE, TIMER_B, i);
             //SysCtlDelay(time);
           }
        }
        SysCtlDelay(3);
        ADCProcessorTrigger(ADC0_BASE, 3);

		while(!ADCIntStatus(ADC0_BASE, 3, false))
		{
		}

		ADCIntClear(ADC0_BASE, 3);

		ADCSequenceDataGet(ADC0_BASE, 3, ADCValues);

		TempValueC = (uint32_t)(147.5 - ((75.0*3.3 *(float)ADCValues[0])) / 4096.0);

		UARTprintf("Sicaklik = %3d*C\r", TempValueC
				);

        //SysCtlDelay(SysCtlClockGet() / (3*20));

    }

}

void itoa(int n, char s[]) {
    int i, sign;

    if ((sign = n) < 0) /* record sign */
        n = -n; /* make n positive */
    i = 0;
    do { /* generate digits in reverse order */
        s[i++] = n % 10 + '0'; /* get next digit */
    } while ((n /= 10) > 0); /* delete it */
    if (sign < 0)
        s[i++] = '-';
    s[i] = '\0';
    reverse(s);
}

/* reverse:  reverse string s in place */
void reverse(char s[]) {
    int i, j;
    char c;

    for (i = 0, j = mystrlen(s) - 1; i < j; i++, j--) {
        c = s[i];
        s[i] = s[j];
        s[j] = c;
    }
}

int mystrlen(char s[]) {
    int len = 0;

    while ((*s) != '\0') {
        ++s;
        ++len;
    }
    return len;
}
