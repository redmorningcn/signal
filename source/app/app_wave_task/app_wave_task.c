/*******************************************************************************
* Description  : 计算波形参数
* Author       : 2018/4/12 星期四, by redmorningcn
*******************************************************************************/


/*******************************************************************************
* INCLUDES
*/
#include <app_wave_task.h>


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
            starttime   =   sCtrl.ch.test[i].time[p_read].low_up_time  * 65536 
                        +   sCtrl.ch.test[i].time[p_read].low_up_cnt  ;
            
            p_read      =   (sCtrl.ch.test[i].p_read + 1) % CH_TIMEPARA_BUF_SIZE;       //防止越界
            endtime     =   sCtrl.ch.test[i].time[p_read].low_up_time  * 65536 
                        +   sCtrl.ch.test[i].time[p_read].low_up_cnt  ;
            
            if(starttime > endtime)             //防止翻转
            {
                endtime += 65536;
            }
            periodtime = endtime - starttime;
            
            sCtrl.ch.para[i].period = (periodtime * 1000*1000*100 )/ sCtrl.sys.cpu_freq;
            
            if(periodtime){
                sCtrl.ch.para[i].freq = sCtrl.sys.cpu_freq  / periodtime;   //计算频率
                
                if(((sCtrl.sys.cpu_freq *10) % periodtime)> 4 )       //四舍五入
                    sCtrl.ch.para[i].freq += 1;
            }
            
            /*******************************************************************************
            * Description  : 计算占空比(xx.xx%)，( hig_down -  low_up ) / period
            * Author       : 2018/3/13 星期二, by redmorningcn
            *******************************************************************************/
            p_read      =   sCtrl.ch.test[i].p_read;
            starttime   =   sCtrl.ch.test[i].time[p_read].low_up_time  * 65536 
                        +   sCtrl.ch.test[i].time[p_read].low_up_cnt  ;
            endtime     =   sCtrl.ch.test[i].time[p_read].low_down_time  * 65536 
                        +   sCtrl.ch.test[i].time[p_read].low_down_cnt  ;            
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
            if(starttime > endtime)             //防止翻转
            {
                endtime += 65536;
            }
            else
            {
                sCtrl.ch.para[i].fail = ( endtime - starttime)*1000*1000*100/ sCtrl.sys.cpu_freq;
            }

            /*******************************************************************************
            * Description  : 计算相位差(xx.xx°)
            * Author       : 2018/3/13 星期二, by redmorningcn
            *******************************************************************************/
            if(i == 1)      
            {
                uint16      ptmp[2];
                
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

            //读指正++
            sCtrl.ch.test[i].p_read++ ;
            sCtrl.ch.test[i].p_read %= CH_TIMEPARA_BUF_SIZE; 
            
            //取信号的高低电平
        }
    }
}


/*******************************************************************************
* Description  : 取信号电平检测值
* Author       : 2018/3/29 星期四, by redmorningcn
*******************************************************************************/
void    app_calc_ch_voltagepara(void)
{
    uint8   i;
    static  uint8   lockflg[2]={0,0};   
    uint16  p_wr;
    uint16  p_rd;
    
    for(i = 0;i< 2;i++)
    {
        /*******************************************************************************
        * Description  : 更具脉冲所在位置，采集高低电平
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

        if(     ( p_wr > p_rd) &&  (p_wr > p_rd+10)           
           ||   ( p_wr < p_rd) &&  (p_wr + CH_VOLTAGE_BUF_SIZE > p_rd+10)           
            )  
        {
            uint32  sum;
            uint16  max,min;
            uint8   tmp8;
            uint16  tmp16;
            
            /*******************************************************************************
            * Description  : 在10个数中，除去最大值、最小值，再取平均
            * Author       : 2018/3/29 星期四, by redmorningcn
            *******************************************************************************/
            //计算低电平
            tmp8 = 0;
            sum  = 0;
            max  = sCtrl.ch.test[i].voltage[p_rd].ch_low_voltage;
            min  = max;
            for(tmp8 = 0;tmp8< 10;tmp8++)
            {
                tmp16 = sCtrl.ch.test[i].voltage[(p_rd + tmp8)%CH_VOLTAGE_BUF_SIZE].ch_low_voltage;
                if(tmp16 > max)
                    max = tmp16;
                
                if(tmp16 < min)
                    min = tmp16;
                
                sum += tmp16;
            }
            sCtrl.ch.para[i].Vol = (sum - max - min)/8;
                
            //计算高电平
            tmp8 = 0;
            sum  = 0;
            max  = sCtrl.ch.test[i].voltage[p_rd].ch_hig_voltage;
            min  = max;
            for(tmp8 = 0;tmp8< 10;tmp8++)
            {
                tmp16 = sCtrl.ch.test[i].voltage[(p_rd + tmp8)%CH_VOLTAGE_BUF_SIZE].ch_hig_voltage;
                if(tmp16 > max)
                    max = tmp16;
                
                if(tmp16 < min)
                    min = tmp16;
                
                sum += tmp16;
            }
            sCtrl.ch.para[i].Voh = (sum - max - min)/8;
            
            
            //计算供电电源
            if(i == 1)
            {
                tmp8 = 0;
                sum  = 0;
                max  = sCtrl.ch.test[i].voltage[p_rd].vcc_hig_voltage;
                min  = max;
                for(tmp8 = 0;tmp8< 10;tmp8++)
                {
                    tmp16 = sCtrl.ch.test[i].voltage[(p_rd + tmp8)%CH_VOLTAGE_BUF_SIZE].vcc_hig_voltage;
                    if(tmp16 > max)
                        max = tmp16;
                    
                    if(tmp16 < min)
                        min = tmp16;
                    
                    sum += tmp16;
                }
                
                sCtrl.ch.vcc_vol = (sum - max - min)/8;
            }
            
            /*******************************************************************************
            * Description  : 调整读指针
            * Author       : 2018/3/29 星期四, by redmorningcn
            *******************************************************************************/
            sCtrl.ch.test[i].p_rd_vol = (p_rd + tmp8) % CH_VOLTAGE_BUF_SIZE;
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
    static  uint32  plusecnt[2];
    static  uint32  errcnt[2];
    uint8   i;
    
    if(sCtrl.sys.time > tick + 300 || sCtrl.sys.time < tick)       //300ms判断一次
    {
        tick = sCtrl.sys.time;
            
        for(i = 0; i < 2;i++)
        {
            if(sCtrl.ch.test[i].pulse_cnt > plusecnt[i])
            {
                errcnt[i] = 0;
                sCtrl.ch.para[i].status       &= ~(0x01);     //脉冲信号正常  
                
            }
            else
            {
                errcnt[i]++;
                if(errcnt[i] > 2)                           //异常，处理(置标识位，数据清零，无脉冲信号)
                {
                    sCtrl.ch.para[i].fail     = 0;
                    sCtrl.ch.para[i].freq     = 0;
                    sCtrl.ch.para[i].period   = 0;
                    sCtrl.ch.para[i].raise    = 0;
                    sCtrl.ch.para[i].ratio    = 0;
                    sCtrl.ch.para[i].status  |= 0x01;         //无脉冲信号
                    sCtrl.ch.para[i].Voh      = 0;
                    sCtrl.ch.para[i].Vol      = 0;
                    
                    if(i == 1){
                        sCtrl.ch.ch1_2phase   = 0;
                        sCtrl.ch.vcc_vol      = 0;
                    }
                    
                }
            }
            plusecnt[i] = sCtrl.ch.test[i].pulse_cnt;
        }
    }
}


/*******************************************************************************
* 				end of file
*******************************************************************************/
