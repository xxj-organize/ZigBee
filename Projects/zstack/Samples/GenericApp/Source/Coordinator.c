/*>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
* �ļ���  �� Coordinator
* ����    �� linhongpeng
* �汾    �� V0.0.1
* ʱ��    �� 2021/5/18
* ����    �� Э�����ļ�
********************************************************************
* ����
*
*>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>*/
/* ͷ�ļ� ----------------------------------------------------------------*/
#include "OSAL.h"
#include "AF.h"
#include "ZDApp.h"
#include "ZDObject.h"
#include "ZDProfile.h"
#include <string.h>
#include "Coordinator.h"
#include "DebugTrace.h"
#if !defined( WIN32 )
#include "OnBoard.h"
#endif
#include "hal_lcd.h"
#include "hal_led.h"
#include "hal_key.h"
#include "hal_uart.h"
/* �궨�� ----------------------------------------------------------------*/
/* �ṹ���ö�� ----------------------------------------------------------*/
/* �ڲ���������-----------------------------------------------------------*/

/* ���� ------------------------------------------------------------------*/


//��Э��ջ�������ݵĶ����ʽ����һ��
const cId_t GenericApp_ClusterList[GENERICAPP_MAX_CLUSTERS] = 
{
    GENERICAPP_CLUSTERID
};

//��������һ��ZigBee���
const SimpleDescriptionFormat_t GenericApp_SimpleDesc =
{
    GENERICAPP_ENDPOINT,
    GENERICAPP_PROFID,
    GENERICAPP_DEVICEID,
    GENERICAPP_DEVICE_VERSION,
    GENERICAPP_FLAGS,
    GENERICAPP_MAX_CLUSTERS,
    (cId_t *)GenericApp_ClusterList,
    0,
    (cId_t *)NULL
};


endPointDesc_t GenericApp_epDesc;  /*�ڵ�������GenericApp_epDesc��
                                     ��ZigBeeЭ��ջ���¶��������һ����_t��β*/
byte GenericApp_TaskID;            //�������ȼ�GenericApp_TaskID
byte GenericApp_TransID;           //���ݷ������к�GenericApp_TransID
unsigned char uartbuf[128];        //����uartbuf

void GenericApp_MessageMSGCB ( afIncomingMSGPacket_t *pckt );  //��Ϣ������
void GenericApp_SendTheMessage ( void );                   //���ݷ��ͺ���
static void rxCB( uint8 port,uint8 event );                //�ص�����

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
* ������  �� GenericApp_Init
* ����    �� byte task_id
* ����    �� void
* ����    �� linhongpeng
* ʱ��    �� 2021/5/18
* ����    �� �����ʼ������
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void GenericApp_Init( byte task_id )
{
    halUARTCfg_t uartConfig;
    GenericApp_TaskID               = task_id;  //��ʼ���������ȼ�
    GenericApp_TransID              = 0;       //�������ݰ���ų�ʼ��Ϊ0
    GenericApp_epDesc.endPoint      = GENERICAPP_ENDPOINT;
    GenericApp_epDesc.task_id       = &GenericApp_TaskID;
    GenericApp_epDesc.simpleDesc    = 
        (SimpleDescriptionFormat_t *)&GenericApp_SimpleDesc;
    GenericApp_epDesc.latencyReq    = noLatencyReqs;
    afRegister ( &GenericApp_epDesc );  //ʹ��afRegister�������ڵ�����������ע��
    uartConfig.configured           = TRUE;
    uartConfig.baudRate             = HAL_UART_BR_115200;  //������
    uartConfig.callBackFunc         = rxCB;
    HalUARTOpen (0, &uartConfig);      //�Դ��ڳ�ʼ��
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
* ������  �� rxCB
* ����    �� uint8 port,uint8 event
* ����    �� void
* ����    �� linhongpeng
* ʱ��    �� 2021/5/18
* ����    �� �ص�����
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
static void rxCB(uint8 port,uint8 event)
{
    HalUARTRead(0,uartbuf,16);
    if(osal_memcmp(uartbuf,"www.wlwmaker.com",16))
    {
        HalUARTWrite(0,uartbuf,16);
    }
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
* ������  �� GenericApp_ProcessEvent
* ����    �� byte task_id, UINT16 events
* ����    �� UINT16
* ����    �� linhongpeng
* ʱ��    �� 2021/5/18
* ����    �� ��Ϣ������
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
UINT16 GenericApp_ProcessEvent ( byte task_id, UINT16 events )
{
    afIncomingMSGPacket_t *MSGpkt;       //����һ��ָ�������Ϣ�ṹ���ָ��
    if ( events & SYS_EVENT_MSG )
    {
        MSGpkt = (afIncomingMSGPacket_t *)osal_msg_receive(GenericApp_TaskID );
        while( MSGpkt )
        {
            switch ( MSGpkt->hdr.event )
            {
            case AF_INCOMING_MSG_CMD:
                GenericApp_MessageMSGCB( MSGpkt );  //Ψһ��Ҫ�޸ĵĲ���
                break;
            default:
                break;
            }
            osal_msg_deallocate( (uint8 *)MSGpkt );
            MSGpkt = (afIncomingMSGPacket_t *)osal_msg_receive
                ( GenericApp_TaskID );
        }
        return (events ^ SYS_EVENT_MSG);
    }
    return 0;
}




void GenericApp_MessageMSGCB( afIncomingMSGPacket_t *pkt )
{
    unsigned char buffer[4] = "   ";
    switch ( pkt->clusterId )
    {
    case GENERICAPP_CLUSTERID:
        osal_memcpy(buffer,pkt->cmd.Data,3);  //���յ������ݿ�����������buffer��
        if((buffer[0] == 'L') || (buffer[1] == 'E') || (buffer[2] == 'D'))
        {
            HalLedBlink(HAL_LED_2,0,50,500);
        }
        else
        {
            HalLedSet(HAL_LED_2,HAL_LED_MODE_ON);
        }
        break;
    }
}