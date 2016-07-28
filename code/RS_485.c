#include "reg52.h"
#define uchar unsigned char
#define uint unsigned int
//---------------I/O口位定义-----------------------
sbit led_show = P3^6;
sbit max485_R_W = P3^7;
//-------------------变量定义----------------------
uchar Num_50ms = 0;			//50ms值
uchar Num_Hour = 0;			//小时
uchar Num_minute = 0;		//分钟
uchar Num_second = 0;		//秒钟
uchar led_state = 0;		//LED显示方式
uchar Num_button = 0;		//外部中断按键次数
uchar student_ADR = 0;		//学生地址
uchar ser_data[16];			//串口接收的数据
uchar send_data[16];		//串口发送的数据
uint m = 0;					//串口数据接收计数
uchar Num_check = 0;		//校验和
uchar xxx;
//------------------函数声明-----------------------
void interrupt_init(); 		//定时器初始化函数
void clock_pro();			//内部时钟运行函数
void led_light(int x);		//二极管显示状态函数
uchar check_add(uchar xx);	//字节校验和计算函数
void command_1();			//第一条命令执行函数
void command_2();			//返回帧生成函数
void command_3();			//返回帧发送函数
//------------------函数主体------------------------
void main()
{
	led_show = 0;			//LED初始闪烁状态
	max485_R_W = 0;			//初始接收模式
	student_ADR = P2;		//读取学生地址
	interrupt_init();		//定时器初始化	
	while(1)
	{							
	}
}
//------------------中断初始化程序---------------------
void interrupt_init()
{	
	TMOD=0x21;	//时钟0为模式1，时钟1为模式2			
	TH1=0xFD;	//波特率9600			
	TL1=0xFD;
	TR1=1;

	TH0=0x3C;		//50ms定时
	TL0=0xB0;
	TR0=1;
	ET0=1;
		
	IT0=1;		
	EX0=1;				
		
	SM0=0;
	SM1=1;	//模式1（SM0=1,SM1=1）,
	SM2=0;	//非多机通信（SM2=0）
	REN=1;	//允许串口通信（REN=1）

	ES=1;
	EA=1;	
}
//-----------------内部时钟函数-----------------------
void clock_pro()
{
		Num_second = Num_second+1; 		
		if(Num_second==60)				//60秒钟进位
		{
			Num_second=0;
			Num_minute=Num_minute+1;			
			if(Num_minute==60)			//60分钟进位
			{
				Num_Hour=Num_Hour+1;
			}		
			if(Num_Hour==24)			//24小时清零
			{
				Num_Hour=0;
			}
		}
}
//---------------LED显示方式函数-----------------------
void led_light(led_state)
{
	switch(led_state)
	{
		case 2:	led_show = 0;break;	
		case 1:	led_show = 1;break;
		case 0:	led_show = ~led_show;break;
	}	
}
//------------第一条命令执行函数---------------------
void command_1()
{
	EX0=1;						//开外部中断
	led_state=2;				//点亮二极管
}
//-------------返回帧生成函数--------------------------	 
void command_2()
{
	send_data[0] = 0xAA;
	send_data[1] = 0x0A;
	send_data[2] = 0xF0;
	send_data[3] = student_ADR;
	send_data[4] = 0x07;
	send_data[5] = TL0;
	send_data[6] = TH0;
	send_data[7] = Num_50ms;
	send_data[8] = Num_second;
	send_data[9] = Num_minute;
	send_data[10] = Num_Hour;
	send_data[11] =	((send_data[1])+(send_data[2])+(send_data[3])+(send_data[4])+(send_data[5])+(send_data[6])+(send_data[7])+(send_data[8])+(send_data[9])+(send_data[10]))%256;
	send_data[12] =	0x66;	
}
//--------------------返回帧发送函数-------------------
void command_3()
{
	int nn;	 
	led_state=1;				//关闭二极管
	max485_R_W = 1;				//485进入发送模式 
	for(nn=0;nn<13;)			//依次发送13字节数据
	{
		SBUF = send_data[nn++];
		while(!TI)
		{
		}
		TI=0;					//软件置0
	}
	max485_R_W = 0;				//485进入接收模式
	Num_button=1;				//为下一次记录时间做准备
	EX0=1;						//开外部中断
}						
//------------定时器0中断服务程序---------------------
void inter_0(void)	interrupt 1	//定时器0的中断服务程序
{	
	Num_50ms++;
	if(Num_50ms==10)
	{
		led_light(led_state);		
	}
	if(Num_50ms==20)
	{		
		clock_pro();
		Num_50ms = 0;
		led_light(led_state);		
	}												
}
//------------外部中断0中断服务程序---------------------
void int0(void)	interrupt 0 	//外部中断0的中断服务程序
{
	EX0=0;						//关闭外部中断
	Num_button++;
	switch(Num_button)
	{
		case 1:led_state = 1;break;
		case 2:	command_2();break;
	}
}

//------------串口中断服务程序---------------------
void ser() interrupt 4			//串口中断0的中断服务程序
{
	RI=0;						//提取缓冲区的数据
	if(ser_data[0]!=0xAA)
	{
		xxx=SBUF;
		if(xxx==0xAA)
		{
			ser_data[0]=SBUF;  	//收到帧起始符号，开始存取收到的数据
		}
	}	
	if(ser_data[0]==0xAA)
	{
		ser_data[m]=SBUF;		//依次接收所有后续字符
		m++;				  	//收到的命令处理完成后记得m要置0 ser_data也要至0
	}
	if(ser_data[m-1]==0x66)		//接收到结束字符0x66
	{
		if((ser_data[2]==student_ADR)||(ser_data[2]==0xFF))		//确认是本机地址或广播后进行校验和计算
		{
			Num_check=(ser_data[1]+ser_data[2]+ser_data[3]+ser_data[4])%256;						
			if(Num_check==ser_data[5])							//校验和验证
			{
				switch(ser_data[4])
				{
					case 0x01:command_1();	break;
					case 0x03:command_3();	break;
				}			
			}
		}
		for(m=0;m<16;m++)		//完成对命令的处理之后清除记录，为下一次存储做准备
		{
			ser_data[m]=0;
		}
		m=0;
	}
}