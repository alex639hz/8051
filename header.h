#ifndef __C8051F020__
#define __C8051F020__


#include "C8051F020.h"



#define NULL 			0		
#define SYSCLK			22118400L	//22.1184Mhz
#define SARCLK			2500000L	//2.5Mhz  
// #define T4CLK			10000L		//hz T=100us
#define T4CLK			22118L		//1000[hz]
// #define BAUDRATE		230400L			//uart   clock frequency [bps]
#define BAUDRATE		115200L			//uart   clock frequency [bps]
#define INTERRUPT_TIMER4 	16	//interrupt index
#define INTERRUPT_TIMER2 	5	//interrupt index
#define INTERRUPT_TIMER1 	3	//interrupt index

#define FALSE 	0 
#define TRUE 	1


#define MAX_DIGITAL_PINS	48		//digital output

#define DIG_PORT_0	P4	//[00..07]MOLEX_P70/P72 Digital OUT
#define DIG_PORT_1	P5	//[08..15]MOLEX_P71/P73 Digital OUT
#define DIG_PORT_2	P3	//[16..23]DIP SWITCH Digital IN
#define DIG_PORT_3	P2	//[24..31]MOLEX_P69 Digital IN
#define DIG_PORT_4	P6	//[32..39]MOLEX_P63 Digital IN
#define DIG_PORT_5	P7	//[40..47]MOLEX_P68 Digital IN
#define DIG_PORT_x	P0	//[??..??]MOLEX_P68 Digital IN

#define ADDRESS_PORT	P3



// sbit	UART_READ_DISABLE		=	P0^2;				//read enable(active low) max3086 only for GPRIO
// sbit	UART_WRITE_ENABLE		=	P0^3;				//write enable(active high) max3086 only for GPRIO
// sbit	DHT22_LINE				=	P0^3;				//write enable(active high) max3086 only for GPRIO



#define BYTELOW(v)   (*(((unsigned char *) (&v) + 1)))
#define BYTEHIGH(v)  (*((unsigned char *) (&v)))



//GPRIO SETUP::::::::::::::::::::::::::::::::::::

//old configurations
// #define PORT_IN_P69				P2	[0..7]	
// P3 definition see DEBBUG section
// #define PORT_IN_DIP_SWITCH 		P3
// #define PORT_OUT_P70	 			P4					
// #define PORT_OUT_P71	 			P5				
// #define PORT_IN_P63				P6	[8..15]			
// #define PORT_IN_P68				P7	[16..23]		//rotation switches port	

//C8051F020KIT SETUP::::::::::::::::::::::::::::
/*
#define MAX_DOUT_PINS	16	//digital output
#define MAX_DIN_PINS	24	//digital output
#define DIN_PORT_0	P2	//MOLEX_P69
#define DIN_PORT_1	P3	//DIP SWITCH
#define DIN_PORT_2	P6	//MOLEX_P63
#define DIN_PORT_3	P7	//MOLEX_P68
#define DOUT_PORT_0	P4	//MOLEX_P70/P72
#define DOUT_PORT_1	P5	//MOLEX_P71/P73
#define DOUT_PORT_2	P5
#define DOUT_PORT_3	P5
#define ADDRESS_PORT	P3
sbit	UART_READ_DISABLE		=	P0^2;	//read max3086 enable(active low)
sbit	UART_WRITE_ENABLE		=	P0^3;	//write max3086 enable(active high)
*/
//C8051F020KIT SETUP::::::::::::::::::::::::::::



// rx string  "<aa,bb,cc,dd,ee,ff,..>"
#define RX_BUFFER_SIZE 	16
#define TX_BUFFER_SIZE 	16
#define RX_START_FRAME  '{'		//pc(tx)->mcu(rx)
#define RX_STOP_FRAME  	'}'		//'\n'
// #define RX_STOP_FRAME  	'>'		//pc(tx)->mcu(rx)
#define TX_START_FRAME 	'<'		//pc(rx)<-mcu(tx)
#define TX_STOP_FRAME 	'>'		//'\n'
// #define TX_STOP_FRAME 	'}'		//pc(rx)<-mcu(tx)

/*var128 is a variables array of 128 bytes which is a buffer for scratch pad memory area*/
	
#define	VAR128_VADDR	0		
#define	VAR128_SN_0		1		
#define	VAR128_SN_1		2	
#define	VAR128_SN_2		3
#define	VAR128_SN_3		4


//P0 uart and other io
//P1 ADC1(x8)+ADC0(x8)+DAC(x2)  //16 ain , 2 aout
//P2 GPIO in					//24 din
//P3 GPIO in
//P4 GPIO in
//P5 GPIO out					//24 dout
//P6 GPIO out
//P7 GPIO out
// #define LOWBYTE(v)   ((unsigned char) (v))
// #define HIGHBYTE(v)  ((unsigned char) (((unsigned int) (v)) >> 8))
/*void test (void)  {
	// unsigned int  x;
	// unsigned char cl, ch;
	// cl = LOWBYTE(x);
	// ch = HIGHBYTE(x);
}*/



/*void test1 (void)  {
  BYTELOW(x) = BYTEHIGH(x);
  BYTEHIGH (x) = 5;
}*/

typedef unsigned char 	u8;
typedef signed   char 	s8;
typedef unsigned int  	u16;
typedef signed   int  	s16;
typedef unsigned long  	u32;
typedef signed   long  	s32;


typedef struct bufType{
	u8 idx;
	u8 size;
	u8 sum;
	u8 buf[32];
	
}bufType;
typedef struct msgType{
	u8 xor;
	u8 size;
	u8 addr;
	u8 type;
	u8 idx;
	u8 val;
	u8 t1;
	u8 t2;
}msgType;
typedef struct {
	u16 us;
	u16 ms;
	u16 sec;
}timeType;
typedef struct {
	u16 ms;
	u16	sec;
}timerType;







/*##########################################################################*/
// function prototypes:

void Tx_init	();
void Tx_set		(u8 i,u8 var);
void Rx_init	();
void Loopback	();
void SendConstMsg(u16 *pmsg);

void Rxmsg_dout_wr	(/*rxmsg*/);
u16 Conv_u8_to_str	(u8 val);
u8 	Conv_str_to_u8	(u16 str2);
u8	Conv_ascii_to_int		(u8 ch);
u8	Conv_int_to_ascii(u8 prm);	//low 4bit hex integer converted to string char, prm=[0x0..0xF]
u8	Rx_get_u8	(u8 i)	;

bit  	get_din	(u8 idx);
u16 	get_ain	(u8 ch,u8 gain);
void 	set_aout(u8 ch, u16 val);
void 	set_dout(u8 idx,u8 val);
void 	set_dout_mode(u8 ch, u8 mode);

void 	ADC0_set_input_pair	(u8 ch,u8 val);

// FLASH read/write/erase routines
void FLASH_ByteWrite (u16 addr, u8 byte, bit SFLE);
u8	 FLASH_ByteRead (u16 addr, bit SFLE);
void FLASH_PageErase (u16 addr, bit SFLE);
void FLASH_Save();
void FLASH_Load();


#endif


