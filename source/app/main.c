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
* Description  :  ָʾ���������ٶ��ź����������ٶ��źſ��������ڵ��ã�100ms��
* Author       : 2018/4/13 ������, by redmorningcn
*******************************************************************************/
void    led_task(void)
{
    static  uint8   blinkcnt= 0;
    
    blinkcnt++;
    
    if(Ctrl.ch.para[0].period == 0 && Ctrl.ch.para[1].period  == 0)   //����������   
    {
        blinkcnt %= 20;                                     //����ͨ�����źţ�����  
    }
    else
    {
        blinkcnt %= 2;                                      //����ͨ�����źţ�����
    }
    
    if(blinkcnt == 0)                                       //�������
    {
        BSP_LED_On(1);
    }
    else
    {
        BSP_LED_Off(1);
    }
}

//�ο���ѹ����λmv
#define     FULL_VOLTAGE            (3300)          /*  ������ѹ            */

#define     MAX_HIG_VOLTAGE         (3000)          /*  ���ߵ�ƽ              */
#define     MAX_STANDARD_VOLTAGE    (1500)          /*  Ĭ�ϲο���ѹ��ֵ        */  
#define     MAX_STANDARD_ZONE       (300)           /*  �ο���ѹ��ֵ��Χ        */
#define     MIN_STANDARD_VOLTAGE    (750)           /*  Ĭ�ϲο���ѹСֵ        */
#define     MIN_STANDARD_ZONE       (150)           /*  Ĭ�ϲο���ѹСֵ��Χ    */ 

#define     REF_VOLTAGE_RATIO       (0.75)          /* �ο���ѹ�͸ߵ�ƽϵ����ϵ */


/*******************************************************************************
* Description  : ���òο���ѹ�����ڵ��ã�100ms��
                �ο���ѹΪ�ߵ�ƽ��0.09����ȡ10%��ѹ��0.9�������ߵ�ƽ��������10%λ��
                ����5�Σ��ο���ѹ��ֵƫ�����10%�����µ����ο���ѹ��
* Author       : 2018/4/16 ����һ, by redmorningcn
*******************************************************************************/
void    set_dac_task(void)
{
    static  uint32  sum;
    static  uint8   diffcnt = 0;    
    
    uint8   i;
    for(i = 0;i< 2;i++)
    if( Ctrl.ch.para[i].freq )                                           //ͨ��0����ͨ��1���ٶ��ź�
    {
        if(fabs(Ctrl.ch.stand_vol -  Ctrl.ch.para[i].Voh * REF_VOLTAGE_RATIO) > (FULL_VOLTAGE/100) )    //��ѹƫ��С��10% ��
        {
            if(Ctrl.ch.para[i].Voh < MAX_HIG_VOLTAGE    )                    //�ߵ�ƽ���3V
            {
                sum += Ctrl.ch.para[i].Voh;
                diffcnt++;
            }
            
            if( diffcnt > 10 )                                              //����5�Σ��ɼ��ĸߵ�ƽ��ѹ�����õ�ѹ
            {
                Ctrl.ch.stand_vol =(u16)((sum / diffcnt)*REF_VOLTAGE_RATIO);
                
                if( Ctrl.ch.stand_vol > Ctrl.sys.ref_limitvol_max  )          //�޶��Ƚ����Ĳο���ѹ��1.5V��0.66V֮��
                {
                    Ctrl.ch.stand_vol   = Ctrl.sys.ref_limitvol_max;          //1.5v
                }
                else if(Ctrl.ch.stand_vol < Ctrl.sys.ref_limitvol_min)
                {
                    Ctrl.ch.stand_vol  = Ctrl.sys.ref_limitvol_min;               //0.8V
                }
                
                bsp_set_dacvalue(Ctrl.ch.stand_vol);                     //�������ñȽ�ֵ
                
                
                /**************************************************************
                * Description  : ���¼���
                * Author       : 2018/7/20 ������, by redmorningcn
                */
                diffcnt = 0;
                sum     = 0;
                
                return;
            }
        }
        else
        {
            sum     = 0;
            diffcnt = 0;
        }
    }else{
                                                                                // (���û���źţ�����Ĭ��ֵ����)
        if(i==1){
        
            if( Ctrl.ch.stand_vol >  Ctrl.sys.ref_limitvol_max  )               //�޶��Ƚ����Ĳο���ѹ��1.5V��0.66V֮��
            {
                Ctrl.ch.stand_vol  = Ctrl.sys.ref_limitvol_max;                 //1.5v
            }
            else if(Ctrl.ch.stand_vol < Ctrl.sys.ref_limitvol_min)
            {
                Ctrl.ch.stand_vol  = Ctrl.sys.ref_limitvol_min;                 //0.8V
            }
            
            bsp_set_dacvalue(Ctrl.ch.stand_vol);                                //�������ñȽ�ֵ  
        }
    }
}

/**************************************************************
* Description  : ��ʼ��ϵͳ����
* Author       : 2018/5/30 ������, by redmorningcn
*/
void    InitCtrl(void)
{
    /**************************************************************
    * Description  : ��ȡУ׼ֵ
    * Author       : 2018/5/30 ������, by redmorningcn
    */
    BSP_FlashReadBytes(STORE_ADDR_CALI,
                       (u8 *)&Ctrl.calitab,
                       sizeof(Ctrl.calitab));
    
    for(u8  i = 0;i <sizeof(Ctrl.calitab)/sizeof(strCalibration);i++){
        //�������Զȼ��
        if(     Ctrl.calitab.CaliBuf[i].line < CALI_LINE_MIN 
            ||  Ctrl.calitab.CaliBuf[i].line > CALI_LINE_MAX
            ){
                Ctrl.calitab.CaliBuf[i].line   = CALI_LINE_BASE; //���Զȳ��ޣ���Ĭ��ֵ
                Ctrl.calitab.CaliBuf[i].Delta  = CALI_DELTA_BASE;//����ƫ��ޣ���Ĭ��ֵ

            }
        
        //����ƫ����
        if(     Ctrl.calitab.CaliBuf[i].Delta < CALI_DELTA_MIN
            ||  Ctrl.calitab.CaliBuf[i].Delta > CALI_DELTA_MAX
            ){
                Ctrl.calitab.CaliBuf[i].Delta = CALI_DELTA_BASE;//����ƫ��ޣ���Ĭ��ֵ
            }
    }
    
    /**************************************************************
    * Description  : �����в���
    * Author       : 2018/7/19 ������, by redmorningcn
    */
    BSP_FlashReadBytes(STORE_ADDR_SYS,
                       (u8 *)&Ctrl.sys,
                       sizeof(Ctrl.sys));
    
    Ctrl.sys.id = get_boardID();
	Ctrl.sys.cpu_freq = BSP_CPU_ClkFreq();  //ʱ��Ƶ��               /* Determine SysTick reference freq.              */

    Ctrl.sys.paraflg.sysflg    = 0;        //����ϵͳ������sys��
    Ctrl.sys.paraflg.califlg   = 0;        //��������������cali��

    if(Ctrl.sys.periodcali > 5*100 || Ctrl.sys.periodcali < 1.5*100)    //����屶����Ĭ��2���ź����� ��*10��
        Ctrl.sys.periodcali = (u32)(2.01*100);
    
    if(Ctrl.sys.loseerrtimes > 30 || Ctrl.sys.loseerrtimes < 3)          //����������жϴ���
        Ctrl.sys.loseerrtimes = 10;
    
    if(     Ctrl.sys.ref_limitvol_min < (MIN_STANDARD_VOLTAGE - MIN_STANDARD_ZONE) 
       ||   Ctrl.sys.ref_limitvol_min > (MIN_STANDARD_VOLTAGE + MIN_STANDARD_ZONE) ) {   //��ȡ�òο���ѹ�������޶�����
           Ctrl.sys.ref_limitvol_min = MIN_STANDARD_VOLTAGE;
    }
    
    if(     Ctrl.sys.ref_limitvol_max < (MAX_STANDARD_VOLTAGE - MAX_STANDARD_ZONE) 
       ||   Ctrl.sys.ref_limitvol_max > (MAX_STANDARD_VOLTAGE + MAX_STANDARD_ZONE) ) {   //��ȡ�òο���ѹ�������޶�����
            Ctrl.sys.ref_limitvol_max = MAX_STANDARD_VOLTAGE;
    }    
}

/**************************************************************
* Description  : �������
* Author       : 2018/5/30 ������, by redmorningcn
*/
void    store_para(void)
{
    if(Ctrl.sys.paraflg.califlg == 1){
        Ctrl.sys.paraflg.califlg = 0; 
        
        BSP_FlashWriteBytes(    STORE_ADDR_CALI,
                                (u8 *)&Ctrl.calitab,
                                sizeof(Ctrl.calitab) );
    }
    
    if(Ctrl.sys.paraflg.sysflg == 1){
        Ctrl.sys.paraflg.sysflg = 0;
        
        BSP_FlashWriteBytes(    STORE_ADDR_SYS,
                                (u8 *)&Ctrl.sys,
                                sizeof(Ctrl.sys)    );
    }
}
        
    
/*******************************************************************************
* Description  : ��������ʱ�䲻���ȵĹ����ڴ�����
* Author       : 2018/4/16 ����һ, by redmorningcn
*******************************************************************************/
void    idle_task(void)      
{
    static  uint32  tick;
    if(Ctrl.sys.time > tick+100 ||  Ctrl.sys.time < tick)   //100ms
    {
        tick = Ctrl.sys.time;                               //ʱ��
        
        led_task();                                         //ָʾ�ƿ���
        
        set_dac_task();                                     //���òο���ѹ
        
        store_para();                                       //�������
    }
}

extern  void mod_bus_rx_task(void);

void main (void)
{
    BSP_WDT_Init(BSP_WDT_MODE_INT);                             // ��ʼ�����Ź�����ֹ�ϵ粻������ redmorningcn 20180719��

	BSP_Init();                                                 /* Initialize BSP functions                 */
	CPU_TS_TmrInit();
	/***********************************************
	* ������ ��ʼ���δ�ʱ��������ʼ��ϵͳ����ʱ�ӡ�
	*/
	Ctrl.sys.cpu_freq = BSP_CPU_ClkFreq();  //ʱ��Ƶ��          /* Determine SysTick reference freq.        */
    
    /*******************************************************************************
    * Description  : �źŷ�ֵ��������Դ��ѹ����ʼ����
    * Author       : 2018/4/12 ������, by redmorningcn
    *******************************************************************************/
	Bsp_ADC_Init();
    
    /*******************************************************************************
    * Description  : �豸ID�Ż�ȡ��ʼ��
    * Author       : 2018/4/13 ������, by redmorningcn
    *******************************************************************************/
    Init_boardID();
    Ctrl.sys.id = get_boardID();
    
    /*******************************************************************************
    * Description  : �ٶ�ͨ��ʱ���������ʼ��
    ��ʱ������+ȫ�ֶ�ʱ��ʱ�䣬��¼���β����ĸ�ʱ��㡣
    * Author       : 2018/4/13 ������, by redmorningcn
    *******************************************************************************/
    init_ch_timepara_detect();
    
    /*******************************************************************************
    * Description  : �Ƚ����ο���ѹ��ʼ����������ʼ��
    * Author       : 2018/4/13 ������, by redmorningcn
    *******************************************************************************/
    BSP_dac_init();
    
    /**************************************************************
    * Description  : ���Ѵ������������
    * Author       : 2018/5/29 ���ڶ�, by redmorningcn
    */
    InitCtrl();
    
    /*******************************************************************************
    * Description  : ����ͨ�ų�ʼ��
    * Author       : 2018/5/7 ����һ, by redmorningcn
    *******************************************************************************/
    MB_Init(1000);      //��ʼ��modbusƵ��					

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

    
    BSP_WDT_Init(BSP_WDT_MODE_INT);             // ��ʼ�����Ź�
                                  
    while(1)
    {
        /*******************************************************************************
        * Description  : �����ٶ�ͨ����ʱ�����
        * Author       : 2018/4/13 ������, by redmorningcn
        *******************************************************************************/
        app_calc_ch_timepara();
        
        /*******************************************************************************
        * Description  : �����ٶ�ͨ���ĵ�ѹ����
        * Author       : 2018/4/13 ������, by redmorningcn
        *******************************************************************************/
        app_calc_ch_voltagepara();
        
        /*******************************************************************************
        * Description  : �����ź��쳣����
        * Author       : 2018/4/13 ������, by redmorningcn
        *******************************************************************************/
        app_ch_judge();
        
        /*******************************************************************************
        * Description  : ��������ʱ�䲻���еĵĹ����ڴ����У�ÿ100ms����һ��
        * Author       : 2018/4/16 ����һ, by redmorningcn
        *******************************************************************************/
        idle_task();
        
        /*******************************************************************************
        * Description  : �����ٶ�ͨ���ĵ�ѹ����
        * Author       : 2018/4/13 ������, by redmorningcn
        *******************************************************************************/
        app_calc_ch_voltagepara();
        
        /*******************************************************************************
        * Description  : ͨѶ����
        * Author       : 2018/5/7 ����һ, by redmorningcn
        *******************************************************************************/
        mod_bus_rx_task();
        
        /*******************************************************************************
        * Description  : �����ٶ�ͨ���ĵ�ѹ����
        * Author       : 2018/4/13 ������, by redmorningcn
        *******************************************************************************/
        app_calc_ch_voltagepara();
        
        /**************************************************************
        * Description  : �������
        * Author       : 2018/7/18 ������, by redmorningcn
        */
        store_para();

        
        BSP_WDT_Rst();
    }
}

