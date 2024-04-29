#include "header.h"

/*##########################################################################*/
//variables:
// sbit	RE		=	P0^2;				//read max3086 enable(active low)
// sbit	DE		=	P0^3;				//write max3086 enable(active high)
// static u8 VIRTUAL_ADDRESS;
static bufType rx;
static msgType rxmsg;
static bufType tx;
static msgType txmsg;		
static timeType time;		
static u16 dout_timeout_sec[MAX_DIGITAL_PINS];
static u16 dout_timeout_ms[MAX_DIGITAL_PINS];
static u8 var128[128];
static bit flag_1sec;	//stores array of 40-bits 
static u8 flag_loopback_debug = TRUE;	// make uart loopback (for debbug)  
/*##########################################################################*/// functions:

// void SendConstMsg(u16 *pmsg){	//send err_msg_x
// 	// long code * idata powtab;      /* ptr in idata to code long */
// 	u8 code * pmsg2=pmsg;			//por to u8 in xdata
// 	// u8 code * xdata pmsg2=pmsg;			//por to u8 in xdata

// 	Tx_init();	//resets tx_buffer[] and tx_idx
// 	if(pmsg2!=NULL){//copy from char[] in code 
// 		while((pmsg2[tx.idx]!=0)&&(tx.idx<TX_BUFFER_SIZE-1)){
// 			tx.buf[tx.idx+1]=pmsg2[tx.idx];
// 			tx.idx++;
// 		}		
// 		tx.buf[++tx.idx]=TX_STOP_FRAME;
// 		tx.buf[0]=TX_START_FRAME;
// 		tx.size=tx.idx;
// 		tx.idx=0; 	//transmition will use tx.idx )in TI0 case
// 		TI0=TRUE;		//start transmition
// 	}
// }//SendConstMsg()

void Loopback(){//loop back input or send msg
	u8 i;
	Tx_init();	//resets tx_buffer[] and tx_idx	
	for(i=0;i<RX_BUFFER_SIZE;i++){
		tx.buf[i]=rx.buf[i];
		if(tx.buf[i]==RX_STOP_FRAME){
			break;
		}
	}
	tx.buf[i]=TX_STOP_FRAME;
	tx.buf[0]=TX_START_FRAME;
	tx.size=i+1;
	tx.idx=0; 	//transmition will use tx.idx )in TI0 case
	TI0=TRUE;		//start transmition

}//Loopback
	
void Rxmsg_dout_wr(/*rxmsg*/){

	if(rxmsg.idx<MAX_DIGITAL_PINS){		
		dout_timeout_sec[rxmsg.idx]=((u16)rxmsg.t1)<<8;
		dout_timeout_sec[rxmsg.idx]+=rxmsg.t2;	
		set_dout(rxmsg.idx,rxmsg.val);	
	}
}//Rxmsg_dout_wr()
	
u8 	Rx_get_u8(u8 i){				//returns "FF" ascii value from tx_buffer start at index
	
	return (Conv_ascii_to_int(rx.buf[i])<<4)+(Conv_ascii_to_int(rx.buf[i+1]));
}

void Tx_set(u8 i,u8 var){		//set tx.buf after converting u8 to "xx" str
	u16 prm=Conv_u8_to_str(var);	
	tx.buf[i]=BYTEHIGH(prm);
	tx.buf[i+1]=BYTELOW(prm);
}

u8 	Conv_ascii_to_int(u8 ch){	//converts ascii['0',..'9','A',..'F'] to int[0..15], returns 0xff if failed
	
	if(ch>='0' && ch<='9'){
		return ch-'0';
	}else if(ch>='A' && ch<='F'){
		return (ch-'A'+10);		
	}else
		return 0xff;
}

u8	Conv_int_to_ascii(u8 prm){	//low 4bit hex integer converted to string char, prm=[0x0..0xF]
	if(prm<10){	//prm=[0,1,2,..9]
		prm+='0';
	}else{			//prm=[0xA,0xB,0xC,..0xF]
		prm+=('A'-10);
	}
	return prm;
}

u8 	Conv_str_to_u8(u16 str2){	//inputs 2 ascii bytes outputs u8 integer (number2string)
	
	u16 str=str2;
	u8 ret=0;
	u8 prm1=BYTEHIGH(str);	//read 1st ascii
	u8 prm2=BYTELOW(str);	//read 2nd ascii
	
	prm1=Conv_ascii_to_int(prm1);
	prm2=Conv_ascii_to_int(prm2);
	
	ret=prm1<<4+prm2;
	return ret;
}

u16 Conv_u8_to_str(u8 val){		//input u8 value output 2 ascii bytes (number2string)
	u8 prm1=val&0x0f;
	u8 prm2=(val&0xf0)>>4;
	u16 ret=0x0;
	
	// if(prm1<10){	//prm=[0,1,2,..9]
		// prm1+='0';
	// }else{			//prm=[0xA,0xB,0xC,..0xF]
		// prm1+=('A'-10);
	// }
	prm1=Conv_int_to_ascii(prm1);
	prm2=Conv_int_to_ascii(prm2);
	
	// if(prm2>10){	//prm=[0xA,0xB,0xC,..0xF]
		// prm2+=('A'-10);
	// }else{			//prm=[0,1,2,..9]
		// prm2+='0';
	// }

	BYTEHIGH(ret) = prm2;
	BYTELOW(ret) = prm1;
	
	return ret;
}

void Rx_init(void){
	
	for(rx.idx=0;rx.idx<RX_BUFFER_SIZE;rx.idx++){
		rx.buf[rx.idx]=0;
	}
	rx.idx=0;
	// rxmsg.
}

void Tx_init(void){
	
	for(tx.idx=0;tx.idx<RX_BUFFER_SIZE;tx.idx++){
		tx.buf[tx.idx]=0;
	}
	tx.idx=0;
}

void Buf_init(u16 *ptr){
	
	bufType xdata  *bufp=ptr; //point to bufType in xdata
	
	for(bufp->idx=0;bufp->idx<RX_BUFFER_SIZE;bufp->idx++){
		bufp->buf[bufp->idx]=0;
	}
	bufp->idx=0;
}

//-----------------------------------------------------------------------------
// FLASH_PageErase
//-----------------------------------------------------------------------------
//
// This routine erases the FLASH page containing the linear FLASH address
// <addr>.
//
void FLASH_PageErase (u16 addr, bit SFLE){

   bit EA_SAVE = EA;                   // preserve EA
   char xdata * data pwrite;           // FLASH write pointer

   EA = 0;                             // disable interrupts

   pwrite = (char xdata *) addr;

   FLSCL |= 0x01;                      // enable FLASH writes/erases
   PSCTL |= 0x03;                      // PSWE = 1; PSEE = 1

   if (SFLE) {
      PSCTL |= 0x04;                   // set SFLE
   }

   *pwrite = 0;                        // initiate page erase

   if (SFLE) {
      PSCTL &= ~0x04;                  // clear SFLE
   }

   PSCTL &= ~0x03;                     // PSWE = 0; PSEE = 0
   FLSCL &= ~0x01;                     // disable FLASH writes/erases

   EA = EA_SAVE;                       // restore interrupts
}

void FLASH_ByteWrite(u16 addr,u8 byte, bit SFLE){
//-----------------------------------------------------------------------------
// FLASH_ByteWrite
//-----------------------------------------------------------------------------
//
// This routine writes <byte> to the linear FLASH address <addr>.
// Linear map is decoded as follows:
// Linear Address       Device Address
// ------------------------------------------------
// 0x00000 - 0x0FFFF    0x0000 - 0xFFFF
//
   bit EA_SAVE = EA;                   // preserve EA
   char xdata * data pwrite;           // FLASH write pointer

   EA = 0;                             // disable interrupts

   pwrite = (char xdata *) addr;

   FLSCL |= 0x01;                      // enable FLASH writes/erases
   PSCTL |= 0x01;                      // PSWE = 1

   if (SFLE) {
      PSCTL |= 0x04;                   // set SFLE
   }

   *pwrite = byte;                     // write the byte

   if (SFLE) {
      PSCTL &= ~0x04;                  // clear SFLE
   }

   PSCTL &= ~0x01;                     // PSWE = 0
   FLSCL &= ~0x01;                     // disable FLASH writes/erases

   EA = EA_SAVE;                       // restore interrupts
}
//-----------------------------------------------------------------------------
// FLASH_ByteRead
//-----------------------------------------------------------------------------
//
// This routine reads a <byte> from the linear FLASH address <addr>.
//
u8 	 FLASH_ByteRead (u16 addr,bit SFLE){

   bit EA_SAVE = EA;                   // preserve EA
   u8 code * data pread;             // FLASH read pointer
   u8 byte;

   EA = 0;                             // disable interrupts

   pread = (u8 code *) addr;

   if (SFLE) {
      PSCTL |= 0x04;                   // set SFLE
   }

   byte = *pread;                      // read the byte

   if (SFLE) {
      PSCTL &= ~0x04;                  // clear SFLE
   }

   EA = EA_SAVE;                       // restore interrupts

   return byte;
}

void FLASH_Save(){
	
	u8 i;
	
	FLASH_PageErase(0,1);
	for(i=0;i<128;i++){
		FLASH_ByteWrite(i,var128[i],1);
	}
	

}

void FLASH_Load(){
	u8 i;
	
	for(i=0;i<128;i++){
		var128[i]=FLASH_ByteRead(i,1);
	}	
}

void ADC0_set_diff(u8 ch,u8 mode){//single-ended mode=0, differential mode=1
	u8 cnt;
	
	if((ch>=0)&&(ch<=7)){
		switch(ch){
			case 0:
			case 1:
				if(!mode){AMX0CF&=~0x01;}	//Bit0=0: AIN0, AIN1 are independent single-ended inputs
				else{AMX0CF|=0x01;}		//Bit0=1: AIN0, AIN1 are (respectively) +, - differential input pair
			break;
			case 2:
			case 3:
				if(!mode){AMX0CF&=~0x02;}	//Bit1=0: AIN2, AIN3 are independent single-ended inputs
				else{AMX0CF|=0x02;}		//Bit1=1: AIN2, AIN3 are (respectively) +, - differential input pair
			break;
			case 4:
			case 5:			
				if(!mode){AMX0CF&=~0x04;}	//Bit2=0: AIN4, AIN5 are independent single-ended inputs
				else{AMX0CF|=0x04;}		//Bit2=1: AIN4, AIN5 are (respectively) +, - differential input pair				
			break;
			case 6:
			case 7:
				if(!mode){AMX0CF&=~0x08;}	//Bit3=0: AIN6, AIN7 are independent single-ended inputs
				else{AMX0CF|=0x08;}		//Bit3=1: AIN6, AIN7 are (respectively) +, - differential input pair				
			break;
		}//switch(ch)	
	}//if((ch>=0)&&(cn<=7)){

	//set_dout(x,1)	//test delay
	for(cnt=5;cnt;cnt--){}	//minimum settling time delay of 1[ns]
	//set_dout(x,0)	//test delay
}

void set_dout(u8 idx,u8 val){//dout[0..23] *digital outputs 	
	u8 bitmask;
	if(idx<MAX_DIGITAL_PINS){
		if(idx>=0 && idx<=7){
			// digital_port_conf|=(0x01<<0);	//set Digital port0 as dout 
			bitmask=0x01<<(idx-0);		
			if(val)	{DIG_PORT_0 &= ~bitmask;}//set to GND
			else	{DIG_PORT_0 |= bitmask;} //set to high-Z
		}
		else if(idx>=8 && idx<=15){
			// digital_port_conf|=(0x01<<1);	//set Digital port1 as dout 
			bitmask=0x01<<(idx-8);		
			if(val)	{DIG_PORT_1 &= ~bitmask;}//set to GND
			else	{DIG_PORT_1 |=bitmask;} //set to high-Z
		}
		else if(idx>=16 && idx<=23){
			// digital_port_conf|=(0x01<<2);	//set Digital port2 as dout 
			bitmask=0x01<<(idx-16);		
			if(val)	{DIG_PORT_2 &= ~bitmask;}//set to GND
			else	{DIG_PORT_2 |=bitmask;} //set to high-Z
		}
		else if(idx>=24 && idx<=31){
			// digital_port_conf|=(0x01<<3);	//set Digital port3 as dout 
			bitmask=0x01<<(idx-24);		
			if(val)	{DIG_PORT_3 &= ~bitmask;}//set to GND
			else	{DIG_PORT_3 |=bitmask;} //set to high-Z
		}
		else if(idx>=32 && idx<=39){
			// digital_port_conf|=(0x01<<4);	//set Digital port4 as dout 			
			bitmask=0x01<<(idx-32);		
			if(val)	{DIG_PORT_4 &= ~bitmask;}//set to GND
			else	{DIG_PORT_4 |=bitmask;} //set to high-Z
		}
		else if(idx>=40 && idx<=47){
			// digital_port_conf|=(0x01<<5);	//set Digital port5 as dout 			
			bitmask=0x01<<(idx-40);		
			if(val)	{DIG_PORT_5 &= ~bitmask;}//set to GND
			else	{DIG_PORT_5 |=bitmask;} //set to high-Z
		}
		
		
	}
}//set_dout()

void set_aout(u8 ch, u16 val){
	
	u8 low	=BYTELOW(val);
	u8 high	=0x0F&BYTEHIGH(val);
	
	switch(ch){
		case 0: {//DAC0
			DAC0L=low;
			DAC0H=high;			
		}break;
		case 1: {//DAC1
			DAC1L=low;
			DAC1H=high;							
		}break;	
	}
	
}

bit  get_din(u8 idx){ 	
	u8 bitmask;
	if(idx<MAX_DIGITAL_PINS){
		if(idx>=0 && idx<=7){			//DIG_PORT_0
			// digital_port_conf&=~(0x01<<0);	//set Digital port0 as din 			
			bitmask=0x01<<(idx-0);		
			DIG_PORT_0=0x0;	//discharge 
			DIG_PORT_0=0xff;//return to open-drain
			if(DIG_PORT_0&bitmask) { return 1;} 
				else {return 0;}
		}
		else if(idx>=8 && idx<=15){		//DIG_PORT_1
			// digital_port_conf&=~(0x01<<1);	//set Digital port1 as din 	
			bitmask=0x01<<(idx-8);		
			DIG_PORT_1=0x0;	//discharge 
			DIG_PORT_1=0xff;//return to open-drain
			if(DIG_PORT_1&bitmask) { return 1;} 
				else {return 0;}
		}
		else if(idx>=16 && idx<=23){	//DIG_PORT_2
			// digital_port_conf&=~(0x01<<2);	//set Digital port2 as din 			
			bitmask=0x01<<(idx-16);		
			DIG_PORT_2=0x0;	//discharge 
			DIG_PORT_2=0xff;//return to open-drain
			if(DIG_PORT_2&bitmask) { return 1;} 
				else {return 0;}
		}
		else if(idx>=24 && idx<=31){	//DIG_PORT_3
			// digital_port_conf&=~(0x01<<3);	//set Digital port3 as din 			
			bitmask=0x01<<(idx-24);		
			DIG_PORT_3=0x0;	//discharge 
			DIG_PORT_3=0xff;//return to open-drain
			if(DIG_PORT_3&bitmask) { return 1;} 
				else {return 0;}
		}
		else if(idx>=32 && idx<=39){	//DIG_PORT_4
			// digital_port_conf&=~(0x01<<4);	//set Digital port4 as din 			
			bitmask=0x01<<(idx-32);		
			DIG_PORT_4=0x0;	//discharge 
			DIG_PORT_4=0xff;//return to open-drain
			if(DIG_PORT_4&bitmask) { return 1;} 
				else {return 0;}
		}
		else if(idx>=40 && idx<=47){	//DIG_PORT_5
			// digital_port_conf&=~(0x01<<5);	//set Digital port5 as din 					
			bitmask=0x01<<(idx-40);		
			DIG_PORT_5=0x0;	//discharge 
			DIG_PORT_5=0xff;//return to open-drain
			if(DIG_PORT_5&bitmask) { return 1;} 
				else {return 0;}
		}
	}
	return 0;

}//get_din()

bit  get_dout(u8 idx){ 	
	u8 bitmask;
	if(idx<MAX_DIGITAL_PINS){
		if(idx>=0 && idx<=7){			//DIG_PORT_0
			// digital_port_conf&=~(0x01<<0);	//set Digital port0 as din 			
			bitmask=0x01<<(idx-0);		
			// DIG_PORT_0=0x0;	//discharge 
			// DIG_PORT_0=0xff;//return to open-drain
			if(DIG_PORT_0&bitmask) { return 1;} 
				else {return 0;}
		}
		else if(idx>=8 && idx<=15){		//DIG_PORT_1
			// digital_port_conf&=~(0x01<<1);	//set Digital port1 as din 	
			bitmask=0x01<<(idx-8);		
			// DIG_PORT_1=0x0;	//discharge 
			// DIG_PORT_1=0xff;//return to open-drain
			if(DIG_PORT_1&bitmask) { return 1;} 
				else {return 0;}
		}
		else if(idx>=16 && idx<=23){	//DIG_PORT_2
			// digital_port_conf&=~(0x01<<2);	//set Digital port2 as din 			
			bitmask=0x01<<(idx-16);		
			// DIG_PORT_2=0x0;	//discharge 
			// DIG_PORT_2=0xff;//return to open-drain
			if(DIG_PORT_2&bitmask) { return 1;} 
				else {return 0;}
		}
		else if(idx>=24 && idx<=31){	//DIG_PORT_3
			// digital_port_conf&=~(0x01<<3);	//set Digital port3 as din 			
			bitmask=0x01<<(idx-24);		
			// DIG_PORT_3=0x0;	//discharge 
			// DIG_PORT_3=0xff;//return to open-drain
			if(DIG_PORT_3&bitmask) { return 1;} 
				else {return 0;}
		}
		else if(idx>=32 && idx<=39){	//DIG_PORT_4
			// digital_port_conf&=~(0x01<<4);	//set Digital port4 as din 			
			bitmask=0x01<<(idx-32);		
			// DIG_PORT_4=0x0;	//discharge 
			// DIG_PORT_4=0xff;//return to open-drain
			if(DIG_PORT_4&bitmask) { return 1;} 
				else {return 0;}
		}
		else if(idx>=40 && idx<=47){	//DIG_PORT_5
			// digital_port_conf&=~(0x01<<5);	//set Digital port5 as din 					
			bitmask=0x01<<(idx-40);		
			// DIG_PORT_5=0x0;	//discharge 
			// DIG_PORT_5=0xff;//return to open-drain
			if(DIG_PORT_5&bitmask) { return 1;} 
				else {return 0;}
		}
	}
	return 0;

}//get_din()

u16  get_ain_adc0(u8 ch0,u8 gain){	//read adc0 channel
	u16 reg16=0L;
	// u8 i;
	// u16 timeout=1;
	// u8 low=0;
	// u8 high=0;
	ADC0CF = (SYSCLK/SARCLK) << 3;     // ADC conversion clock = 2.5MHz	
	ADC0CF |= gain;                     // PGA gain = 1 ( default)
	if((ch0>=0)&&(ch0<=8)){//configure analog input pair as single-ended or differential 
		// AD0EN=1;	//enable adc0		
		AMX0SL=ch0;		
		AD0INT=0;
		AD0INT=0;
		// for(reg16=0;reg16<1000;i++);	//delay
		AD0BUSY=1;
		// while(!AD0INT&&timeout++){};
		while(!AD0INT){};
		AD0INT=0;	
		reg16=ADC0;		
		// AD0EN=0;	//disable adc0
		
	}//if((ch<=0)&&(cn<=7))
	return reg16;	
}//get_ain_adc0()

u8   get_ain_adc1(u8 ch1,u8 gain){	//read adc0 channel
	u16 reg16=0L;
	// u16 timeout=1;
	// u8 low=0;
	// u8 high=0;
	ADC1CF = (SYSCLK/SARCLK) << 3;     // ADC conversion clock = 2.5MHz	
	ADC1CF |= gain;                     // PGA gain = 1 ( default)	
	
	// sbit AD0EN     =   ADC0CN ^ 7;      /* ADC 0 ENABLE                              */
	// sbit AD0TM     =   ADC0CN ^ 6;      /* ADC 0 TRACK MODE                          */
	// sbit AD0INT    =   ADC0CN ^ 5;      /* ADC 0 CONVERISION COMPLETE INTERRUPT FLAG */
	// sbit AD0BUSY   =   ADC0CN ^ 4;      /* ADC 0 BUSY FLAG                           */
	// sbit AD0CM1    =   ADC0CN ^ 3;      /* ADC 0 START OF CONVERSION MODE BIT 1      */
	// sbit AD0CM0    =   ADC0CN ^ 2;      /* ADC 0 START OF CONVERSION MODE BIT 0      */
	// sbit AD0WINT   =   ADC0CN ^ 1;      /* ADC 0 WINDOW COMPARE INTERRUPT FLAG       */
	// sbit AD0LJST   =   ADC0CN ^ 0;      /* ADC 0 RIGHT JUSTIFY DATA BIT              */

	
	if((ch1>=0)&&(ch1<=7)){//only x8 single-ended 

		// ADC1CN|=0x80;	//set adc1 enable
		AMX1SL=ch1;
		ADC1CN&=~0x30;	//ADC1INT=0  
		ADC1CN&=~0x30;	//ADC1INT=0  
		ADC1CN|=0x10;	//AD1BUSY=1
		
		
		// while(!AD0INT&&timeout++){};
		// while(!AD0INT){};
		while(!(ADC1CN&0x30)){};
		// AD0INT=0;
		ADC1CN&=~0x30;	//ADC1INT=0  
		
		// BYTEHIGH(reg16)=0x00;
		BYTELOW(reg16)=ADC1;
		
		// AD0EN=0;	//disable adc0
		// ADC1CN&=~0x80;	//set adc1 enable
		
		// set_aout(0,adc0);		
		// low=((unsigned char) (((unsigned int) (adc0)) >> 8));
		// high=((unsigned char) (adc0));				
		// low=BYTELOW(adc0);
		// high=BYTEHIGH(adc0);
		// BYTEHIGH(adc0)= 	((unsigned char) (((unsigned int) (ADC0)) >> 8));
		// BYTELOW(adc0)	=  	((unsigned char) (ADC0));
	}//if((ch<=0)&&(cn<=7))
	return reg16;	
}//get_ain_adc0()

u16  get_ain(u8 ch,u8 gain){
	u16 reg16=0L;
	
	if(ch>=0&&ch<=8){	//read adc0.x as single-ended analog input
		ADC0_set_diff(ch,FALSE);
		reg16=get_ain_adc0(ch,gain);
	}
	else if (ch==9){	//read AIN0-AIN1 (ADC0) as differential analog input
		ch=0;
		ADC0_set_diff(ch,TRUE);	
		reg16=get_ain_adc0(ch,gain);
	}
	else if (ch==10){	//read AIN2-AIN3 (ADC0) as differential analog input
		ch=2;
		ADC0_set_diff(ch,TRUE);	
		reg16=get_ain_adc0(ch,gain);
	}
	else if (ch==11){	//read AIN4-AIN5 (ADC0) as differential analog input
		ch=4;
		ADC0_set_diff(4,TRUE);	
		reg16=get_ain_adc0(ch,gain);
	}
	else if (ch==12){	//read AIN6-AIN7 (ADC0) as differential analog input
		ch=6;
		ADC0_set_diff(6,TRUE);
		reg16=get_ain_adc0(ch,gain);		
	}
	else if (ch>=13&&ch<=20){	//read AIN[0..7] (ADC1), single-ended
		ch-=13;	
		reg16=get_ain_adc1(ch,gain);
			
	}
	else{
		return 0xFFFF;
	}
	
	return reg16;
}

/*##########################################################################*/// interrupts
void uart_init(void){
			// set timer1 for uart0 
			CKCON|=0x10;						//set T1M=1 (timer1 uses system clk)			
			TMOD |=0x20;						//timer1 mode2 - 8bit autoreload .		
			TH1=-SYSCLK/16L/BAUDRATE;			// TH1=-12;			
			TL1=TH1;							//reload		
			TR1=1;								//run timer1	

			// set uart0
			PCON  |= 0x80;			//set SMOD0=1, baud rate generator clock devider is 16
			SCON0=0x50; 				//mode1: 8-Bit UART, Variable Baud Rate.
			Rx_init();
			Tx_init();
}

void uart_service(void){
	u8 ch;
	u16 val_16;
	u8 tmp_1;
	u8 tmp_2;
	u8 tmp_3;
	u8 tmp_4;
	
	EA=0;
	if(RI0){	
		RI0=0;			//reset interrupt bit
		ch=SBUF0;		//copy register		
		if(flag_loopback_debug){
			tx.idx=0;
			tx.buf[tx.idx]=ch;
			TI0=TRUE;
		}
		else if((rx.idx>0)&&(rx.idx<RX_BUFFER_SIZE)&&((ch>='0'&&ch<='9')||(ch>='A'&&ch<='F'))){	//detect valid char
			rx.buf[rx.idx++]=ch;	
		}	
		else if(ch==RX_START_FRAME){			//detect rx_frame START 
			Rx_init();							//init rx_buffer pointer&rx_idx
			rx.buf[0]=RX_START_FRAME;		//read char into rx_buffer and increament pointer
			rx.idx=1;
		}			
		else if(ch==RX_STOP_FRAME && rx.idx>4 && rx.idx<RX_BUFFER_SIZE){//detect END verify error rx_idx minmimum char =4  <aabb>			
			rx.sum=Rx_get_u8(rx.idx-2);
			rx.buf[rx.idx++]=RX_STOP_FRAME;
			rx.size=rx.idx;			
			rxmsg.addr=Rx_get_u8(1); //addr index 	<AABB..>
			rxmsg.type=Rx_get_u8(3);	//type index <AABB..>
			// if(ADDRESS_PORT==0x80?rxmsg.addr==VIRTUAL_ADDRESS:rxmsg.addr==ADDRESS_PORT){	//read virtual address ONLY IF phisical address == 0x80
			// if(ADDRESS_PORT==0x80?(rxmsg.addr==var128[VAR128_VADDR]||var128[VAR128_VADDR]==0xff):rxmsg.addr==ADDRESS_PORT){
			if((rxmsg.addr==var128[VAR128_VADDR])||(var128[VAR128_VADDR]==0xff)){
				switch(rxmsg.type){	//rx_msg_type
					case 0xBB :{	//187 loop-back		<AABB...>		
						Loopback();//copy rx to tx 
					}break;			
					case 0x11 :{	//17 set dout		<AA11aabbccdd>		
						if(rx.size==14){								
						rxmsg.idx=Rx_get_u8(5);		//dout(aa)
						rxmsg.val=Rx_get_u8(7);		//value(bb)
						rxmsg.t1=Rx_get_u8(9);		//timer_high(cc)
						rxmsg.t2=Rx_get_u8(11);		//timer_low(dd)
						// Rxmsg_dout_wr:::::::::::::::::::
						if(rxmsg.idx<MAX_DIGITAL_PINS){		
							dout_timeout_sec[rxmsg.idx]=((u16)rxmsg.t1)<<8;
							dout_timeout_sec[rxmsg.idx]+=rxmsg.t2;	
							dout_timeout_ms[rxmsg.idx]=0;	
							set_dout(rxmsg.idx,rxmsg.val);	
						}
						// Rxmsg_dout_wr:::::::::::::::::::
						// Rxmsg_dout_wr();				
						Loopback();				//copy rx to tx 	
						}
						
					}break;							
					case 0x13 :{	//19 get_ain		<AA13aabb> -> {qqww}
						if(rx.size==10){								
							rxmsg.idx	=Rx_get_u8(5);		//(aa)	analog input
							rxmsg.t1	=Rx_get_u8(7);		//(bb)	analog gain1(000),2(001),4(010),8(011),16(10x),0.5(11x)
							val_16		=get_ain(rxmsg.idx,rxmsg.t1);
							val_16		=get_ain(rxmsg.idx,rxmsg.t1);
							
							Tx_init();	//resets tx_buffer[] and tx_idx	
							Rx_init();	//resets tx_buffer[] and tx_idx	
							Tx_set(1,BYTEHIGH(val_16));	//qq
							Tx_set(3,BYTELOW(val_16));	//ww			
							tx.buf[5]=TX_STOP_FRAME;
							tx.buf[0]=TX_START_FRAME;
							tx.size=6;
							tx.idx=0; 	//transmition will use tx.idx )in TI0 case
							TI0=TRUE;		//start transmition					
						}

					}break;					
					case 0x14 :{	//20 get_din		<AA14aa> -> {qq}
            u8 val;	
										// read_ain()
										// convert_to_ascii()
										// send_string()
						if(rx.size==8){								
							rxmsg.idx	=Rx_get_u8(5);		//digital input(aa)

							if(get_din(rxmsg.idx)){
								val=0x01;
							}
							else{
								val=0x00;
							}
							
							Tx_init();	//resets tx_buffer[] and tx_idx	
							tx.buf[0]=TX_START_FRAME;
							Tx_set(1,*(&val));		//qq												
							tx.buf[3]=TX_STOP_FRAME;
							tx.size=4;
							tx.idx=0; 	//transmition will use tx.idx )in TI0 case
							TI0=TRUE;		//start transmition					
						}				
					}break;					
					case 0x15 :{	//set_aout		<AA15aabbcc> 
						if(rx.size==12){								
							rxmsg.idx			=Rx_get_u8(5);		//analog output(aa)
							BYTEHIGH(val_16)	=Rx_get_u8(7);		//analog high register 0x[0..F](bb)
							BYTELOW(val_16)		=Rx_get_u8(9);		//analog low register 0x[0..FF](ccc)
							
							set_aout(rxmsg.idx,val_16);
							
							Tx_init();	//resets tx_buffer[] and tx_idx	
							Tx_set(1,BYTEHIGH(val_16));	//qq
							Tx_set(3,BYTELOW(val_16));	//ww			
							tx.buf[5]=TX_STOP_FRAME;
							tx.buf[0]=TX_START_FRAME;
							tx.size=6;
							tx.idx=0; 	//transmition will use tx.idx )in TI0 case
							TI0=TRUE;		//start transmition					
						}

					}break;
					case 0x16 :{	//get_digital ports <AA16> -> {kkllmmnnoopp}							
						if(rx.size==6){									
							Tx_init();	//resets tx_buffer[] and tx_idx	
							tx.buf[0]=TX_START_FRAME;
							
							// DIG_PORT_0=0x0;
							// DIG_PORT_0=0xff;
							Tx_set(1,DIG_PORT_0);		//kk:DIG_PORT_0
							// DIG_PORT_1=0x0;
							// DIG_PORT_1=0xff;
							Tx_set(3,DIG_PORT_1);		//ll:DIG_PORT_1
							// DIG_PORT_2=0x0;
							// DIG_PORT_2=0xff;
							Tx_set(5,DIG_PORT_2);		//mm:DIG_PORT_2
							// DIG_PORT_3=0x0;
							// DIG_PORT_3=0xff;
							Tx_set(7,DIG_PORT_3);		//nn::DIG_PORT_3
							// DIG_PORT_4=0x0;
							// DIG_PORT_4=0xff;
							Tx_set(9,DIG_PORT_4);		//oo::DIG_PORT_4
							// DIG_PORT_5=0x0;
							// DIG_PORT_5=0xff;
							Tx_set(11,DIG_PORT_5);		//pp::DIG_PORT_5
							
							// Tx_set(13,digital_port_conf);	//qq::DIG_PORT_5
							
							tx.buf[13]=TX_STOP_FRAME;
							tx.size=14;
							tx.idx=0; 	//transmition will use tx.idx )in TI0 case
							TI0=TRUE;		//start transmition					
						}				
					}break;	
					case 0x17 :{	//set_virtual_address		<AA17bb\n -> {AA17bb\n								
						if(rx.size==8){	
							var128[VAR128_VADDR]=Rx_get_u8(5);		//value_u8(bb)
							FLASH_Save();
							Loopback();				//copy rx to tx 		
						}//if(rx.size==10)				
					}break;					
					case 0x18 :{	//set_serial number		<AA18bbccddee\n -> {AA18bbccddee\n
						if(rx.size==14){	
							var128[VAR128_SN_0]=Rx_get_u8(5);		//value_u8(bb)
							var128[VAR128_SN_1]=Rx_get_u8(7);		//value_u8(cc)
							var128[VAR128_SN_2]=Rx_get_u8(9);		//value_u8(dd)
							var128[VAR128_SN_3]=Rx_get_u8(11);		//value_u8(ee)
							FLASH_Save();
							Loopback();				//copy rx to tx 
							// Tx_init();	//resets tx_buffer[] and tx_idx	
							// tx.buf[0]=TX_START_FRAME;
							// Tx_set(1,DOUT_PORT_0);		//ww												
							// Tx_set(3,DOUT_PORT_1);		//xx												
							// Tx_set(5,DOUT_PORT_2);		//yy												
							// Tx_set(7,DOUT_PORT_3);		//zz												
							// tx.buf[9]=TX_STOP_FRAME;
							// tx.size=10;
							// tx.idx=0; 	//transmition will use tx.idx )in TI0 case
							// TI0=TRUE;		//start transmition					
						}//if(rx.size==10)				
					}break;					
					case 0x19 :{	//get_serial number		<AA19\n -> {bbccddee\n
						if(rx.size==6){	
							Tx_init();	//resets tx_buffer[] and tx_idx	
							tx.buf[0]=TX_START_FRAME;
							Tx_set(1,var128[VAR128_SN_0]);		//value_u8(bb)
							Tx_set(3,var128[VAR128_SN_1]);		//value_u8(cc)
							Tx_set(5,var128[VAR128_SN_2]);		//value_u8(dd)
							Tx_set(7,var128[VAR128_SN_3]);		//value_u8(ee)
							
							tx.buf[9]=TX_STOP_FRAME;
							tx.size=10;
							tx.idx=0; 	//transmition will use tx.idx )in TI0 case
							TI0=TRUE;		//start transmition					
						}//if(rx.size==10)				
					}break;				
					case 0x20 :{	//set time.sec	<AA20aabbccdd>	
						if(rx.size==14){	
							time.sec=0;
							//read value from msg 
							tmp_1=(u8)Rx_get_u8(5);
							tmp_2=(u8)Rx_get_u8(7);
							tmp_3=(u8)Rx_get_u8(9);
							tmp_4=(u8)Rx_get_u8(11);						
							//set clock
							time.sec=0x0L;
							time.sec+=(tmp_1+0x0000L)<<24;
							time.sec+=(tmp_2+0x0000L)<<16;
							time.sec+=(tmp_3+0x0000L)<<8;
							time.sec+=(tmp_4+0x0000L);							
							//read clock
							tmp_1=(u8)(time.sec>>24);
							tmp_2=(u8)(time.sec>>16);
							tmp_3=(u8)(time.sec>>8);
							tmp_4=(u8)(time.sec>>0);
              //send result as tx		
							Tx_init();							
							Tx_set(1,tmp_1);	//cc
							Tx_set(3,tmp_2);	//dd
							Tx_set(5,tmp_3);	//ee
							Tx_set(7,tmp_4);	//ff
							tx.buf[9]=TX_STOP_FRAME;
							tx.buf[0]=TX_START_FRAME;
							tx.size=10;
							tx.idx=0; 	//transmition will use tx.idx )in TI0 case
							TI0=TRUE;		//start transmition
						}			
					}break;				
					case 0x21 :{	//get time.sec	<xxAA21>, tx={ccddeeff}

						// tmp_1=BYTE3(time.sec);	
						// tmp_2=BYTE4(time.sec);
						// "aabbccdd"
						
						//read clock
						tmp_1=(u8)(time.sec>>24);
						tmp_2=(u8)(time.sec>>16);
						tmp_3=(u8)(time.sec>>8);
						tmp_4=(u8)(time.sec>>0);
						
						Tx_init();	//resets tx_buffer[] and tx_idx	
						
						Tx_set(1,tmp_1);	//cc
						Tx_set(3,tmp_2);	//dd
						Tx_set(5,tmp_3);	//ee
						Tx_set(7,tmp_4);	//ff
						
						tx.buf[9]=TX_STOP_FRAME;
						tx.buf[0]=TX_START_FRAME;
						tx.size=10;
						tx.idx=0; 	//transmition will use tx.idx )in TI0 case
						TI0=TRUE;		//start transmition
						
					}break;
					case 0x22 :{	//set value in scratch, index range is [00..99]		<AA22aabb\n -> {AA22aabb}
						if(rx.size==10){	
							tmp_1=Rx_get_u8(5);	//index (aa)
							tmp_2=Rx_get_u8(7);	//u8 value (bb)
							tmp_1+=28;
							if(27<tmp_1 && tmp_1<128){
								var128[tmp_1]=tmp_2;		
								FLASH_Save();
								Loopback();				
							}
						}
					}break;
					case 0x23 :{	//get value from scratch, index range is [00..99]		<AA22aa\n -> {AA22aabb}
						if(rx.size==8){	
							tmp_1=Rx_get_u8(5);	//index (aa)
							
							tmp_1+=28;
							if(27<tmp_1 && tmp_1<128){
								Tx_init();						//resets tx_buffer[] and tx_idx	
								tx.buf[0]=TX_START_FRAME;
								Tx_set(1,var128[tmp_1]);		//nn::DIG_PORT_3
								tx.buf[3]=TX_STOP_FRAME;
								tx.size=4;
								// tx.idx=0; 	//transmition will use tx.idx )in TI0 case
								TI0=TRUE;		//start transmition	
								
							}
						}	

					}break;	
				
				}//switch(BB)
			}//(rxmsg.addr==0xAA)	
			
			
		}
		else{
			Rx_init();
		}
		
	}//if(RI0)

  if(TI0){
		TI0=FALSE;
		if( tx.idx<TX_BUFFER_SIZE && tx.buf[tx.idx]!=NULL ){
			// UART_READ_DISABLE=UART_WRITE_ENABLE=TRUE;	//enable write
			SBUF0=tx.buf[tx.idx++];		//send char
		}
		else{
			// RE=DE=0;	//max3086_enable_read
			// UART_READ_DISABLE=UART_WRITE_ENABLE=FALSE;	//enable read
		}
	}//if(TI0)

	EA=1;
}//uart0_int

