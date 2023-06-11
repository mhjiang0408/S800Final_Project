/*author:王奕滔
studentID:521021911082
Class:F2103902
date:2023/06/04*/
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include "hw_memmap.h"
#include "gpio.h"
#include "hw_i2c.h"
#include "hw_types.h"
#include "i2c.h"
#include "pin_map.h"

#include "systick.h"

#include "uart.h"
#include "eeprom.h"
#include <string.h>
#include "pwm.h"
#include "driverlib/hibernate.c"
#define SYSTICK_FREQUENCY		1000			//1000hz

#define	I2C_FLASHTIME				500				//500mS
#define GPIO_FLASHTIME			300				//300mS
#define PWM_PERIOD					4000
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
int  column=0,row=0,flag=0,timer=0,button_time=0,pmw_status=0;
int year=2023,month=6,day=11;  //初始化日期
int x=0,i=0,y=0,z=0;
int x1=0,x2=0,x3=0;
volatile uint8_t y1,y2,y3,y4;
int t=0;
int hour=8,minute=0,second=59;  //初始时间设置
int alarm_second=0,alarm_minute=0,alarm_hour=0;
int button_status=0;
int t1=0;
int t2=0;
int t3=0;
int t4=0;
int t5=0;
int countdown_value=6000;//初始倒计时
int countdown[4];


volatile uint8_t countdown_start;
volatile uint8_t countdown_stop;

volatile uint8_t Sw_status=0x00;
volatile uint8_t result,result1;
int date_value[8];
int date_value1[8];
int time_value[8];
int time_value1[8];
int alarm_value[8];
int alarm_value1[8];
int student_number[8]={2,1,9,1,1,0,8,2};//学号后8位显示
void 		Delay(uint32_t value);
int j=0,alarmcount=0;


void 		S800_GPIO_Init(void);
uint8_t 	I2C0_WriteByte(uint8_t DevAddr, uint8_t RegAddr, uint8_t WriteData);
uint8_t 	I2C0_ReadByte(uint8_t DevAddr, uint8_t RegAddr);

void		S800_I2C0_Init(void);
void 		S800_UART_Init(void);
void 		UARTStringPut(uint8_t *cMessage);
volatile float delay=0;

volatile uint16_t systick_10ms_couter,systick_100ms_couter,systick_1ms_couter,systick_1ms_couter1,systick_15ms_couter,systick_1ms_couter2;
volatile uint8_t	systick_10ms_status,systick_100ms_status,systick_1ms_status,systick_1ms_status1,systick_15ms_status,systick_1ms_status2;
volatile uint16_t  beat=0;
volatile uint8_t cnt,key_value,gpio_status;
volatile uint8_t rightshift = 0x01;
volatile uint8_t rightshift1 = 0x01;
volatile uint32_t result2;
uint32_t ui32SysClock;
uint32_t ui32IntPrioritySystick,ui32IntPriorityUart0;
int32_t uart_receive_status;
void Display_student_number();
void Clock_time();
void Alarm_time();
void Alarmring();
void Uart_set();
void SW_press();
void change_day();
int change_day2(int month1);
void Display_date();
void Display_time();
void Display_alarm();
void Display_countdown();
void PWM_Init2(void);
void UARTStringGet(uint32_t ui32Base,char *cMessage,const char end);
void UARTStringPutNonBlocking(const char *cMessage);
void settime(char* a);
void inctime(char* a);
void gettime();
void setdate(char* str);
void setalarm(char* str);
void setcountdown(char* str);
void transdate(char* str);
void transtime(char* str);
void transalarm(char* str);
void reset(void);
volatile uint16_t systick_1s_couter;
volatile uint8_t	systick_1s_status;
char* transfer(int num,char* str);
uint8_t seg7[] = {0x3f,0x06,0x5b,0x4f,0x66,0x6d,0x7d,0x07,0x7f,0x6f,0x77,0x7c,0x39,0x5e,0x079,0x71,0x5c,0x40,0x73,0x3e};
char Rxbuff[100];//接收到的字符串
char Txbuff[100];//发送字符串
char Txbuff1[100];
char Txbuff2[100];
char Txbuff3[100];
char p[16]={'S','E','T','T','I','M','E','1','2','-','0','0','-','0','0',' '};
char p2[18]={'S','E','T','D','A','T','E','2','0','2','3','/','0','6','/','1','1',' '};
char p3[19]={'S','E','T','A','L','A','R','M','1','0','-','0','0','-','0','0',' '};
int main(void)
{
	volatile uint16_t	i2c_flash_cnt,gpio_flash_cnt;	
	ui32SysClock = SysCtlClockFreqSet((SYSCTL_XTAL_16MHZ |SYSCTL_OSC_INT | SYSCTL_USE_PLL |SYSCTL_CFG_VCO_480), 20000000);
	
  SysTickPeriodSet(ui32SysClock/SYSTICK_FREQUENCY);
	SysTickEnable();
	SysTickIntEnable();															
	  
	IntEnable(INT_GPIOJ);
	S800_GPIO_Init();
	S800_I2C0_Init();
	S800_UART_Init();
	
	IntEnable(INT_UART0);
  UARTIntEnable(UART0_BASE, UART_INT_RX | UART_INT_RT);	
  IntMasterEnable();		
	
	while (1)
	{
		Display_student_number();    //启动跑马灯显示学号后8位
		Sw_status=~I2C0_ReadByte(TCA6424_I2CADDR, TCA6424_INPUT_PORT0);  //read the SW status
		Clock_time();    //时钟设定
		Alarm_time();    //闹钟设定
		countdown_value%=9999;  //初始化
		Alarmring();  //闹钟响起
		SW_press();   //按键识别
		Uart_set();   //串口通信设置

		Display_date();   //日期显示
		Display_time();   //时间显示
		Display_alarm();    //闹钟显示
		Display_countdown();    //倒计时显示
		
			}
		
}
void PWM_Init(void)    //设定闹钟蜂鸣器响起
{
  SysCtlPeripheralEnable(SYSCTL_PERIPH_PWM0);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOK);

  GPIOPinConfigure(GPIO_PK5_M0PWM7);

	GPIOPinTypePWM(GPIO_PORTK_BASE, GPIO_PIN_5);
  PWMGenConfigure(PWM0_BASE, PWM_GEN_3, PWM_GEN_MODE_UP_DOWN |
                    PWM_GEN_MODE_NO_SYNC);	
  PWMGenPeriodSet(PWM0_BASE, PWM_GEN_3, PWM_PERIOD);	
	PWMPulseWidthSet(PWM0_BASE, PWM_OUT_7,
                     PWM_PERIOD / 2);
	PWMOutputState(PWM0_BASE, PWM_OUT_7_BIT, true);
	PWMGenEnable(PWM0_BASE, PWM_GEN_3);
}
void PWM_Init2(void)
{
  SysCtlPeripheralEnable(SYSCTL_PERIPH_PWM0);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOK);

  GPIOPinConfigure(GPIO_PK5_M0PWM7);

	GPIOPinTypePWM(GPIO_PORTK_BASE, GPIO_PIN_5);
  PWMGenConfigure(PWM0_BASE, PWM_GEN_3, PWM_GEN_MODE_UP_DOWN |
                    PWM_GEN_MODE_NO_SYNC);	
  PWMGenPeriodSet(PWM0_BASE, PWM_GEN_3, PWM_PERIOD);	
	PWMPulseWidthSet(PWM0_BASE, PWM_OUT_7,
                     PWM_PERIOD / 2);
	PWMOutputState(PWM0_BASE, PWM_OUT_7_BIT, true);

	PWMGenEnable(PWM0_BASE, PWM_GEN_3);
}
void Display_student_number(){  //学号后8位，LED跑马灯
while(x<3){
	while(y<900)
	{
		if (systick_1ms_status)
		{
			systick_1ms_status	= 0;
			y++;
		                          
				I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,seg7[student_number[z]]);
				I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,rightshift);
				if((y>0&&y<100))   I2C0_WriteByte(PCA9557_I2CADDR,PCA9557_OUTPUT,0x7f);
				if((y>100&&y<200))   I2C0_WriteByte(PCA9557_I2CADDR,PCA9557_OUTPUT,0xbf);
				if((y>200&&y<300))   I2C0_WriteByte(PCA9557_I2CADDR,PCA9557_OUTPUT,0xdf);
				if((y>300&&y<400))   I2C0_WriteByte(PCA9557_I2CADDR,PCA9557_OUTPUT,0xef);
				if((y>400&&y<500))   I2C0_WriteByte(PCA9557_I2CADDR,PCA9557_OUTPUT,0xf7);
				if((y>500&&y<600))   I2C0_WriteByte(PCA9557_I2CADDR,PCA9557_OUTPUT,0xfb);
				if((y>600&&y<700))   I2C0_WriteByte(PCA9557_I2CADDR,PCA9557_OUTPUT,0xfd);
				if((y>700&&y<800))   	I2C0_WriteByte(PCA9557_I2CADDR,PCA9557_OUTPUT,0xfe);
				rightshift= rightshift<<1;
				z++;
				t++;
					if (t>=8)
					{
						rightshift= 0x01;
						t=0;
						z=0;
					}
				Delay(5000);
				I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0x00);
			//}
	}
}
		y=0;
		x++;
		}

}
void Clock_time(){     //时钟跳动，进位
if(systick_1s_status){
	systick_1s_status=0;
	second++;
	if(second>=60){
		minute++;
		second=0;
	}
	if(minute>=60){
		hour++;
		minute=0;
	}
	if(hour>=24){
		day++;
		hour=0;
	}
switch(month){
	case 1:if(day>31){month++;day=1;}break;
	case 2:if(year%400==0||(year%4==0&&year%100!=0)){
		if(day>29){month++;day=1;break;}
	}
	else if(day>28){month++;day=1;break;}
	case 3:if(day>31){month++;day=1;}break;
	case 4:if(day>30){month++;day=1;}break;
	case 5:if(day>31){month++;day=1;}break;
	case 6:if(day>30){month++;day=1;}break;
	case 7:if(day>31){month++;day=1;}break;
	case 8:if(day>31){month++;day=1;}break;
	case 9:if(day>30){month++;day=1;}break;
	case 10:if(day>31){month++;day=1;}break;
	case 11:if(day>30){month++;day=1;}break;
	case 12:if(day>31){month++;day=1;}break;

}
if (month>12){
	year++;
	month=1;
}
}

}
void Alarm_time(){   //闹钟设定跳动，进位
if(alarm_second>=60){
	alarm_second=0;
	alarm_minute++;
}
if(alarm_minute>=60){
	alarm_minute=0;
	alarm_hour++;
}
if(alarm_hour>=24){
	alarm_hour=0;
}
}

void Alarmring(){  //注意添加，按下按键止闹
if((alarm_hour==hour&&alarm_minute==minute&&alarm_second==second)||pmw_status!=0){
	PWM_Init();
	pmw_status=1;
}
}

void Uart_set(){   //串口通信部分
if(uart_receive_status){
while(UARTCharsAvail(UART0_BASE))
	{
		UARTStringGet(UART0_BASE,Rxbuff,' ');
  }
	if (Rxbuff[0]=='?'){      //
		char *kk1="ERROR COMMANDS!\n";
		char *k1="COMMANDS + FUNCTIONS HELP\n";
		char *s1="INITCLOCK :initialize the clock\n";
		char *s2="SETDATEXXXX/XX/XX :set the date(year/month/day)\n";
		char *s3="SETTIMEXX:XX:XX :set the clock time(hour:minute:second)\n";
		char *s4="SETALARMXX:XX:XX :set the alarm time(hour:minute:second)\n";
		char *s5="GETTIME :get the current clock time\n";
		char *s6="GETDATE :get the current date\n";
		char *s7="GETALARM :get the current alarm time\n";
		char *s8="RUNDATE :display date\n";
		char *s9="RUNTIME :display time\n";
		char *s10="RUNSTWATCHXX.XX :run XX.XXs countdown stwatch\n";
		char *s11="TIPS:end with ' '\n";
	UARTStringPutNonBlocking(k1);
	UARTStringPutNonBlocking(s1);
	UARTStringPutNonBlocking(s2);
	UARTStringPutNonBlocking(s3);
	UARTStringPutNonBlocking(s4);
	UARTStringPutNonBlocking(s5);
	UARTStringPutNonBlocking(s6);
	UARTStringPutNonBlocking(s7);
	UARTStringPutNonBlocking(s8);
	UARTStringPutNonBlocking(s9);
	UARTStringPutNonBlocking(s10);
	UARTStringPutNonBlocking(s11);
	flag=0;
	}
	else if((Rxbuff[0]=='G'&&Rxbuff[3]=='D')||(Rxbuff[0]=='g'&&Rxbuff[3]=='d'))  //当前日期获取
	{
		const char *p4;
		char *p7="DATE";
		transdate(Txbuff);
		p4=Txbuff;
		UARTStringPutNonBlocking(p7);
		UARTStringPutNonBlocking(p4);
		*Txbuff='0';
		flag=0;
	}
	else if((Rxbuff[0]=='R'&&Rxbuff[3]=='D')||(Rxbuff[0]=='r'&&Rxbuff[3]=='d'))  //日期运行RUNDATE
	{
		const char *p4;
		char *p7="DATE";
		transdate(Txbuff);
		p4=Txbuff;
		UARTStringPutNonBlocking(p7);
		UARTStringPutNonBlocking(p4);
		*Txbuff='0';
		flag=0;
	}
	else if((Rxbuff[0]=='G'&&Rxbuff[3]=='T')||(Rxbuff[0]=='g'&&Rxbuff[3]=='t'))  //当前时间获取
	{
		const char *p5;
		char *p8="TIME";
		flag=1;
		transtime(Txbuff);
		p5=Txbuff;
		UARTStringPutNonBlocking(p8);
		UARTStringPutNonBlocking(p5);
		*Txbuff='0';

	}
	else if((Rxbuff[0]=='R'&&Rxbuff[3]=='T')||(Rxbuff[0]=='r'&&Rxbuff[3]=='t'))  //时间运行RUNTIME
	{
		const char *p5;
		char *p8="TIME";
		flag=1;
		transtime(Txbuff);
		p5=Txbuff;
		UARTStringPutNonBlocking(p8);
		UARTStringPutNonBlocking(p5);
		*Txbuff='0';

	}
	else if((Rxbuff[0]=='R'&&Rxbuff[3]=='S')||(Rxbuff[0]=='r'&&Rxbuff[3]=='s'))  //秒表运行RUNSTWATCH
	{
		UARTStringPut((uint8_t *)"countdown");
		UARTStringPut((uint8_t *)Rxbuff+10);
		UARTStringPut((uint8_t *)"s");
		countdown_value=(atof(Rxbuff+10)*100);
	  *Rxbuff='0';
		countdown_start=1;
		flag=3;
	}
	else if((Rxbuff[0]=='G'&&Rxbuff[3]=='A')||(Rxbuff[0]=='g'&&Rxbuff[3]=='a'))  //闹钟时间获取
	{
		const char *p6;
		char *p9="ALARM";
		transalarm(Txbuff);
		p6=Txbuff;
		UARTStringPutNonBlocking(p9);
		UARTStringPutNonBlocking(p6);
		*Txbuff='0';
		flag=2;
	}
	
	else if((Rxbuff[0]=='I')||(Rxbuff[0]=='i'))
	{
		strcpy(Rxbuff,p);
		settime(Rxbuff+7);
	  *Rxbuff='0';
		strcpy(Rxbuff,p2);
		setdate(Rxbuff+7);
	  *Rxbuff='0';
		strcpy(Rxbuff,p3);
		setalarm(Rxbuff+8);
	  *Rxbuff='0';
	}//初始化时钟
	else if((Rxbuff[0]=='S'&&Rxbuff[3]=='T')||(Rxbuff[0]=='s'&&Rxbuff[3]=='t'))
	{
		settime(Rxbuff+7);
		flag=1;
	  *Rxbuff='0';
	}//时间设置
	else if((Rxbuff[0]=='S'&&Rxbuff[3]=='D')||(Rxbuff[0]=='s'&&Rxbuff[3]=='d'))
	{
		setdate(Rxbuff+7);
		flag=0;
	 *Rxbuff='0';
	}//日期设置
	else if((Rxbuff[0]=='S'&&Rxbuff[3]=='A')||(Rxbuff[0]=='s'&&Rxbuff[3]=='a'))
	{
		setalarm(Rxbuff+8);
		flag=2;
	 *Rxbuff='0';
	}//时钟1设置
	else{
		char *kk1="ERROR COMMANDS!\n";
		char *k1="COMMANDS + FUNCTIONS HELP\n";
		char *s1="INITCLOCK :initialize the clock\n";
		char *s2="SETDATEXXXX/XX/XX :set the date(year/month/day)\n";
		char *s3="SETTIMEXX:XX:XX :set the clock time(hour:minute:second)\n";
		char *s4="SETALARMXX:XX:XX :set the alarm time(hour:minute:second)\n";
		char *s5="GETTIME :get the current clock time\n";
		char *s6="GETDATE :get the current date\n";
		char *s7="GETALARM :get the current alarm time\n";
		char *s8="RUNDATE :display date\n";
		char *s9="RUNTIME :display time\n";
		char *s10="RUNSTWATCHXX.XX :run XX.XXs countdown stwatch\n";
		char *s11="TIPS:end with ' '\n";
	UARTStringPutNonBlocking(kk1);
	UARTStringPutNonBlocking(k1);
	UARTStringPutNonBlocking(s1);
	UARTStringPutNonBlocking(s2);
	UARTStringPutNonBlocking(s3);
	UARTStringPutNonBlocking(s4);
	UARTStringPutNonBlocking(s5);
	UARTStringPutNonBlocking(s6);
	UARTStringPutNonBlocking(s7);
	UARTStringPutNonBlocking(s8);
	UARTStringPutNonBlocking(s9);
	UARTStringPutNonBlocking(s10);
	UARTStringPutNonBlocking(s11);
	flag=0;
	
	}
		uart_receive_status=0;
}
}

void SW_press(){     //按键情况获取
if(systick_1ms_status){
	if(Sw_status&0x01){
			button_time++;
		if(button_time>=100){
			button_time=0;
			flag=(flag+1)%4;
		}
	}
	if(Sw_status&0x01){
			button_time++;
		if(button_time>=100){
			button_time=0;
			flag=(flag+1)%4;
		}
	}
	if(Sw_status&0x02&&flag==0){  //按下SW2，加年份
			button_time++;
		if(button_time>=100){
			button_time=0;
			year++;
		}	
	}
	if(Sw_status&0x04&&flag==0){   //按下SW3，加月份
			button_time++;
		if(button_time>=100){
			button_time=0;
			month++;
			if(month>12){
				month=12;
				year++;
			}
		}	
	}
	if(Sw_status&0x08&&flag==0){  //按下SW4，加天数
			button_time++;
		if(button_time>=100){
			button_time=0;
			day++;
			if(day>change_day2(month)){
				day=1;
				month++;
			}
		}	
	}
	if(Sw_status&0x10&&flag==0){  //按下SW5，减天数
			button_time++;
		if(button_time>=100){
			button_time=0;
			day--;
			if(day<=0){
			month--;
			change_day();
				if(month<=0){
					month=12;
					year--;
				}
			}
		}	
	}
	if(Sw_status&0x20&&flag==0){    //按下SW6，减月份
			button_time++;
		if(button_time>=100){
			button_time=0;
			month--;
			if(month<=0){
				month=12;
				year--;
			}
		}	
	}
	if(Sw_status&0x40&&flag==0){   //按下SW7，减年份
			button_time++;
		if(button_time>=100){
			button_time=0;
			year--;
		}	
	}
	if(Sw_status&0x02&&flag==1){    //按下SW2，加小时
			button_time++;
		if(button_time>=100){
			button_time=0;
			hour++;
		}
	}
	if(Sw_status&0x04&&flag==1){   //按下SW3，加分钟
			button_time++;
		if(button_time>=100){
			button_time=0;
			minute++;
			if(minute>=60){
				minute=0;
				hour++;
			}
		}
	}
	if(Sw_status&0x08&&flag==1){   //按下SW4，减分钟
			button_time++;
		if(button_time>=100){
			button_time=0;
			minute--;
			if(minute<0){
				minute=59;
				hour--;
			}
		}
	}
	if(Sw_status&0x10&&flag==1){   //按下SW5,减小时
			button_time++;
		if(button_time>=100){
			button_time=0;
			hour--;
		}
	}
	if(Sw_status&0x02&&flag==2)   //按下SW2，加小时
	{ 
		 button_time++;
		 if(button_time==100)
		 {
			 button_time=0;
			 alarm_hour++;
			 }
	}
	if(Sw_status&0x04&&flag==2)   //按下SW3，加分钟
	{ 
		 button_time++;
		 if(button_time==100)
		 {
			 button_time=0;
			 alarm_minute++;
			 if(alarm_minute>=60){
					alarm_minute=0;
				  alarm_hour++;
			 }
			 }
	}
	if(Sw_status&0x08&&flag==2)    //按下SW4，加一秒
	{ 
		 button_time++;
		 if(button_time==100)
		 {
			 button_time=0;
			 alarm_second++;
			 if(alarm_second>=60){
					alarm_second=0;
				  alarm_minute++;
			 }
			 }
	}    
	if(Sw_status&0x80&&pmw_status==1)  //按下SW8，止闹
		{  button_time++;
			if(button_time==100)
			{ button_time=0;
				PWMOutputState(PWM0_BASE, PWM_OUT_7_BIT, 0);
				pmw_status=0;
			}
		}
	if(Sw_status&0x02&&flag==3)   //按下SW2，开始计时
		{  button_time++;
			if(button_time==100)
			{ button_time=0;
				countdown_start=1;
				countdown_stop=0;
			}
		}
	if(Sw_status&0x04&&flag==3)   //按下SW3，停止计时
		{ button_time++;
			if(button_time==100)
			{ button_time=0;
				countdown_stop=1;
				countdown_start=0;
			}
		}	
	if(Sw_status&0x08&&flag==3)   //按下SW4，加1s
		{ button_time++;
			if(button_time==100)
			{ button_time=0;
				countdown_value+=100;
			}
		}		
	if(Sw_status&0x10&&flag==3)   //按下SW5，减一秒
		{ button_time++;
			if(button_time==100)
			{ button_time=0;
				countdown_value-=100;
			}
		}		
	if(Sw_status&0x20&&flag==3)   //按下SW6，加0.1s
		{ button_time++;
			if(button_time==100)
			{ button_time=0;
				countdown_value+=10;
			}
		}
	if(Sw_status&0x40&&flag==3)   //按下SW7，加0.01s
		{ button_time++;
			if(button_time==100)
			{ button_time=0;
				countdown_value+=1;
			}
		}
	if(Sw_status&0x80&&flag==3)   //按下SW8，减0.01s
		{ button_time++;
			if(button_time==100)
			{ button_time=0;
				countdown_value-=1;
			}
		}
}

}
void change_day(){     //针对日期进位
switch(month){
	case 1:day=31;break;
	case 2:if(year%400==0||(year%4==0&&year%100!=0)){day=29;break;}
					else {day=28;break;}
	case 3:day=31;break;
	case 4:day=30;break;
	case 5:day=31;break;
	case 6:day=30;break;
	case 7:day=31;break;
	case 8:day=31;break;
	case 9:day=30;break;
	case 10:day=31;break;
	case 11:day=30;break;
	case 12:day=31;break;
}
}

int change_day2(int month1){    //针对月份返回本月最大时间
switch(month1){
	case 1:return 31;break;
	case 2:if(year%400==0||(year%4==0&&year%100!=0)){return 29;break;}
					else {return 28;break;}
	case 3:return 31;break;
	case 4:return 30;break;
	case 5:return 31;break;
	case 6:return 30;break;
	case 7:return 31;break;
	case 8:return 31;break;
	case 9:return 30;break;
	case 10:return 31;break;
	case 11:return 30;break;
	case 12:return 31;break;
}
}

void Display_date(){   //日期显示
if(flag==0)
{
	I2C0_WriteByte(PCA9557_I2CADDR,PCA9557_OUTPUT,0xaa);
	date_value[0]=year/1000;
	date_value[1]=year/100%10;
	date_value[2]=year/10%10;
	date_value[3]=year%10;
	date_value[4]=month/10;
	date_value[5]=month%10;
	date_value[6]=day/10;
	date_value[7]=day%10;
		if (systick_1ms_status)
	{
		systick_1ms_status	= 0;
		I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,seg7[date_value[t1]]);                                                 		//write port 1
		I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,rightshift);
    t1++;
		y1++;
		rightshift= rightshift<<1;
		if (y1 >= 0x8)
		{
			rightshift= 0x01;
			y1 = 0;
			t1 = 0;
		}
		Delay(2000);             
    I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0x00);     //避免出现在其他模式下显示
	}
}
}



void Display_time(){    //时间显示
if(flag==1)
{
	I2C0_WriteByte(PCA9557_I2CADDR,PCA9557_OUTPUT,0x55);
	time_value[0]=hour/10;
	time_value[1]=hour%10;
	time_value[2]=17;    //显示中间横杆
	time_value[3]=minute/10;
	time_value[4]=minute%10;
	time_value[5]=17;   //显示中间横杠
	time_value[6]=second/10;
	time_value[7]=second%10;
		if (systick_1ms_status)
	{
		systick_1ms_status	= 0;
		I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,seg7[time_value[t2]]);                                                 		//write port 1
		I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,rightshift);
    t2++;
		y2++;
		rightshift= rightshift<<1;
		if (y2 >= 0x08)
		{
			rightshift= 0x01;
			y2 = 0;
			t2 = 0;
		}
		Delay(2000);             
    I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0x00);     //避免出现在其他模式下显示

	}
}

}

void Display_alarm(){   //闹钟显示
if(flag==2)
{
	I2C0_WriteByte(PCA9557_I2CADDR,PCA9557_OUTPUT,0xa0);
	alarm_value[0]=alarm_hour/10;
	alarm_value[1]=alarm_hour%10;
	alarm_value[2]=17;    //显示中间横杆
	alarm_value[3]=alarm_minute/10;
	alarm_value[4]=alarm_minute%10;
	alarm_value[5]=17;   //显示中间横杠
	alarm_value[6]=alarm_second/10;
	alarm_value[7]=alarm_second%10;
	if (systick_1ms_status)
	{
		systick_1ms_status	= 0;
		I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,seg7[alarm_value[t3]]);                                                 		//write port 1
		I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,rightshift);
    t3++;
		y3++;
		rightshift= rightshift<<1;
		if (y3 >= 0x8)
		{
			rightshift= 0x01;
			y3 = 0;
			t3 = 0;
		}
		Delay(2000);             
    I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0x00);     //避免出现在其他模式下显示
	}
}

}


void Display_countdown(){   //倒计时显示
if(flag==3){
	I2C0_WriteByte(PCA9557_I2CADDR,PCA9557_OUTPUT,0x0a);
	if(countdown_start==1){
		if(systick_10ms_status){
			systick_10ms_status=0;
			countdown_value--;
		}
	}
	if(countdown_value<=0){
		countdown_start=0;
		countdown_value=0;
		I2C0_WriteByte(PCA9557_I2CADDR,PCA9557_OUTPUT,0x00);}
	countdown[0]=countdown_value/1000;
	countdown[1]=countdown_value/100%10;
	countdown[2]=countdown_value/10%10;
	countdown[3]=countdown_value%10;
	if (systick_1ms_status)
	{
		systick_1ms_status	= 0;
		I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,seg7[countdown[t4]]);                                                 		//write port 1
		I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,rightshift);
    t4++;
		y4++;
		rightshift= rightshift<<1;
		if (y4 >= 0x4)
		{
			rightshift= 0x01;
			y4 = 0;
			t4 = 0;
		}
		Delay(2000);             
    I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0x00);     //避免出现在其他模式下显示
	}
}

}
void Delay(uint32_t value)  //延时程序
{
	uint32_t ui32Loop;
	for(ui32Loop = 0; ui32Loop < value; ui32Loop++){};
}


void UARTStringPut(uint8_t *cMessage)  //阻塞式发送
{
	while(*cMessage!='\0')
		UARTCharPut(UART0_BASE,*(cMessage++));
}

void UARTStringPutNonBlocking(const char *cMessage)  //非阻塞式发送
{
	
	while(*cMessage!='\0'){
		if (UARTSpaceAvail(UART0_BASE))
			UARTCharPutNonBlocking(UART0_BASE,*(cMessage++));
	}
}

void UARTStringGet(uint32_t ui32Base,char *cMessage,const char end)	//空格跳过，接受字符串
{
    while(1)
	{
	  *cMessage=UARTCharGet(ui32Base);
	  if(*cMessage!=end)
	  {
    	cMessage=cMessage+1;
	  }
  	 else if(UARTCharsAvail(UART0_BASE)==0)
	  {
	   *cMessage='\0';
  	    break;
	  }
	}
}
void S800_UART_Init(void)
{
	SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);						//Enable PortA
	while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOA));			//Wait for the GPIO moduleA ready

	GPIOPinConfigure(GPIO_PA0_U0RX);												// Set GPIO A0 and A1 as UART pins.
  GPIOPinConfigure(GPIO_PA1_U0TX);

  GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);


  UARTConfigSetExpClk(UART0_BASE, ui32SysClock,115200,(UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE |UART_CONFIG_PAR_NONE));
	UARTStringPut((uint8_t *)"\n\n");
	UARTStringPut((uint8_t *)"\n    111          111    1111        1111    1111111111111111    ");
	UARTStringPut((uint8_t *)"\n    111          111    1111        1111    1111111111111111    ");
	UARTStringPut((uint8_t *)"\n    111          111    1111        1111          1111          ");
	UARTStringPut((uint8_t *)"\n    111   1111   111     1111      1111           1111          ");
	UARTStringPut((uint8_t *)"\n    111   1111   111       1111  1111             1111          ");
	UARTStringPut((uint8_t *)"\n     111  1111  111         11111111              1111          ");
	UARTStringPut((uint8_t *)"\n     111  1111  111           1111                1111          ");
	UARTStringPut((uint8_t *)"\n     111  1111  111           1111                1111          ");
	UARTStringPut((uint8_t *)"\n     111 11  11 111           1111                1111          ");
	UARTStringPut((uint8_t *)"\n      11111  11111            1111                1111          ");
	UARTStringPut((uint8_t *)"\n      111      111            1111                1111          ");
	UARTStringPut((uint8_t *)"\n      11        11            1111                1111          ");
	UARTStringPut((uint8_t *)"\n\n");
}
void S800_GPIO_Init(void)
{
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);						//Enable PortF
	while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOF));			//Wait for the GPIO moduleF ready
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOJ);						//Enable PortJ
	while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOJ));			//Wait for the GPIO moduleJ ready
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPION);						//Enable PortN
	while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPION)){};			//Wait for the GPIO moduleN ready
  GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_0);			//Set PF0 as Output pin
	GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_1);
	GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_2);
	GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_3);
  GPIOPinTypeGPIOOutput(GPIO_PORTN_BASE, GPIO_PIN_0);			//Set PN0 as Output pin
  GPIOPinTypeGPIOOutput(GPIO_PORTN_BASE, GPIO_PIN_1);			//Set PN1 as Output pin
  GPIOPinTypeGPIOOutput(GPIO_PORTN_BASE, GPIO_PIN_2);
	GPIOPinTypeGPIOInput(GPIO_PORTJ_BASE,GPIO_PIN_0 | GPIO_PIN_1);//Set the PJ0,PJ1 as input pin
	GPIOPadConfigSet(GPIO_PORTJ_BASE,GPIO_PIN_0 | GPIO_PIN_1,GPIO_STRENGTH_2MA,GPIO_PIN_TYPE_STD_WPU);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOK);						//Enable PortK
  GPIOPinTypeGPIOOutput(GPIO_PORTK_BASE, GPIO_PIN_5);			//Set PK5 as Output pin
	GPIOPadConfigSet(GPIO_PORTK_BASE,GPIO_PIN_5,GPIO_STRENGTH_2MA,GPIO_PIN_TYPE_STD_WPU);
}

void S800_I2C0_Init(void)
{

  SysCtlPeripheralEnable(SYSCTL_PERIPH_I2C0);
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
	GPIOPinConfigure(GPIO_PB2_I2C0SCL);
  GPIOPinConfigure(GPIO_PB3_I2C0SDA);
  GPIOPinTypeI2CSCL(GPIO_PORTB_BASE, GPIO_PIN_2);
  GPIOPinTypeI2C(GPIO_PORTB_BASE, GPIO_PIN_3);

	I2CMasterInitExpClk(I2C0_BASE,ui32SysClock, true);										//config I2C0 400k
	I2CMasterEnable(I2C0_BASE);

	I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_CONFIG_PORT0,0x0ff);		//config port 0 as input
	I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_CONFIG_PORT1,0x0);			//config port 1 as output
	I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_CONFIG_PORT2,0x0);			//config port 2 as output

	I2C0_WriteByte(PCA9557_I2CADDR,PCA9557_CONFIG,0x00);					//config port as output
	I2C0_WriteByte(PCA9557_I2CADDR,PCA9557_OUTPUT,0x0ff);				//turn off the LED1-8

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

	while(I2CMasterBusy(I2C0_BASE)){};	
	I2CMasterSlaveAddrSet(I2C0_BASE, DevAddr, false); 
	I2CMasterDataPut(I2C0_BASE, RegAddr); 
	I2CMasterControl(I2C0_BASE,I2C_MASTER_CMD_SINGLE_SEND);
	while(I2CMasterBusBusy(I2C0_BASE));
	if (I2CMasterErr(I2C0_BASE) != I2C_MASTER_ERR_NONE)
		return 0; 
	Delay(100);

	//receive data
	I2CMasterSlaveAddrSet(I2C0_BASE, DevAddr, true); 
	I2CMasterControl(I2C0_BASE,I2C_MASTER_CMD_SINGLE_RECEIVE);
	while(I2CMasterBusBusy(I2C0_BASE));
	value=I2CMasterDataGet(I2C0_BASE);
	if (I2CMasterErr(I2C0_BASE) != I2C_MASTER_ERR_NONE)
		return 0; 
	Delay(100);

	return value;
}

void SysTick_Handler(void)
{
	if (systick_1ms_couter==0)
	{
		systick_1ms_couter=SYSTICK_FREQUENCY/1000;
		systick_1ms_status=1;
	}
	else{
		systick_1ms_couter--;}
	if (systick_10ms_couter==0)
	{
		systick_10ms_couter=SYSTICK_FREQUENCY/100;
		systick_10ms_status=1;
	}
	else{
		systick_10ms_couter--;}
	if (systick_100ms_couter==0)
	{
		systick_100ms_couter=SYSTICK_FREQUENCY/1000;
		systick_100ms_status=1;
	}
	else{
		systick_100ms_couter--;}
	if (systick_1s_couter	== 0)
	{
		systick_1s_couter	 = SYSTICK_FREQUENCY;
			systick_1s_status=1;
	}
	else
		systick_1s_couter--;

	if (systick_1ms_couter2	== 0)
	{
		systick_1ms_couter2	 = SYSTICK_FREQUENCY/1000;
			systick_1ms_status2=1;
	}
	else
		systick_1ms_couter2--;
	if (systick_1ms_couter1	== 0)
	{
		systick_1ms_couter1	 = SYSTICK_FREQUENCY/1000;
			systick_1ms_status1=1;
	}
	else
		systick_1ms_couter1--;

}


void UART0_Handler(void)
{
  int32_t uart0_int_status;
  uart0_int_status 		= UARTIntStatus(UART0_BASE, true);		// Get the interrrupt status.
  UARTIntClear(UART0_BASE, uart0_int_status);				
if (uart0_int_status & (UART_INT_RX | UART_INT_RT)) 
	{
		uart_receive_status = 1;
	}
	while(GPIOPinRead(GPIO_PORTJ_BASE,GPIO_PIN_1)==0)
	{
	    	I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,seg7[0]);                                                                  		//write port 1
			  I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0xff);
	}
}
void settime(char* str)
{
		char Hour[2];
		char Minute[2];
		char Second[2];
	 strncpy(Hour,str,2);
	 strncpy(Minute,str+3,2);
	 strncpy(Second,str+6,2);
	 hour=atoi(Hour);
	 minute=atoi(Minute);
	 second=atoi(Second);
}
void setdate(char* str)
{
	 char Hour[4];char Minute[2];char Second[2];
	 strncpy(Hour,str,4);
	 strncpy(Minute,str+5,2);
	 strncpy(Second,str+8,2);
	 year=atoi(Hour);
	 month=atoi(Minute);
	 day=atoi(Second);
}
void setalarm(char* str)
{
	 char Hour[2];char Minute[2];char Second[2];
	 strncpy(Hour,str,2);
	 strncpy(Minute,str+3,2);
	 strncpy(Second,str+6,2);
	 alarm_hour=atoi(Hour);
	 alarm_minute=atoi(Minute);
	 alarm_second=atoi(Second);
}
void setcountdown(char* str)
{
	 char Hour[2];
	 char Minute[2];
	 strncpy(Hour,str,2);
	 strncpy(Minute,str+3,2);
	 countdown_value=atoi(Hour)*100+atoi(Minute);
}

void transdate(char* str)
{
	sprintf(str,"%04u%s%02u%s%02u",year,"-",month,"-",day);   
	str[12]='\0';
}

void transtime(char* str)
{
	sprintf(str,"%02u%s%02u%s%02u",hour,":",minute,":",second);   
	str[12]='\0';
}

void transalarm(char* str)
{
	sprintf(str,"%02u%s%02u%s%02u",alarm_hour,":",alarm_minute,":",alarm_second);   
	str[12]='\0';
}
