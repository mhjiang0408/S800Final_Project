// author:mhjiang0408@sjtu.edu.cn
// date:2023/6/11
#include <stdint.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h>
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
#define M4  698
#define M5  784
#define M6  880
#define M7  988
#define H1  1048

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

#define	I2C_FLASHTIME				1000				//1S
#define GPIO_FLASHTIME			500				//500mS

void 		S800_Clock_Init(void);
void 		S800_GPIO_Init(void);
void		S800_I2C0_Init(void);
void 		Delay(uint32_t value);
uint8_t I2C0_WriteByte(uint8_t DevAddr, uint8_t RegAddr, uint8_t WriteData);
uint8_t I2C0_ReadByte(uint8_t DevAddr, uint8_t RegAddr);
void 		S800_SysTick_Init(void);
void 		S800_UART_Init(void);
void 		UARTStringPut(const char *cMessage);
void 		UARTStringPut(const char *cMessage);
void        timeUpdate(uint8_t byte);
void        showTime(void);
void        showDate(void);
void AuthorDisplay(void);
void Alarmring();
void PWM_Init(void);
void SWPressed();
void change_day();
void Uart_set();
void setTime(char* str);
void setDate(char* str);
void setAlarm(char* str);
void setCountDown(char* str);
void transDate(char* str);
void transTime(char* str);
void transAlarm(char* str);
void Alarm_time();
void Display_date();
void Display_time();
void Display_alarm();
void Display_countdown();
void BootMusic(void);



uint8_t result;
uint8_t upsidedown=0;
int cnt=0;
uint32_t ui32SysClock;
uint8_t seg7[] = {0x3f,0x06,0x5b,0x4f,0x66,0x6d,0x7d,0x07,0x7f,0x6f,0x77,0x7c,0x58,0x5e,0x079,0x71,0x5c};
uint8_t author[] = {2,1,9,1,0,0,1,7};
uint8_t welcome[]={0x76,0x79,0x38,0x0e,0x37,0x76};
uint8_t x=0,y=0,z=0,i=0,t=0;
//systick software counter define

uint16_t i2c_flash_cnt1=0;
volatile uint16_t systick_1ms_couter2=0,systick_1ms_couter=0,systick_1ms_couter1=0,systick_10ms_couter=0, systick_100ms_couter=0,systick_500ms_couter=0; //10ms和100ms计时器
volatile uint8_t	systick_1ms_status2=0,systick_1ms_status1=0,systick_10ms_status=0, systick_100ms_status=0,systick_1ms_status=0; //10ms和100ms计时状态

// 毫秒存储变量
volatile int msCounter=0;
volatile int pressStart[2];
volatile int pressEnd[2];
volatile int PJPressed=0;
volatile uint16_t systick_1s_couter;
volatile uint8_t	systick_1s_status;
volatile uint8_t rightshift = 0x01;    //右移标志
// volatile uint8_t rightshift1 = 0x01;
volatile uint8_t functionChoice = 1;	//功能选择标志
volatile uint8_t permitted = 0;	//允许设置功能标志
volatile uint8_t  column=0,row=0,timer=0,button_time=0;
volatile uint8_t  initCount = 0;

// 初始化时间
volatile uint8_t button_status=0;
volatile uint8_t t1=0;
volatile uint8_t t2=0;
volatile uint8_t t3=0;
volatile uint8_t t4=0;
volatile uint8_t t5=0;
volatile int countdown_value=6000;//初始倒计时
volatile int countdown[7];
volatile int countdown_start;
volatile int countdown_stop;
char p[16]={'S','E','T','T','I','M','E','0','0','-','0','0','-','0','0',' '};
char p2[18]={'S','E','T','D','A','T','E','2','0','2','3','/','0','6','/','1','3',' '};
char p3[19]={'S','E','T','A','L','A','R','M','1','0','-','0','0','-','0','0',' '};

volatile uint8_t Sw_status=0x00;
volatile int year=2023,month=6,day=13;    //从左到右分别代表年份，月份、日期
volatile uint8_t setHour=0,setMinute=0,setSecond=0;	//从左到右分别代表设置闹钟的时钟、分钟、秒钟
volatile int hour=12, minute=0, second=0;	//从左到右分别代表时钟、分钟、秒钟
volatile uint8_t carryFlag1 = 0, carryFlag2 = 0,carryFlag3=0,carryFlag4=0,carryFlag5=0; //判断低位加1后相邻高位是否需要加1的标志
volatile uint8_t daysOfMonth[12] = {32,29,32,31,32,31,32,32,31,32,31,32}; //每月的天数
volatile uint8_t clockFlag = 0;    //闹钟响应标志
int audio=784;
char const disp_7seg[] = {0x3F,0x06,0x5B,0x4F,0x66,0x6D,  
												  0x7D,0x07,0x7F,0x6F,
												  0x80,0x40,0x08,0x00};
char const updown_disp_7seg[] = {0x3F,0x30,0x5B,0x79,0x74,0x6D,  
																 0x6F,0x38,0x7F,0x7D,
																 0x80,0x40,0x01,0x00};
// 字符串存储

char send_char[100]; //发出字符串
char send_char1[100]; //发出字符串
char send_char2[100]; //发出字符串


volatile uint8_t uart_receive_status = 0;
char uart_receive_char[100];

int main(void)
{
	volatile uint8_t commandIndex;
	volatile uint16_t	i2c_flash_cnt_s=0;
	int gapTime[2];
	char timeChar[20];

    char num_char[12][3] = {"00","01","02","03","04","05","06","07","08","09","10","11"};  
	
	//分别存储PC端发来的时、分、秒
	char hour_char[3];
	char minute_char[3];
	char second_char[3];
	


	IntMasterDisable();	//关中断

	S800_Clock_Init();
	S800_GPIO_Init();
	S800_I2C0_Init();
	S800_SysTick_Init();
	S800_UART_Init();
	
	IntMasterEnable();	//开中断	
	
	AuthorDisplay();
    while (1)
    {
		if(upsidedown){
			t1=7-t1;
			t2=7-t2;
			t3=7-t3;
			t4=7-t4;
			for(i=0;i<10;i++){
				seg7[i]=updown_disp_7seg[i];
			}
		}
		if(upsidedown==0){
			t1=7-t1;
			t2=7-t2;
			t3=7-t3;
			t4=7-t4;
			for(i=0;i<10;i++){
				seg7[i]=disp_7seg[i];
			}
		}
		if(PJPressed){
			if(GPIOPinRead(GPIO_PORTJ_BASE,GPIO_PIN_1)){
				PJPressed=0;
				pressEnd[0]=second;
				pressEnd[1]=msCounter;
				gapTime[0]=pressEnd[0]-pressStart[0];
				gapTime[1]=pressEnd[1]-pressStart[1];
				if(gapTime[1]<0)	//若msCounter溢出
				{
					gapTime[0]--;
					gapTime[1]+=1000;
				}
				sprintf(timeChar,"StartTime%02d:%03d\n",pressStart[0],pressStart[1]);
				UARTStringPut(timeChar);
				sprintf(timeChar,"EndTime%02d:%03d\n",pressEnd[0],pressEnd[1]);
				UARTStringPut(timeChar);
				sprintf(timeChar,"GapTime%02d:%03d\n",gapTime[0],gapTime[1]);
				UARTStringPut(timeChar);
			}
		}
		Sw_status=~I2C0_ReadByte(TCA6424_I2CADDR, TCA6424_INPUT_PORT0);  //read the SW status
        if (systick_100ms_status) //100ms
		{
			systick_100ms_status	= 0; //Reset 100ms
			
			if (++i2c_flash_cnt1		>= I2C_FLASHTIME/100)  //1s
			{
				i2c_flash_cnt1				= 0;
				timeUpdate(0);
			}
		}
        countdown_value%=9999;
        Alarmring();
        SWPressed();
        Uart_set();
        switch (functionChoice)
        {
        case 0:
            Display_date();
            break;
        case 1:
			Display_time();
			break;
		case 2:
			Display_countdown();
			break;
		case 3:
			Display_alarm();
			break;
		
        default:
            break;
        }
    }//end while
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


/***********************功能集成部分*****************/

void AuthorDisplay(void)
{
	uint16_t count1=0,count2=0;
	uint8_t flag=8;
	volatile uint8_t result;
	volatile uint16_t	i2c_flash_cnt=0;
	
	SysCtlDelay(ui32SysClock/3); //1s
	UARTStringPut("HELLO,JMH!.\n");
	
	while(count2<1000)
	{
	  while(count1<1000)
	  {
			
		  if (systick_1ms_status) //1ms定时到
		  {
				if (systick_10ms_status) //100ms定时到
				{
					systick_10ms_status	= 0; //重置100ms定时状态
					
					if (++i2c_flash_cnt		>= I2C_FLASHTIME/100)  //5*100ms=500ms
					{
						i2c_flash_cnt				= 0;

						//LED跑马灯
						I2C0_WriteByte(PCA9557_I2CADDR,PCA9557_OUTPUT,~(1<<cnt));	

						cnt = (cnt+1) % 8;
					}
				}
				systick_1ms_status	= 0; //重置1ms定时状态
				flag = flag%8+1;
				// I2C0_WriteByte(PCA9557_I2CADDR,PCA9557_OUTPUT,0x00);
					
				//Welcome sentence:"HELLOJMH"
				if(flag==1)
					{
						I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0x0);			//P2写0
						I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,welcome[0]);	//write port 1:"H" 					
						I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,1);				//write port 2
					}
				if(flag==2)
					{
						I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0x0);			//P2写0
						I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,welcome[1]);	//write port 1:"E" 					
						I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,2);				//write port 2
					}
				if(flag==3)
					{
						I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0x0);			//P2写0
						I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,welcome[2]);	//write port 1:"L" 					
						I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,4);				//write port 2
					}
				if(flag==4)
					{
						I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0x0);			//P2写0
						I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,welcome[2]);	//write port 1:"L" 					
						I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,8);				//write port 2
					}
				if(flag==5)
					{
						I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0x0);			//P2写0
						I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,welcome[3]);	//write port 1:"O" 					
						I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,16);				//write port 2
					}
				if(flag==6)
					{
						I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0x0);			//P2写0
						I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,welcome[4]);	//write port 1:"J" 					
						I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,32);				//write port 2
					}
				if(flag==7)
					{
						I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0x0);			//P2写0
						I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,welcome[5]);	//write port 1:"M" 					
						I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,64);				//write port 2
					}
				if(flag==8)
					{
						I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0x0);			//P2写0
						I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,welcome[0]);	//write port 1:"H" 					
						I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,128);				//write port 2
					}
					count1++;
				}
		}
		count1=0;
		// I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0x0);			//P2写0
		// I2C0_WriteByte(PCA9557_I2CADDR,PCA9557_OUTPUT,0xff);
    	// BootMusic();
		//SysCtlDelay(ui32SysClock/3); //1s
		while(count2<1000)
		{
			// I2C0_WriteByte(PCA9557_I2CADDR,PCA9557_OUTPUT,0x00);
			
			if (systick_1ms_status) //1ms
			{
				flag=flag%8+1;
				systick_1ms_status	= 0; //Reset 1ms
				//StudentCode:521021910017
				if(flag==1)
					{
						I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0x0);			//P2写0
						I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,seg7[author[0]]);	//write port 1:"2" 					
						I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,1);				//write port 2
					}
				if(flag==2)
					{
						I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0x0);			//P2写0
						I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,seg7[author[1]]);	//write port 1:"1" 					
						I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,2);				//write port 2
					}
				if(flag==3)
					{
						I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0x0);			//P2写0
						I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,seg7[author[2]]);	//write port 1:"9" 					
						I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,4);				//write port 2
					}
				if(flag==4)
					{
						I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0x0);			//P2写0
						I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,seg7[author[3]]);	//write port 1:"1" 					
						I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,8);				//write port 2
					}
				if(flag==5)
					{
						I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0x0);			//P2写0
						I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,seg7[author[4]]);	//write port 1:"0" 					
						I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,16);				//write port 2
					}
				if(flag==6)
					{
						I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0x0);			//P2写0
						I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,seg7[author[5]]);	//write port 1:"0" 					
						I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,32);				//write port 2
					}
				if(flag==7)
					{
						I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0x0);			//P2写0
						I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,seg7[author[6]]);	//write port 1:"1" 					
						I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,64);				//write port 2
					}
				if(flag==8)
					{
						I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0x0);			//P2写0
						I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,seg7[author[7]]);	//write port 1:"7" 					
						I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,128);				//write port 2
					}
					count2++;
				}
		}
		// I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0x0);			//P2写0
		// I2C0_WriteByte(PCA9557_I2CADDR,PCA9557_OUTPUT,0xff);
		SysCtlDelay(ui32SysClock/3); //1s
	}
}

void Display_date(){   //日期显示
    uint8_t date_value[8];
    uint8_t y1=0x0;
	// I2C0_WriteByte(PCA9557_I2CADDR,PCA9557_OUTPUT,0xaa);
	date_value[0]=year/1000;
	date_value[1]=year/100-10*date_value[0];
	date_value[2]=year/10-10*date_value[1]-100*date_value[0];
	date_value[3]=year%10;
	date_value[4]=month/10;
	date_value[5]=month%10;
	date_value[6]=day/10;
	date_value[7]=day%10;
	for(t1=0;t1<8;t1++){
		if(upsidedown){
			result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0);	//先往port 2写0，防止拖影
			result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,seg7[date_value[7-t1]]);
			result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,1<<t1);
			Delay(1000);
		}
		else{
			result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0);	//先往port 2写0，防止拖影
			result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,seg7[date_value[t1]]);
			result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,1<<t1);
			Delay(1000);
		}

	}
}

void Display_time(){    //时间显示
    uint8_t time_value[8];
    uint8_t y2=0x0;
	I2C0_WriteByte(PCA9557_I2CADDR,PCA9557_OUTPUT,0x55);
	time_value[0]=hour/10;
	time_value[1]=hour%10;
	time_value[2]=0x40;    //显示中间横杆
	time_value[3]=minute/10;
	time_value[4]=minute%10;
	time_value[5]=0x40;   //显示中间横杠
	time_value[6]=second/10;
	time_value[7]=second%10;
	for(t2=0;t2<8;t2++)
	{
		if(upsidedown){
			result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0);	//先往port 2写0，防止拖影
			if(7-t2==2||7-t2==5) result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,0x40);  //显示中间横杠
			else result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,seg7[time_value[7-t2]]);                                                 		//write port 1                                             		//write port 1
			result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,1<<t2);
			Delay(1000);
		}
		else{
			result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0);	//先往port 2写0，防止拖影
			if(t2==2||t2==5) result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,0x40);  //显示中间横杠
			else result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,seg7[time_value[t2]]);                                                 		//write port 1                                             		//write port 1
			result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,1<<t2);
			Delay(1000);
		}

	}		
}

void Display_alarm(){   //闹钟显示
    uint8_t alarm_value[8];
    uint8_t y3=0x0;
	I2C0_WriteByte(PCA9557_I2CADDR,PCA9557_OUTPUT,0xa0);
	alarm_value[0]=setHour/10;
	alarm_value[1]=setHour%10;
	alarm_value[2]=0x40;    //显示中间横杆
	alarm_value[3]=setMinute/10;
	alarm_value[4]=setMinute%10;
	alarm_value[5]=0x40;   //显示中间横杠
	alarm_value[6]=setSecond/10;
	alarm_value[7]=setSecond%10;
	for(t3=0;t3<8;t3++){
		if(upsidedown){
			result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0);	//先往port 2写0，防止拖影
			if(7-t3==2||7-t3==5) result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,0x40);  //显示中间横杠
			else result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,seg7[alarm_value[7-t3]]);                                                 		//write port 1                                             		//write port 1
			result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,1<<t3);
			Delay(1000);
		}
		else{
			result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0);	//先往port 2写0，防止拖影
			if(t3==2||t3==5) result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,0x40);  //显示中间横杠
			else result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,seg7[alarm_value[t3]]);                                                 		//write port 1                                             		//write port 1
			result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,1<<t3);
			Delay(1000);
		}
	}
}

void Display_countdown(){   //倒计时显示
    uint8_t y4=0x0;
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
	countdown[0]=countdown_value/100000;
	countdown[1]=countdown_value/10000-10*countdown[0];
	countdown[3]=countdown_value/1000-100*countdown[0]-10*countdown[1];
	countdown[4]=(countdown_value/100)-1000*countdown[0]-100*countdown[1]-10*countdown[3];
	countdown[6]=(countdown_value/10)-10000*countdown[0]-1000*countdown[1]-100*countdown[3]-10*countdown[4];
	countdown[7]=countdown_value%10;
	for(t4=0;t4<8;t4++){
		if(upsidedown){
			result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0);	//先往port 2写0，防止拖影
			if(7-t4==2||7-t4==5) result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,0x40);  //显示中间横杠
			else result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,seg7[countdown[7-t4]]);                                                 		//write port 1                                             		//write port 1
			result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,1<<t4);
			Delay(1000);
		}
		else{
			result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0);	//先往port 2写0，防止拖影
			if(t4==2||t4==5) result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,0x40);  //显示中间横杠
			else result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,seg7[countdown[t4]]);                                                 		//write port 1                                             		//write port 1
			result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,1<<t4);
			Delay(1000);
		}
	}
}


/*****************工具函数部分***********************/
void Alarmring(){  //注意添加，按下按键止闹
    if((setHour==hour&&setMinute==minute&&setSecond==second)||clockFlag!=0){
		if(clockFlag==1){
			BootMusic();
		}
        clockFlag=1;
    }
	if(clockFlag==0){
		GPIOPinWrite(GPIO_PORTK_BASE,GPIO_PIN_5,0x0);
	}
}
void PWM_Init(void)    //设定闹钟蜂鸣器响起
{
    GPIOPinWrite(GPIO_PORTK_BASE, GPIO_PIN_5,~GPIOPinRead(GPIO_PORTK_BASE, GPIO_PIN_5));
    SysCtlDelay(ui32SysClock/(audio*3));			
    GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_1, GPIO_PIN_1);			// Turn on the indicator LED.
    I2C0_WriteByte(PCA9557_I2CADDR,PCA9557_OUTPUT,0x00);        // Turn on the LEDs.
}

void SWPressed(void)
{
	uint8_t old_key;
	uint8_t key;
	int temYear = year-2000;
	old_key = ~I2C0_ReadByte(TCA6424_I2CADDR,TCA6424_INPUT_PORT0);
	if(old_key&0x01)
	{
		SysCtlDelay(ui32SysClock/15); //2ms
		key = ~I2C0_ReadByte(TCA6424_I2CADDR,TCA6424_INPUT_PORT0);
		if((key&0x01)!=0x00){
			functionChoice=(functionChoice+1)%4;
		}
	}
	if(old_key&0x02)
	{
		SysCtlDelay(ui32SysClock/1500); //2ms
		key = ~I2C0_ReadByte(TCA6424_I2CADDR,TCA6424_INPUT_PORT0);
		if((key&0x02)!=0x00){
			if(functionChoice==0){//按下SW2，加年份
				year=(temYear+1)%100+2000;
				SysCtlDelay(ui32SysClock/15); //2ms
				Display_date();

			}
			if(functionChoice==2){//开始计时
				countdown_start=1;
				countdown_stop=0;
			}
		}
	}
	if(old_key&0x04)
	{
		SysCtlDelay(ui32SysClock/1500); //2ms
		key = ~I2C0_ReadByte(TCA6424_I2CADDR,TCA6424_INPUT_PORT0);
		if((key&0x04)!=0x00){
			if(functionChoice==0){//按下SW3，加月份
				month=(month+1)%13;
				if(month==0)month++;
				SysCtlDelay(ui32SysClock/15); //2ms
				Display_date();
			}
			if(functionChoice==2){//暂停计时
				countdown_start=0;
				countdown_stop=1;
			}
		}
	}
	if(old_key&0x08)
	{
		SysCtlDelay(ui32SysClock/1500); //2ms
		key = ~I2C0_ReadByte(TCA6424_I2CADDR,TCA6424_INPUT_PORT0);
		if((key&0x08)!=0x00){
			if(functionChoice==0){//按下SW4，加天数
				day=(day+1)%(daysOfMonth[month-1]+1);
				if(day==0)day++;

				SysCtlDelay(ui32SysClock/15); //2ms
				Display_date();

			}
			if(functionChoice==2&&countdown_start==0){
				countdown_value+=100;
				SysCtlDelay(ui32SysClock/15); //2ms
				Display_countdown();

			}
		}
		
	}
	if(old_key&0x10)
	{
		SysCtlDelay(ui32SysClock/1500); //2ms
		key = ~I2C0_ReadByte(TCA6424_I2CADDR,TCA6424_INPUT_PORT0);
		if((key&0x10)!=0x00){
			if(functionChoice==1){//按下SW5，加小时
				hour=(hour+1)%24;
				SysCtlDelay(ui32SysClock/15); //2ms
				Display_time();

			}
			if(functionChoice==3){
				setHour=(setHour+1)%24;
				SysCtlDelay(ui32SysClock/15); //2ms
				Display_alarm();

			}
			if(functionChoice==2&&countdown_start==0){
				countdown_value-=100;
				SysCtlDelay(ui32SysClock/15); //2ms
				Display_countdown();

			}
		}

		
	}
	if(old_key&0x20)
	{
		SysCtlDelay(ui32SysClock/1500); //2ms
		key = ~I2C0_ReadByte(TCA6424_I2CADDR,TCA6424_INPUT_PORT0);
		if((key&0x20)!=0x00){
			if(functionChoice==1){//按下SW6，加分钟
				minute=(minute+1)%60;
				SysCtlDelay(ui32SysClock/15); //2ms
				Display_time();

			}
			if(functionChoice==3){
				setMinute=(setMinute+1)%60;
				SysCtlDelay(ui32SysClock/15); //2ms
				Display_alarm();

			}
			if(functionChoice==2&&countdown_start==0){
				countdown_value+=10;
				SysCtlDelay(ui32SysClock/15); //2ms
				Display_countdown();

			}
		}
		
		
	}
	if(old_key&0x40)
	{
		SysCtlDelay(ui32SysClock/1500); //2ms
		key = ~I2C0_ReadByte(TCA6424_I2CADDR,TCA6424_INPUT_PORT0);
		if((key&0x40)!=0x00){
			if(functionChoice==1){//按下SW7，加秒数
				second=(second+1)%60;
				SysCtlDelay(ui32SysClock/15); //2ms
				Display_time();

			}
			if(functionChoice==3){
				setSecond=(setSecond+1)%60;
				SysCtlDelay(ui32SysClock/15); //2ms
				Display_alarm();

			}
			if(functionChoice==2&&countdown_start==0){
				countdown_value-=10;
				SysCtlDelay(ui32SysClock/15); //2ms
				Display_countdown();

			}
		}
		
	}
	if(old_key&0x80)
	{
		SysCtlDelay(ui32SysClock/1500); //2ms
		key = ~I2C0_ReadByte(TCA6424_I2CADDR,TCA6424_INPUT_PORT0);
		if((key&0x80)!=0x00){
			clockFlag=0;
			if(functionChoice==2&&countdown_start==0){//按下SW8，加秒数
				countdown_value+=1;
				SysCtlDelay(ui32SysClock/15); //2ms
				Display_countdown();
			}
			
		}
	}
}


void Uart_set(){   //串口通信部分
	if(uart_receive_status){
		if (uart_receive_char[0]=='?'){      //
			char *kk1="ERROR COMMANDS!\n";
			char *k1="COMMANDS + FUNCTIONS HELP\n";
			char *s1="INITCLOCK :initialize the clock\n";
			char *s12="InITALARM`:clear the alarm\n";
			char *s2="SETDATEXXXX-XX-XX :set the date(year/month/day)\n";
			char *s3="SETTIMEXX:XX:XX :set the clock time(hour:minute:second)\n";
			char *s4="SETALARMXX:XX:XX :set the alarm time(hour:minute:second)\n";
			char *s5="GETTIME :get the current clock time\n";
			char *s6="GETDATE :get the current date\n";
			char *s7="GETALARM :get the current alarm time\n";
			char *s8="RUNDATE :display date\n";
			char *s9="RUNTIME :display time\n";
			char *s10="RUNSTWATCH:run countdown stwatch\n";
			char *s11="TIPS:end with '$'\n";
		UARTStringPut(k1);
		UARTStringPut(s1);
		UARTStringPut(s12);
		UARTStringPut(s2);
		UARTStringPut(s3);
		UARTStringPut(s4);
		UARTStringPut(s5);
		UARTStringPut(s6);
		UARTStringPut(s7);
		UARTStringPut(s8);
		UARTStringPut(s9);
		UARTStringPut(s10);
		UARTStringPut(s11);
		functionChoice=1;
		}
		else if(uart_receive_char[0]=='G'&&uart_receive_char[3]=='D')  //当前日期获取
		{
			const char *p4;
			char *p7="DATE";
			transDate(send_char);
			p4=send_char;
			UARTStringPut(p7);
			UARTStringPut(p4);
			*send_char='0';
			functionChoice=0;
		}
		else if((uart_receive_char[0]=='R'&&uart_receive_char[3]=='D'))  //日期运行RUNDATE
		{
			const char *p4;
			char *p7="DATE";
			transDate(send_char);
			p4=send_char;
			UARTStringPut(p7);
			UARTStringPut(p4);
			*send_char='0';
			functionChoice=0;
		}
		else if((uart_receive_char[0]=='G'&&uart_receive_char[3]=='T'))  //当前时间获取
		{
			const char *p5;
			char *p8="TIME";
			functionChoice=1;
			transTime(send_char);
			p5=send_char;
			UARTStringPut(p8);
			UARTStringPut(p5);
			*send_char='0';
			functionChoice=1;
		}
		else if((uart_receive_char[0]=='R'&&uart_receive_char[3]=='T'))  //时间运行RUNTIME
		{
			const char *p5;
			char *p8="TIME";
			functionChoice=1;
			transTime(send_char);
			p5=send_char;
			UARTStringPut(p8);
			UARTStringPut(p5);
			*send_char='0';
			functionChoice=1;
		}
		else if((uart_receive_char[0]=='R'&&uart_receive_char[3]=='S'))  //秒表运行RUNSTWATCH
		{
			UARTStringPut((uint8_t *)"countdown");
			UARTStringPut((uint8_t *)uart_receive_char+10);
			UARTStringPut((uint8_t *)"s");
			*uart_receive_char='0';
			countdown_start=1;
			functionChoice=2;
		}
		else if((uart_receive_char[0]=='G'&&uart_receive_char[3]=='A'))  //闹钟时间获取
		{
			const char *p6;
			char *p9="ALARM";
			transAlarm(send_char);
			p6=send_char;
			UARTStringPut(p9);
			UARTStringPut(p6);
			*send_char='0';
			functionChoice=3;
		}
		else if((uart_receive_char[0]=='I'&&uart_receive_char[4]=='C'))
		{
			strcpy(uart_receive_char,p);
			setTime(uart_receive_char+7);
			*uart_receive_char='0';
			strcpy(uart_receive_char,p2);
			setDate(uart_receive_char+7);
		*uart_receive_char='0';
			strcpy(uart_receive_char,p3);
			setAlarm(uart_receive_char+8);
		*uart_receive_char='0';
		functionChoice=1;
		}//初始化时钟
		else if(uart_receive_char[0]=='I'&&uart_receive_char[4]=='A')
		{
			setHour=0;
			setMinute=0;
			setSecond=0;
			functionChoice=3;
		}
		else if((uart_receive_char[0]=='S'&&uart_receive_char[3]=='T'))
		{
			setTime(uart_receive_char+7);
			functionChoice=1;
			*uart_receive_char='0';
			functionChoice=1;
		}//时间设置
		else if((uart_receive_char[0]=='S'&&uart_receive_char[3]=='D'))
		{
			setDate(uart_receive_char+7);
			functionChoice=0;
			*uart_receive_char='0';
			functionChoice=0;
		}//日期设置
		else if((uart_receive_char[0]=='S'&&uart_receive_char[3]=='A'))
		{
			setAlarm(uart_receive_char+8);
			functionChoice=2;
			*uart_receive_char='0';
			functionChoice=3;
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
			char *s10="RUNSTWATCH:run countdown stwatch\n";
			char *s11="TIPS:end with '$'\n";
		UARTStringPut(kk1);
		UARTStringPut(k1);
		UARTStringPut(s1);
		UARTStringPut(s2);
		UARTStringPut(s3);
		UARTStringPut(s4);
		UARTStringPut(s5);
		UARTStringPut(s6);
		UARTStringPut(s7);
		UARTStringPut(s8);
		UARTStringPut(s9);
		UARTStringPut(s10);
		UARTStringPut(s11);
		functionChoice=1;
		}
	uart_receive_status=0;
	}
}

void timeUpdate(uint8_t byte){
    /*时间更新函数，
    byte=0:inc second
    byte=1:inc min
    byte=2:inc hour
    3:inc day
    4:inc month*/
    
    //每过1s，秒钟加1
    //秒钟
    switch (byte)
    {
    case 0:
        if((second+1) == 60)
        {
            carryFlag1 = 1;
        }
        second = (second+1) % 60;	//秒钟加1
        //分钟
        if(carryFlag1 == 1)
        {	
            if((minute+1) == 60)
                carryFlag2 = 1;
            minute = (minute + 1) % 60;	//分钟加1
            carryFlag1 = 0;	//进位标志归零	
        }
        //时钟
        if(carryFlag2 == 1)
        {	
            if((hour+1)==24){
                carryFlag3=1;
            }
            hour = (hour+1) % 60;	//分钟个位加1
            carryFlag2 = 0;	//进位标志归零
        }
        // 日期进位
        if(carryFlag3==1){
            if((day+1)==daysOfMonth[month-1]){
                carryFlag4=1;
            }
            day=(day+1)%(daysOfMonth[month-1]);
			if(day==0)day++;
            carryFlag3=0;
        }
        // 月份进位
        if(carryFlag4==1){
            if((month+1)==13){
                carryFlag5=1;
            }
            month=(month+1)%13;
            carryFlag4=0;
        }
        // 年份进位
        if(carryFlag5==1){
            year=(year+1)%100;
            carryFlag5=0;
        }
        break;
    case 1:
        if((minute+1) == 60)
        {
            carryFlag2 = 1;
        }
		minute = (minute + 1) % 60;	//分钟加1
        //时钟
        if(carryFlag2 == 1)
        {	
            if((hour+1)==24){
                carryFlag3=1;
            }
            hour = (hour+1) % 60;	//分钟个位加1
            carryFlag2 = 0;	//进位标志归零
        }
        // 日期进位
        if(carryFlag3==1){
            if((day+1)==daysOfMonth[month-1]){
                carryFlag4=1;
            }
            day=(day+1)%(daysOfMonth[month-1]-1);
            carryFlag3=0;
        }
        // 月份进位
        if(carryFlag4==1){
            if((month+1)==13){
                carryFlag5=1;
            }
            month=(month+1)%13;
            carryFlag4=0;
        }
        // 年份进位
        if(carryFlag5==1){
            year=(year+1)%100;
            carryFlag5=0;
        }
        break;
    case 2:
        if((hour+1) == 24)
        {
            carryFlag3 = 1;
        }
		hour = (hour+1) % 24;	//分钟个位加1
        // 日期进位
        if(carryFlag3==1){
            if((day+1)==daysOfMonth[month-1]){
                carryFlag4=1;
            }
            day=(day+1)%(daysOfMonth[month-1]-1);
			if(day==0)day++;
            carryFlag3=0;
        }
        // 月份进位
        if(carryFlag4==1){
            if((month+1)==13){
                carryFlag5=1;
            }
            month=(month+1)%13;
            carryFlag4=0;
        }
        // 年份进位
        if(carryFlag5==1){
            year=(year+1)%100;
            carryFlag5=0;
        }
        break;
    case 3:
        if((day+1) == daysOfMonth[month-1])
        {
            carryFlag3 = 1;
        }
		day=(day+1)%(daysOfMonth[month-1]-1);
		if(day==0)day++;
        // 月份进位
        if(carryFlag4==1){
            if((month+1)==13){
                carryFlag5=1;
            }
            month=(month+1)%13;
            carryFlag4=0;
        }
        // 年份进位
        if(carryFlag5==1){
            year=(year+1)%100;
            carryFlag5=0;
        }
        break;
    case 4:
        if((month+1) == 13)
        {
            carryFlag4 = 1;
        }
        month=(month+1)%13;
        carryFlag4=0;
        // 年份进位
        if(carryFlag5==1){
            year=(year+1)%100;
            carryFlag5=0;
        }
        break;
    case 5:
        year=(year+1)%100;
        carryFlag5=0;
        break;
    default:
        break;
    }
    
}
void showTime(void){
    //显示时间
    uint8_t result;
    result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0);	//先往port 2写0，防止拖影
    result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,seg7[hour/10]);	//write port 1				
    result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,1<<0);			//write port 2
    Delay(5000);

    result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0);	//先往port 2写0，防止拖影
    result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,seg7[hour%10]);	//write port 1				
    result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,1<<1);			//write port 2
    Delay(5000);

    //显示”-”
    result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0);	//先往port 2写0，防止拖影
    result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,0x40);	//write port 1				
    result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,1<<2);			//write port 2
    Delay(5000);
	
    //显示分钟
    result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0);	//先往port 2写0，防止拖影
    result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,seg7[minute/10]);	//write port 1				
    result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,1<<3);			//write port 2
    Delay(5000);

    result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0);	//先往port 2写0，防止拖影
    result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,seg7[minute%10]);	//write port 1				
    result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,1<<4);			//write port 2
    Delay(5000);

    //显示”-”
    result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0);	//先往port 2写0，防止拖影
    result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,0x40);	//write port 1			
    result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,1<<5);			//write port 2
    Delay(5000);

    //显示秒钟
    result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0);	//先往port 2写0，防止拖影
    result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,seg7[second/10]);	//write port 1
    result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,1<<6);			//write port 2
    Delay(5000);

    result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0);	//先往port 2写0，防止拖影
    result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,seg7[second%10]);	//write port 1	
    result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,1<<7);			//write port 2
    Delay(5000);

}

void showDate(void){
    uint8_t result;
    //显示年
    result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0);	//先往port 2写0，防止拖影
    result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,seg7[year/1000]);	//write port 1				
    result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,1<<0);			//write port 2
    Delay(5000);

    result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0);	//先往port 2写0，防止拖影
    result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,seg7[year/100]);	//write port 1				
    result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,1<<1);			//write port 2
    Delay(5000);

    result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0);	//先往port 2写0，防止拖影
    result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,seg7[year/10]);	//write port 1				
    result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,1<<2);			//write port 2
    Delay(5000);

    result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0);	//先往port 2写0，防止拖影
    result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,seg7[year%10]);	//write port 1				
    result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,1<<3);			//write port 2
    Delay(5000);

    //显示月
    result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0);	//先往port 2写0，防止拖影
    result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,seg7[(month+1)/10]);	//write port 1				
    result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,1<<4);			//write port 2
    Delay(5000);

    result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0);	//先往port 2写0，防止拖影
    result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,seg7[(1+month)%10]);	//write port 1			
    result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,1<<5);			//write port 2
    Delay(5000);

    //显示日
    result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0);	//先往port 2写0，防止拖影
    result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,seg7[(1+day)/10]);	//write port 1				
    result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,1<<6);			//write port 2
    Delay(5000);

    result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0);	//先往port 2写0，防止拖影
    result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,seg7[(1+day)%10]);	//write port 1				
    result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,1<<7);			//write port 2
    Delay(5000);

}

void setTime(char* str)
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
void setDate(char* str)
{
    char Hour[4];char Minute[2];char Second[2];
    strncpy(Hour,str,4);
    strncpy(Minute,str+5,2);
    strncpy(Second,str+8,2);
    year=atoi(Hour);
    month=atoi(Minute);
    day=atoi(Second);
}
void setAlarm(char* str)
{
    char Hour[2];char Minute[2];char Second[2];
    strncpy(Hour,str,2);
    strncpy(Minute,str+3,2);
    strncpy(Second,str+6,2);
    setHour=atoi(Hour);
    setMinute=atoi(Minute);
    setSecond=atoi(Second);
}
void setCountDown(char* str)
{
    char Hour[2];
    char Minute[2];
    strncpy(Hour,str,2);
    strncpy(Minute,str+3,2);
    countdown_value=atoi(Hour)*100+atoi(Minute);
}

void transDate(char* str)
{
	sprintf(str,"%04u%s%02u%s%02u",year,"-",month,"-",day);   
	str[12]='\0';
}

void transTime(char* str)
{
	sprintf(str,"%02u%s%02u%s%02u",hour,":",minute,":",second);   
	str[12]='\0';
}

void transAlarm(char* str)
{
	sprintf(str,"%02u%s%02u%s%02u",setHour,":",setMinute,":",setSecond);   
	str[12]='\0';
}

void Alarm_time(){   //闹钟设定跳动，进位
    if(setSecond>=60){
        setSecond=0;
        setMinute++;
    }
    if(setMinute>=60){
        setMinute=0;
        setHour++;
    }
    if(setHour>=24){
        setHour=0;
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
	// 使能中断
	//设置PJ0,PJ1下降沿触发
	GPIOIntTypeSet(GPIO_PORTJ_BASE, GPIO_PIN_0|GPIO_PIN_1, GPIO_FALLING_EDGE);
	//使能PJ0,PJ1中断
	GPIOIntEnable(GPIO_PORTJ_BASE, GPIO_PIN_0|GPIO_PIN_1);
	//使能PJ口中断
	IntEnable(INT_GPIOJ);
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
	
	for(ui32Loop = 0; ui32Loop < value; ui32Loop++){};
}

//--------------- SysTick -----------------
void S800_SysTick_Init(void)
{
	SysTickPeriodSet(ui32SysClock/SYSTICK_FREQUENCY); //定时1ms
	SysTickEnable();
	SysTickIntEnable();
}

/*
	Corresponding to the startup_TM4C129.s vector table systick interrupt program name
*/
void SysTick_Handler(void)
{
	msCounter=(msCounter+1)%1000;
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

	UARTFIFOLevelSet(UART0_BASE,UART_FIFO_TX2_8,UART_FIFO_RX4_8);//set FIFO Level

  	UARTIntEnable(UART0_BASE, UART_INT_RX | UART_INT_RT);	//Enable UART0 RX,TX interrupt
	IntEnable(INT_UART0);
	
	UARTStringPut("\r\nHello, world!\r\n");
    UARTStringPut((uint8_t *)"\n\n");
	UARTStringPut((uint8_t *)"\n     111111111111  111          1111   1111             1111    ");
	UARTStringPut((uint8_t *)"\n          11       11111      111111   1111             1111    ");
	UARTStringPut((uint8_t *)"\n          11       111111    1111111   1111             1111    ");
	UARTStringPut((uint8_t *)"\n          11       11111111111111111   1111             1111    ");
	UARTStringPut((uint8_t *)"\n          11       11   11111     11   1111             1111    ");
	UARTStringPut((uint8_t *)"\n          11       11    111      11   111111111111111111111    ");
	UARTStringPut((uint8_t *)"\n          11       11             11   111111111111111111111    ");
	UARTStringPut((uint8_t *)"\n          11       11             11   1111             1111    ");
	UARTStringPut((uint8_t *)"\n     11   11       11             11   1111             1111    ");
	UARTStringPut((uint8_t *)"\n     111  11       11             11   1111             1111    ");
	UARTStringPut((uint8_t *)"\n       11111       11             11   1111             1111    ");
	UARTStringPut((uint8_t *)"\n        11         11             11   1111             1111    ");
	UARTStringPut((uint8_t *)"\n\n");
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
	static uint8_t k = 0;
	
  uart0_int_status = UARTIntStatus(UART0_BASE, true);			// Get the interrrupt status.
  UARTIntClear(UART0_BASE, uart0_int_status);							//Clear the asserted interrupts

	if (uart0_int_status & (UART_INT_RX | UART_INT_RT)) 	//接收或接收超时
	{
		while(UARTCharsAvail(UART0_BASE))    							// Loop while there are characters in the receive FIFO.
		{
			//Read the next character from the UART
			uart_receive_char[k++] = UARTCharGetNonBlocking(UART0_BASE);	//若接受到的字符为小写字母，则将其转换为大写字母后再存储，以保证对大小写的适应性
		}
		
		if(uart_receive_char[k-1] == '$')	//若接收到结束标记$
		{
			uart_receive_char[k-1] = '\0';	//去除结束标记$
			uart_receive_status = 1;	//只有当PC端所发送字符串的所有字符都接收到了才将接收标志置1
			k = 0;
		}
	}

}
void GPIOJ_Handler(void) 	//PortJ中断处理
{
	char timeChar[20];
	unsigned long intStatus;
	int gapTime[2];

	intStatus = GPIOIntStatus(GPIO_PORTJ_BASE, true); //获取中断状态
	SysCtlDelay(ui32SysClock / 150); //delay 20ms 以消抖
	GPIOIntClear(GPIO_PORTJ_BASE, intStatus );  //清除中断请求信号
	if(intStatus & GPIO_INT_PIN_0){
		upsidedown=(upsidedown+1)%2;
		SysCtlDelay(ui32SysClock / 15); //delay 200ms 以消抖
	}
	if ((intStatus & GPIO_PIN_1)&&PJPressed==0) {	//PJ1触发中断,停止计数
		PJPressed=1;
		pressStart[0]=second;
		pressStart[1]=msCounter;
	}
}
void BootMusic(void)
{
	// 开机音乐
	
	int k;
	if(clockFlag==0)return;
	second++;
	for(k=0;k<200*M4/M6;k++)
	{
		if(clockFlag==0){
		GPIOPinWrite(GPIO_PORTK_BASE,GPIO_PIN_5,0x0);
		return;
	}
		GPIOPinWrite(GPIO_PORTK_BASE,GPIO_PIN_5,~GPIOPinRead(GPIO_PORTK_BASE,GPIO_PIN_5));
		SysCtlDelay(ui32SysClock/(M4*3));
	}

	
	GPIOPinWrite(GPIO_PORTK_BASE,GPIO_PIN_5,0);
	SysCtlDelay(10*ui32SysClock/3000);
	
	for(k=0;k<200*M4/M6;k++)
	{
			if(clockFlag==0){
		GPIOPinWrite(GPIO_PORTK_BASE,GPIO_PIN_5,0x0);
		return;
	}
		GPIOPinWrite(GPIO_PORTK_BASE,GPIO_PIN_5,~GPIOPinRead(GPIO_PORTK_BASE,GPIO_PIN_5));
		SysCtlDelay(ui32SysClock/(M4*3));
	}

	GPIOPinWrite(GPIO_PORTK_BASE,GPIO_PIN_5,0);
  SysCtlDelay(10*ui32SysClock/3000);
	
	for(k=0;k<200*M4/M6;k++)
	{
			if(clockFlag==0){
		GPIOPinWrite(GPIO_PORTK_BASE,GPIO_PIN_5,0x0);
		return;
	}
		GPIOPinWrite(GPIO_PORTK_BASE,GPIO_PIN_5,~GPIOPinRead(GPIO_PORTK_BASE,GPIO_PIN_5));
		SysCtlDelay(ui32SysClock/(M4*3));
	}

		GPIOPinWrite(GPIO_PORTK_BASE,GPIO_PIN_5,0);
  SysCtlDelay(10*ui32SysClock/3000);
	if(clockFlag==0){
		GPIOPinWrite(GPIO_PORTK_BASE,GPIO_PIN_5,0x0);
		return;
	}
	for(k=0;k<200*M4/M6;k++)
	{
			if(clockFlag==0){
		GPIOPinWrite(GPIO_PORTK_BASE,GPIO_PIN_5,0x0);
		return;
	}
		GPIOPinWrite(GPIO_PORTK_BASE,GPIO_PIN_5,~GPIOPinRead(GPIO_PORTK_BASE,GPIO_PIN_5));
		SysCtlDelay(ui32SysClock/(M4*3));
	}

		GPIOPinWrite(GPIO_PORTK_BASE,GPIO_PIN_5,0);
  		SysCtlDelay(10*ui32SysClock/3000);
	
	for(k=0;k<200;k++)
	{
			if(clockFlag==0){
		GPIOPinWrite(GPIO_PORTK_BASE,GPIO_PIN_5,0x0);
		return;
	}
		GPIOPinWrite(GPIO_PORTK_BASE,GPIO_PIN_5,~GPIOPinRead(GPIO_PORTK_BASE,GPIO_PIN_5));
		SysCtlDelay(ui32SysClock/(M6*3));
	}

		GPIOPinWrite(GPIO_PORTK_BASE,GPIO_PIN_5,0);
  SysCtlDelay(10*ui32SysClock/3000);
	
	for(k=0;k<200;k++)
	{
			if(clockFlag==0){
		GPIOPinWrite(GPIO_PORTK_BASE,GPIO_PIN_5,0x0);
		return;
	}
		GPIOPinWrite(GPIO_PORTK_BASE,GPIO_PIN_5,~GPIOPinRead(GPIO_PORTK_BASE,GPIO_PIN_5));
		SysCtlDelay(ui32SysClock/(M6*3));
	}

		GPIOPinWrite(GPIO_PORTK_BASE,GPIO_PIN_5,0);
  SysCtlDelay(10*ui32SysClock/3000);
	
	for(k=0;k<200;k++)
	{
			if(clockFlag==0){
		GPIOPinWrite(GPIO_PORTK_BASE,GPIO_PIN_5,0x0);
		return;
	}
		GPIOPinWrite(GPIO_PORTK_BASE,GPIO_PIN_5,~GPIOPinRead(GPIO_PORTK_BASE,GPIO_PIN_5));
		SysCtlDelay(ui32SysClock/(M6*3));
	}

		GPIOPinWrite(GPIO_PORTK_BASE,GPIO_PIN_5,0);
  SysCtlDelay(10*ui32SysClock/3000);
	
	for(k=0;k<200;k++)
	{
			if(clockFlag==0){
		GPIOPinWrite(GPIO_PORTK_BASE,GPIO_PIN_5,0x0);
		return;
	}
		GPIOPinWrite(GPIO_PORTK_BASE,GPIO_PIN_5,~GPIOPinRead(GPIO_PORTK_BASE,GPIO_PIN_5));
		SysCtlDelay(ui32SysClock/(M6*3));
	}

		GPIOPinWrite(GPIO_PORTK_BASE,GPIO_PIN_5,0);
  SysCtlDelay(10*ui32SysClock/3000);

	for(k=0;k<200*M5/M6;k++)
	{
			if(clockFlag==0){
		GPIOPinWrite(GPIO_PORTK_BASE,GPIO_PIN_5,0x0);
		return;
	}
		GPIOPinWrite(GPIO_PORTK_BASE,GPIO_PIN_5,~GPIOPinRead(GPIO_PORTK_BASE,GPIO_PIN_5));
		SysCtlDelay(ui32SysClock/(M5*3));
	}

		GPIOPinWrite(GPIO_PORTK_BASE,GPIO_PIN_5,0);
  SysCtlDelay(10*ui32SysClock/3000);
	
	for(k=0;k<200*M5/M6;k++)
	{
			if(clockFlag==0){
		GPIOPinWrite(GPIO_PORTK_BASE,GPIO_PIN_5,0x0);
		return;
	}
		GPIOPinWrite(GPIO_PORTK_BASE,GPIO_PIN_5,~GPIOPinRead(GPIO_PORTK_BASE,GPIO_PIN_5));
		SysCtlDelay(ui32SysClock/(M5*3));
	}

		GPIOPinWrite(GPIO_PORTK_BASE,GPIO_PIN_5,0);
  SysCtlDelay(10*ui32SysClock/3000);
	
	for(k=0;k<200*M5/M6;k++)
	{
			if(clockFlag==0){
		GPIOPinWrite(GPIO_PORTK_BASE,GPIO_PIN_5,0x0);
		return;
	}
		GPIOPinWrite(GPIO_PORTK_BASE,GPIO_PIN_5,~GPIOPinRead(GPIO_PORTK_BASE,GPIO_PIN_5));
		SysCtlDelay(ui32SysClock/(M5*3));
	}
		GPIOPinWrite(GPIO_PORTK_BASE,GPIO_PIN_5,0);
  SysCtlDelay(10*ui32SysClock/3000);

	for(k=0;k<200*M5/M6;k++)
	{
			if(clockFlag==0){
		GPIOPinWrite(GPIO_PORTK_BASE,GPIO_PIN_5,0x0);
		return;
	}
		GPIOPinWrite(GPIO_PORTK_BASE,GPIO_PIN_5,~GPIOPinRead(GPIO_PORTK_BASE,GPIO_PIN_5));
		SysCtlDelay(ui32SysClock/(M5*3));
	}

		GPIOPinWrite(GPIO_PORTK_BASE,GPIO_PIN_5,0);
  SysCtlDelay(10*ui32SysClock/3000);
	
	for(k=0;k<200*H1/M6;k++)
	{
			if(clockFlag==0){
		GPIOPinWrite(GPIO_PORTK_BASE,GPIO_PIN_5,0x0);
		return;
	}
		GPIOPinWrite(GPIO_PORTK_BASE,GPIO_PIN_5,~GPIOPinRead(GPIO_PORTK_BASE,GPIO_PIN_5));
		SysCtlDelay(ui32SysClock/(H1*3));
	}

		GPIOPinWrite(GPIO_PORTK_BASE,GPIO_PIN_5,0);
  SysCtlDelay(10*ui32SysClock/3000);
	
	for(k=0;k<200*H1/M6;k++)
	{
			if(clockFlag==0){
		GPIOPinWrite(GPIO_PORTK_BASE,GPIO_PIN_5,0x0);
		return;
	}
		GPIOPinWrite(GPIO_PORTK_BASE,GPIO_PIN_5,~GPIOPinRead(GPIO_PORTK_BASE,GPIO_PIN_5));
		SysCtlDelay(ui32SysClock/(H1*3));
	}
		GPIOPinWrite(GPIO_PORTK_BASE,GPIO_PIN_5,0);
  SysCtlDelay(10*ui32SysClock/3000);

	for(k=0;k<200*H1/M6;k++)
	{
			if(clockFlag==0){
		GPIOPinWrite(GPIO_PORTK_BASE,GPIO_PIN_5,0x0);
		return;
	}
		GPIOPinWrite(GPIO_PORTK_BASE,GPIO_PIN_5,~GPIOPinRead(GPIO_PORTK_BASE,GPIO_PIN_5));
		SysCtlDelay(ui32SysClock/(H1*3));
	}

		GPIOPinWrite(GPIO_PORTK_BASE,GPIO_PIN_5,0);
  SysCtlDelay(10*ui32SysClock/3000);
	
	for(k=0;k<200*H1/M6;k++)
	{
			if(clockFlag==0){
		GPIOPinWrite(GPIO_PORTK_BASE,GPIO_PIN_5,0x0);
		return;
	}
		GPIOPinWrite(GPIO_PORTK_BASE,GPIO_PIN_5,~GPIOPinRead(GPIO_PORTK_BASE,GPIO_PIN_5));
		SysCtlDelay(ui32SysClock/(H1*3));
	}

		GPIOPinWrite(GPIO_PORTK_BASE,GPIO_PIN_5,0);
  SysCtlDelay(10*ui32SysClock/3000);
}
