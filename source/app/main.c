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
    
    if(Ctrl.ch.para[0].period == 0 && Ctrl.ch.para[1].period  == 0)   //闪快慢控制   
    {
        blinkcnt %= 20;                                     //所有通道无信号，慢闪  
    }
    else
    {
        blinkcnt %= 2;                                      //任意通道有信号，快闪
    }
    
    if(blinkcnt == 0)                                       //亮、灭灯
    {
        BSP_LED_On(1);
    }
    else
    {
        BSP_LED_Off(1);
    }
}

//参考电压，单位mv
#define     FULL_VOLTAGE            (3300)
#define     STANDARD_VOLTAGE        (FULL_VOLTAGE/4)
#define     MAX_HIG_VOLTAGE         (3000)
#define     MAX_STANDARD_VOLTAGE    (1500)        
#define     REF_VOLTAGE_RATIO       (0.80)
/*******************************************************************************
* Description  : 设置参考电压，定期调用（100ms）
                参考电压为高电平的0.09，（取10%电压的0.9倍），高电平采样点在10%位置
                连续5次，参考电压差值偏差大于10%，重新调整参考电压。
* Author       : 2018/4/16 星期一, by redmorningcn
*******************************************************************************/
void    set_dac_task(void)
{
    static  uint32  sum;
    static  uint8   diffcnt = 0;    
    
    uint8   i;
    for(i = 0;i< 2;i++)
    if( Ctrl.ch.para[i].freq )                                           //通道0、或通道1有速度信号
    {
        if(fabs(Ctrl.ch.stand_vol -  Ctrl.ch.para[i].Voh * REF_VOLTAGE_RATIO) > (FULL_VOLTAGE/100)*2 )    //电压偏差小于10% ；
        {
            if(Ctrl.ch.para[i].Voh < MAX_HIG_VOLTAGE)                    //高电平最高3V
            {
                sum += Ctrl.ch.para[i].Voh;
                diffcnt++;
            }
            
            if( diffcnt > 10 )                                              //连续5次，采集的高电平电压和设置电压
            {
                Ctrl.ch.stand_vol =(u16)((sum / diffcnt)*REF_VOLTAGE_RATIO)/10;
                
                if( Ctrl.ch.stand_vol > MAX_STANDARD_VOLTAGE  )          //限定比较器的参考电压在1.5V和0.66V之间
                {
                    Ctrl.ch.stand_vol   = MAX_STANDARD_VOLTAGE;          //1.5v
                }
                else if(Ctrl.ch.stand_vol < STANDARD_VOLTAGE)
                {
                    Ctrl.ch.stand_vol  = STANDARD_VOLTAGE;               //0.8V
                }
                
                bsp_set_dacvalue(Ctrl.ch.stand_vol);                     //重新设置比较值
                
                return;
            }
        }
        else
        {
            sum     = 0;
            diffcnt = 0;
        }
    }else{
                                                              // (如果没有信号，按照默认值设置)
        if(i==1){
        
            if( Ctrl.ch.stand_vol > MAX_STANDARD_VOLTAGE  )                //限定比较器的参考电压在1.5V和0.66V之间
            {
                Ctrl.ch.stand_vol  = MAX_STANDARD_VOLTAGE;                 //1.5v
            }
            else if(Ctrl.ch.stand_vol < STANDARD_VOLTAGE)
            {
                Ctrl.ch.stand_vol  = STANDARD_VOLTAGE;                      //0.8V
            }
            
            bsp_set_dacvalue(Ctrl.ch.stand_vol);                           //重新设置比较值  
        }
    }
}

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
    BSP_FlashReadBytes(STORE_ADDR_CALI,
                       (u8 *)&Ctrl.calitab,
                       sizeof(Ctrl.calitab));
    
    for(u8  i = 0;i <sizeof(Ctrl.calitab)/sizeof(strCalibration);i++){
        //修正线性度检查
        if(     Ctrl.calitab.CaliBuf[i].line < CALI_LINE_MIN 
            ||  Ctrl.calitab.CaliBuf[i].line > CALI_LINE_MAX
            ){
                Ctrl.calitab.CaliBuf[i].line   = CALI_LINE_BASE; //线性度超限，置默认值
                Ctrl.calitab.CaliBuf[i].Delta  = CALI_DELTA_BASE;//修正偏差超限，置默认值

            }
        
        //修正偏差检查
        if(     Ctrl.calitab.CaliBuf[i].Delta < CALI_DELTA_MIN
            ||  Ctrl.calitab.CaliBuf[i].Delta > CALI_DELTA_MAX
            ){
                Ctrl.calitab.CaliBuf[i].Delta = CALI_DELTA_BASE;//修正偏差超限，置默认值
            }
    }
    
    /**************************************************************
    * Description  : 读运行参数
    * Author       : 2018/7/19 星期四, by redmorningcn
    */
    BSP_FlashReadBytes(STORE_ADDR_SYS,
                       (u8 *)&Ctrl.sys,
                       sizeof(Ctrl.sys));
    
    Ctrl.sys.id = get_boardID();
	Ctrl.sys.cpu_freq = BSP_CPU_ClkFreq();  //时钟频率               /* Determine SysTick reference freq.              */

    Ctrl.sys.paraflg.sysflg    = 0;        //不存系统参数（sys）
    Ctrl.sys.paraflg.califlg   = 0;        //不存修正参数（cali）

    if(Ctrl.sys.periodcali > 3*100 || Ctrl.sys.periodcali < 1*100)    //丢秒冲倍数，默认2倍信号周期 （*10）
        Ctrl.sys.periodcali = 2*100;
    
    if(Ctrl.sys.loseerrtimes > 20 || Ctrl.sys.loseerrtimes < 2)          //丢脉冲故障判断次数
        Ctrl.sys.loseerrtimes = 4;
    
}

/**************************************************************
* Description  : 保存参数
* Author       : 2018/5/30 星期三, by redmorningcn
*/
void    store_para(void)
{
    if(Ctrl.sys.paraflg.califlg == 1){
        Ctrl.sys.paraflg.califlg = 0; 
        
        BSP_FlashWriteBytes(STORE_ADDR_CALI,
                        (u8 *)&Ctrl.calitab,
                        sizeof(Ctrl.calitab));
    }
    
    if(Ctrl.sys.paraflg.sysflg == 1){
        Ctrl.sys.paraflg.sysflg = 0;
        
        BSP_FlashWriteBytes(STORE_ADDR_SYS,
                            (u8 *)&Ctrl.sys,
                            sizeof(Ctrl.sys));
    }
}
        
    
/*******************************************************************************
* Description  : 闲置任务，时间不紧迫的工作在此运行
* Author       : 2018/4/16 星期一, by redmorningcn
*******************************************************************************/
void    idle_task(void)      
{
    static  uint32  tick;
    if(Ctrl.sys.time > tick+100 ||  Ctrl.sys.time < tick) //100ms
    {
        tick = Ctrl.sys.time;                  //时间
        
        led_task();                             //指示灯控制
        
        set_dac_task();                         //设置参考电压
        
        store_para();                           //保存参数
    }
}

extern  void mod_bus_rx_task(void);

void main (void)
{
    BSP_WDT_Init(BSP_WDT_MODE_INT);                             // 初始化看门狗（防止上电不能启动 redmorningcn 20180719）

	BSP_Init();                                                 /* Initialize BSP functions                             */
	CPU_TS_TmrInit();
	/***********************************************
	* 描述： 初始化滴答定时器，即初始化系统节拍时钟。
	*/
	Ctrl.sys.cpu_freq = BSP_CPU_ClkFreq();  //时钟频率               /* Determine SysTick reference freq.              */
    
    /*******************************************************************************
    * Description  : 信号幅值及工作电源电压检测初始化化
    * Author       : 2018/4/12 星期四, by redmorningcn
    *******************************************************************************/
	Bsp_ADC_Init();
    
    /*******************************************************************************
    * Description  : 设备ID号获取初始化
    * Author       : 2018/4/13 星期五, by redmorningcn
    *******************************************************************************/
    Init_boardID();
    Ctrl.sys.id = get_boardID();
    
    /*******************************************************************************
    * Description  : 速度通道时间参数检测初始化
    定时器捕获+全局定时器时间，记录波形产生的各时间点。
    * Author       : 2018/4/13 星期五, by redmorningcn
    *******************************************************************************/
    init_ch_timepara_detect();
    
    /*******************************************************************************
    * Description  : 比较器参考电压初始化参数检测初始化
    * Author       : 2018/4/13 星期五, by redmorningcn
    *******************************************************************************/
    BSP_dac_init();
    
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
                        57600,                  // ... Baud Rate
                        USART_WordLength_8b,    // ... Number of data bits 7 or 8
                        USART_Parity_No,        // ... Parity: _NONE, _ODD or _EVEN
                        USART_StopBits_1,       // ... Number of stop bits 1 or 2
                        MODBUS_WR_EN);          // ... Enable (_EN) or disable (_DIS) writes

    
    BSP_WDT_Init(BSP_WDT_MODE_INT);             // 初始化看门狗
                                  
    while(1)
    {
        /*******************************************************************************
        * Description  : 计算速度通道的时间参数
        * Author       : 2018/4/13 星期五, by redmorningcn
        *******************************************************************************/
        app_calc_ch_timepara();
        
        /*******************************************************************************
        * Description  : 计算速度通道的电压参数
        * Author       : 2018/4/13 星期五, by redmorningcn
        *******************************************************************************/
        app_calc_ch_voltagepara();
        
        /*******************************************************************************
        * Description  : 脉冲信号异常处理
        * Author       : 2018/4/13 星期五, by redmorningcn
        *******************************************************************************/
        app_ch_judge();
        
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
        
        /*******************************************************************************
        * Description  : 计算速度通道的电压参数
        * Author       : 2018/4/13 星期五, by redmorningcn
        *******************************************************************************/
        app_calc_ch_voltagepara();
        
        /**************************************************************
        * Description  : 保存参数
        * Author       : 2018/7/18 星期三, by redmorningcn
        */
        store_para();

        
        BSP_WDT_Rst();
    }
}

