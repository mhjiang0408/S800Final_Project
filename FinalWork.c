#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include "hw_memmap.h"
#include "debug.h"
#include "gpio.h"
#include "hw_i2c.h"
#include "hw_types.h"
#include "i2c.h"
#include "pin_map.h"
#include "sysctl.h"
#include "systick.h"
#include "interrupt.h"
#include "uart.h"
#include "hw_ints.h"
#include "string.h"
#include "hw_qei.h"
#include "qei.h"

//*****************************************************************************
//
//I2C GPIO chip address and resigster define
//
//*****************************************************************************
#define TCA6424_I2CADDR 					0x22
#define PCA9557_I2CADDR						0x18

#define PCA9557_INPUT							0x00
#define	PCA9557_OUTPUT						0x01
#define PCA9557_POLINVERT					0x02
#define PCA9557_CONFIG						0x03

#define TCA6424_CONFIG_PORT0			0x0c
#define TCA6424_CONFIG_PORT1			0x0d
#define TCA6424_CONFIG_PORT2			0x0e

#define TCA6424_INPUT_PORT0				0x00
#define TCA6424_INPUT_PORT1				0x01
#define TCA6424_INPUT_PORT2				0x02

#define TCA6424_OUTPUT_PORT0			0x04
#define TCA6424_OUTPUT_PORT1			0x05
#define TCA6424_OUTPUT_PORT2			0x06

#define SYSTICK_FREQUENCY		1000			//1000hz

#define	I2C_FLASHTIME				500				//500mS
#define GPIO_FLASHTIME			500				//500mS

#define MAX_DISP_LEN    8

#define M4  698
#define M5  784
#define M6  880
#define M7  988
#define H1  1048

//Universal function
void 		S800_Clock_Init(void);
void 		S800_GPIO_Init(void);
void		S800_I2C0_Init(void);
void 		Delay(uint32_t value);
uint8_t I2C0_WriteByte(uint8_t DevAddr, uint8_t RegAddr, uint8_t WriteData);
uint8_t I2C0_ReadByte(uint8_t DevAddr, uint8_t RegAddr);
void 		S800_SysTick_Init(void);
void 		S800_UART_Init(void);
void 		UARTStringPut(const char *cMessage);
void 		UARTStringPutNonBlocking(const char *cMessage);
void    S800_QEI_Init(void);
//Clock rules
void    cal_s2(void);
void    cal_s1(void);
void    cal_m2(void);
void    cal_m1(void);
void    cal_h2(void);
//Buttons controling
bool    ShortPressCheckJ0(void);
bool    ShortPressCheckJ1(void);
void    isPressed(void);
//Boot animation
void    BootAnimation(void);
void    BootMusic(void);
//Basic clock
void    ClockRun(void);
void    ClockDisplay(void);
//Alarm
void    AlarmSet(void);
void    AlarmBeep(void);
void    AlarmDisplay(void);
//Canlender
void    CanlendarRun(void);
void    CanlendarDisplay(void);
//UART order-checking
void    OrderCheck(void);
//Timer
void    TimerRun(void);
void    TimerDisplay(void);
//Author
void    AuthorDisplay(void);

bool timerFlag=0;
bool alarmFlag=0;
bool timerFlag2=0;
bool bootFlag=0;
int pressFlag=1;
int hour=00,minute=00,second=00;
int h1=0,h2=0,m1=0,m2=0,s1=0,s2=0;
int dh1=0,dh2=0,dm1=0,dm2=0,ds1=0,ds2=0;
int clkh1=10,clkh2=2,clkm1=0,clkm2=0,clks1=0,clks2=0;
int year=2022,month=06,day=30;
int y1,y2,y3,y4,mo1,mo2,d1,d2;
int leftnum=0,rightnum=0;
int l1,l2,l3,l4,r1,r2;
int flag1=8;
int flag2=8;
int num=0;
int beepLevel[]={524,588,660,698,784,880,988,1048,1176,1320};
int beep_num=4;
int audio=784;
uint16_t i2c_flash_cnt1=0;
uint32_t ui32SysClock;
uint8_t seg7[] = {0x3f,0x06,0x5b,0x4f,0x66,0x6d,0x7d,0x07,0x7f,0x6f,0x77,0x7c,0x58,0x5e,0x079,0x71,0x5c};
uint8_t segWel[]= {0x76,0x79,0x38,0x3f,0x0c,0x71};
uint8_t author[]= {0x7c,0x6e,0x71,0x77,0x37,0x76,0x77,0x3f};
char input[24];
char output[16];
char str1[8];
char str2[8];
char str3[10];
char str4[8];

//            a a a
//         f         b
//         f         b
//         f         b
//            g g g 
//         e         c
//         e         c
//         e         c
//            d d d     Dp
// ----------------------------------
//       b7 b6 b5 b4 b3 b2 b1 b0
//       Dp  g  f  e  d  c  b  a
	
//systick software counter define
volatile uint16_t systick_1ms_couter=0, systick_10ms_couter=0, systick_100ms_couter=0; //10ms和100ms计时器
volatile uint8_t	systick_1ms_status=0,systick_10ms_status=0, systick_100ms_status=0; //10ms和100ms计时状态

volatile uint8_t uart_receive_status = 0;
//------------- Defininition Finished ----------

int main(void)
{
	IntMasterDisable();	//关中断

	S800_Clock_Init();
	S800_GPIO_Init();
	S800_I2C0_Init();
	S800_SysTick_Init();
	S800_UART_Init();
	
	IntMasterEnable();	//开中断	
	
	BootAnimation();
	
	while(1)
	{
		ClockRun();
		CanlendarRun();
		if (timerFlag)
			TimerRun();
		if(bootFlag)
		 OrderCheck();
		AlarmSet();
		isPressed();
		AlarmBeep();
		bootFlag=1;
		if(pressFlag==1)
		  ClockDisplay();
		if(pressFlag==2)
			TimerDisplay();
		if(pressFlag==3)
			CanlendarDisplay();
		if(pressFlag==4)
			AlarmDisplay();
		if(pressFlag==8)
			AuthorDisplay();
	}
}

//------------- System Clock -------------------
void S800_Clock_Init(void)
{
	//use internal 16M oscillator, HSI
	//ui32SysClock = SysCtlClockFreqSet((SYSCTL_OSC_INT |SYSCTL_USE_OSC), 16000000);		

	//use extern 25M crystal
	//ui32SysClock = SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ |SYSCTL_OSC_MAIN |SYSCTL_USE_OSC), 25000000);		

	//use PLL
	ui32SysClock = SysCtlClockFreqSet((SYSCTL_XTAL_16MHZ |SYSCTL_OSC_INT | SYSCTL_USE_PLL |SYSCTL_CFG_VCO_480), 20000000);
}

//------------- GPIO -------------------
void S800_GPIO_Init(void)
{
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);						//Enable PortF
	while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOF));			//Wait for the GPIO moduleF ready
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOJ);						//Enable PortJ	
	while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOJ));			//Wait for the GPIO moduleJ ready	
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPION);						//Enable PortN	
	while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPION));			//Wait for the GPIO moduleN ready		
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOK);						//Enable PortK	
	while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOK));			//Wait for the GPIO moduleK ready		
	
  GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_0);			//Set PF0 as Output pin
  GPIOPinTypeGPIOOutput(GPIO_PORTN_BASE, GPIO_PIN_0 | GPIO_PIN_1);			//Set PN0,PN1 as Output pin
	GPIOPinTypeGPIOOutput(GPIO_PORTK_BASE, GPIO_PIN_5);			//Set PK5 as Output pin
	
	GPIOPinTypeGPIOInput(GPIO_PORTJ_BASE,GPIO_PIN_0 | GPIO_PIN_1);//Set the PJ0,PJ1 as input pin
	GPIOPadConfigSet(GPIO_PORTJ_BASE,GPIO_PIN_0 | GPIO_PIN_1,GPIO_STRENGTH_2MA,GPIO_PIN_TYPE_STD_WPU);
}

//-------------- I2C ------------------
void S800_I2C0_Init(void)
{
	uint8_t result;
	
  SysCtlPeripheralEnable(SYSCTL_PERIPH_I2C0);
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
	GPIOPinConfigure(GPIO_PB2_I2C0SCL);
  GPIOPinConfigure(GPIO_PB3_I2C0SDA);
  GPIOPinTypeI2CSCL(GPIO_PORTB_BASE, GPIO_PIN_2);
  GPIOPinTypeI2C(GPIO_PORTB_BASE, GPIO_PIN_3);

	I2CMasterInitExpClk(I2C0_BASE,ui32SysClock, true);										//config I2C0 400k
	I2CMasterEnable(I2C0_BASE);	

	result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_CONFIG_PORT0,0x0ff);		//config port 0 as input
	result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_CONFIG_PORT1,0x0);			//config port 1 as output
	result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_CONFIG_PORT2,0x0);			//config port 2 as output 

	result = I2C0_WriteByte(PCA9557_I2CADDR,PCA9557_CONFIG,0x00);					//config port as output
	result = I2C0_WriteByte(PCA9557_I2CADDR,PCA9557_OUTPUT,0x0ff);				//turn off the LED1-8
	
}

uint8_t I2C0_WriteByte(uint8_t DevAddr, uint8_t RegAddr, uint8_t WriteData)
{
	uint8_t rop;
	while(I2CMasterBusy(I2C0_BASE)){};
	I2CMasterSlaveAddrSet(I2C0_BASE, DevAddr, false);
	I2CMasterDataPut(I2C0_BASE, RegAddr);
	I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_BURST_SEND_START);
	while(I2CMasterBusy(I2C0_BASE)){};
	rop = (uint8_t)I2CMasterErr(I2C0_BASE);

	I2CMasterDataPut(I2C0_BASE, WriteData);
	I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_BURST_SEND_FINISH);
	while(I2CMasterBusy(I2C0_BASE)){};

	rop = (uint8_t)I2CMasterErr(I2C0_BASE);
	return rop;
}

uint8_t I2C0_ReadByte(uint8_t DevAddr, uint8_t RegAddr)
{
	uint8_t value;

	while(I2CMasterBusy(I2C0_BASE)){};	//忙等待
	I2CMasterSlaveAddrSet(I2C0_BASE, DevAddr, false); //设从机地址，写
	I2CMasterDataPut(I2C0_BASE, RegAddr); //设数据地址
	I2CMasterControl(I2C0_BASE,I2C_MASTER_CMD_SINGLE_SEND); //启动总线发送
	while(I2CMasterBusBusy(I2C0_BASE));
	if (I2CMasterErr(I2C0_BASE) != I2C_MASTER_ERR_NONE)
		return 0; //错误
	Delay(100);

	//receive data
	I2CMasterSlaveAddrSet(I2C0_BASE, DevAddr, true); //设从机地址，读
	I2CMasterControl(I2C0_BASE,I2C_MASTER_CMD_SINGLE_RECEIVE); //启动总线接收
	while(I2CMasterBusBusy(I2C0_BASE));
	value=I2CMasterDataGet(I2C0_BASE);
	if (I2CMasterErr(I2C0_BASE) != I2C_MASTER_ERR_NONE)
		return 0; //错误
	Delay(100);

	return value;
}

void Delay(uint32_t value)
{
	uint32_t ui32Loop;
	
	for(ui32Loop = 0; ui32Loop < value; ui32Loop++);
}

//--------------- SysTick -----------------
void S800_SysTick_Init(void)
{
	SysTickPeriodSet(ui32SysClock/SYSTICK_FREQUENCY); //1ms
	SysTickEnable();
	SysTickIntEnable();
}

/*
	Corresponding to the startup_TM4C129.s vector table systick interrupt program name
*/
void SysTick_Handler(void)
{
	if (systick_100ms_couter == 0) //利用1ms的SysTick产生100ms的定时器
	{
		systick_100ms_couter = 100;
		systick_100ms_status = 1;
	}
	else
		systick_100ms_couter--;
	
	if (systick_10ms_couter	== 0) //利用1ms的SysTick产生10ms的定时器
	{
		systick_10ms_couter	 = 10;
		systick_10ms_status  = 1;
	}
	else
		systick_10ms_couter--;
	
		if (systick_1ms_couter	== 0) //利用1ms的SysTick产生1ms的定时器
	{
		systick_1ms_couter	 = 1;
		systick_1ms_status  = 1;
	}
	else
		systick_1ms_couter--;
	
	if (GPIOPinRead(GPIO_PORTJ_BASE,GPIO_PIN_0) == 0)
	{
		systick_100ms_status	= systick_10ms_status = 0; //阻止任务1和2的调度
		GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_0,GPIO_PIN_0);		//点亮PN0
	}
	else
		GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_0,0);		//熄灭PN0

}

//----------- UART ---------------------
void S800_UART_Init(void)
{
	SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);						//Enable PortA
	while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOA));			//Wait for the GPIO moduleA ready

	GPIOPinConfigure(GPIO_PA0_U0RX);												// Set GPIO A0 and A1 as UART pins.
  GPIOPinConfigure(GPIO_PA1_U0TX);    			

  GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);

	// Configure the UART for 115,200, 8-N-1 operation.
  UARTConfigSetExpClk(UART0_BASE, ui32SysClock,115200,(UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE |UART_CONFIG_PAR_NONE));

	UARTFIFOLevelSet(UART0_BASE,UART_FIFO_TX2_8,UART_FIFO_RX7_8);//set FIFO Level

  UARTIntEnable(UART0_BASE, UART_INT_RX | UART_INT_RT);	//Enable UART0 RX,TX interrupt
	IntEnable(INT_UART0);
	
	UARTStringPut("\r\nHello, world!\r\n");
}

void UARTStringPut(const char *cMessage)
{
	while(*cMessage!='\0')
		UARTCharPut(UART0_BASE,*(cMessage++));
}

void UARTStringPutNonBlocking(const char *cMessage)
{
	while(*cMessage!='\0')
		UARTCharPutNonBlocking(UART0_BASE,*(cMessage++));
}

/*
	Corresponding to the startup_TM4C129.s vector table UART0_Handler interrupt program name
*/
void UART0_Handler(void)
{
	int32_t uart0_int_status;
//	char uart_receive_char;
	
  uart0_int_status = UARTIntStatus(UART0_BASE, true);			// Get the interrrupt status.
  UARTIntClear(UART0_BASE, uart0_int_status);							//Clear the asserted interrupts

	if (uart0_int_status & (UART_INT_RX | UART_INT_RT)) 	//接收或接收超时
	{
		uart_receive_status = 1;
	}
}
/*
QEI
*/
void S800_QEI_Init(void)
{
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOL);
	// Enable the QEI0 peripheral
	SysCtlPeripheralEnable(SYSCTL_PERIPH_QEI0);
	// Wait for the QEI0 module to be ready.
	while(!SysCtlPeripheralReady(SYSCTL_PERIPH_QEI0)){}

	GPIOPinConfigure(GPIO_PL1_PHA0);
  GPIOPinConfigure(GPIO_PL2_PHB0);
	//software patch to force the PL3 to low voltage, 即引脚C（接PL3）必须接地
	GPIOPinTypeGPIOOutput(GPIO_PORTL_BASE, GPIO_PIN_3);			
	GPIOPinWrite(GPIO_PORTL_BASE, GPIO_PIN_3,0);	
  
	GPIOPinTypeQEI(GPIO_PORTL_BASE, GPIO_PIN_1);
  GPIOPinTypeQEI(GPIO_PORTL_BASE, GPIO_PIN_2);
	//
	// Configure the quadrature encoder to capture edges on both signals and
	// maintain an absolute position by resetting on index pulses. Using a
	// 1000 line encoder at four edges per line, there are 4000 pulses per
	// revolution; therefore set the maximum position to 3999 as the count
	// is zero based.
	//
	QEIConfigure(QEI0_BASE, (QEI_CONFIG_CAPTURE_A_B | QEI_CONFIG_NO_RESET |	QEI_CONFIG_QUADRATURE | QEI_CONFIG_NO_SWAP), 100);
	// Enable the quadrature encoder.
	QEIEnable(QEI0_BASE);
}

void cal_h2(void)
{
	if(h2==0)
	h1=(h1+1)%6;
}

bool ShortPressCheckJ0(void)
{
	uint8_t key_value0;
	key_value0 = GPIOPinRead(GPIO_PORTJ_BASE,GPIO_PIN_0)	;				//read the PJ0 key value
	
	if(key_value0==0)
	{
		SysCtlDelay(ui32SysClock/1500); //2ms
		if(GPIOPinRead(GPIO_PORTJ_BASE,GPIO_PIN_0)==1)
		return 1;
	}
	return 0;
}

bool ShortPressCheckJ1(void)
{
	uint8_t key_value1;
	key_value1 = GPIOPinRead(GPIO_PORTJ_BASE,GPIO_PIN_1)	;				//read the PJ1 key value
	
	if(key_value1==0)
	{
		SysCtlDelay(ui32SysClock/1500); //2ms
		if(GPIOPinRead(GPIO_PORTJ_BASE,GPIO_PIN_1)==1)
		return 1;
	}
	return 0;
}

void BootAnimation(void)
{
	uint16_t count1=0,count2=0;
	uint8_t flag=8;
	volatile uint8_t result;
	volatile uint16_t	i2c_flash_cnt=0;
	
	SysCtlDelay(ui32SysClock/3); //1s
	UARTStringPutNonBlocking("Welcome!FH.\n");
	
	while(count2<1000)
	{
	  while(count1<1000)
	  {
		  if (systick_1ms_status) //1ms定时到
		  {
				systick_1ms_status	= 0; //重置1ms定时状态
				flag = flag%8+1;
				I2C0_WriteByte(PCA9557_I2CADDR,PCA9557_OUTPUT,0x00);
					
				//Welcome sentence:"HELLO,FH"
				if(flag==1)
					{
						I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0x0);			//P2写0
						I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,segWel[0]);	//write port 1:"H" 					
						I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,1);				//write port 2
					}
				if(flag==2)
					{
						I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0x0);			//P2写0
						I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,segWel[1]);	//write port 1:"E" 					
						I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,2);				//write port 2
					}
				if(flag==3)
					{
						I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0x0);			//P2写0
						I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,segWel[2]);	//write port 1:"L" 					
						I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,4);				//write port 2
					}
				if(flag==4)
					{
						I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0x0);			//P2写0
						I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,segWel[2]);	//write port 1:"L" 					
						I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,8);				//write port 2
					}
				if(flag==5)
					{
						I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0x0);			//P2写0
						I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,segWel[3]);	//write port 1:"O" 					
						I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,16);				//write port 2
					}
				if(flag==6)
					{
						I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0x0);			//P2写0
						I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,segWel[4]);	//write port 1:"," 					
						I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,32);				//write port 2
					}
				if(flag==7)
					{
						I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0x0);			//P2写0
						I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,segWel[5]);	//write port 1:"F" 					
						I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,64);				//write port 2
					}
				if(flag==8)
					{
						I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0x0);			//P2写0
						I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,segWel[0]);	//write port 1:"H" 					
						I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,128);				//write port 2
					}
					count1++;
				}
		}
		count1=0;
		I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0x0);			//P2写0
		I2C0_WriteByte(PCA9557_I2CADDR,PCA9557_OUTPUT,0xff);
    // BootMusic();
		//SysCtlDelay(ui32SysClock/3); //1s
		while(count2<1000)
		{
			I2C0_WriteByte(PCA9557_I2CADDR,PCA9557_OUTPUT,0x00);
			if (systick_1ms_status) //1ms
			{
				flag=flag%8+1;
				systick_1ms_status	= 0; //Reset 1ms
				//StudentCode:520021910596
				if(flag==1)
					{
						I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0x0);			//P2写0
						I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,seg7[2]);	//write port 1:"2" 					
						I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,1);				//write port 2
					}
				if(flag==2)
					{
						I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0x0);			//P2写0
						I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,seg7[1]);	//write port 1:"1" 					
						I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,2);				//write port 2
					}
				if(flag==3)
					{
						I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0x0);			//P2写0
						I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,seg7[9]);	//write port 1:"9" 					
						I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,4);				//write port 2
					}
				if(flag==4)
					{
						I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0x0);			//P2写0
						I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,seg7[1]);	//write port 1:"1" 					
						I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,8);				//write port 2
					}
				if(flag==5)
					{
						I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0x0);			//P2写0
						I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,seg7[0]);	//write port 1:"0" 					
						I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,16);				//write port 2
					}
				if(flag==6)
					{
						I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0x0);			//P2写0
						I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,seg7[5]);	//write port 1:"5" 					
						I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,32);				//write port 2
					}
				if(flag==7)
					{
						I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0x0);			//P2写0
						I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,seg7[9]);	//write port 1:"9" 					
						I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,64);				//write port 2
					}
				if(flag==8)
					{
						I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0x0);			//P2写0
						I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,seg7[6]);	//write port 1:"6" 					
						I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,128);				//write port 2
					}
					count2++;
				}
		}
		I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0x0);			//P2写0
		I2C0_WriteByte(PCA9557_I2CADDR,PCA9557_OUTPUT,0xff);
		SysCtlDelay(ui32SysClock/3); //1s
	}
}

void ClockRun(void)
{
	
	if (systick_100ms_status) //100ms
		{
			systick_100ms_status	= 0; //Reset 100ms
			
			if (++i2c_flash_cnt1		>= I2C_FLASHTIME/50)  //1s
			{
				i2c_flash_cnt1				= 0;
				second=second+1;
				if(second>=60) 
				{
					minute=minute+second/60;
					second=second%60;
				}
				if(minute>=60)
				{
					hour=hour+minute/60;
					minute=minute%60;
				}
				if(hour>=24)
				{
					day=day+hour/24;
					hour=hour%24;
				}
			}
		}
	}

void ClockDisplay(void)
{	
	I2C0_WriteByte(PCA9557_I2CADDR,PCA9557_OUTPUT,0xfe);
	if (systick_1ms_status) //1ms
		{
			systick_1ms_status	= 0; //Reset 1ms
			flag1 = flag1%8+1;
			h1=hour/10;
			h2=hour%10;
			m1=minute/10;
			m2=minute%10;
			s1=second/10;
			s2=second%10;
			//Clock:h1h2-m1m2-s1s2
			if(flag1==1)
				{
					I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0x0);			//P2写0
					I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,seg7[h1]);	//write port 1 					
					I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,1);				//write port 2
				}
			if(flag1==2)
				{
					I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0x0);			//P2写0
					I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,seg7[h2]);	//write port 1 					
					I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,2);				//write port 2
				}
			if(flag1==3)
				{
					I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0x0);			//P2写0
					I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,0x40);	//write port 1 					
					I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,4);				//write port 2
				}
			if(flag1==4)
				{
					I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0x0);			//P2写0
					I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,seg7[m1]);	//write port 1 					
					I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,8);				//write port 2
				}
			if(flag1==5)
				{
					I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0x0);			//P2写0
					I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,seg7[m2]);	//write port 1 					
					I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,16);				//write port 2
				}
			if(flag1==6)
				{
					I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0x0);			//P2写0
					I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,0x40);	//write port 1 					
					I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,32);				//write port 2
				}
			if(flag1==7)
				{
					I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0x0);			//P2写0
					I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,seg7[s1]);	//write port 1 					
					I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,64);				//write port 2
				}
			if(flag1==8)
				{
					I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0x0);			//P2写0
					I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,seg7[s2]);	//write port 1 					
					I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,128);				//write port 2
				}
			}
		
}

void CanlendarRun(void)
{
	if((year%4==0&&month==2&&day==30)||(year%4!=0&&month==2&&day==29)||((month==1||month==3||month==5||month==7||month==8||month==10||month==12)&&day==32))
	{
		day=1;
		year=year+month/12;
		month=(month+1)%12;
	}
	
	if((month==4||month==6||month==9||month==11)&&day==31)
	{
		day=1;
		year=year+month/12;
		month=(month+1)%12;
	}
}	

void CanlendarDisplay(void)
{	 
	I2C0_WriteByte(PCA9557_I2CADDR,PCA9557_OUTPUT,0xfb);
	y1=year/1000;
	y2=year/100-10*y1;
	y3=year/10-10*y2-100*y1;
	y4=year%10;
	mo1=month/10;
	mo2=month%10;
	d1=day/10;
	d2=day%10;
	if (systick_1ms_status) //1ms
		{
			systick_1ms_status	= 0; //Reset1ms
			flag1 = flag1%8+1;
			//Canlendar:year-month-day
			if(flag1==1)
				{
					I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0x0);			//P2写0
					I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,seg7[y1]);	//write port 1 					
					I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,1);				//write port 2
				}
			if(flag1==2)
				{
					I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0x0);			//P2写0
					I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,seg7[y2]);	//write port 1 					
					I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,2);				//write port 2
				}
			if(flag1==3)
				{
					I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0x0);			//P2写0
					I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,seg7[y3]);	//write port 1 					
					I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,4);				//write port 2
				}
			if(flag1==4)
				{
					I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0x0);			//P2写0
					I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,seg7[y4]);	//write port 1 					
					I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,8);				//write port 2
				}
			if(flag1==5)
				{
					I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0x0);			//P2写0
					I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,seg7[mo1]);	//write port 1 					
					I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,16);				//write port 2
				}
			if(flag1==6)
				{
					I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0x0);			//P2写0
					I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,seg7[mo2]);	//write port 1 					
					I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,32);				//write port 2
				}
			if(flag1==7)
				{
					I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0x0);			//P2写0
					I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,seg7[d1]);	//write port 1 					
					I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,64);				//write port 2
				}
			if(flag1==8)
				{
					I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0x0);			//P2写0
					I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,seg7[d2]);	//write port 1 					
					I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,128);				//write port 2
				}
			}
}

void isPressed(void)
{
	 uint8_t old_key;
	 uint8_t key;
	 old_key = ~I2C0_ReadByte(TCA6424_I2CADDR,TCA6424_INPUT_PORT0);
	if(old_key&0x01)
	{
		SysCtlDelay(ui32SysClock/1500); //2ms
		key = ~I2C0_ReadByte(TCA6424_I2CADDR,TCA6424_INPUT_PORT0);
		if((key&0x01)!=0x00)
		pressFlag=1;
	}
	if(old_key&0x02)
	{
		SysCtlDelay(ui32SysClock/1500); //2ms
		key = ~I2C0_ReadByte(TCA6424_I2CADDR,TCA6424_INPUT_PORT0);
		if((key&0x02)!=0x00)
		pressFlag=2;
	}
	if(old_key&0x04)
	{
		SysCtlDelay(ui32SysClock/1500); //2ms
		key = ~I2C0_ReadByte(TCA6424_I2CADDR,TCA6424_INPUT_PORT0);
		if((key&0x04)!=0x00)
		pressFlag=3;
	}
	if(old_key&0x08)
	{
		SysCtlDelay(ui32SysClock/1500); //2ms
		key = ~I2C0_ReadByte(TCA6424_I2CADDR,TCA6424_INPUT_PORT0);
		if((key&0x08)!=0x00)
		pressFlag=4;
	}
	if(old_key&0x10)
	{
		SysCtlDelay(ui32SysClock/1500); //2ms
		key = ~I2C0_ReadByte(TCA6424_I2CADDR,TCA6424_INPUT_PORT0);
		if((key&0x10)!=0x00)
		alarmFlag=0;
	}
	if(old_key&0x40)
	{
		SysCtlDelay(ui32SysClock/1500); //2ms
		key = ~I2C0_ReadByte(TCA6424_I2CADDR,TCA6424_INPUT_PORT0);
		if((key&0x40)!=0x00)
		timerFlag=1;
	}
	if(old_key&0x80)
	{
		SysCtlDelay(ui32SysClock/1500); //2ms
		key = ~I2C0_ReadByte(TCA6424_I2CADDR,TCA6424_INPUT_PORT0);
		if((key&0x80)!=0x00)
		pressFlag=8;
	}
}
/*
void AlarmSet(void)
{
	if(clkh1<10)
	{
		if(h1==clkh1&&h2==clkh2&&m1==clkm1&&m2==clkm2&&s1==clks1&&s2==clks2)
		{	
			alarmFlag=1;
			GPIOPinWrite(GPIO_PORTK_BASE, GPIO_PIN_5, GPIO_PIN_5);			// Turn on the Beep.
			GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_1, GPIO_PIN_1);			// Turn on the indicator LED.
			I2C0_WriteByte(PCA9557_I2CADDR,PCA9557_OUTPUT,0x00);        // Turn on the LEDs.
		}
		if(alarmFlag&&ShortPressCheckJ0())
		{
			GPIOPinWrite(GPIO_PORTK_BASE, GPIO_PIN_5, 0x0);			// Turn off the Beep.
			GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_1, 0x0);			// Turn off the indicator LED.
			I2C0_WriteByte(PCA9557_I2CADDR,PCA9557_OUTPUT,0xff);        // Turn off the LEDs.
			alarmFlag=0;
		}
	}
}
*/
void AlarmSet(void)
{
	if(clkh1<10)
	{
		if(h1==clkh1&&h2==clkh2&&m1==clkm1&&m2==clkm2&&s1==clks1&&s2==clks2)
		{	
			alarmFlag=1;
		}
/*
		if(alarmFlag)
		{
				GPIOPinWrite(GPIO_PORTK_BASE, GPIO_PIN_5,~GPIOPinRead(GPIO_PORTK_BASE, GPIO_PIN_5));
				SysCtlDelay(ui32SysClock/(audio*3));			
				//GPIOPinWrite(GPIO_PORTK_BASE, GPIO_PIN_5, GPIO_PIN_5);			// Turn on the Beep.
			GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_1, GPIO_PIN_1);			// Turn on the indicator LED.
			I2C0_WriteByte(PCA9557_I2CADDR,PCA9557_OUTPUT,0x00);        // Turn on the LEDs.
		}
		*/
		if(ShortPressCheckJ0())
		{
			GPIOPinWrite(GPIO_PORTK_BASE, GPIO_PIN_5, 0x0);			// Turn off the Beep.
			GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_1, 0x0);			// Turn off the indicator LED.
			I2C0_WriteByte(PCA9557_I2CADDR,PCA9557_OUTPUT,0xff);        // Turn off the LEDs.
			alarmFlag=0;
		}
	}
}

void    AlarmBeep(void)
{
	if(alarmFlag)
		{
			GPIOPinWrite(GPIO_PORTK_BASE, GPIO_PIN_5,~GPIOPinRead(GPIO_PORTK_BASE, GPIO_PIN_5));
			SysCtlDelay(ui32SysClock/(audio*3));			
			
			GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_1, GPIO_PIN_1);			// Turn on the indicator LED.
			I2C0_WriteByte(PCA9557_I2CADDR,PCA9557_OUTPUT,0x00);        // Turn on the LEDs.
		}
}
void AlarmDisplay(void)
{
	I2C0_WriteByte(PCA9557_I2CADDR,PCA9557_OUTPUT,0xf7);
	if(clkh1<10)
	{
		if (systick_1ms_status) //1ms
		{
			systick_1ms_status	= 0; //Reset1ms
			flag2 = flag2%8+1;
			//Alarm:clkh1clkh2-clkm1clkm2-clks1clks2
			if(flag2==1)
				{
					I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0x0);			//P2写0
					I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,seg7[clkh1]);	//write port 1 					
					I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,1);				//write port 2
				}
			if(flag2==2)
				{
					I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0x0);			//P2写0
					I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,seg7[clkh2]);	//write port 1 					
					I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,2);				//write port 2
				}
			if(flag2==3)
				{
					I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0x0);			//P2写0
					I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,0x40);	//write port 1 					
					I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,4);				//write port 2
				}
			if(flag2==4)
				{
					I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0x0);			//P2写0
					I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,seg7[clkm1]);	//write port 1 					
					I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,8);				//write port 2
				}
			if(flag2==5)
				{
					I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0x0);			//P2写0
					I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,seg7[clkm2]);	//write port 1 					
					I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,16);				//write port 2
				}
			if(flag2==6)
				{
					I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0x0);			//P2写0
					I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,0x40);	//write port 1 					
					I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,32);				//write port 2
				}
			if(flag2==7)
				{
					I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0x0);			//P2写0
					I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,seg7[clks1]);	//write port 1 					
					I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,64);				//write port 2
				}
			if(flag2==8)
				{
					I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0x0);			//P2写0
					I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,seg7[clks2]);	//write port 1 					
					I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,128);				//write port 2
				}
			}
	}
	else
	{
		if (systick_1ms_status) //1ms
		{
			systick_1ms_status	= 0; //Reset1ms
			flag2 = flag2%8+1;
			//Alarm initialed:--------
			if(flag2==1)
				{
					I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0x0);			//P2写0
					I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,0x40);	//write port 1 					
					I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,1);				//write port 2
				}
			if(flag2==2)
				{
					I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0x0);			//P2写0
					I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,0x40);	//write port 1 					
					I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,2);				//write port 2
				}
			if(flag2==3)
				{
					I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0x0);			//P2写0
					I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,0x40);	//write port 1 					
					I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,4);				//write port 2
				}
			if(flag2==4)
				{
					I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0x0);			//P2写0
					I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,0x40);	//write port 1 					
					I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,8);				//write port 2
				}
			if(flag2==5)
				{
					I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0x0);			//P2写0
					I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,0x40);	//write port 1 					
					I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,16);				//write port 2
				}
			if(flag2==6)
				{
					I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0x0);			//P2写0
					I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,0x40);	//write port 1 					
					I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,32);				//write port 2
				}
			if(flag2==7)
				{
					I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0x0);			//P2写0
					I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,0x40);	//write port 1 					
					I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,64);				//write port 2
				}
			if(flag2==8)
				{
					I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0x0);			//P2写0
					I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,0x40);	//write port 1 					
					I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,128);				//write port 2
				}
			}
	}
}

void OrderCheck(void)
{
	  bool checkFlag1=0;
		char uart_receive_char;
	//Task: uart echo
		if (uart_receive_status)
		{
			GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_1,GPIO_PIN_1 );	//turn on PN1

			while(uart_receive_char!='\n')    										// Loop while there are characters in the receive FIFO.
			{
				//Read the next character from the UART and write it back to the UART.
				uart_receive_char = UARTCharGet(UART0_BASE);
				if(uart_receive_char>='a'&&uart_receive_char<='z')
				  input[num]=uart_receive_char-32;
				else
					input[num]=uart_receive_char;
				num++;				
			}

			GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_1,0);	//turn off PN1
			sscanf(input,"%s%s%s%s",str1,str2,str3,str4);
						
			if(strcmp(str1,"INIT")==0)
			{
				if(strcmp(str2,"CLOCK")==0&&strlen(str3)==0&&strlen(str4)==0)
				{
					checkFlag1=1;
					hour=0;
					minute=0;
					second=0;
					sprintf(output,"TIME:%02d:%02d:%02d",hour,minute,second);
					UARTStringPutNonBlocking(output);
				}
				if(strcmp(str2,"ALARM")==0&&strlen(str3)==0&&strlen(str4)==0)
				{
					checkFlag1=1;
					clkh1=10;
					UARTStringPutNonBlocking("Alarm cleared");
				}
				if(strcmp(str2,"TIMER")==0&&strlen(str3)==0&&strlen(str4)==0)
				{
					checkFlag1=1;
					leftnum=0;
					rightnum=0;
					UARTStringPutNonBlocking("Timer initialed");
				}
			}
			
			if(strcmp(str1,"SET")==0)
			{				
					if(strcmp(str2,"TIME")==0&&strlen(str4)==0&&strlen(str3)>6&&strlen(str3)<10)
					{
						checkFlag1=1;
						sscanf(str3,"%02d:%02d:%02d",&hour,&minute,&second);
						sprintf(output,"TIME:%02d:%02d:%02d",hour,minute,second);
						UARTStringPutNonBlocking(output);
					}
					
					if(strcmp(str2,"ALARM")==0&&strcmp(str3,"TIME")==0&&strlen(str4)>6&&strlen(str4)<10)
					{
						checkFlag1=1;
						
						sscanf(str4,"%01d%01d:%01d%01d:%01d%01d",&dh1,&dh2,&dm1,&dm2,&ds1,&ds2);
						clkh1=dh1;
						clkh2=dh2;
						clkm1=dm1;
						clkm2=dm2;
						clks1=ds1;
						clks2=ds2;
						sprintf(output,"ALARM:%01d%01d:%01d%01d:%01d%01d",clkh1,clkh2,clkm1,clkm2,clks1,clks2);
						UARTStringPutNonBlocking(output);
					}
					
					if(strcmp(str2,"TIMER")==0&&strlen(str4)==0&&strlen(str3)!=0&&strlen(str3)>5&&strlen(str3)<9)
					{
						checkFlag1=1;
						sscanf(str3,"%04d.%02d",&leftnum,&rightnum);
						sprintf(output,"Timer:%04d.%02d",leftnum,rightnum);
						UARTStringPutNonBlocking(output);
					}
					
					if(strcmp(str2,"DATE")==0&&strlen(str4)==0)
					{
						UARTStringPutNonBlocking("1");
						checkFlag1=1;
						
						sscanf(str3,"%04d-%02d-%02d",&year,&month,&day);
						sprintf(output,"DATE:%04d-%02d-%02d",year,month,day);
						UARTStringPutNonBlocking(output);
					}
					
					if(strcmp(str2,"BEEP")==0&&strcmp(str3,"LEVEL")==0)
					{
						checkFlag1=1;
						sscanf(input,"%s%s%s%d",str1,str2,str3,&beep_num);
						audio=500*beep_num;
						sprintf(output,"BEEP LEVEL:%d",beep_num);
						UARTStringPutNonBlocking(output);
					}
					
			}
			
			if(strcmp(str1,"GET")==0)
			{
				if(strcmp(str2,"DATE")==0&&strlen(str3)==0&&strlen(str4)==0)
				{
					checkFlag1=1;
					sprintf(output,"DATE:%04d-%02d-%02d",year,month,day);
					UARTStringPutNonBlocking(output);
				}
				if(strcmp(str2,"TIME")==0&&strlen(str3)==0&&strlen(str4)==0)
				{
					checkFlag1=1;
					sprintf(output,"TIME:%01d%01d:%01d%01d:%01d%01d",h1,h2,m1,m2,s1,s2);
					UARTStringPutNonBlocking(output);
				}
				if(strcmp(str2,"ALARM")==0&&strlen(str3)==0&&strlen(str4)==0)
				{
					checkFlag1=1;
					if(clkh1==10)
						UARTStringPutNonBlocking("NO ALARM");
						if(clkh1<10)
						{
							sprintf(output,"ALARM:%01d%01d:%01d%01d:%01d%01d",clkh1,clkh2,clkm1,clkm2,clks1,clks2);
							UARTStringPutNonBlocking(output);
						}
				}
			}
			
			
			if(!checkFlag1||!strcmp(input,"?"))
			{
				if(!checkFlag1)
				UARTStringPut("NOT FOUND\n");
				UARTStringPut(" Please choose from:\nINIT CLOCK\nINIT ALARM\nINIT TIMER\nSET TIME hh:mm:ss\n");
				UARTStringPut("SET TIMER xxxx.xx\nSET DATE yyyy-mm-dd\nSET ALARM hh:mm:ss\n");
				UARTStringPut("GET TIME\nGET DATE\nGET ALARM\n");
				UARTStringPut("@@Click 'ENTER' as end mark.@@");
			}

			memset(input,'\0',sizeof(input));
			num=0;
			memset(str1,'\0',sizeof(str1));
			memset(str2,'\0',sizeof(str2));
			memset(str3,'\0',sizeof(str3));
			memset(str4,'\0',sizeof(str4));
			memset(output,'\0',sizeof(output));
			checkFlag1=0;
			uart_receive_status=0;
		}
}


void TimerRun(void)
{
	if(timerFlag==1)
	{
		if(leftnum!=0||rightnum!=0)
	  {	
	    if (systick_10ms_status) //10ms
				{
					systick_10ms_status	= 0; //Reset 10ms
					rightnum--;
					if(leftnum==0&&rightnum==0)
					{
						GPIOPinWrite(GPIO_PORTK_BASE, GPIO_PIN_5, GPIO_PIN_5);			// Turn on the Beep.
       			GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_1, GPIO_PIN_1);			// Turn on the indicator LED.
						I2C0_WriteByte(PCA9557_I2CADDR,PCA9557_OUTPUT,0x00);        // Turn on the LEDs.
					}
					if(rightnum<0)
					{
						leftnum--;
						rightnum=rightnum+100;
					}
				}
		}
		if(GPIOPinRead(GPIO_PORTK_BASE, GPIO_PIN_5)!=0)
		{
			if(ShortPressCheckJ0())
			{
				GPIOPinWrite(GPIO_PORTK_BASE, GPIO_PIN_5, 0x0);			// Turn off the Beep.
				GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_1, 0x0);		// Turn off the indicator LED.
				I2C0_WriteByte(PCA9557_I2CADDR,PCA9557_OUTPUT,0xff);        // Turn off the LEDs.
				timerFlag=0;
			}
		}
	}
}
/*
void TimerRun(void)
{
	if(timerFlag==1)
	{
		if(leftnum!=0||rightnum!=0)
	  {	
	    if (systick_10ms_status) //10ms
				{
					systick_10ms_status	= 0; //Reset 10ms
					rightnum--;
					if(leftnum==0&&rightnum==0)
					{
       			GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_1, GPIO_PIN_1);			// Turn on the indicator LED.
						I2C0_WriteByte(PCA9557_I2CADDR,PCA9557_OUTPUT,0x00);        // Turn on the LEDs.
						timerFlag2=1;
					}
					if(rightnum<0)
					{
						leftnum--;
						rightnum=rightnum+100;
					}
				}
		}
		  if(timerFlag2)
			{
				GPIOPinWrite(GPIO_PORTK_BASE, GPIO_PIN_5, ~GPIOPinRead(GPIO_PORTK_BASE, GPIO_PIN_5));			// Turn on the Beep.
				SysCtlDelay(ui32SysClock/(audio*3));
			}

			if(timerFlag2==1&&ShortPressCheckJ0())
			{
				GPIOPinWrite(GPIO_PORTK_BASE, GPIO_PIN_5, 0x0);			// Turn off the Beep.
				GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_1, 0x0);		// Turn off the indicator LED.
				I2C0_WriteByte(PCA9557_I2CADDR,PCA9557_OUTPUT,0xff);        // Turn off the LEDs.
				timerFlag=0;
				timerFlag2=0;
			}

	}
}
*/

void TimerDisplay(void)
{
	I2C0_WriteByte(PCA9557_I2CADDR,PCA9557_OUTPUT,0xfd);
	l1=leftnum/1000;
	l2=(leftnum/100)%10;
	l3=(leftnum/10)%10;
	l4=leftnum%10;
	r1=rightnum/10;
	r2=rightnum%10;

	if (systick_1ms_status) //1ms
		{
			systick_1ms_status	= 0; //Reset 1ms
			flag2 = flag2%8+1;
			//Timer:l1l2l3l4.r1r2S
			if(flag2==1)
				{
					I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0x0);			//P2写0
					I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,seg7[l1]);	//write port 1 					
					I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,1);				//write port 2
				}
			if(flag2==2)
				{
					I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0x0);			//P2写0
					I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,seg7[l2]);	//write port 1 					
					I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,2);				//write port 2
				}
			if(flag2==3)
				{
					I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0x0);			//P2写0
					I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,seg7[l3]);	//write port 1 					
					I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,4);				//write port 2
				}
			if(flag2==4)
				{
					I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0x0);			//P2写0
					I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,seg7[l4]);	//write port 1 					
					I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,8);				//write port 2
				}
			if(flag2==5)
				{
					I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0x0);			//P2写0
					I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,0x80);	//write port 1 					
					I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,16);				//write port 2
				}
			if(flag2==6)
				{
					I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0x0);			//P2写0
					I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,seg7[r1]);	//write port 1 					
					I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,32);				//write port 2
				}
			if(flag2==7)
				{
					I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0x0);			//P2写0
					I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,seg7[r2]);	//write port 1 					
					I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,64);				//write port 2
				}
			if(flag2==8)
				{
					I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0x0);			//P2写0
					I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,seg7[5]);	//write port 1 					
					I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,128);				//write port 2
				}
		}
}

void AuthorDisplay(void)
{
	I2C0_WriteByte(PCA9557_I2CADDR,PCA9557_OUTPUT,0x7f);
	if (systick_1ms_status) //1ms
		{
			systick_1ms_status	= 0; //Reset 1ms
			flag2 = flag2%8+1;
			//author:by FanHao
			if(flag2==1)
				{
					I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0x0);			//P2写0
					I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,author[0]);	//write port 1 					
					I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,1);				//write port 2
				}
			if(flag2==2)
				{
					I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0x0);			//P2写0
					I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,author[1]);	//write port 1 					
					I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,2);				//write port 2
				}
			if(flag2==3)
				{
					I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0x0);			//P2写0
					I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,author[2]);	//write port 1 					
					I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,4);				//write port 2
				}
			if(flag2==4)
				{
					I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0x0);			//P2写0
					I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,author[3]);	//write port 1 					
					I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,8);				//write port 2
				}
			if(flag2==5)
				{
					I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0x0);			//P2写0
					I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,author[4]);	//write port 1 					
					I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,16);				//write port 2
				}
			if(flag2==6)
				{
					I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0x0);			//P2写0
					I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,author[5]);	//write port 1 					
					I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,32);				//write port 2
				}
			if(flag2==7)
				{
					I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0x0);			//P2写0
					I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,author[6]);	//write port 1 					
					I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,64);				//write port 2
				}
			if(flag2==8)
				{
					I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0x0);			//P2写0
					I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,author[7]);	//write port 1 					
					I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,128);				//write port 2
				}
			}
}

void BootMusic(void)
{
	int k;
	for(k=0;k<200*M4/M6;k++)
	{
		GPIOPinWrite(GPIO_PORTK_BASE,GPIO_PIN_5,~GPIOPinRead(GPIO_PORTK_BASE,GPIO_PIN_5));
		SysCtlDelay(ui32SysClock/(M4*3));
	}
	
	GPIOPinWrite(GPIO_PORTK_BASE,GPIO_PIN_5,0);
	SysCtlDelay(10*ui32SysClock/3000);
	
	for(k=0;k<200*M4/M6;k++)
	{
		GPIOPinWrite(GPIO_PORTK_BASE,GPIO_PIN_5,~GPIOPinRead(GPIO_PORTK_BASE,GPIO_PIN_5));
		SysCtlDelay(ui32SysClock/(M4*3));
	}
	GPIOPinWrite(GPIO_PORTK_BASE,GPIO_PIN_5,0);
  SysCtlDelay(10*ui32SysClock/3000);
	
	for(k=0;k<200*M4/M6;k++)
	{
		GPIOPinWrite(GPIO_PORTK_BASE,GPIO_PIN_5,~GPIOPinRead(GPIO_PORTK_BASE,GPIO_PIN_5));
		SysCtlDelay(ui32SysClock/(M4*3));
	}
		GPIOPinWrite(GPIO_PORTK_BASE,GPIO_PIN_5,0);
  SysCtlDelay(10*ui32SysClock/3000);
	
	for(k=0;k<200*M4/M6;k++)
	{
		GPIOPinWrite(GPIO_PORTK_BASE,GPIO_PIN_5,~GPIOPinRead(GPIO_PORTK_BASE,GPIO_PIN_5));
		SysCtlDelay(ui32SysClock/(M4*3));
	}
		GPIOPinWrite(GPIO_PORTK_BASE,GPIO_PIN_5,0);
  SysCtlDelay(10*ui32SysClock/3000);
	
	for(k=0;k<200;k++)
	{
		GPIOPinWrite(GPIO_PORTK_BASE,GPIO_PIN_5,~GPIOPinRead(GPIO_PORTK_BASE,GPIO_PIN_5));
		SysCtlDelay(ui32SysClock/(M6*3));
	}
		GPIOPinWrite(GPIO_PORTK_BASE,GPIO_PIN_5,0);
  SysCtlDelay(10*ui32SysClock/3000);
	
	for(k=0;k<200;k++)
	{
		GPIOPinWrite(GPIO_PORTK_BASE,GPIO_PIN_5,~GPIOPinRead(GPIO_PORTK_BASE,GPIO_PIN_5));
		SysCtlDelay(ui32SysClock/(M6*3));
	}
		GPIOPinWrite(GPIO_PORTK_BASE,GPIO_PIN_5,0);
  SysCtlDelay(10*ui32SysClock/3000);
	
	for(k=0;k<200;k++)
	{
		GPIOPinWrite(GPIO_PORTK_BASE,GPIO_PIN_5,~GPIOPinRead(GPIO_PORTK_BASE,GPIO_PIN_5));
		SysCtlDelay(ui32SysClock/(M6*3));
	}
		GPIOPinWrite(GPIO_PORTK_BASE,GPIO_PIN_5,0);
  SysCtlDelay(10*ui32SysClock/3000);
	
	for(k=0;k<200;k++)
	{
		GPIOPinWrite(GPIO_PORTK_BASE,GPIO_PIN_5,~GPIOPinRead(GPIO_PORTK_BASE,GPIO_PIN_5));
		SysCtlDelay(ui32SysClock/(M6*3));
	}
		GPIOPinWrite(GPIO_PORTK_BASE,GPIO_PIN_5,0);
  SysCtlDelay(10*ui32SysClock/3000);
	
	for(k=0;k<200*M5/M6;k++)
	{
		GPIOPinWrite(GPIO_PORTK_BASE,GPIO_PIN_5,~GPIOPinRead(GPIO_PORTK_BASE,GPIO_PIN_5));
		SysCtlDelay(ui32SysClock/(M5*3));
	}
		GPIOPinWrite(GPIO_PORTK_BASE,GPIO_PIN_5,0);
  SysCtlDelay(10*ui32SysClock/3000);
	
	for(k=0;k<200*M5/M6;k++)
	{
		GPIOPinWrite(GPIO_PORTK_BASE,GPIO_PIN_5,~GPIOPinRead(GPIO_PORTK_BASE,GPIO_PIN_5));
		SysCtlDelay(ui32SysClock/(M5*3));
	}
		GPIOPinWrite(GPIO_PORTK_BASE,GPIO_PIN_5,0);
  SysCtlDelay(10*ui32SysClock/3000);
	
	for(k=0;k<200*M5/M6;k++)
	{
		GPIOPinWrite(GPIO_PORTK_BASE,GPIO_PIN_5,~GPIOPinRead(GPIO_PORTK_BASE,GPIO_PIN_5));
		SysCtlDelay(ui32SysClock/(M5*3));
	}
		GPIOPinWrite(GPIO_PORTK_BASE,GPIO_PIN_5,0);
  SysCtlDelay(10*ui32SysClock/3000);
	
	for(k=0;k<200*M5/M6;k++)
	{
		GPIOPinWrite(GPIO_PORTK_BASE,GPIO_PIN_5,~GPIOPinRead(GPIO_PORTK_BASE,GPIO_PIN_5));
		SysCtlDelay(ui32SysClock/(M5*3));
	}
		GPIOPinWrite(GPIO_PORTK_BASE,GPIO_PIN_5,0);
  SysCtlDelay(10*ui32SysClock/3000);
	
	for(k=0;k<200*H1/M6;k++)
	{
		GPIOPinWrite(GPIO_PORTK_BASE,GPIO_PIN_5,~GPIOPinRead(GPIO_PORTK_BASE,GPIO_PIN_5));
		SysCtlDelay(ui32SysClock/(H1*3));
	}
		GPIOPinWrite(GPIO_PORTK_BASE,GPIO_PIN_5,0);
  SysCtlDelay(10*ui32SysClock/3000);
	
	for(k=0;k<200*H1/M6;k++)
	{
		GPIOPinWrite(GPIO_PORTK_BASE,GPIO_PIN_5,~GPIOPinRead(GPIO_PORTK_BASE,GPIO_PIN_5));
		SysCtlDelay(ui32SysClock/(H1*3));
	}
		GPIOPinWrite(GPIO_PORTK_BASE,GPIO_PIN_5,0);
  SysCtlDelay(10*ui32SysClock/3000);
	
	for(k=0;k<200*H1/M6;k++)
	{
		GPIOPinWrite(GPIO_PORTK_BASE,GPIO_PIN_5,~GPIOPinRead(GPIO_PORTK_BASE,GPIO_PIN_5));
		SysCtlDelay(ui32SysClock/(H1*3));
	}
		GPIOPinWrite(GPIO_PORTK_BASE,GPIO_PIN_5,0);
  SysCtlDelay(10*ui32SysClock/3000);
	
	for(k=0;k<200*H1/M6;k++)
	{
		GPIOPinWrite(GPIO_PORTK_BASE,GPIO_PIN_5,~GPIOPinRead(GPIO_PORTK_BASE,GPIO_PIN_5));
		SysCtlDelay(ui32SysClock/(H1*3));
	}
		GPIOPinWrite(GPIO_PORTK_BASE,GPIO_PIN_5,0);
  SysCtlDelay(10*ui32SysClock/3000);
}
//Last edition:2022/06/01 15:09
