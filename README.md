# S800Final_Project
[本项目](https://github.com/mhjiang0408/S800Final_Project)为SJTU自动化专业工科创2A课程结项大作业，本项目实现了包括时间日期显示，闹钟设置，倒计时秒表，按键时长计时以及串口通讯控制等功能，充分实践了课程中所学习到的S800开发板相关知识。
## 按键功能
- USR_SW1:按键计时开始按键；
- USR_SW2:按键计时结束按键，按下后会同时通过串口通讯向PC端发送计时开始时间、结束时间、持续时长，显示格式均如"16:235"，秒数:毫秒数；
- SW1:功能切换，初始时为时钟模式，点击一次为倒计时秒表，点击两次为闹钟模式，点击三次为日期模式；
- SW2:在日期模式下，为年份循环增大设置按钮；在倒计时模式下，为倒计时开始按钮；
- SW3:在日期模式下，为月份循环增大设置按钮；在倒计时模式下，为倒计时开始按钮；
- SW4:在日期模式下，为天数循环增大设置按钮；在倒计时模式下，为倒计时增加1s；
- SW5:在日期模式下，为小时循环增大设置按钮；在倒计时模式下，为倒计时减少1s；在闹钟模式下，为小时循环增大设置按钮；
- SW6:在日期模式下，为分钟循环增大设置按钮；在倒计时模式下，为倒计时增加0.1s；在闹钟模式下，为分钟循环增大设置按钮；
- SW7:在日期模式下，为秒数循环增大设置按钮；在倒计时模式下，为倒计时减少0.1s；在闹钟模式下，为秒数循环增大设置按钮；
- SW8:按下停下闹钟声响；在倒计时模式下，为倒计时增加0.01s

## 串口指令
- `INITCLOCK`:将时钟初始化为00:00:00
- `InITALARM`:清除闹钟
- `SETDATEXXXX-XX-XX`:设置日期
- `SETTIMEXX:XX:XX`:设置时间
- `SETALARMXX:XX:XX`:设置闹钟时间
- `GETTIME`:获取时间
- `GETDATE`:获取日期
- `GETALARM`:获取闹钟时间
- `RUNDATE`:运行时钟功能
- `RUNTIME`:运行倒计时秒表
- `?`:提示所有串口命令
- 当错误命令，会反馈ERROR COMMANDS!，同时显示串口命令提示

## 其他特殊功能
- LED用于辅助提示当前模式，不同模式下LED显示分布不同；
- 开机时流水LED灯，同时在数据管上显示欢迎语句和学号521021910017，串口显示作者姓名与“HELLO,WORLD"。

## 亮点难点
### 特殊亮点
1. 本项目在开机时通过蜂鸣器配以开机音乐，实现本闹钟的独特性与创新性；开机音乐的实现主要是通过调节蜂鸣器鸣响频率从而实现乐调的呈现。
```cpp
void BootMusic(void)
{
	// 开机音乐
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
```

2. 同时，本程序使用模块化开发思路，在主循环内只放置功能函数，在功能函数内使用大量工具函数，将整体程序分割为主循环、功能函数模块、工具函数模块三部分。主循环代码见下所示。
```cpp
while (1)
{
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
```
3. 为最大限度利用蓝板上8个按键，本程序利用一个按键SW1来进行模式切换，同时在按键识别程序中均使用200ms的延时来进行按键消抖以实现更好的识别效果。
```cpp
old_key = ~I2C0_ReadByte(TCA6424_I2CADDR,TCA6424_INPUT_PORT0);
if(old_key&0x01)
{
    SysCtlDelay(ui32SysClock/15); //2ms
    key = ~I2C0_ReadByte(TCA6424_I2CADDR,TCA6424_INPUT_PORT0);
    if((key&0x01)!=0x00){
        functionChoice=(functionChoice+1)%4;
    }
}
```

### 项目难点
1. 在程序中需要频繁用到时间的进位，如果全部放置于各函数体内部会非常臃肿，因此设计专门的时间更新函数`timeUpdate`来进行时间进位；
2. 由于模式切换，在初始版本中会出现数码管频繁闪动，经过研究发现是由于按键选择时不够稳定，造成在模式间不断跳跃。因此，我增加了在按键检测中的延时，从而使得按键更加稳定（相关核心代码见项目亮点2）
3. 前序相关Lab中由于while主循环的存在，数码管显示部分我通常是一次性列出所有的数码管，利用while循环来进行显示。但由于本项目需要进行模块化，因此上述方法不能使用。经过研究，我引入显示value数组和for循环来进行显示。以时间显示为例，首先将待输出数据整合进数组`time_value`，利用for循环，在循环中循环输出一遍数码管。由于外部while是每1ms刷新一次，因此可以不断重复访问该函数，实现循环输出以视觉暂留的效果。

```cpp
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
		result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,0);	//先往port 2写0，防止拖影
		if(t2==2||t2==5) result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,0x40);  //显示中间横杠
		else result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT1,seg7[time_value[t2]]);                                                 		//write port 1                                             		//write port 1
		result = I2C0_WriteByte(TCA6424_I2CADDR,TCA6424_OUTPUT_PORT2,1<<t2);
		Delay(1000);
	}		
}
```