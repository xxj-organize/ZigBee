/*>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
* 文件名  ： Coordinator
* 作者    ： linhongpeng
* 版本    ： V0.0.1
* 时间    ： 2021/5/18
* 描述    ： 协调器文件
********************************************************************
* 副本
*
*>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>*/
/* 头文件 ----------------------------------------------------------------*/
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
/* 宏定义 ----------------------------------------------------------------*/
/* 结构体或枚举 ----------------------------------------------------------*/
/* 内部函数声明-----------------------------------------------------------*/

/* 函数 ------------------------------------------------------------------*/


//跟协议栈里面数据的定义格式保持一致
const cId_t GenericApp_ClusterList[GENERICAPP_MAX_CLUSTERS] = 
{
    GENERICAPP_CLUSTERID
};

//用来描述一个ZigBee结点
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


endPointDesc_t GenericApp_epDesc;  /*节点描述符GenericApp_epDesc，
                                     在ZigBee协议栈中新定义的类型一般以_t结尾*/
byte GenericApp_TaskID;            //任务优先级GenericApp_TaskID
byte GenericApp_TransID;           //数据发送序列号GenericApp_TransID
unsigned char uartbuf[128];        //定义uartbuf

void GenericApp_MessageMSGCB ( afIncomingMSGPacket_t *pckt );  //消息处理函数
void GenericApp_SendTheMessage ( void );                   //数据发送函数
static void rxCB( uint8 port,uint8 event );                //回调函数

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
* 函数名  ： GenericApp_Init
* 参数    ： byte task_id
* 返回    ： void
* 作者    ： linhongpeng
* 时间    ： 2021/5/18
* 描述    ： 任务初始化函数
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void GenericApp_Init( byte task_id )
{
    halUARTCfg_t uartConfig;
    GenericApp_TaskID               = task_id;  //初始化任务优先级
    GenericApp_TransID              = 0;       //发送数据包序号初始化为0
    GenericApp_epDesc.endPoint      = GENERICAPP_ENDPOINT;
    GenericApp_epDesc.task_id       = &GenericApp_TaskID;
    GenericApp_epDesc.simpleDesc    = 
        (SimpleDescriptionFormat_t *)&GenericApp_SimpleDesc;
    GenericApp_epDesc.latencyReq    = noLatencyReqs;
    afRegister ( &GenericApp_epDesc );  //使用afRegister函数将节点描述符进行注册
    uartConfig.configured           = TRUE;
    uartConfig.baudRate             = HAL_UART_BR_115200;  //波特率
    uartConfig.callBackFunc         = rxCB;
    HalUARTOpen (0, &uartConfig);      //对串口初始化
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
* 函数名  ： rxCB
* 参数    ： uint8 port,uint8 event
* 返回    ： void
* 作者    ： linhongpeng
* 时间    ： 2021/5/18
* 描述    ： 回调函数
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
* 函数名  ： GenericApp_ProcessEvent
* 参数    ： byte task_id, UINT16 events
* 返回    ： UINT16
* 作者    ： linhongpeng
* 时间    ： 2021/5/18
* 描述    ： 消息处理函数
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
UINT16 GenericApp_ProcessEvent ( byte task_id, UINT16 events )
{
    afIncomingMSGPacket_t *MSGpkt;       //定义一个指向接收消息结构体的指针
    if ( events & SYS_EVENT_MSG )
    {
        MSGpkt = (afIncomingMSGPacket_t *)osal_msg_receive(GenericApp_TaskID );
        while( MSGpkt )
        {
            switch ( MSGpkt->hdr.event )
            {
            case AF_INCOMING_MSG_CMD:
                GenericApp_MessageMSGCB( MSGpkt );  //唯一需要修改的部分
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
        osal_memcpy(buffer,pkt->cmd.Data,3);  //将收到的数据拷贝到缓冲区buffer中
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