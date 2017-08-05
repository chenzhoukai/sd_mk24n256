#include "stdio.h"

/* FreeRTOS kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"

/* Freescale includes. */
#include "fsl_device_registers.h"
#include "fsl_debug_console.h"
#include "board.h"

#include "pin_mux.h"
#include "clock_config.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/

/* Task priorities. */
#define hello_task_PRIORITY (configMAX_PRIORITIES - 1)
/*******************************************************************************
 * Prototypes
 ******************************************************************************/
static void hello_task(void *pvParameters);

/*******************************************************************************
 * Code
 ******************************************************************************/
/*!
 * @brief Application entry point.
 */
//int main(void)
//{
//    /* Init board hardware. */
//    BOARD_InitPins();
//    BOARD_BootClockRUN();
//    BOARD_InitDebugConsole();
//    xTaskCreate(hello_task, "Hello_task", configMINIMAL_STACK_SIZE + 10, NULL, hello_task_PRIORITY, NULL);
//    vTaskStartScheduler();
//    for (;;)
//        ;
//}

void us_delay(uint32_t usec)
{
    volatile uint32_t i = 0;
    for (i = 0; i < 10*usec; ++i)
    {
        __asm("NOP"); /* delay */
    }
}

static uint8_t COM(void)
{
    uint8_t     i;
    uint8_t     U8temp, U8comdata;
    uint8_t     to_flag = 0;

    for(i=0;i<8;i++)
    {
        to_flag = 0;
        while(!DHT11_1W_DI())
        {
            us_delay(1);
            if (++to_flag > 50) break;
        }

        us_delay(30);

        U8temp=0;
        DHT11_1W_TO(LOGIC_1W_H);
        if(DHT11_1W_DI())U8temp=1;
        DHT11_1W_TO(LOGIC_1W_L);

        to_flag = 0;
        while(DHT11_1W_DI())
        {
            us_delay(1);
            if (++to_flag > 50) break;
        }

        //----------------------
        //��ʱ������forѭ��
        if(to_flag >= 50)break;
        //�ж�����λ��0����1
        // ����ߵ�ƽ�߹�Ԥ��0�ߵ�ƽֵ������λΪ 1
        U8comdata<<=1;
        U8comdata|=U8temp;
    }

    return U8comdata;
}

bool RH(char *rh_data, char *t_data)
{
//    uint8_t U8T_data_H, U8T_data_L, U8RH_data_H,U8RH_data_L, U8checkdata;
    uint16_t U16T_data_A, U16RH_data_A;
    uint8_t U8RH_data_H_temp, U8RH_data_L_temp, U8T_data_H_temp, U8T_data_L_temp, U8checkdata_temp;
    uint8_t U8temp;

    //��������20ms
    DHT11_1W_DO(LOGIC_1W_L);
    //vTaskDelay(20);
    vTaskDelay(2);
    DHT11_1W_DO(LOGIC_1W_H);

    //������������������ ������ʱ20us
    DHT11_1W_TO(LOGIC_1W_H);
    //us_delay(20);
    us_delay(50);
    DHT11_1W_TO(LOGIC_1W_L);
    //������Ϊ���� �жϴӻ���Ӧ�ź�

    //�жϴӻ��Ƿ��е͵�ƽ��Ӧ�ź� �粻��Ӧ����������Ӧ����������
    if(!DHT11_1W_DI()) //T !
    {
        //�жϴӻ��Ƿ񷢳� 80us �ĵ͵�ƽ��Ӧ�ź��Ƿ����
        DHT11_1W_TO(LOGIC_1W_H);
        while(!DHT11_1W_DI())
        {
            us_delay(1);
        }
        DHT11_1W_TO(LOGIC_1W_L);

        //�жϴӻ��Ƿ񷢳� 80us �ĸߵ�ƽ���緢����������ݽ���״̬
        DHT11_1W_TO(LOGIC_1W_H);
        while(DHT11_1W_DI())
        {
            us_delay(1);
        }
        DHT11_1W_TO(LOGIC_1W_L);
        //���ݽ���״̬

        U8RH_data_H_temp=COM();

        U8RH_data_L_temp=COM();

        U8T_data_H_temp=COM();

        U8T_data_L_temp=COM();

        U8checkdata_temp=COM();

        //����У��
        U8temp=(U8T_data_H_temp+U8T_data_L_temp+U8RH_data_H_temp+U8RH_data_L_temp);
        if(U8temp==U8checkdata_temp) {
//            U8RH_data_H=U8RH_data_H_temp;
//            U8RH_data_L=U8RH_data_L_temp;
//            U8T_data_H=U8T_data_H_temp;
//            U8T_data_L=U8T_data_L_temp;
//            U8checkdata=U8checkdata_temp;
//            sprintf(rh_data, "%d.%dRH", U8RH_data_H, U8RH_data_L);
//            if ((U8T_data_L >> 7) && 0x01)
//                sprintf(t_data, "-%d.%dT", U8T_data_H, U8T_data_L&0x7F);
//            else
//                sprintf(t_data, "%d.%dT", U8T_data_H, U8T_data_L&0x7F);
//            PRINTF("%x %x %x %x %x\r\n", U8RH_data_H, U8RH_data_L, U8T_data_H, U8T_data_L, U8checkdata);

            U16RH_data_A = (U8RH_data_H_temp<<8)|U8RH_data_L_temp;
            U16T_data_A  = (U8T_data_H_temp<<8)|U8T_data_L_temp;
            sprintf(rh_data, "%d.%dRH", U16RH_data_A/10, U16RH_data_A%10);
            if ((U16T_data_A >> 15) && 0x0001)
                sprintf(t_data, "-%d.%dT", (U16T_data_A&0x7FFF)/10, (U16T_data_A&0x7FFF)%10);
            else
                sprintf(t_data, "%d.%dT", U16T_data_A/10, U16T_data_A%10);

            //PRINTF("%x %x %x %x %x\r\n", U8RH_data_H_temp, U8RH_data_L_temp, U8T_data_H_temp, U8T_data_L_temp, U8checkdata_temp);
        }
        else
            return false;
    }
    return true;
}

/*!
 * @brief Task responsible for printing of "Hello world." message.
 */
//static void hello_task(void *pvParameters)
//{
//    char    rh_data[8], t_data[8];
//    for (;;)
//    {
//        vTaskDelay(3000);
//        RH(rh_data, t_data);
//        PRINTF("%s %s\r\n", rh_data, t_data);
//    }
//}
