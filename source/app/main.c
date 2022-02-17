#include <includes.h>
#include <bsp_adc.h>
#include <bsp_boardID.h>
#include <bsp_wave_detect.h>
#include <app_wave_task.h>
#include <app.h>
#include <bsp.h>
#include <bsp_dac.h>
#include <bsp_flash.h>
#include <bsp_wdt.h>



/*******************************************************************************
* Description  :  指示灯任务，无速度信号慢闪，有速度信号快闪，定期调用（100ms）
* Author       : 2018/4/13 星期五, by redmorningcn
*******************************************************************************/
void    led_task(void)
{
    static  uint8   blinkcnt= 0;
    
    blinkcnt++;
    
//    if(Ctrl.ch.para[0].period == 0 && Ctrl.ch.para[1].period  == 0)   //闪快慢控制   
//    {
        blinkcnt %= 20;                                     //所有通道无信号，慢闪  
//    }
//    else
//    {
//        blinkcnt %= 2;                                      //任意通道有信号，快闪
//    }
    
    if(blinkcnt == 0)                                       //亮、灭灯
    {
        BSP_LED_On(1);
        BSP_LED_On(2);

    }
    else
    {
        BSP_LED_Off(1);
        BSP_LED_Off(2);

    }
}

//参考电压，单位mv
#define     FULL_VOLTAGE            (3300)          /*  工作电压            */

#define     MAX_HIG_VOLTAGE         (3000)          /*  最大高电平              */
#define     MAX_STANDARD_VOLTAGE    (1500)          /*  默认参考电压大值        */  
#define     MAX_STANDARD_ZONE       (300)           /*  参考电压大值范围        */
#define     MIN_STANDARD_VOLTAGE    (750)           /*  默认参考电压小值        */
#define     MIN_STANDARD_ZONE       (150)           /*  默认参考电压小值范围    */ 

#define     REF_VOLTAGE_RATIO       (0.75)          /* 参考电压和高电平系数关系 */


/**************************************************************
* Description  : 初始化系统参数
* Author       : 2018/5/30 星期三, by redmorningcn
*/
void    InitCtrl(void)
{
    /**************************************************************
    * Description  : 读取校准值
    * Author       : 2018/5/30 星期三, by redmorningcn
    */
//    BSP_FlashReadBytes(STORE_ADDR_CALI,
//                       (u8 *)&Ctrl.calitab,
//                       sizeof(Ctrl.calitab));
//    
//    for(u8  i = 0;i <sizeof(Ctrl.calitab)/sizeof(strCalibration);i++){
//        //修正线性度检查
//        if(     Ctrl.calitab.CaliBuf[i].line < CALI_LINE_MIN 
//            ||  Ctrl.calitab.CaliBuf[i].line > CALI_LINE_MAX
//            ){
//                Ctrl.calitab.CaliBuf[i].line   = CALI_LINE_BASE; //线性度超限，置默认值
//                Ctrl.calitab.CaliBuf[i].Delta  = CALI_DELTA_BASE;//修正偏差超限，置默认值
//
//            }
//        
//        //修正偏差检查
//        if(     Ctrl.calitab.CaliBuf[i].Delta < CALI_DELTA_MIN
//            ||  Ctrl.calitab.CaliBuf[i].Delta > CALI_DELTA_MAX
//            ){
//                Ctrl.calitab.CaliBuf[i].Delta = CALI_DELTA_BASE;//修正偏差超限，置默认值
//            }
//    }
    
    /**************************************************************
    * Description  : 读运行参数
    * Author       : 2018/7/19 星期四, by redmorningcn
    */
    BSP_FlashReadBytes(STORE_ADDR_SYS,
                       (u8 *)&Ctrl.sys,
                       sizeof(Ctrl.sys));
    
    Ctrl.sys.cpu_freq = BSP_CPU_ClkFreq();      //时钟频率               /* Determine SysTick reference freq.              */

    Ctrl.sys.paraflg.sysflg    = 0;             //不存系统参数（sys）
    Ctrl.sys.paraflg.califlg   = 0;             //不存修正参数（cali）
    
    Ctrl.sys.setfrq     = 1000;                     //默认频率1000Hz
    Ctrl.sys.setphase   = 90;                       //默认相位90°
    Ctrl.sys.LocoDim    = 1050;                     //默认相1050轮径°
    Ctrl.sys.PluseNum   = 200;                      //默认脉冲常数200

    uint8 i = 0;
    while (i < 8){
      Ctrl.sys.setCH[i++] = 50;                 //默认占空比50%
    }

    Ctrl.sys.id = 1;
}

/**************************************************************
* Description  : 保存参数
* Author       : 2018/5/30 星期三, by redmorningcn
*/
void    store_para(void)
{
//    if(Ctrl.sys.paraflg.califlg == 1){
//        Ctrl.sys.paraflg.califlg = 0; 
//        
//        BSP_FlashWriteBytes(    STORE_ADDR_CALI,
//                                (u8 *)&Ctrl.calitab,
//                                sizeof(Ctrl.calitab) );
//    }
//    
  //210414 增加脉冲数调试，将周期改为脉冲
    //Ctrl.sys.frqCnt[0]= Ctrl.ch.para[0].period;
    //Ctrl.sys.frqCnt[1]= Ctrl.ch.para[1].period;    
    
    if(Ctrl.sys.paraflg.sysflg == 1){
        Ctrl.sys.paraflg.sysflg = 0;
        
        BSP_FlashWriteBytes(    STORE_ADDR_SYS,
                                (u8 *)&Ctrl.sys,
                                sizeof(Ctrl.sys)    );
    }
}
        
    
/*******************************************************************************
* Description  : 闲置任务，时间不紧迫的工作在此运行
* Author       : 2018/4/16 星期一, by redmorningcn
*******************************************************************************/
void    idle_task(void)      
{
    static  uint32  tick;
    if(Ctrl.sys.time > tick+100 ||  Ctrl.sys.time < tick)   //100ms
    {
        tick = Ctrl.sys.time;                               //时间
        
        //led_task();                                         //指示灯控制
        
        store_para();                                       //保存参数
        
        TIM2_Change_Freq();
    }
}

extern  void mod_bus_rx_task(void);

void main (void)
{
    //BSP_WDT_Init(BSP_WDT_MODE_INT);                             // 初始化看门狗（防止上电不能启动 redmorningcn 20180719）

    BSP_Init();                                                 /* Initialize BSP functions                 */
    CPU_TS_TmrInit();
    /***********************************************
    * 描述： 初始化滴答定时器，即初始化系统节拍时钟。
    */
    Ctrl.sys.cpu_freq = BSP_CPU_ClkFreq();  //时钟频率          /* Determine SysTick reference freq.        */
    
    /*******************************************************************************
    * Description  : 信号幅值及工作电源电压检测初始化化
    * Author       : 2018/4/12 星期四, by redmorningcn
    *******************************************************************************/
    Bsp_ADC_Init();

    /*******************************************************************************
    * Description  : 速度通道时间参数检测初始化
    定时器捕获+全局定时器时间，记录波形产生的各时间点。
    * Author       : 2018/4/13 星期五, by redmorningcn
    *******************************************************************************/
    init_ch_timepara_detect();

    /**************************************************************
    * Description  : 读已存修正计算参数
    * Author       : 2018/5/29 星期二, by redmorningcn
    */
    InitCtrl();
    
    /*******************************************************************************
    * Description  : 串口通信初始化
    * Author       : 2018/5/7 星期一, by redmorningcn
    *******************************************************************************/
    MB_Init(1000);      //初始化modbus频率					

    Ctrl.pch         = MB_CfgCh( Ctrl.sys.id,        	// ... Modbus Node # for this slave channel
                        //MB_CfgCh( ModbusNode,        	// ... Modbus Node # for this slave channel
                        MODBUS_SLAVE,           // ... This is a MASTER
                        500,                    // ... 0 when a slave
                        MODBUS_MODE_RTU,        // ... Modbus Mode (_ASCII or _RTU)
                        0,                      // ... Specify UART #1
                        115200,                  // ... Baud Rate
                        USART_WordLength_8b,    // ... Number of data bits 7 or 8
                        USART_Parity_No,        // ... Parity: _NONE, _ODD or _EVEN
                        USART_StopBits_1,       // ... Number of stop bits 1 or 2
                        MODBUS_WR_EN);          // ... Enable (_EN) or disable (_DIS) writes

    
    //BSP_WDT_Init(BSP_WDT_MODE_INT);             // 初始化看门狗
                                  
    while(1)
    {        
        /*******************************************************************************
        * Description  : 空闲任务，时间不敏感的的工作在此运行，每100ms运行一次
        * Author       : 2018/4/16 星期一, by redmorningcn
        *******************************************************************************/
        idle_task();
        
        /*******************************************************************************
        * Description  : 计算速度通道的电压参数
        * Author       : 2018/4/13 星期五, by redmorningcn
        *******************************************************************************/
        app_calc_ch_voltagepara();
        
        /*******************************************************************************
        * Description  : 通讯任务
        * Author       : 2018/5/7 星期一, by redmorningcn
        *******************************************************************************/
        mod_bus_rx_task();
        
        
        /**************************************************************
        * Description  : 保存参数
        * Author       : 2018/7/18 星期三, by redmorningcn
        */
        //store_para();

        
        //BSP_WDT_Rst();
    }
}

