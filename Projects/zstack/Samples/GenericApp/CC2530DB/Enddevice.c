/*>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
* 文件名  ： Enddevice
* 作者    ： zhengwei
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
/* 结构体或枚举 ----------------------------------------------------------*/
/* 内部函数声明-----------------------------------------------------------*/

/* 函数 ------------------------------------------------------------------*/


const cId_t GenericApp_ClusterList[GENERICAPP_MAX_CLUSTERS] = 
{
    GENERICAPP_CLUSTERID
}; //GENERICAPP_MAX_CLUSTERS是在Coordinator.h文件中定义的宏

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
    (cId_t *)GenericApp_ClusterList,
};//描述一个zigbee设备节点

endPointDesc_t GenericApp_epDesc; //节点描述符
byte GenericApp_TaskID;           //任务优先级
byte GenericApp_TransID;          //数据发送序列号
devStates_t GenericApp_NwkState;  //保存节点状态的变量(记录设备状态)

void GenericApp_MessageMSGCB( afIncomingMSGPacket_t *pckt );//消息处理函数
void GenericApp_SendTheMessage( void ); //数据发送函数
void GenericApp_Init( byte task_id )    //任务初始化函数
{
    GenericApp_TaskID            = task_id;
    GenericApp_NwkState          = DEV_INIT;
    GenericApp_TransID           = 0;
    GenericApp_epDesc.endPoint   = GENERICAPP_ENDPOINT;
    GenericApp_epDesc.task_id    = &GenericApp_TaskID;
    GenericApp_epDesc.simpleDesc = 
        (SimpleDescriptionFormat_t *)&GenericApp_SimpleDesc;
    GenericApp_epDesc.latencyReq = noLatencyReqs;
    afRegister( &GenericApp_epDesc );
}

UINT16 GenericApp_ProcessEvent( byte task_id, UINT16 events )
{
    afIncomingMSGPacket_t *MSGpkt;
    if ( events & SYS_EVENT_MSG )
    {
        MSGpkt = (afIncomingMSGPacket_t *)osal_msg_receive(GenericApp_TaskID );
        while ( MSGpkt )
        {
            switch( MSGpkt->hdr.event )
            {
            case ZDO_STATE_CHANGE:
                GenericApp_NwkState = (devStates_t)(MSGpkt->hdr.status);
                if (GenericApp_NwkState == DEV_END_DEVICE)
                {
                    GenericApp_SendTheMessage();
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
    return 0;
} //消息处理函数

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
* 函数名  ： GenericApp_SendTheMessage
* 参数    ： void
* 返回    ： void
* 作者    ： zhengwei
* 时间    ： 2021/5/18
* 描述    ： 数据发送
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void GenericApp_SendTheMessage( void )
{
    unsigned char theMessageData[4] = "LED";
    afAddrType_t my_DstAddr;
    my_DstAddr.addrMode = (afAddrMode_t)Addr16Bit;
    my_DstAddr.endPoint = GENERICAPP_ENDPOINT;
    my_DstAddr.addr.shortAddr = 0x0000;
    AF_DataRequest( &my_DstAddr, &GenericApp_epDesc,
                   GENERICAPP_CLUSTERID,
                   3,
                   theMessageData,
                   &GenericApp_TransID,
                   AF_DISCV_ROUTE,
                   AF_DEFAULT_RADIUS );
    HalLedBlink(HAL_LED_2,0,50,500);
} //实现数据发送