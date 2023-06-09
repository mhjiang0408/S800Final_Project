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
void        timeUpdate(uint8_t byte);
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

uint32_t ui32SysClock;
uint8_t seg7[] = {0x3f,0x06,0x5b,0x4f,0x66,0x6d,0x7d,0x07,0x7f,0x6f,0x77,0x7c,0x58,0x5e,0x079,0x71,0x5c};

//systick software counter define
volatile uint16_t systick_10ms_couter=0, systick_100ms_couter=0; //10ms和100ms计时器
volatile uint8_t	systick_10ms_status=0, systick_100ms_status=0; //10ms和100ms计时状态
volatile uint8_t functionChoice = 0;	//功能选择标志
volatile uint8_t month=0,day=0;    //从左到右分别代表月份、日期
volatile uint8_t setHour=0,setMinute=0,setSecond=0;	//从左到右分别代表设置闹钟的时钟、分钟、秒钟
volatile uint8_t hour=12, minute=0, second=0;	//从左到右分别代表时钟、分钟、秒钟
volatile uint8_t carryFlag1 = 0, carryFlag2 = 0,carryFlag3=0,carryFlag4=0; //判断低位加1后相邻高位是否需要加1的标志
volatile uint8_t daysOfMonth[12] = {32,29,32,31,32,31,32,32,31,32,31,32}; //每月的天数
volatile uint8_t clockFlag = 0;    //闹钟响应标志

volatile uint8_t uart_receive_status = 0;
char uart_receive_char[10];

int main(void)
{
	volatile uint8_t commandIndex;
	volatile uint16_t	i2c_flash_cnt_s=0;
	
    
    char month_char[12][4] = {"JAN","FEB","MAR","APR","MAY","JUN","JUL","AUG","SEP","OCT","NOV","DEC"};
    char num_char[12][3] = {"00","01","02","03","04","05","06","07","08","09","10","11"};  
	
	
	char time_char[10];
	
	//分别存储PC端发来的时、分、秒
	char hour_char[3];
	char minute_char[3];
	char second_char[3];
	
	char command_char[8]; //存储PC端发来的“SET”、“INC”或“GETTIME”
	char gap_char[2];	//存储“-”或“：”

	IntMasterDisable();	//关中断

	S800_Clock_Init();
	S800_GPIO_Init();
	S800_I2C0_Init();
	S800_SysTick_Init();
	S800_UART_Init();
	
	IntMasterEnable();	//开中断	
	
	while (1)
	{
        if(uart_receive_status)
        {
            // 串口信息接收
            uart_receive_status = 0;
            
            //存储PC端发来的指令
            if(strlen(uart_receive_char) == 11)
            {
                sscanf(uart_receive_char,"%3s%2s%1s%2s%1s%2s",command_char,hour_char,gap_char,minute_char,gap_char,second_char);	
            }
            else if(strlen(uart_receive_char) == 7)
            {
                sscanf(uart_receive_char,"%s",command_char);
            }
            else if(strlen(uart_receive_char) == 8)
            {
                sscanf(uart_receive_char,"%7s%1s",command_char,commandIndex);
            }
            else
                UARTStringPut("Unknown!\r\n");
            
            //判断PC端发来的命令并执行，若为无效指令，回以“Unknown”
            if(strcmp(command_char, "SET") == 0)
            {
                hour = atoi(hour_char);
                minute = atoi(minute_char);
                second = atoi(second_char);
                
                sprintf(time_char,"TIME%02d:%02d:%02d",hour,minute,second);
                UARTStringPut(time_char);
            }
            else if(strcmp(command_char, "GETTIME") == 0)
            {
                sprintf(time_char,"TIME%02d:%02d:%02d",hour,minute,second);
                UARTStringPut(time_char);
            }
            else if(strcmp(command_char,"INITSET")==0){
                // 调整当前功能状态
                sprintf(time_char,"FUNCTIONSET%01d",commandIndex);
                functionChoice = commandIndex;
                UARTStringPut(time_char);
            }
            else
                UARTStringPut("Unknown!\r\n");
        }
        switch (functionChoice)
        {
        case 0: //功能1：时间的显示及设置
            if (systick_100ms_status) //100ms定时到
            {
                systick_100ms_status	= 0; //重置100ms定时状态
                
                if (++i2c_flash_cnt_s >= I2C_FLASHTIME/100)  //10*100ms=1s
                {
                    i2c_flash_cnt_s	= 0;
                    
                    //每过1s，秒钟加1
                    timeUpdate(0);
                }
            }
            //数码管显示”分钟-秒钟”
            //显示时钟
            showTime();
            break;
        case 1://显示日期
        // 要照常更新时间，否则会出错
            if (systick_100ms_status) //100ms定时到
            {
                systick_100ms_status	= 0; //重置100ms定时状态
                
                if (++i2c_flash_cnt_s >= I2C_FLASHTIME/100)  //10*100ms=1s
                {
                    i2c_flash_cnt_s	= 0;
                    
                    //每过1s，秒钟加1
                    timeUpdate(0);
                }
            }
            //显示日期
            showDate();
            break;
        default:
            break;
        }

		
	}//end while
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
            minute = (minute+1) % 60;	//分钟加1
            carryFlag1 = 0;	//进位标志归零	
        }
        
        break;
    case 1:
        if((minute+1) == 60)
        {
            carryFlag2 = 1;
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
            if((day+1)==daysOfMonth[month]){
                carryFlag4=1;
            }
            day=(day+1)%daysOfMonth[month];
            carryFlag3=0;
        }
        // 月份进位
        if(carryFlag4==1){
            month=(month+1)%13;
            carryFlag4=0;
        }
        break;
    case 2:
        if((hour+1) == 24)
        {
            carryFlag3 = 1;
        }
        // 日期进位
        if(carryFlag3==1){
            if((day+1)==daysOfMonth[month]){
                carryFlag4=1;
            }
            day=(day+1)%daysOfMonth[month];
            carryFlag3=0;
        }
        // 月份进位
        if(carryFlag4==1){
            month=(month+1)%13;
            carryFlag4=0;
        }
        break;
    case 3:
        if((day+1) == month[daysOfMonth]])
        {
            carryFlag3 = 1;
        }
        // 月份进位
        if(carryFlag4==1){
            month=(month+1)%13;
            carryFlag4=0;
        }
        break;
    case 4:
        month = (month+1)%12;
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
    SysCtlDelay(ui32SysClock/1500); //延时2ms
    result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0);	//先往port 2写0，防止拖影
    result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,seg7[hour%10]);	//write port 1				
    result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,1<<1);			//write port 2
    SysCtlDelay(ui32SysClock/1500); //延时2ms
    //显示”-”
    result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0);	//先往port 2写0，防止拖影
    result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,0x40);	//write port 1				
    result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,1<<2);			//write port 2
    SysCtlDelay(ui32SysClock/1500); //延时2ms	
    //显示分钟
    result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0);	//先往port 2写0，防止拖影
    result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,seg7[minute/10]);	//write port 1				
    result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,1<<3);			//write port 2
    SysCtlDelay(ui32SysClock/1500); //延时2ms
    result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0);	//先往port 2写0，防止拖影
    result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,seg7[minute%10]);	//write port 1				
    result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,1<<4);			//write port 2
    SysCtlDelay(ui32SysClock/1500); //延时2ms
    //显示”-”
    result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0);	//先往port 2写0，防止拖影
    result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,0x40);	//write port 1			
    result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,1<<5);			//write port 2
    SysCtlDelay(ui32SysClock/1500); //延时2ms
    //显示秒钟
    result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0);	//先往port 2写0，防止拖影
    result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,seg7[second/10]);	//write port 1				
    result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,1<<6);			//write port 2
    SysCtlDelay(ui32SysClock/1500); //延时2ms
    result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0);	//先往port 2写0，防止拖影
    result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,seg7[second%10]);	//write port 1				
    result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,1<<7);			//write port 2
    SysCtlDelay(ui32SysClock/1500); //延时2ms
}

void showDate(void){
    uint8_t result;
    //显示月份
    result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0);	//先往port 2写0，防止拖影
    result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,seg7[(month+1)/10]);	//write port 1				
    result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,1<<3);			//write port 2
    SysCtlDelay(ui32SysClock/1500); //延时2ms
    result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0);	//先往port 2写0，防止拖影
    result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,seg7[(1+month)%10]);	//write port 1				
    result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,1<<4);			//write port 2
    SysCtlDelay(ui32SysClock/1500); //延时2ms
    //显示”-”
    result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0);	//先往port 2写0，防止拖影
    result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,0x40);	//write port 1			
    result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,1<<5);			//write port 2
    SysCtlDelay(ui32SysClock/1500); //延时2ms
    //显示日期
    result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0);	//先往port 2写0，防止拖影
    result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,seg7[(1+day)/10]);	//write port 1				
    result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,1<<6);			//write port 2
    SysCtlDelay(ui32SysClock/1500); //延时2ms
    result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0);	//先往port 2写0，防止拖影
    result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,seg7[(1+day)%10]);	//write port 1				
    result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,1<<7);			//write port 2
    SysCtlDelay(ui32SysClock/1500); //延时2ms
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
	
  GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_0);			//Set PF0 as Output pin
  GPIOPinTypeGPIOOutput(GPIO_PORTN_BASE, GPIO_PIN_0 | GPIO_PIN_1);			//Set PN0,PN1 as Output pin

	GPIOPinTypeGPIOInput(GPIO_PORTJ_BASE,GPIO_PIN_0 | GPIO_PIN_1);//Set the PJ0,PJ1 as input pin
	GPIOPadConfigSet(GPIO_PORTJ_BASE,GPIO_PIN_0 | GPIO_PIN_1,GPIO_STRENGTH_2MA,GPIO_PIN_TYPE_STD_WPU);
    //设置PJ0下降沿触发
	GPIOIntTypeSet(GPIO_PORTJ_BASE, GPIO_PIN_0, GPIO_FALLING_EDGE);
	//使能PJ0中断
	GPIOIntEnable(GPIO_PORTJ_BASE, GPIO_PIN_0);
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
    // 按键下的功能选择
    uint8_t SW = I2C0_ReadByte(TCA6424_I2CADDR,TCA6424_INPUT_PORT0);

	
	if (SW&1)   //按键1
	{
		systick_100ms_status	= systick_10ms_status = 0; //阻止任务1和2的调度
        functionChoice = 0;
		GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_0,GPIO_PIN_0);		//点亮PN0
	}
    else if(SW&2)   //按键2
    {
        systick_100ms_status	= systick_10ms_status = 0; //阻止任务1和2的调度
        functionChoice = 1;
        GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_0,GPIO_PIN_0);		//点亮PN0
    }
    else if(SW&4){  //按键3
        systick_100ms_status	= systick_10ms_status = 0; //阻止任务1和2的调度
        functionChoice = 2;
        GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_0,GPIO_PIN_0);		//点亮PN0
    }
    else if(SW&8){  //按键4——关闭闹钟
        systick_100ms_status	= systick_10ms_status = 0; //阻止任务1和2的调度
        clockFlag = 0;
    }
    else if(SW&16){
        // 调最右边的LED
        if(functionChoice==0){
            timeUpdate(0);
        }
        if(functionChoice==1){
            timeUpdate(3)
        }
    }
    else if(SW&32){
        // 调中间的LED
        if(functionChoice==0){
            timeUpdate(1);
        }
        if(functionChoice==1){
            timeUpdate(4)
        }
    }
    else if(SW&64){
        if(functionChoice==0){
            timeUpdate(2);
        }
    }
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
		
		if(uart_receive_char[k-1] == '@')	//若接收到结束标记@
		{
			uart_receive_char[k-1] = '\0';	//去除结束标记@
			uart_receive_status = 1;	//只有当PC端所发送字符串的所有字符都接收到了才将接收标志置1
			k = 0;
		}
	}

}
void GPIOJ_Handler(void) 	//PortJ中断处理
{
	unsigned long intStatus;

	intStatus = GPIOIntStatus(GPIO_PORTJ_BASE, true); //获取中断状态
	SysCtlDelay(ui32SysClock / 150); //delay 20ms 以消抖
	GPIOIntClear(GPIO_PORTJ_BASE, intStatus );  //清除中断请求信号

	if (intStatus & GPIO_PIN_0) {	//PJ0触发中断
	    SW_cnt = SW_cnt % 4 + 1;
	}
}
