/* ����ͷ�ļ� */
#include <ioCC2530.h>
#include <stdio.h>
#include <string.h>

/*�궨��*/
#define D3 P1_0 
#define D4 P1_1
#define D5 P1_3 
#define D6 P1_4
#define SW1 P1_2

/*�������*/
unsigned char counter = 0; //ͳ�ƶ�ʱ���������
unsigned char LedState = 1;//4��LED�ƵĹ���ģʽ��ȡֵ��Χ1��2��3
unsigned char keyCount = 0;//ͳ��SW1�������´���
unsigned char outputStr[100]={0};//��Ŵ����������

/*��������*/
void InitCLK(void);//ϵͳʱ�ӳ�ʼ��������Ϊ32MHz
void UART0SendByte(unsigned char c);//UART0����һ���ֽں���
void UART0SendString(unsigned char *str);//UART0���������ַ���
void delay(void);//��ʱ����
/*���庯��*/
void InitCLK(void)
{
  CLKCONCMD &= 0x80;
  while(CLKCONSTA & 0x40);
}

void UART0SendByte(unsigned char c)
{
  U0DBUF = c;// ��Ҫ���͵�1�ֽ�����д��U0DBUF
  while (!UTX0IF) ;// �ȴ�TX�жϱ�־����U0DBUF����
  UTX0IF = 0;// ����TX�жϱ�־
}

void UART0SendString(unsigned char *str)
{
  while(*str != '\0')
  {
    UART0SendByte(*str++);
  }
}

void delay(void)
{
  unsigned long t = 50000;
  while(t--);
}

/*������*/
void main(void)
{
  InitCLK();
  
  /*.......������1��ʼ��LED��IO�˿ڳ�ʼ��....................*/
  P1SEL &= ~0x1F;//����P1_0��P1_4ΪGPIO��
  P1DIR |= 0x1B;//����P1_0��P1_1��P1_3��P1_4Ϊ���
  D3=D4=D5=D6=0;//Ϩ������LED��
  /*.......������1����.......................................*/
 
  /*.......������2��ʼ�������ж����빦�ܳ�ʼ��...............*/
  P1DIR &= ~0x04;//����P1_2Ϊ����
  P1INP &= ~0x04;//����P1_2�˿�Ϊ������/������ģʽ
  P2INP &= ~0x40;//��������P1�˿�Ϊ��������
  PICTL |= 0x02;//����P1_2�˿��жϴ�����ʽΪ���½��ش���
  IEN2 |= 0x10;//ʹ��P1�˿��ж�
  P1IEN |= 0x04;//ʹ��P1_2�˿��ж�
  /*.......������2����.......................................*/
  
  /*.......������3��ʼ����ʱ��1��ʼ��........................*/
  T1CTL = 0x08;//���ö�ʱ��1Ϊ32��Ƶ����ͣ����
  TIMIF |= 0x40;//ʹ�ܶ�ʱ��1������ж�
  T1IE = 1;//ʹ�ܶ�ʱ��1�ж� 
  /*.......������3����.......................................*/

  /*....... ����0��ʼ��.........................*/
  PERCFG = 0x00;	
  P0SEL = 0x3c;	
  U0CSR |= 0x80;//����USART0ΪUARTģʽ
  U0BAUD = 216;//���ò�����14400
  U0GCR = 8;//���ò�����14400
  U0UCR |= 0x80;
  UTX0IF = 0;  // ����USART0 TX�жϱ�־ 
  /*....... ......................................*/
    
  EA = 1;//ʹ�����ж�  
  while(1)
  {
  /*.......������4��ʼ��LED�ƹ���ģʽ.........................*/
    switch(LedState)
    {
    case 1:
      D3=D4=D5=D6=0;//����D3��D4��D5��D6ȫ��Ϩ��
      break;
    case 2:
      D3=D4=D5=D6=0;//����D3��D4��D5��D6ȫ����˸��ʾ
      delay();
      D3=D4=D5=D6=1;
      delay();
      break;
    case 3:
      D3=D4=0;//����D3��D4Ϩ��D5��D6������˸
      D5=1;
      D6=0;
      delay();
      D5=0;
      D6=1;
      delay();
      break;
    }
  /*.......������4������..................................*/    
  }
}

/*�жϷ�����*/

#pragma vector = P1INT_VECTOR
__interrupt void P1_ISR(void)
{
  if(P1IFG & 0x04)
  {
    /*.......������5��ʼ��...........*/
    keyCount++;//����ͳ�ƴ����Լ�1
    T1CTL |= 0x01;//������ʱ��1��������������ģʽ
    P1IFG &= ~0x04;//���P1_2���жϱ�־λ
    /*.......������5����...........*/
  }
  P1IF = 0;//���P1�˿��жϱ�־λ
}



//��1���ڣ�SW1�������´�����ͬ����ͬ�ĵƵĹ���ģʽ
#pragma vector = T1_VECTOR
__interrupt void T1_ISR(void)
{
  counter++;
  if(counter>10)
  {
      /*.......������6��ʼ��...........*/
    T1CTL &= ~0x01;//��ʱ��1��ͣ����
    T1CNTL = 1;//����T1CNTL���㶨ʱ��1�����Ĵ���
    LedState = keyCount;//����LED�ƹ���ģʽ���ڰ�������
     /*.......������6����...........*/
    keyCount = 0;//����SW1�������´���
    counter = 0;
    memset(outputStr,'\0',100);//���outputStr����
    sprintf((char *)outputStr,"��ǰ������mode%dģʽ\r\n",LedState);//�������ڷ�����Ϣ����
    UART0SendString(outputStr);//���ڷ��͵�ǰ����ģʽ��Ϣ
  }
}
