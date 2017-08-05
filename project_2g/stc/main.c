#include "stc15w10x.h"
#include "intrins.h"
#define GIO_set(x) (1<<x)

#define GIO_setWeakOutput(x) do {P3M1 &= ~(1<<x);	P3M0 &= ~(1<<x);} while(0)
#define GIO_setStrongOutput(x) do {P3M1 &= ~(1<<x);	P3M0 |= (1<<x);} while(0)
#define GIO_setInput(x) do {P3M1 |= (1<<x); P3M0 &= ~(1<<x);} while(0)
#define LongDelay() 	do{unsigned char loop = 200;while(loop--)	Delay20ms();}while(0)
#define ClrWDT()	do{WDT_CONTR |= 0x10;}while(0)

sbit DC_Power = P3^2;
#define DC_Power_ON 1
#define DC_Power_OFF 0

sbit SYSPower_Enabele = P3^3;
#define SYS_POWER_ON 1
#define SYS_POWER_OFF 0

sbit SYSPower_Status = P3^5;
sbit PowerDown_By_K24 = P3^4;			//K24����ػ��ùܽţ�K24���ùܽ�����100ms���ϣ����ж�����ϵͳ��Դ

sbit PowerDown_Status = P3^1;

sbit KEY = P3^0;
#define KEY_PRESS 0

bit KeyPress_Poweron =0;

void Delay20ms();
void sleep();

//-----------------------------------------------
//�жϷ������
void exint0() interrupt 0       //INT0�ⲿ��Դ���
{
	EA = 0;EX0 = 0;
	
	if(DC_Power == DC_Power_ON)			//�ⲿ��Դ�ϵ�
	{
		Delay20ms();
		if(DC_Power == DC_Power_ON)
		{
			SYSPower_Enabele = SYS_POWER_ON;
			INT_CLKO |= 0x10;               //����K24�����ػ��ж�
		}
	}
	
	EX0 = 1;EA = 1;
}
void exint2() interrupt 10          //K24�����ػ��ź�
{
	EA = 0;
  INT_CLKO &= 0xEF;               //�ر��ж�,��ʱϵͳ���Զ�����ڲ����жϱ�־
	
	if(PowerDown_By_K24 == 0)
	{
		Delay20ms();
		Delay20ms();
		Delay20ms();
		Delay20ms();
		Delay20ms();		//ȥ����100ms����ֹϵͳ����ʱ�ܽŵ���
		if(PowerDown_By_K24 == 0)
		{
			SYSPower_Enabele = SYS_POWER_OFF;
		}
	}
	
	INT_CLKO |= 0x10;               //�����ٿ��ж�
	EA = 1;
}

void exint4() interrupt 16          //��Դ�����ж�
{
	EA = 0;
	INT_CLKO &= 0xBF;		//���ж�
	
	//SYSPower_Enabele = POWER_ON;
	if (SYSPower_Status == SYS_POWER_OFF)
		KeyPress_Poweron = 1;
	
	INT_CLKO |= 0x40;               //�����ٿ��ж�
	EA = 1;
}

void main()
{
	//P30������̬����������
	GIO_setInput(0);
	//P31��ǿ�����������K24ϵͳ���͹ػ��ź�
	GIO_setStrongOutput(1);
	PowerDown_Status = 1;
	//P32������̬���ⲿ��Դ��⣬���ⲿ��ԴΪ�ߵ�ƽ���������ж�
	GIO_setInput(2);
	//P33�����������ϵͳ��Դ���أ�ON=1��OFF=0
	GIO_setStrongOutput(3);
	//P34������̬���½��ػ��ѡ�K24����رյ�ԴʱΪ0������Ϊ�ߵ�ƽ
	GIO_setInput(4);
	//P35������̬������Ҫ�жϡ�ϵͳ��Դ״̬��⣬ϵͳ��Դ�ѹ���Ϊ�ߵ�ƽ
	GIO_setInput(5);
	
	
	if( DC_Power == DC_Power_ON )
	{
		SYSPower_Enabele = SYS_POWER_ON;
		LongDelay();		//�ȴ�K24�������ȶ�
	}
	

	
	while(1)
	{
		if(KEY == KEY_PRESS)		//��Դ��������
		{
			if (SYSPower_Status == SYS_POWER_ON)			//����״̬�£�����7����йػ�
			{
				unsigned int loop = 300;
				do
				{
					Delay20ms();
					if(KEY == KEY_PRESS)
					{
						--loop;
						if(loop < 200)
						{
							PowerDown_Status = 0;
						}
						if (loop == 0)
						{
							SYSPower_Enabele = SYS_POWER_OFF;
							ClrWDT();
							while (KEY == KEY_PRESS)	ClrWDT();
						}
						else
						{
							
						}
					}
					else			//δ��ɳ�����7S�Ķ������м䰴�����ɵ�
					{
						PowerDown_Status = 1;
						loop = 0;
					}
				}while(loop);
			}
			else //�ػ�״̬��׼������
			{
				if(KeyPress_Poweron)	//��������������ִ�п�������
				{
					KeyPress_Poweron = 0;
					Delay20ms();
					if(KEY == KEY_PRESS)
					{
						SYSPower_Enabele = SYS_POWER_ON;
						INT_CLKO |= 0x10;               //����K24�����ػ��ж�
						PowerDown_Status = 1;
					}
				}
			}
		}
		
		ClrWDT();
		INT_CLKO |= 0x10;				//ʹ���ⲿ�ػ��ź��ж�
		INT_CLKO |= 0x40;       //ʹ�ܿ��ػ������ж�
		IT0 = 0;
		EX0 = 1;                //ʹ���ⲿ��Դ����ж�
		sleep();
	}		
}


void sleep()
{
	KeyPress_Poweron = 0;		//����������־λȡ��
//	INT_CLKO |= 0x40;       //(EX4 = 1)ʹ�ܿ��ػ������ж�
//	INT_CLKO |= 0x10;				//��K24�ػ��źŻ���
//	EX0 = 1;                //ʹ���ⲿ��Դ����ж�
	
	EA = 1;
	_nop_();
	_nop_();
	PCON |= 0x02;           //��STOP(PCON.1)��1,MCU���������ģʽ
	_nop_();                //��ʱCPU��ʱ��,��ִ��ָ��,����������ֹͣ����
	_nop_();                //�ⲿ�ж��źź��ⲿ��λ�źſ�����ֹ����ģʽ
	_nop_();
	_nop_();
}


void Delay20ms()		//@11.0592MHz
{
	unsigned char i, j, k;

	i = 1;
	j = 216;
	k = 35;
	do
	{
		do
		{
			while (--k);
		} while (--j);
	} while (--i);
	ClrWDT();
}
