/*>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
* 文件名  ： Enddevice
* 作者    ： ZigBee
* 版本    ： V0.0.1
* 时间    ： 2021/5/18
* 描述    ： 终端
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
#define SEND_DATA_EVENT 0x01
/* 结构体或枚举 ----------------------------------------------------------*/
/* 内部函数声明-----------------------------------------------------------*/

/* 函数 ------------------------------------------------------------------*/

const cId_t GenericApp_ClusterList[GENERICAPP_MAX_CLUSTERS] = 
{
    GENERICAPP_CLUSTERID
};
const SimpleDescriptionFormat_t GenericApp_SimpleDesc =
{
    GENERICAPP_ENDPOINT,
    GENERICAPP_PROFID,
    GENERICAPP_DEVICEID,
    GENERICAPP_DEVICE_VERSION,
    GENERICAPP_FLAGS,
    0,
    (cId_t *)NULL,
    GENERICAPP_MAX_CLUSTERS,
    (cId_t *)GenericApp_ClusterList
};

endPointDesc_t GenericApp_epDesc;
byte GenericApp_TaskID;
byte GenericApp_TransID;
devStates_t GenericApp_NwkState;  //保存节点状态变量GenericApp_NwkState

void GenericApp_MessageMSGCB ( afIncomingMSGPacket_t *pckt );   //消息处理函数
void GenericApp_SendTheMessage ( void );     //数据发送函数

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
* 函数名  ： GenericApp_Init
* 参数    ： byte task_id
* 返回    ： void
* 作者    ： ZigBee
* 时间    ： 2021/5/18
* 描述    ： 任务初始化函数
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void GenericApp_Init( byte task_id )
{
    GenericApp_TaskID               = task_id;
    GenericApp_NwkState             = DEV_INIT;   //设备状态初始化
    GenericApp_TransID              = 0;
    GenericApp_epDesc.endPoint      = GENERICAPP_ENDPOINT;
    GenericApp_epDesc.task_id       = &GenericApp_TaskID;
    GenericApp_epDesc.simpleDesc    = 
        (SimpleDescriptionFormat_t *)&GenericApp_SimpleDesc;
    GenericApp_epDesc.latencyReq    = noLatencyReqs;
    afRegister ( &GenericApp_epDesc );
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
* 函数名  ： GenericApp_ProcessEvent
* 参数    ： byte task_id, UINT16 events
* 返回    ： UINT16
* 作者    ： ZigBee
* 时间    ： 2021/5/18
* 描述    ： 消息处理函数
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
UINT16 GenericApp_ProcessEvent ( byte task_id, UINT16 events )
{
    afIncomingMSGPacket_t *MSGpkt;
    if ( events & SYS_EVENT_MSG )
    {
        MSGpkt = (afIncomingMSGPacket_t *)osal_msg_receive(GenericApp_TaskID );
        while( MSGpkt )
        {
            switch ( MSGpkt->hdr.event )
            {
            case ZDO_STATE_CHANGE:
                GenericApp_NwkState = (devStates_t)(MSGpkt ->hdr.status);  //读取节点的设备类型
                if (GenericApp_NwkState == DEV_END_DEVICE)  //判断是否为终端节点
                {
                    osal_set_event(GenericApp_TaskID,SEND_DATA_EVENT);
                }
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
    if (events & SEND_DATA_EVENT)
    {
        GenericApp_SendTheMessage();
        osal_start_timerEx(GenericApp_TaskID,SEND_DATA_EVENT,1000);
        return (events ^ SEND_DATA_EVENT); //清除事件标志
    }
    return 0;
}

void GenericApp_SendTheMessage( void )
{
    unsigned char theMessageData[4] = "LED";
    afAddrType_t my_DstAddr;
    my_DstAddr.addrMode = (afAddrMode_t)Addr16Bit;  //将发送地址模式设置为单播
    my_DstAddr.endPoint = GENERICAPP_ENDPOINT;      //初始化目的端口号
    my_DstAddr.addr.shortAddr = 0x0000;       //协调器网络地址固定为0x0000
    AF_DataRequest( &my_DstAddr, &GenericApp_epDesc,
                   GENERICAPP_CLUSTERID,
                   3,
                   theMessageData,  //用于存放要发送的数据
                   &GenericApp_TransID,
                   AF_DISCV_ROUTE,
                   AF_DEFAULT_RADIUS );    //调用数据发送函数AF_DataRequest
    HalLedBlink(HAL_LED_2,0,50,500);
}