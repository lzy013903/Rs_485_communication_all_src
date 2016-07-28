#include "reg52.h"
#define uchar unsigned char
#define uint unsigned int
//---------------I/O��λ����-----------------------
sbit led_show = P3^6;
sbit max485_R_W = P3^7;
//-------------------��������----------------------
uchar Num_50ms = 0;			//50msֵ
uchar Num_Hour = 0;			//Сʱ
uchar Num_minute = 0;		//����
uchar Num_second = 0;		//����
uchar led_state = 0;		//LED��ʾ��ʽ
uchar Num_button = 0;		//�ⲿ�жϰ�������
uchar student_ADR = 0;		//ѧ����ַ
uchar ser_data[16];			//���ڽ��յ�����
uchar send_data[16];		//���ڷ��͵�����
uint m = 0;					//�������ݽ��ռ���
uchar Num_check = 0;		//У���
uchar xxx;
//------------------��������-----------------------
void interrupt_init(); 		//��ʱ����ʼ������
void clock_pro();			//�ڲ�ʱ�����к���
void led_light(int x);		//��������ʾ״̬����
uchar check_add(uchar xx);	//�ֽ�У��ͼ��㺯��
void command_1();			//��һ������ִ�к���
void command_2();			//����֡���ɺ���
void command_3();			//����֡���ͺ���
//------------------��������------------------------
void main()
{
	led_show = 0;			//LED��ʼ��˸״̬
	max485_R_W = 0;			//��ʼ����ģʽ
	student_ADR = P2;		//��ȡѧ����ַ
	interrupt_init();		//��ʱ����ʼ��	
	while(1)
	{							
	}
}
//------------------�жϳ�ʼ������---------------------
void interrupt_init()
{	
	TMOD=0x21;	//ʱ��0Ϊģʽ1��ʱ��1Ϊģʽ2			
	TH1=0xFD;	//������9600			
	TL1=0xFD;
	TR1=1;

	TH0=0x3C;		//50ms��ʱ
	TL0=0xB0;
	TR0=1;
	ET0=1;
		
	IT0=1;		
	EX0=1;				
		
	SM0=0;
	SM1=1;	//ģʽ1��SM0=1,SM1=1��,
	SM2=0;	//�Ƕ��ͨ�ţ�SM2=0��
	REN=1;	//������ͨ�ţ�REN=1��

	ES=1;
	EA=1;	
}
//-----------------�ڲ�ʱ�Ӻ���-----------------------
void clock_pro()
{
		Num_second = Num_second+1; 		
		if(Num_second==60)				//60���ӽ�λ
		{
			Num_second=0;
			Num_minute=Num_minute+1;			
			if(Num_minute==60)			//60���ӽ�λ
			{
				Num_Hour=Num_Hour+1;
			}		
			if(Num_Hour==24)			//24Сʱ����
			{
				Num_Hour=0;
			}
		}
}
//---------------LED��ʾ��ʽ����-----------------------
void led_light(led_state)
{
	switch(led_state)
	{
		case 2:	led_show = 0;break;	
		case 1:	led_show = 1;break;
		case 0:	led_show = ~led_show;break;
	}	
}
//------------��һ������ִ�к���---------------------
void command_1()
{
	EX0=1;						//���ⲿ�ж�
	led_state=2;				//����������
}
//-------------����֡���ɺ���--------------------------	 
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
//--------------------����֡���ͺ���-------------------
void command_3()
{
	int nn;	 
	led_state=1;				//�رն�����
	max485_R_W = 1;				//485���뷢��ģʽ 
	for(nn=0;nn<13;)			//���η���13�ֽ�����
	{
		SBUF = send_data[nn++];
		while(!TI)
		{
		}
		TI=0;					//�����0
	}
	max485_R_W = 0;				//485�������ģʽ
	Num_button=1;				//Ϊ��һ�μ�¼ʱ����׼��
	EX0=1;						//���ⲿ�ж�
}						
//------------��ʱ��0�жϷ������---------------------
void inter_0(void)	interrupt 1	//��ʱ��0���жϷ������
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
//------------�ⲿ�ж�0�жϷ������---------------------
void int0(void)	interrupt 0 	//�ⲿ�ж�0���жϷ������
{
	EX0=0;						//�ر��ⲿ�ж�
	Num_button++;
	switch(Num_button)
	{
		case 1:led_state = 1;break;
		case 2:	command_2();break;
	}
}

//------------�����жϷ������---------------------
void ser() interrupt 4			//�����ж�0���жϷ������
{
	RI=0;						//��ȡ������������
	if(ser_data[0]!=0xAA)
	{
		xxx=SBUF;
		if(xxx==0xAA)
		{
			ser_data[0]=SBUF;  	//�յ�֡��ʼ���ţ���ʼ��ȡ�յ�������
		}
	}	
	if(ser_data[0]==0xAA)
	{
		ser_data[m]=SBUF;		//���ν������к����ַ�
		m++;				  	//�յ����������ɺ�ǵ�mҪ��0 ser_dataҲҪ��0
	}
	if(ser_data[m-1]==0x66)		//���յ������ַ�0x66
	{
		if((ser_data[2]==student_ADR)||(ser_data[2]==0xFF))		//ȷ���Ǳ�����ַ��㲥�����У��ͼ���
		{
			Num_check=(ser_data[1]+ser_data[2]+ser_data[3]+ser_data[4])%256;						
			if(Num_check==ser_data[5])							//У�����֤
			{
				switch(ser_data[4])
				{
					case 0x01:command_1();	break;
					case 0x03:command_3();	break;
				}			
			}
		}
		for(m=0;m<16;m++)		//��ɶ�����Ĵ���֮�������¼��Ϊ��һ�δ洢��׼��
		{
			ser_data[m]=0;
		}
		m=0;
	}
}