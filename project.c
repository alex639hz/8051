#include "header.h"
#include "uart.h"

/*##########################################################################*/	
static timeType time;		
static u16 dout_timeout_sec[MAX_DIGITAL_PINS];
static u16 dout_timeout_ms[MAX_DIGITAL_PINS];
static u8 var128[128];
static bit flag_1sec;	//stores array of 40-bits 
// static u8 flag_loopback_debug = TRUE;	// make uart loopback (for debbug)  
/*##########################################################################*/// functions:
/*##########################################################################*/// main
void Main (void){		
	
	// set_dout(8,0);
	uart_init();
	{//initialization section
		u16 i;
		{//WatchDog Disable
				EA=0;
				WDTCN=0xDE;
				WDTCN=0xAD;
		}
		
		{//XOSC
			OSCXCN = 0x67;				//XOSC Mode=Crystal oscillator mode, freq>6.7 MHZ 
			
			for(i=2000; i>0 ;i--);		//1ms delay(2MHZ Clock)
				
			while(!(OSCXCN & 0x80)); 	//poll XTLVLD=>'1' 		
			OSCICN =0x08;				//Clock Select = external osc
		}	
			
		{//timers
			//TIMER4 - general clock, Timer4_ISR		
			TMR4=RCAP4=-22118;	//0x5666	//1ms cycle
			// TMR4=RCAP4=-SYSCLK/T4CLK;
			CKCON|=0x40;						//set T4M=1 (timer4 is timer function and uses system clk)
			T4CON|=0x04; //set TR4 bit - start timer4
		}
		
		{//PORTS
		
		
			// #define PORT_IN_P69			P2		
			// #define PORT_IN_DIP_SWITCH 	P3
			// #define PORT_OUT_P70	 		P4					
			// #define PORT_OUT_P71	 		P5				
			// #define PORT_IN_P63			P6				
			// #define PORT_IN_P68			P7			//rotation switches port	

			
			// P0MDOUT |=0x0d;				//P0.0(TX0),P0.2(RE_),P0.3(DE) are push-pull. support GPRIO boards
			// P0MDOUT =0x0d;				//P0.0(TX0),P0.2(RE_),P0.3(DE) are push-pull. support GPRIO boards
			P0MDOUT |= 0x01;                    // Set TX0(P0.0) port pin to push-pull
		
			//analog inputs P1(ADC1)
			
			P1MDOUT=0x0;	//set open-drain mode on P1
			P1MDIN=0x0;		//set analog input mode
			
			// P1MDOUT |= 0x40;    //Enable P1.6 (LED) as push-pull output.					
			//*digital inputs P3,P6,P7
			//*digital outputs P2,P4,P5 			
			// P2MDOUT=0x0;	//set open-drain mode on P2
			// P2MDOUT|=0x01;	//set P2.0 push-pull others are open-drain,  DIG_PORT_3 [24..31]
			P3MDOUT=0x0;	//set open-drain mode on P3
			P74OUT=0x00;	//set open-drain mode on P4,P5,P6,P7 
			
			P2=0xff;
			P3=0xff;
			P4=0xff;
			P5=0xff;
			P6=0xff;
			P7=0xff;			
			
		}//PORTS
		
		{//cross-bar
			XBR0=0x04;	//route TX0,RX0 to P0.0, P0.1	//ENABLED FOR C8051F020			
			// XBR2=0x40;	//enable cross-bar,enable week-pull-ups. 	
			XBR2=0xC0;	//enable cross-bar,disable week-pull-ups. 	
		}/*cross-bar*/		
		
		{//ADC[0..1] 
			ADC0CN=0x80;	//set adc0 enable
			ADC1CN=0x80;	//set adc1 enable
			REF0CN=0x07;		//VREF0 must externally connect to VREF , BIASE=REFBE=TEMPE='1'	,ADC[0..1] voltage reference from VREF[0..1] pin accordingly.
			ADC0CF = (SYSCLK/SARCLK) << 3;     // ADC conversion clock = 2.5MHz
			ADC0CF |= 0x00;                     // PGA gain = 1 (default)
			ADC1CF=0xfa;
		}//ADC[0..1]
		
		{//DAC0,DAC1		
			DAC0L=0x0;
			DAC0H=0x0;
			DAC0CN=0x80;		
			DAC1L=0x0;
			DAC1H=0x0;
			DAC1CN=0x80;
		}//DAC[0..1]
	
		{//clock init
			time.us=0L;
			time.ms=0L;
			time.sec=0x0L;
			
			for(i=0;i<MAX_DIGITAL_PINS;i++){
				dout_timeout_sec[i]=0;
			}
		}
	
		{//Interrupts and CPU stack 
			SP=0x30;	//stack initiale offset
			PS0=1;		//uart ISR gets high priority
			ES0=1;		//uart0_enable_interrupt
			EIE2|=0X04;	//ENABLE timer4 INTERRUPT
			EA=1;
		}	

		{//FLASH
			FLASH_Load();
		}		
	}//initialization section

	while(1){
		WDTCN=0xAD;
		if(flag_1sec){
			flag_1sec=0;
		}	
	}//main loop
}

/*##########################################################################*/// interrupts
/*general clock 1khz,T=1ms, function_duration~0.1[ms] */
void interrupt_timer4(void) interrupt 16 using 3 {
	u8 i;
	T4CON&=~0x80; 	//clear TF4 interrupt flag
	WDTCN=0xAD;
	WDTCN=0xFF;
	time.ms++;			
	for(i=0;i<MAX_DIGITAL_PINS;i++){	//process dout timeout 
		if(dout_timeout_sec[i]>0){		//check if digital pin have a timeout counter
			dout_timeout_ms[i]++;
			if(dout_timeout_ms[i]>999){
				dout_timeout_ms[i]=0;
				dout_timeout_sec[i]--;
				if(dout_timeout_sec[i]==0){
					set_dout(i,FALSE);	//set digital pin to high-z (OFF)
				}				
			}	
		}
	}
	if(time.ms>999){		//increas timer_sec
		time.ms=0;		
		time.sec++;
		flag_1sec=1;
	}
}//Timer4_ISR (void)

void interrupt_uart(void) interrupt 4 using 2 {
	uart_service();
}//uart0_int

