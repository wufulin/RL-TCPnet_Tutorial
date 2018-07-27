/*
*********************************************************************************************************
*
*	ģ������ : TCPnet����Э��ջ����
*	�ļ����� : app_tcpnet_lib.c
*	��    �� : V1.0
*	˵    �� : ���ԵĹ���˵��
*              1. ǿ���Ƽ������߽ӵ�·�������߽�����������ԣ���Ϊ�Ѿ�ʹ����DHCP�������Զ���ȡIP��ַ��
*              2. ������һ��TCP Server������ʹ���˾���������NetBIOS���û�ֻ���ڵ��Զ�ping armfly
*                 �Ϳ��Ի�ð��ӵ�IP��ַ���˿ں�1001��
*              3. �û������ڵ��Զ������������������TCP Client���Ӵ˷������ˡ�
*              4. ����K1���£�����8�ֽڵ����ݸ�TCP Client��
*              5. ����K2���£�����1024�ֽڵ����ݸ�TCP Client��
*              6. ����K3���£�����5MB�ֽڵ����ݸ�TCP Client��
*
*	�޸ļ�¼ :
*		�汾��   ����         ����        ˵��
*		V1.0    2017-04-17   Eric2013     �׷�
*
*	Copyright (C), 2015-2020, ���������� www.armfly.com
*
*********************************************************************************************************
*/	
#include "includes.h"	



/*
*********************************************************************************************************
*	                                  ���ڱ��ļ��ĵ���
*********************************************************************************************************
*/
#if 1
	#define printf_debug printf
#else
	#define printf_debug(...)
#endif


/*
*********************************************************************************************************
*	                                  �궨��
*********************************************************************************************************
*/
#define PORT_NUM       1001    /* TCP�����������˿ں� */


/*
*********************************************************************************************************
*	                                     ����
*********************************************************************************************************
*/
uint8_t socket_tcp;


/*
*********************************************************************************************************
*	�� �� ��: tcp_callback
*	����˵��: TCP Socket�Ļص�����
*	��    ��: soc  TCP Socket����
*             evt  �¼�����
*             ptr  �¼�������TCP_EVT_DATA��ptrָ��Ļ�������¼�Ž��յ���TCP���ݣ������¼���¼IP��ַ
*             par  �¼�������TCP_EVT_DATA����¼���յ������ݸ����������¼���¼�˿ں�
*	�� �� ֵ: 
*********************************************************************************************************
*/
U16 tcp_callback (U8 soc, U8 evt, U8 *ptr, U16 par)
{
	char buf[50];
	uint16_t i;
	
	/* ȷ����socket_tcp�Ļص� */
	if (soc != socket_tcp) 
	{
		return (0);
	}

	switch (evt) 
	{
		/*
			Զ�̿ͻ���������Ϣ
		    1������ptr�洢Զ���豸��IP��ַ��par�д洢�˿ںš�
		    2��������ֵ1�������ӣ�������ֵ0��ֹ���ӡ�
		*/
		case TCP_EVT_CONREQ:
			sprintf(buf, "Զ�̿ͻ�����������IP: %d.%d.%d.%d", ptr[0], ptr[1], ptr[2], ptr[3]);
			printf_debug("IP:%s  port:%d\r\n", buf, par);
			return (1);
		
		/* ������ֹ */
		case TCP_EVT_ABORT:
			break;
		
		/* SocketԶ�������Ѿ����� */
		case TCP_EVT_CONNECT:
			printf_debug("Socket is connected to remote peer\r\n");
			break;
		
		/* ���ӶϿ� */
		case TCP_EVT_CLOSE:
		   	printf_debug("Connection has been closed\r\n");
			break;
		
		/* ���͵������յ�Զ���豸Ӧ�� */
		case TCP_EVT_ACK:
			break;
		
		/* ���յ�TCP����֡��ptrָ�����ݵ�ַ��par��¼���ݳ��ȣ���λ�ֽ� */
		case TCP_EVT_DATA:
			printf_debug("Data length = %d\r\n", par);
			for(i = 0; i < par; i++)
			{
				printf_debug("ptr[%d] = %d\r\n", i, ptr[i]);
			}
			break;
	}
	
	return (0);
}

/*
*********************************************************************************************************
*	�� �� ��: TCP_StatusCheck
*	����˵��: ���TCP������״̬����Ҫ�������߲�ε��ж�
*	��    ��: ��
*	�� �� ֵ: __TRUE  ����
*             __FALSE �Ͽ�
*********************************************************************************************************
*/
uint8_t TCP_StatusCheck(void) 
{
	uint8_t res;
	
	switch (tcp_get_state(socket_tcp)) 
	{
		case TCP_STATE_FREE:
		case TCP_STATE_CLOSED:
			res = tcp_listen (socket_tcp, PORT_NUM);
			printf_debug("tcp listen res = %d\r\n", res);
			break;
		
		case TCP_STATE_LISTEN:
			break;
		
		case TCP_STATE_CONNECT:
			return (__TRUE);
			
		default:  
			break;
	}
	
	return (__FALSE);
}

/*
*********************************************************************************************************
*	�� �� ��: TCPnetTest
*	����˵��: TCPnetӦ��
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void TCPnetTest(void)
{  
	int32_t iCount;
	uint8_t *sendbuf;
	uint8_t tcp_status;
	uint16_t maxlen;
	uint8_t res;
	OS_RESULT xResult;
	const uint16_t usMaxBlockTime = 2; /* �ӳ����� */

	/* 
	   ����TCP Socket�������������ͻ������ӷ�������10����������ͨ�Ž��Ͽ����ӡ�
	   ������������ʹ����TCP_TYPE_KEEP_ALIVE����һֱ�������ӣ�����10���ʱ�����ơ�
	*/
    socket_tcp = tcp_get_socket (TCP_TYPE_SERVER|TCP_TYPE_KEEP_ALIVE, 0, 10, tcp_callback);
	if(socket_tcp != 0)
	{
		res = tcp_listen (socket_tcp, PORT_NUM);
		printf_debug("tcp listen res = %d\r\n", res);
	}
	
	while (1) 
	{
		/* RL-TCPnet�������� */
		main_TcpNet();
		
		/* �������߲�εĴ��� */
		tcp_status = TCP_StatusCheck();
		
		/* ������Ϣ�Ĵ��� */
		if((os_evt_wait_or(0xFFFF, usMaxBlockTime) == OS_R_EVT)&&(tcp_status == __TRUE))
		{
			xResult = os_evt_get ();
			switch (xResult)
			{
				/* ���յ�K1�����£���Զ��TCP�ͻ��˷���8�ֽ����� */
				case KEY1_BIT0:			  
					printf_debug("tcp_get_state(socket_tcp) = %d\r\n", tcp_get_state(socket_tcp));
					iCount = 8;
					do
					{
						main_TcpNet();
						if (tcp_check_send (socket_tcp) == __TRUE) 
						{
							maxlen = tcp_max_dsize (socket_tcp);
							iCount -= maxlen;
							
							if(iCount < 0)
							{
								/* ��ô����û����� */
								maxlen = iCount + maxlen;
							}
							
							sendbuf = tcp_get_buf(maxlen);
							sendbuf[0] = '1';
							sendbuf[1] = '2';
							sendbuf[2] = '3';
							sendbuf[3] = '4';
							sendbuf[4] = '5';
							sendbuf[5] = '6';
							sendbuf[6] = '7';
							sendbuf[7] = '8';
							
							/* ���Է���ֻ��ʹ�û�ȡ���ڴ� */
							tcp_send (socket_tcp, sendbuf, maxlen);
						}
						
					}while(iCount > 0);
					break;

				/* ���յ�K2�����£���Զ��TCP�ͻ��˷���1024�ֽڵ����� */
				case KEY2_BIT1:		
					printf_debug("tcp_get_state(socket_tcp) = %d\r\n", tcp_get_state(socket_tcp));
					iCount = 1024;
					do
					{
						main_TcpNet();
						if (tcp_check_send (socket_tcp) == __TRUE) 
						{
							maxlen = tcp_max_dsize (socket_tcp);
							iCount -= maxlen;
							
							if(iCount < 0)
							{
								/* ��ô����û����� */
								maxlen = iCount + maxlen;
							}
							
							/* �������ʼ����ÿ�����������ݰ���ǰ8���ֽ� */
							sendbuf = tcp_get_buf(maxlen);
							sendbuf[0] = 'a';
							sendbuf[1] = 'b';
							sendbuf[2] = 'c';
							sendbuf[3] = 'd';
							sendbuf[4] = 'e';
							sendbuf[5] = 'f';
							sendbuf[6] = 'g';
							sendbuf[7] = 'h';
							
							/* ���Է���ֻ��ʹ�û�ȡ���ڴ� */
							tcp_send (socket_tcp, sendbuf, maxlen);
						}
						
					}while(iCount > 0);					
					break;
					
				/* ���յ�K3�����£���Զ��TCP�ͻ��˷���5MB���� */
				case KEY3_BIT2:			  
					printf_debug("tcp_get_state(socket_tcp) = %d\r\n", tcp_get_state(socket_tcp));
					iCount = 5*1024*1024;
					do
					{
						main_TcpNet();
						if (tcp_check_send (socket_tcp) == __TRUE) 
						{
							maxlen = tcp_max_dsize (socket_tcp);
							iCount -= maxlen;
							
							if(iCount < 0)
							{
								/* ��ô����û����� */
								maxlen = iCount + maxlen;
							}
							
							/* �������ʼ����ÿ�����������ݰ���ǰ8���ֽ� */
							sendbuf = tcp_get_buf(maxlen);
							sendbuf[0] = 'a';
							sendbuf[1] = 'b';
							sendbuf[2] = 'c';
							sendbuf[3] = 'd';
							sendbuf[4] = 'e';
							sendbuf[5] = 'f';
							sendbuf[6] = 'g';
							sendbuf[7] = 'h';
							
							/* ���Է���ֻ��ʹ�û�ȡ���ڴ� */
							tcp_send (socket_tcp, sendbuf, maxlen);
						}
						
					}while(iCount > 0);
					break;
				
				 /* �����ļ�ֵ������ */
				default:                     
					break;
			}
		}
	}
}

/***************************** ���������� www.armfly.com (END OF FILE) *********************************/