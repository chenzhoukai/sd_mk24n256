#ifndef __UART_ENCODE_DECODE_H__
#define __UART_ENCODE_DECODE_H__  

#define UART_FRAME_FLAG 0x7e /* ��־�ַ� */
#define UART_FRAME_ESC  0x7d /* ת���ַ� */
#define UART_FRAME_ENC  0x20 /* �����ַ� */

int uartEncode(const void *iBuf, int iLen, unsigned char * oBuf);
int uartDecode(const void *iBuf, int iLen, unsigned char * oBuf);

#endif