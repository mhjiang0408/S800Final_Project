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
volatile uint16_t systick_10ms_couter=0, systick_100ms_couter=0; //10ms��100ms��ʱ��
volatile uint8_t	systick_10ms_status=0, systick_100ms_status=0; //10ms��100ms��ʱ״̬
volatile uint8_t functionChoice = 0;	//����ѡ���־
volatile uint8_t month=0,day=0;    //�����ҷֱ�����·ݡ�����
volatile uint8_t setHour=0,setMinute=0,setSecond=0;	//�����ҷֱ�����������ӵ�ʱ�ӡ����ӡ�����
volatile uint8_t hour=12, minute=0, second=0;	//�����ҷֱ����ʱ�ӡ����ӡ�����
volatile uint8_t carryFlag1 = 0, carryFlag2 = 0,carryFlag3=0,carryFlag4=0; //�жϵ�λ��1�����ڸ�λ�Ƿ���Ҫ��1�ı�־
volatile uint8_t daysOfMonth[12] = {32,29,32,31,32,31,32,32,31,32,31,32}; //ÿ�µ�����
volatile uint8_t clockFlag = 0;    //������Ӧ��־

volatile uint8_t uart_receive_status = 0;
char uart_receive_char[10];

int main(void)
{
	volatile uint8_t commandIndex;
	volatile uint16_t	i2c_flash_cnt_s=0;
	
    
    char month_char[12][4] = {"JAN","FEB","MAR","APR","MAY","JUN","JUL","AUG","SEP","OCT","NOV","DEC"};
    char num_char[12][3] = {"00","01","02","03","04","05","06","07","08","09","10","11"};  
	
	
	char time_char[10];
	
	//�ֱ�洢PC�˷�����ʱ���֡���
	char hour_char[3];
	char minute_char[3];
	char second_char[3];
	
	char command_char[8]; //�洢PC�˷����ġ�SET������INC����GETTIME��
	char gap_char[2];	//�洢��-���򡰣���

	IntMasterDisable();	//���ж�

	S800_Clock_Init();
	S800_GPIO_Init();
	S800_I2C0_Init();
	S800_SysTick_Init();
	S800_UART_Init();
	
	IntMasterEnable();	//���ж�	
	
	while (1)
	{
        if(uart_receive_status)
        {
            // ������Ϣ����
            uart_receive_status = 0;
            
            //�洢PC�˷�����ָ��
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
            
            //�ж�PC�˷��������ִ�У���Ϊ��Чָ����ԡ�Unknown��
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
                // ������ǰ����״̬
                sprintf(time_char,"FUNCTIONSET%01d",commandIndex);
                functionChoice = commandIndex;
                UARTStringPut(time_char);
            }
            else
                UARTStringPut("Unknown!\r\n");
        }
        switch (functionChoice)
        {
        case 0: //����1��ʱ�����ʾ������
            if (systick_100ms_status) //100ms��ʱ��
            {
                systick_100ms_status	= 0; //����100ms��ʱ״̬
                
                if (++i2c_flash_cnt_s >= I2C_FLASHTIME/100)  //10*100ms=1s
                {
                    i2c_flash_cnt_s	= 0;
                    
                    //ÿ��1s�����Ӽ�1
                    timeUpdate(0);
                }
            }
            //�������ʾ������-���ӡ�
            //��ʾʱ��
            showTime();
            break;
        case 1://��ʾ����
        // Ҫ�ճ�����ʱ�䣬��������
            if (systick_100ms_status) //100ms��ʱ��
            {
                systick_100ms_status	= 0; //����100ms��ʱ״̬
                
                if (++i2c_flash_cnt_s >= I2C_FLASHTIME/100)  //10*100ms=1s
                {
                    i2c_flash_cnt_s	= 0;
                    
                    //ÿ��1s�����Ӽ�1
                    timeUpdate(0);
                }
            }
            //��ʾ����
            showDate();
            break;
        default:
            break;
        }

		
	}//end while
}

void timeUpdate(uint8_t byte){
    /*ʱ����º�����
    byte=0:inc second
    byte=1:inc min
    byte=2:inc hour
    3:inc day
    4:inc month*/
    
    //ÿ��1s�����Ӽ�1
    //����
    switch (byte)
    {
    case 0:
        if((second+1) == 60)
        {
            carryFlag1 = 1;
        }
        second = (second+1) % 60;	//���Ӽ�1
        //����
        if(carryFlag1 == 1)
        {	
            if((minute+1) == 60)
                carryFlag2 = 1;
            minute = (minute+1) % 60;	//���Ӽ�1
            carryFlag1 = 0;	//��λ��־����	
        }
        
        break;
    case 1:
        if((minute+1) == 60)
        {
            carryFlag2 = 1;
        }
        //ʱ��
        if(carryFlag2 == 1)
        {	
            if((hour+1)==24){
                carryFlag3=1;
            }
            hour = (hour+1) % 60;	//���Ӹ�λ��1
            carryFlag2 = 0;	//��λ��־����
        }
        // ���ڽ�λ
        if(carryFlag3==1){
            if((day+1)==daysOfMonth[month]){
                carryFlag4=1;
            }
            day=(day+1)%daysOfMonth[month];
            carryFlag3=0;
        }
        // �·ݽ�λ
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
        // ���ڽ�λ
        if(carryFlag3==1){
            if((day+1)==daysOfMonth[month]){
                carryFlag4=1;
            }
            day=(day+1)%daysOfMonth[month];
            carryFlag3=0;
        }
        // �·ݽ�λ
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
        // �·ݽ�λ
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
    //��ʾʱ��
    uint8_t result;
    result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0);	//����port 2д0����ֹ��Ӱ
    result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,seg7[hour/10]);	//write port 1				
    result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,1<<0);			//write port 2
    SysCtlDelay(ui32SysClock/1500); //��ʱ2ms
    result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0);	//����port 2д0����ֹ��Ӱ
    result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,seg7[hour%10]);	//write port 1				
    result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,1<<1);			//write port 2
    SysCtlDelay(ui32SysClock/1500); //��ʱ2ms
    //��ʾ��-��
    result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0);	//����port 2д0����ֹ��Ӱ
    result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,0x40);	//write port 1				
    result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,1<<2);			//write port 2
    SysCtlDelay(ui32SysClock/1500); //��ʱ2ms	
    //��ʾ����
    result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0);	//����port 2д0����ֹ��Ӱ
    result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,seg7[minute/10]);	//write port 1				
    result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,1<<3);			//write port 2
    SysCtlDelay(ui32SysClock/1500); //��ʱ2ms
    result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0);	//����port 2д0����ֹ��Ӱ
    result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,seg7[minute%10]);	//write port 1				
    result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,1<<4);			//write port 2
    SysCtlDelay(ui32SysClock/1500); //��ʱ2ms
    //��ʾ��-��
    result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0);	//����port 2д0����ֹ��Ӱ
    result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,0x40);	//write port 1			
    result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,1<<5);			//write port 2
    SysCtlDelay(ui32SysClock/1500); //��ʱ2ms
    //��ʾ����
    result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0);	//����port 2д0����ֹ��Ӱ
    result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,seg7[second/10]);	//write port 1				
    result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,1<<6);			//write port 2
    SysCtlDelay(ui32SysClock/1500); //��ʱ2ms
    result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0);	//����port 2д0����ֹ��Ӱ
    result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,seg7[second%10]);	//write port 1				
    result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,1<<7);			//write port 2
    SysCtlDelay(ui32SysClock/1500); //��ʱ2ms
}

void showDate(void){
    uint8_t result;
    //��ʾ�·�
    result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0);	//����port 2д0����ֹ��Ӱ
    result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,seg7[(month+1)/10]);	//write port 1				
    result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,1<<3);			//write port 2
    SysCtlDelay(ui32SysClock/1500); //��ʱ2ms
    result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0);	//����port 2д0����ֹ��Ӱ
    result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,seg7[(1+month)%10]);	//write port 1				
    result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,1<<4);			//write port 2
    SysCtlDelay(ui32SysClock/1500); //��ʱ2ms
    //��ʾ��-��
    result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0);	//����port 2д0����ֹ��Ӱ
    result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,0x40);	//write port 1			
    result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,1<<5);			//write port 2
    SysCtlDelay(ui32SysClock/1500); //��ʱ2ms
    //��ʾ����
    result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0);	//����port 2д0����ֹ��Ӱ
    result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,seg7[(1+day)/10]);	//write port 1				
    result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,1<<6);			//write port 2
    SysCtlDelay(ui32SysClock/1500); //��ʱ2ms
    result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0);	//����port 2д0����ֹ��Ӱ
    result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,seg7[(1+day)%10]);	//write port 1				
    result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,1<<7);			//write port 2
    SysCtlDelay(ui32SysClock/1500); //��ʱ2ms
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
    //����PJ0�½��ش���
	GPIOIntTypeSet(GPIO_PORTJ_BASE, GPIO_PIN_0, GPIO_FALLING_EDGE);
	//ʹ��PJ0�ж�
	GPIOIntEnable(GPIO_PORTJ_BASE, GPIO_PIN_0);
	//ʹ��PJ���ж�
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

	while(I2CMasterBusy(I2C0_BASE)){};	//æ�ȴ�
	I2CMasterSlaveAddrSet(I2C0_BASE, DevAddr, false); //��ӻ���ַ��д
	I2CMasterDataPut(I2C0_BASE, RegAddr); //�����ݵ�ַ
	I2CMasterControl(I2C0_BASE,I2C_MASTER_CMD_SINGLE_SEND); //�������߷���
	while(I2CMasterBusBusy(I2C0_BASE));
	if (I2CMasterErr(I2C0_BASE) != I2C_MASTER_ERR_NONE)
		return 0; //����
	Delay(100);

	//receive data
	I2CMasterSlaveAddrSet(I2C0_BASE, DevAddr, true); //��ӻ���ַ����
	I2CMasterControl(I2C0_BASE,I2C_MASTER_CMD_SINGLE_RECEIVE); //�������߽���
	while(I2CMasterBusBusy(I2C0_BASE));
	value=I2CMasterDataGet(I2C0_BASE);
	if (I2CMasterErr(I2C0_BASE) != I2C_MASTER_ERR_NONE)
		return 0; //����
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
	SysTickPeriodSet(ui32SysClock/SYSTICK_FREQUENCY); //��ʱ1ms
	SysTickEnable();
	SysTickIntEnable();
}

/*
	Corresponding to the startup_TM4C129.s vector table systick interrupt program name
*/
void SysTick_Handler(void)
{
    if (systick_100ms_couter == 0) //����1ms��SysTick����100ms�Ķ�ʱ��
	{
		systick_100ms_couter = 100;
		systick_100ms_status = 1;
	}
	else
		systick_100ms_couter--;
	
	if (systick_10ms_couter	== 0) //����1ms��SysTick����10ms�Ķ�ʱ��
	{
		systick_10ms_couter	 = 10;
		systick_10ms_status  = 1;
	}
	else
		systick_10ms_couter--;
    // �����µĹ���ѡ��
    uint8_t SW = I2C0_ReadByte(TCA6424_I2CADDR,TCA6424_INPUT_PORT0);

	
	if (SW&1)   //����1
	{
		systick_100ms_status	= systick_10ms_status = 0; //��ֹ����1��2�ĵ���
        functionChoice = 0;
		GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_0,GPIO_PIN_0);		//����PN0
	}
    else if(SW&2)   //����2
    {
        systick_100ms_status	= systick_10ms_status = 0; //��ֹ����1��2�ĵ���
        functionChoice = 1;
        GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_0,GPIO_PIN_0);		//����PN0
    }
    else if(SW&4){  //����3
        systick_100ms_status	= systick_10ms_status = 0; //��ֹ����1��2�ĵ���
        functionChoice = 2;
        GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_0,GPIO_PIN_0);		//����PN0
    }
    else if(SW&8){  //����4�����ر�����
        systick_100ms_status	= systick_10ms_status = 0; //��ֹ����1��2�ĵ���
        clockFlag = 0;
    }
    else if(SW&16){
        // �����ұߵ�LED
        if(functionChoice==0){
            timeUpdate(0);
        }
        if(functionChoice==1){
            timeUpdate(3)
        }
    }
    else if(SW&32){
        // ���м��LED
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

	if (uart0_int_status & (UART_INT_RX | UART_INT_RT)) 	//���ջ���ճ�ʱ
	{
		while(UARTCharsAvail(UART0_BASE))    							// Loop while there are characters in the receive FIFO.
		{
			//Read the next character from the UART
			uart_receive_char[k++] = UARTCharGetNonBlocking(UART0_BASE);	//�����ܵ����ַ�ΪСд��ĸ������ת��Ϊ��д��ĸ���ٴ洢���Ա�֤�Դ�Сд����Ӧ��
		}
		
		if(uart_receive_char[k-1] == '@')	//�����յ��������@
		{
			uart_receive_char[k-1] = '\0';	//ȥ���������@
			uart_receive_status = 1;	//ֻ�е�PC���������ַ����������ַ������յ��˲Ž����ձ�־��1
			k = 0;
		}
	}

}
void GPIOJ_Handler(void) 	//PortJ�жϴ���
{
	unsigned long intStatus;

	intStatus = GPIOIntStatus(GPIO_PORTJ_BASE, true); //��ȡ�ж�״̬
	SysCtlDelay(ui32SysClock / 150); //delay 20ms ������
	GPIOIntClear(GPIO_PORTJ_BASE, intStatus );  //����ж������ź�

	if (intStatus & GPIO_PIN_0) {	//PJ0�����ж�
	    SW_cnt = SW_cnt % 4 + 1;
	}
}
