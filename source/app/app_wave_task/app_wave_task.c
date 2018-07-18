/*******************************************************************************
* Description  : 计算波形参数
* Author       : 2018/4/12 星期四, by redmorningcn
*******************************************************************************/


/*******************************************************************************
* INCLUDES
*/
#include <app_wave_task.h>
#include <Algorithm.h>


/*******************************************************************************
* Description  : 通道时间参数计算
                 根据通道buf的读写指针，确定是否需要进运算
* Author       : 2018/3/13 星期二, by redmorningcn
*******************************************************************************/
void    app_calc_ch_timepara(void)
{
    uint8   p_write;
    uint8   p_read;
    uint64  starttime;
    uint64  endtime;
    
    uint64  periodtime;
    uint64  ratiotime;
    
    uint8   i;
        
    for(i = 0;i< 2;i++)
    {
        p_write = sCtrl.ch.test[i].p_write;
        p_read  = sCtrl.ch.test[i].p_read;
        
        /*******************************************************************************
        * Description  : 速度通道时间参数运算
        * Author       : 2018/3/13 星期二, by redmorningcn
        *******************************************************************************/
        if(     ( p_write > p_read) &&  (p_write > p_read+10)           
           ||   ( p_write < p_read) &&  (p_write + CH_TIMEPARA_BUF_SIZE > p_read+10)           
               )  
        {

            /*******************************************************************************
            * Description  : 计算周期(0.01us) 周期信号任意一点再次出现，取low_up中断为标准
            * Author       : 2018/3/13 星期二, by redmorningcn
            *******************************************************************************/
            p_read      =   sCtrl.ch.test[i].p_read;
            starttime   =   sCtrl.ch.test[i].time[p_read].hig_up_time  * 65536 
                        +   sCtrl.ch.test[i].time[p_read].hig_up_cnt  ;
            
            p_read      =   (sCtrl.ch.test[i].p_read + 1) % CH_TIMEPARA_BUF_SIZE;   //防止越界
            endtime     =   sCtrl.ch.test[i].time[p_read].hig_up_time  * 65536 
                        +   sCtrl.ch.test[i].time[p_read].hig_up_cnt  ;
            
            if(starttime > endtime)                                                 //防止翻转
            {
                endtime += 65536;
            }
            periodtime = endtime - starttime;
            
            sCtrl.ch.para[i].period = (periodtime * 1000*1000*100 )/ sCtrl.sys.cpu_freq;
            
            if(periodtime){
                sCtrl.ch.para[i].freq = sCtrl.sys.cpu_freq  / periodtime;           //计算频率
                
                if(((sCtrl.sys.cpu_freq *10) % periodtime)> 4 )                     //四舍五入
                    sCtrl.ch.para[i].freq += 1;
            }
            
            /**************************************************************
            * Description  : 如果频率为0，后面计算，直接赋值0
            * Author       : 2018/7/18 星期三, by redmorningcn
            */
            if(sCtrl.ch.para[i].freq    == 0){
                sCtrl.ch.para[i].ratio  = 0;
                sCtrl.ch.para[i].raise  = 0;
                sCtrl.ch.para[i].fail   = 0; 
                sCtrl.ch.ch1_2phase     = 0;
                return;
            }
            
            /*******************************************************************************
            * Description  : 计算占空比(xx.xx%)，( hig_down -  low_up ) / period
            * Author       : 2018/3/13 星期二, by redmorningcn
            *******************************************************************************/
            p_read      =   sCtrl.ch.test[i].p_read;
            starttime   =   sCtrl.ch.test[i].time[p_read].hig_up_time  * 65536 
                        +   sCtrl.ch.test[i].time[p_read].hig_up_cnt  ;
            endtime     =   sCtrl.ch.test[i].time[p_read].hig_down_time  * 65536 
                        +   sCtrl.ch.test[i].time[p_read].hig_down_cnt  ;            
            if(starttime > endtime)             //防止翻转
            {
                endtime += 65536;
            }
            ratiotime = endtime - starttime;
            
            if( periodtime )
                sCtrl.ch.para[i].ratio = ( ratiotime * 100* 100 ) / periodtime; 
            
            /*******************************************************************************
            * Description  : 计算上升沿（0.01us）
            * Author       : 2018/3/13 星期二, by redmorningcn
            *******************************************************************************/
            p_read      =   sCtrl.ch.test[i].p_read;
            starttime   =   sCtrl.ch.test[i].time[p_read].low_up_time  * 65536 
                        +   sCtrl.ch.test[i].time[p_read].low_up_cnt  ;
            endtime     =   sCtrl.ch.test[i].time[p_read].hig_up_time  * 65536 
                        +   sCtrl.ch.test[i].time[p_read].hig_up_cnt  ;            
            if(starttime > endtime)             //防止翻转,
            {
                endtime += 65536;
            }
            else
            {
                sCtrl.ch.para[i].raise = ( endtime - starttime) *1000*1000*100 / sCtrl.sys.cpu_freq;
                
                if(sCtrl.ch.para[i].raise  > 500)
                    sCtrl.ch.para[i].raise  = 500;
            }
            
            /*******************************************************************************
            * Description  : 计算下降沿(0.01us)
            * Author       : 2018/3/13 星期二, by redmorningcn
            *******************************************************************************/
            p_read      =   sCtrl.ch.test[i].p_read;
            starttime   =   sCtrl.ch.test[i].time[p_read].hig_down_time  * 65536 
                        +   sCtrl.ch.test[i].time[p_read].hig_down_cnt  ;
            endtime     =   sCtrl.ch.test[i].time[p_read].low_down_time  * 65536 
                        +   sCtrl.ch.test[i].time[p_read].low_down_cnt  ;     
            
            uint16      failtime; 
            if(starttime > endtime)             //防止翻转
            {
                endtime += 65536;
            }
            else
            {
                failtime = ( endtime - starttime)*1000*1000*100/ sCtrl.sys.cpu_freq;
                
                if(failtime < 25 )  {               //补偿采样电路误差 180712 (减去固有误差)
                    failtime = failtime / 5;
                }else{
                    failtime -= 25;
                    
                    if(failtime < 5)
                        failtime = 5;
                }
                    
                sCtrl.ch.para[i].fail = failtime;
            }

            /*******************************************************************************
            * Description  : 计算相位差(xx.xx°)
            * Author       : 2018/3/13 星期二, by redmorningcn
            *******************************************************************************/
            if(i == 1)      
            {
                uint16      ptmp[2];
                
                if(sCtrl.ch.para[1].freq  && sCtrl.ch.para[0].freq ){
                
                    ptmp[0]    = sCtrl.ch.test[0].p_write;
                    ptmp[1]    = sCtrl.ch.test[1].p_write;
                    
                    p_read      =   (ptmp[0] + CH_TIMEPARA_BUF_SIZE -1)%CH_TIMEPARA_BUF_SIZE ;
                    starttime   =   sCtrl.ch.test[0].time[p_read].low_up_time  * 65536 
                                +   sCtrl.ch.test[0].time[p_read].low_up_cnt  ;
                    
                    p_read      =   (ptmp[1] + CH_TIMEPARA_BUF_SIZE -1)%CH_TIMEPARA_BUF_SIZE ;

                    endtime     =   sCtrl.ch.test[1].time[p_read].low_up_time  * 65536 
                                +   sCtrl.ch.test[1].time[p_read].low_up_cnt  ;            
                    if(starttime > endtime)             //防止翻转 (用最近的两信号进行计算)
                    {
                        endtime += periodtime;          //加一周期时间
                    }
                    
                    //   phase/360 = difftime/peirod
                    sCtrl.ch.ch1_2phase = 360*(endtime - starttime)*100 / periodtime; 
                }
            }

            /**************************************************************
            * Description  : 传感器丢脉冲判断
            判断方法：对比前后脉冲周期，如果周期偏差大于%50，认为丢脉冲。
            * Author       : 2018/5/30 星期三, by redmorningcn
            */
            static  u32     plusetimes[2];          //正常脉冲计算器。从产生错误开始计数
            static  u8      perioderrcnt[2];        //周期错误计数器。每产生一次错误加1，一圈内周期都正常，清除。
            static  u32     lastperiod[2];          //上次周期
            
            //前后周期对比
            if(sCtrl.ch.para[i].period * PERIOD_ERR_CALI < lastperiod[i] ){
                plusetimes[i] = 0;
                perioderrcnt[i]++;
                
                if(perioderrcnt[i] > PERIOD_ERR_CNT){
                    sCtrl.ch.para[i].status.lose = 1;   
                }
            }else{
                plusetimes[i]++;                        
                
                if(plusetimes[i] > CIRCLE_PLUSE_NUM )   // 如果产生的错误，经过1圈后，未在发生，则认为是误判。
                {
                    perioderrcnt[i] = 0;                // 错误计数器清零
                    sCtrl.ch.para[i].status.lose = 0;   
                }
            }
            lastperiod[i] = sCtrl.ch.para[i].period;    // 下次判断使用
            
            //读指正++
            sCtrl.ch.test[i].p_read++ ;
            sCtrl.ch.test[i].p_read %= CH_TIMEPARA_BUF_SIZE; 
            
            //取信号的高低电平
        }
    }
}

#define     CALC_NUM_VOL  (10)
/*******************************************************************************
* Description  : 取信号电平检测值
* Author       : 2018/3/29 星期四, by redmorningcn
*******************************************************************************/
void    app_calc_ch_voltagepara(void)
{
    uint8    i;
    static  uint8   lockflg[2]={0,0};   
    uint16   p_wr;
    uint16   p_rd;
    static  u32 systime     = 0;
    static  u8  nocalatimes = 0; 
    uint8    calcnum         = CALC_NUM_VOL;    //滤波常数
    uint8    divisor         = 3;
    
    for(i = 0;i< 2;i++)
    {
        /*******************************************************************************
        * Description  : 根据脉冲所在位置，采集高低电平
                        高电平：上升沿，90%后采集；
                        低电平：下降沿，10%后采集；
                        在高、低电平均采集一次后（lockflg实现），才进行下组数据采集。
        * Author       : 2018/3/29 星期四, by redmorningcn
        *******************************************************************************/
        p_wr = sCtrl.ch.test[i].p_wr_vol;

        
        if(( sCtrl.ch.test[i].station.fall_90)   && lockflg[i] == 0 )    
        {
            lockflg[i] = 1;                                     //互锁信号
           
            sCtrl.ch.test[i].voltage[p_wr].ch_low_voltage =   Get_ADC(ADC_Channel_10+i);
        }
        
        if((sCtrl.ch.test[i].station.raise_10) && lockflg[i]  )
        //if(sCtrl.ch.test[i].station.raise_10 && lockflg[i]  )
        {
            lockflg[i] = 0;                                    //互锁信号,必须取完低电平才能执行
            
            sCtrl.ch.test[i].voltage[p_wr].ch_hig_voltage = Get_ADC(ADC_Channel_10+i);
            
            /*******************************************************************************
            * Description  : 取供电电平及写指针++
            * Author       : 2018/3/29 星期四, by redmorningcn
            *******************************************************************************/
            if(i == 1)  //取供电电平
                sCtrl.ch.test[i].voltage[p_wr].vcc_hig_voltage = Get_ADC(ADC_Channel_10+2);
            
            p_wr++;
            
            sCtrl.ch.test[i].p_wr_vol = p_wr % CH_VOLTAGE_BUF_SIZE;
        }
        
        /*******************************************************************************
        * Description  : 计算高低电平
        * Author       : 2018/3/29 星期四, by redmorningcn
        *******************************************************************************/
        p_wr = sCtrl.ch.test[i].p_wr_vol;
        p_rd = sCtrl.ch.test[i].p_rd_vol;

        if(     ( p_wr > p_rd) &&  (p_wr > p_rd+ calcnum)           
           ||   ( p_wr < p_rd) &&  (p_wr + CH_VOLTAGE_BUF_SIZE > p_rd+calcnum)           
            )  
        {
            int32       voh,vol,vcc;  
            
            nocalatimes     = 0;                //
            systime         = sCtrl.sys.time;
            
            /*******************************************************************************
            * Description  : 在10个数中，除去最大值、最小值，再取平均
            * Author       : 2018/3/29 星期四, by redmorningcn
            *******************************************************************************/
            //计算低电平
            u16   buftmp[CALC_NUM_VOL];
            
            for(u8 j=0;j < calcnum;j++){
                buftmp[j] = sCtrl.ch.test[i].voltage[(p_rd + j)%CH_VOLTAGE_BUF_SIZE].ch_low_voltage;
            }
            vol = App_GetFilterValue(buftmp, buftmp, calcnum, calcnum/divisor, 0);
            
                
            //计算高电平
            for(u8 j=0;j < 10;j++){
                buftmp[j] = sCtrl.ch.test[i].voltage[(p_rd + j)%CH_VOLTAGE_BUF_SIZE].ch_hig_voltage;
            }
            voh = App_GetFilterValue(buftmp, buftmp, calcnum, calcnum/divisor, 0);
            

            //计算供电电源
            if(i == 1)
            {
                //计算高电平
                for(u8 j=0;j < 10;j++){
                    buftmp[j] = sCtrl.ch.test[i].voltage[(p_rd + j)%CH_VOLTAGE_BUF_SIZE].vcc_hig_voltage;
                }
                vcc = App_GetFilterValue(buftmp, buftmp, calcnum, calcnum/divisor, 0);
                
                sCtrl.ch.vcc_vol     = (vcc * sCtrl.calitab.VccVol.line / CALI_LINE_BASE)       + sCtrl.calitab.VccVol.Delta;
            }
            
            /**************************************************************
            * Description  : 计算值修正
            * Author       : 2018/5/30 星期三, by redmorningcn
            */
            sCtrl.ch.para[i].Vol = (vol * sCtrl.calitab.CaliBuf[i+1].line / CALI_LINE_BASE) + sCtrl.calitab.CaliBuf[i+1].Delta;
            sCtrl.ch.para[i].Voh = (voh * sCtrl.calitab.CaliBuf[i+1].line / CALI_LINE_BASE) + sCtrl.calitab.CaliBuf[i+1].Delta;
            
            /*******************************************************************************
            * Description  : 调整读指针
            * Author       : 2018/3/29 星期四, by redmorningcn
            *******************************************************************************/
            sCtrl.ch.test[i].p_rd_vol = (p_rd + calcnum) % CH_VOLTAGE_BUF_SIZE;
        }
    }
    
    /**************************************************************
    * Description  : 如果长时间未进行采集信号VCC，
    * Author       : 2018/5/30 星期三, by redmorningcn
    */
    if(sCtrl.sys.time  > systime + 1000){
        systime = sCtrl.sys.time;
        nocalatimes++;
        
        if(nocalatimes >20 ){
            nocalatimes = 0;
            int32 vcc = Get_ADC(ADC_Channel_10+2);      //采集电压
            sCtrl.ch.vcc_vol     = (vcc * sCtrl.calitab.VccVol.line / CALI_LINE_BASE) + sCtrl.calitab.VccVol.Delta;
        }
    }
}


/*******************************************************************************
* Description  : 速度信号的异常判读
* Author       : 2018/4/13 星期五, by redmorningcn
*******************************************************************************/
void    app_ch_judge(void)
{
    static  uint32  tick;
    static  uint32  errcnt[2];
    uint8    i;
    static  int pluse[2];
        
    if(sCtrl.sys.time > tick + 300 || sCtrl.sys.time < tick)    //300ms判断一次
    {
        tick = sCtrl.sys.time;
            
        /**************************************************************
        * Description  : 无信号判断
        * Author       : 2018/7/18 星期三, by redmorningcn
        */
        for(i = 0; i < 2;i++)
        {
            if(pluse[i] !=  sCtrl.ch.test[i].pulse_cnt)
            {
                errcnt[i] = 0;
                
                sCtrl.ch.para[i].status.nopluse = 0;            //有脉冲信号        
            }
            else
            {   
                if(sCtrl.ch.para[i].status.nopluse == 0){       //有信號
                    errcnt[i]++;
                    
                    if(errcnt[i] > 2)                           //异常，处理(置标识位，数据清零，无脉冲信号)
                    {
                        sCtrl.ch.para[i].fail     = 0;
                        sCtrl.ch.para[i].freq     = 0;
                        sCtrl.ch.para[i].period   = 0;
                        sCtrl.ch.para[i].raise    = 0;
                        sCtrl.ch.para[i].ratio    = 0;
                        sCtrl.ch.para[i].status.nopluse = 1;    //无脉冲信号
                        sCtrl.ch.para[i].Voh      = 0;
                        sCtrl.ch.para[i].Vol      = 0;
                        
                        sCtrl.ch.ch1_2phase   = 0;
                    }
                }
            }
            pluse[i] = sCtrl.ch.test[i].pulse_cnt;
        }
    }
}


/*******************************************************************************
* 				end of file
*******************************************************************************/
