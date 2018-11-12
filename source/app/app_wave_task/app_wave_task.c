/*******************************************************************************
* Description  : ���㲨�β���
* Author       : 2018/4/12 ������, by redmorningcn
*******************************************************************************/


/*******************************************************************************
* INCLUDES
*/
#include <app_wave_task.h>
#include <Algorithm.h>

//ʱ���������ֵ����
#define     CH_PARA_BUF_SIZE             (10)     
//����ʱ�䲹��ϵ��
#define     CH_EDGE_CALI                 (1.1)

typedef   struct   _strChTimeParaFliter_{
    uint32              period[CH_PARA_BUF_SIZE];                         //���ڣ�  0.00-2000000.00us ��0.5Hz��
    uint32              freq[CH_PARA_BUF_SIZE];                           //Ƶ�ʣ�  0-100000hz              
    uint32              raise[CH_PARA_BUF_SIZE];                          //�����أ�0.00-50.00us
    uint32              fail[CH_PARA_BUF_SIZE];                           //�½��أ�0.00-50.00us
    uint32              ratio[CH_PARA_BUF_SIZE];                          //ռ�ձȣ�0.00-100.00%
    uint32              phase[CH_PARA_BUF_SIZE];
    int32               Acceleration[CH_PARA_BUF_SIZE];                   //���ٶ�
}strChTimeParaFliter;

strChTimeParaFliter     lsChTimeFliterBuf[2];
u32                     timetmpbuf[CH_PARA_BUF_SIZE];





/*******************************************************************************
* Description  : ͨ��ʱ���������
                 ����ͨ��buf�Ķ�дָ�룬ȷ���Ƿ���Ҫ������
* Author       : 2018/3/13 ���ڶ�, by redmorningcn
*******************************************************************************/
void    app_calc_ch_timepara(void)
{
    uint8   p_write;
    uint8   p_read;

    u16     p_next     ;
    u32     now_time   ;   
    u32     next_time  ;   
    u32     now_cnt    ;   
    u32     next_cnt   ;   
    
    s32     periodtime; 
    s32     ratiotime;
    s32     raisetime;
    s32     failtime;
    s32     phasetime;
    
    u32     freq;
    s32     Acceleration;
    
    u32     pri_period[2];          //���㶪����ʹ�ã�ԭʼ����ֵ
        
    static  u8      i = 0;
    static  u8      pparabuf[2] = {0,0};
    
    
    i++;
    i %= 2;
        
    //for(i = 0;i< 2;i++)
    {
        p_write = Ctrl.ch.test[i].p_write;
        p_read  = Ctrl.ch.test[i].p_read;
        
        /*******************************************************************************
        * Description  : �ٶ�ͨ��ʱ���������
        * Author       : 2018/3/13 ���ڶ�, by redmorningcn
        *******************************************************************************/
        if(     ( p_write > p_read) &&  (p_write > p_read+2)           
           ||   ( p_write < p_read) &&  (p_write + CH_TIMEPARA_BUF_SIZE > p_read+2)           
               )  
        {
            /*******************************************************************************
            * Description  : ��������(0.01us) �����ź�����һ���ٴγ��֣�ȡlow_up�ж�Ϊ��׼
            * Author       : 2018/3/13 ���ڶ�, by redmorningcn
            *******************************************************************************/
            p_read      = Ctrl.ch.test[i].p_read;
            p_next      = (p_read + 1) % CH_TIMEPARA_BUF_SIZE;
            
            now_time    = Ctrl.ch.test[i].time[p_read].hig_up_time;
            now_cnt     = Ctrl.ch.test[i].time[p_read].hig_up_cnt;
            
            next_time   = Ctrl.ch.test[i].time[p_next].hig_up_time;
            next_cnt    = Ctrl.ch.test[i].time[p_next].hig_up_cnt;
            
            if(now_time <= next_time)
            {
                periodtime = (next_time - now_time) * 65536 + next_cnt - now_cnt;               //redmonringcn 20180719 ȡ��64λ�˷�����
                
                while(periodtime < 0)
                    periodtime += 65536;
                
                   pri_period[i] =  (periodtime * 100 )/ (Ctrl.sys.cpu_freq / (1000*1000));//���㶪����ʱʹ�á�
                       
                   App_FillAndMoveBuf32((u32 *)lsChTimeFliterBuf[i].period,CH_PARA_BUF_SIZE,periodtime);  //������д��buf���������CH_PARA_BUF_SIZE������
                
                if(pparabuf[i] > CH_PARA_BUF_SIZE)
                    periodtime = App_GetFilterValue32((u32 *)lsChTimeFliterBuf[i].period, timetmpbuf, CH_PARA_BUF_SIZE, CH_PARA_BUF_SIZE/3, 0);    //���ݹ���
                
            }
            else   //ʱ�ӷ�ת��ֱ������
            {
                //��ָ��++
                Ctrl.ch.test[i].p_read++ ;
                Ctrl.ch.test[i].p_read %= CH_TIMEPARA_BUF_SIZE; 
                //continue;           //�����˴�ѭ��
                return;
            }
            
            Ctrl.ch.para[i].period = (periodtime * 100 )/ (Ctrl.sys.cpu_freq / (1000*1000));
            
            
            if(periodtime){
                //Ctrl.ch.para[i].freq = Ctrl.sys.cpu_freq  / periodtime;                       //����Ƶ��
                
                freq = Ctrl.sys.cpu_freq  / periodtime;                                         //����Ƶ��
                  
                App_FillAndMoveBuf32((u32 *)lsChTimeFliterBuf[i].freq,CH_PARA_BUF_SIZE,freq);   //������д��buf���������CH_PARA_BUF_SIZE������

                if(pparabuf[i] > CH_PARA_BUF_SIZE)
                    periodtime = App_GetFilterValue32((u32 *)lsChTimeFliterBuf[i].period, timetmpbuf, CH_PARA_BUF_SIZE, CH_PARA_BUF_SIZE/3, 0);    //���ݹ���
                
                Ctrl.ch.para[i].freq = freq;
                
                if(((Ctrl.sys.cpu_freq *10) % periodtime)> 4 )                     //��������
                    Ctrl.ch.para[i].freq += 1;
            }
            
            /**************************************************************
            * Description  : ���Ƶ��Ϊ0��������㣬ֱ�Ӹ�ֵ0
            * Author       : 2018/7/18 ������, by redmorningcn
            */
            if(Ctrl.ch.para[i].freq    == 0 || periodtime == 0){
                Ctrl.ch.para[i].ratio  = 0;
                Ctrl.ch.para[i].raise  = 0;
                Ctrl.ch.para[i].fail   = 0; 
                Ctrl.ch.ch1_2phase     = 0;
                
                //��ָ��++
                Ctrl.ch.test[i].p_read++ ;
                Ctrl.ch.test[i].p_read %= CH_TIMEPARA_BUF_SIZE; 
                //continue;           //�����˴�ѭ��
                return;
            }
            
            /**************************************************************
            * Description  : ������ٶ�
            a = K * fm * ��fm-fn��/(m-n)
            k = 200 / 3.14 /1.05 = 60
            * Author       : 2018/7/31 ���ڶ�, by redmorningcn
            */
            if(pparabuf[i] > CH_PARA_BUF_SIZE){
                s32 freqend,freqstart;
                
                static  u8  acceltimes = 0;
                
                freqend     = lsChTimeFliterBuf[i].freq[CH_PARA_BUF_SIZE - 1];
                freqstart   = lsChTimeFliterBuf[i].freq[0];
                Acceleration =  freqend * (freqend - freqstart) / (60*CH_PARA_BUF_SIZE);     //������ٶ�
                
                u8  accelbufsize = CH_PARA_BUF_SIZE;
                App_FillAndMoveBuf32((u32 *)lsChTimeFliterBuf[i].Acceleration,accelbufsize,Acceleration);   //������д��buf���������CH_PARA_BUF_SIZE������

                if(acceltimes > accelbufsize){
                    acceltimes = 0;
                    s8  addtimes    = 0;
                    int accelsum    = 0;
                    
                    for(u8 j= 0;j < accelbufsize  ;j++ ){
                        
                        accelsum += lsChTimeFliterBuf[i].Acceleration[j];
                        if(lsChTimeFliterBuf[i].Acceleration[j] > 100 ){
                            addtimes++;
                        }else if(lsChTimeFliterBuf[i].Acceleration[j] < -100 ){
                            addtimes--;
                        }
                    }
                    
                    if(abs(addtimes) == accelbufsize ){                         //ȫ����ȫ��
                        Ctrl.ch.para[i].status.acceleration = accelsum/(accelbufsize * 100 );         
                        
                    }else{
                        Ctrl.ch.para[i].status.acceleration = 0;
                    }
                }
                acceltimes++;
                    
            }

            /*******************************************************************************
            * Description  : ����ռ�ձ�(xx.xx%)��( hig_down -  low_up ) / period
            * Author       : 2018/3/13 ���ڶ�, by redmorningcn
            *******************************************************************************/
            now_time    = Ctrl.ch.test[i].time[p_read].hig_up_time;
            now_cnt     = Ctrl.ch.test[i].time[p_read].hig_up_cnt;
            
            next_time   = Ctrl.ch.test[i].time[p_read].hig_down_time;
            next_cnt    = Ctrl.ch.test[i].time[p_read].hig_down_cnt;
            
            if(now_time <= next_time)
            {
                ratiotime = (next_time - now_time) * 65536 + next_cnt - now_cnt;                    //redmonringcn 20180719 ȡ��64λ�˷�����
                
                while(ratiotime < 0)
                    ratiotime += 65536;
                
                App_FillAndMoveBuf32((u32 *)lsChTimeFliterBuf[i].ratio,CH_PARA_BUF_SIZE,ratiotime);  //������д��buf���������CH_PARA_BUF_SIZE������
                
                if(pparabuf[i] > 10)
                    ratiotime = App_GetFilterValue32((u32 *)lsChTimeFliterBuf[i].ratio, timetmpbuf, CH_PARA_BUF_SIZE, CH_PARA_BUF_SIZE/3, 0);    //���ݹ���
                
                if(periodtime)
                    Ctrl.ch.para[i].ratio = ( (uint64  )ratiotime * 100 * 100 ) / (periodtime ) ;                 
            }
            
            /*******************************************************************************
            * Description  : ���������أ�0.01us��
            * Author       : 2018/3/13 ���ڶ�, by redmorningcn
            *******************************************************************************/
            now_time    = Ctrl.ch.test[i].time[p_read].low_up_time;
            now_cnt     = Ctrl.ch.test[i].time[p_read].low_up_cnt;
            
            next_time   = Ctrl.ch.test[i].time[p_read].hig_up_time;
            next_cnt    = Ctrl.ch.test[i].time[p_read].hig_up_cnt;
            
            if(now_time <= next_time)
            {
                raisetime = (next_time - now_time) * 65536 + next_cnt - now_cnt;                    //redmonringcn 20180719 ȡ��64λ�˷�����
                
                while(raisetime < 0)
                    raisetime += 65536;
                
                App_FillAndMoveBuf32((u32 *)lsChTimeFliterBuf[i].raise,CH_PARA_BUF_SIZE,raisetime);  //������д��buf���������CH_PARA_BUF_SIZE������
                
                if(pparabuf[i] > CH_PARA_BUF_SIZE)
                    raisetime = App_GetFilterValue32((u32 *)lsChTimeFliterBuf[i].raise, timetmpbuf, CH_PARA_BUF_SIZE, CH_PARA_BUF_SIZE/3, 0);    //���ݹ���
                
                Ctrl.ch.para[i].raise = CH_EDGE_CALI*(raisetime *100) / (Ctrl.sys.cpu_freq / (1000*1000)); 

                if(Ctrl.ch.para[i].raise  > 1500)
                    Ctrl.ch.para[i].raise  = 1500;
            }
            
            /*******************************************************************************
            * Description  : �����½���(0.01us)
            * Author       : 2018/3/13 ���ڶ�, by redmorningcn
            *******************************************************************************/
            now_time    = Ctrl.ch.test[i].time[p_read].hig_down_time;
            now_cnt     = Ctrl.ch.test[i].time[p_read].hig_down_cnt;
            
            next_time   = Ctrl.ch.test[i].time[p_read].low_down_time;
            next_cnt    = Ctrl.ch.test[i].time[p_read].low_down_cnt;
            
            if(now_time <= next_time)
            {
                failtime = (next_time - now_time) * 65536 + next_cnt - now_cnt; //redmonringcn 20180719 ȡ��64λ�˷�����
                
                while(failtime < 0)
                    failtime += 65536;
                
                App_FillAndMoveBuf32((u32 *)lsChTimeFliterBuf[i].fail,CH_PARA_BUF_SIZE,failtime);  //������д��buf���������CH_PARA_BUF_SIZE������
                
                if(pparabuf[i] > CH_PARA_BUF_SIZE)
                    failtime = App_GetFilterValue32((u32 *)lsChTimeFliterBuf[i].fail, timetmpbuf, CH_PARA_BUF_SIZE, CH_PARA_BUF_SIZE/3, 0);    //���ݹ���
                                
                Ctrl.ch.para[i].fail = CH_EDGE_CALI*(failtime *100) / (Ctrl.sys.cpu_freq / (1000*1000)); 

                if(Ctrl.ch.para[i].fail < 25 )  {                               //r����������·��� 180712 (��ȥ�������)
                    Ctrl.ch.para[i].fail /=  5;
                }else if(Ctrl.ch.para[i].fail < 45){
                    Ctrl.ch.para[i].fail -= 25;
                    
                    Ctrl.ch.para[i].fail = 5 + (Ctrl.ch.para[i].fail / 4);
                }else{
                    Ctrl.ch.para[i].fail -= 45;
                    
                    if(Ctrl.ch.para[i].fail < ( 5 + 5 ))
                        Ctrl.ch.para[i].fail = 5 + 5;
                }
                
                if(Ctrl.ch.para[i].fail  > 1500)
                    Ctrl.ch.para[i].fail  = 1500;
            }

            /*******************************************************************************
            * Description  : ������λ��(xx.xx��)
            * Author       : 2018/3/13 ���ڶ�, by redmorningcn
            *******************************************************************************/
            if(i == 1)      
            {
                if(Ctrl.ch.para[1].freq  && Ctrl.ch.para[0].freq ){
                    
                    u8  ptmp    =(Ctrl.ch.test[0].p_write + CH_TIMEPARA_BUF_SIZE -1) % CH_TIMEPARA_BUF_SIZE ;   //redmonringcn 20180719 ��λ��ʵʱ����
                    now_time    = Ctrl.ch.test[0].time[ptmp].low_up_time;
                    now_cnt     = Ctrl.ch.test[0].time[ptmp].low_up_cnt;
                    
                    ptmp        =(Ctrl.ch.test[1].p_write + CH_TIMEPARA_BUF_SIZE -1) % CH_TIMEPARA_BUF_SIZE ;
                    next_time   = Ctrl.ch.test[1].time[ptmp].low_up_time;
                    next_cnt    = Ctrl.ch.test[1].time[ptmp].low_up_cnt;
                    
                    phasetime = (next_time - now_time) * 65536 + next_cnt - now_cnt;    //redmonringcn 20180719 ȡ��64λ�˷�����
                    
                    while(phasetime < 0)
                        phasetime += periodtime;
                    
                    App_FillAndMoveBuf32((u32 *)lsChTimeFliterBuf[i].phase,CH_PARA_BUF_SIZE,phasetime);  //������д��buf���������CH_PARA_BUF_SIZE������
                    
                    if(pparabuf[i] > CH_PARA_BUF_SIZE)
                        phasetime = App_GetFilterValue32((u32 *)lsChTimeFliterBuf[i].phase, timetmpbuf, CH_PARA_BUF_SIZE, CH_PARA_BUF_SIZE/3, 0);    //���ݹ���
                    
                    if(periodtime)
                        Ctrl.ch.ch1_2phase = ((uint64)phasetime * 360 * 100) / periodtime ; 

                    if( Ctrl.ch.ch1_2phase   >  360*100)
                         Ctrl.ch.ch1_2phase   =  360*100;
                }
            }

            /**************************************************************
            * Description  : �������������ж�
            �жϷ������Ա�ǰ���������ڣ��������ƫ�����%50����Ϊ�����塣
            * Author       : 2018/5/30 ������, by redmorningcn
            */
            static  u32     plusetimes[2]   ={0,0};         //����������������Ӳ�������ʼ����
            static  u8      perioderrcnt[2] ={0,0};        //���ڴ����������ÿ����һ�δ����1��һȦ�����ڶ������������
            static  u32     lastperiod[2]   ={0,0};         //�ϴ�����
            
            //ǰ�����ڶԱ�
            if(pri_period[i] * (Ctrl.sys.periodcali / 100) < lastperiod[i] ){
                plusetimes[i] = 0;
                perioderrcnt[i]++;
                
                if(perioderrcnt[i] > (Ctrl.sys.loseerrtimes)){
                    Ctrl.ch.para[i].status.lose = 1;   
                }
            }else{
                plusetimes[i]++;                        
                
                if(plusetimes[i] > CIRCLE_PLUSE_NUM*(1.05) )     // ��������Ĵ��󣬾���1Ȧ��δ�ڷ���������Ϊ�����С�
                {
                    perioderrcnt[i] = 0;                        // �������������
                    Ctrl.ch.para[i].status.lose = 0;   
                }
            }
            lastperiod[i] = pri_period[i];                      // �´��ж�ʹ��
            
            //��ָ��++
            Ctrl.ch.test[i].p_read++ ;
            Ctrl.ch.test[i].p_read %= CH_TIMEPARA_BUF_SIZE; 
            
            //ȡ�źŵĸߵ͵�ƽ
            
            if(pparabuf[i] < CH_PARA_BUF_SIZE +5)
                pparabuf[i]++;
        }
    }
}

#define     CALC_NUM_VOL  (10)


/*******************************************************************************
* Description  : ȡ�źŵ�ƽ���ֵ
* Author       : 2018/3/29 ������, by redmorningcn
*******************************************************************************/
void    app_calc_ch_voltagepara(void)
{
    static  uint8    i;
    static  uint8   lockflg[2]={0,0};   
    uint16   p_wr;
    uint16   p_rd;
    static  u32 systime     = 0;
    static  u8  nocalatimes = 0; 
    
    uint8    calcnum         = CALC_NUM_VOL;    //�˲�����
    uint8    divisor         = 3;
    uint16               pin[2] ={GPIO_Pin_7,GPIO_Pin_9};
    GPIO_TypeDef *       port[2]={GPIOC     ,GPIOC}; 
    
    /**************************************************************
    * Description  : ͨ����ѭ��ִ��
    * Author       : 2018/7/19 ������, by redmorningcn
    */
    i++;
    i %=2;
    //for(i = 0;i< 2;i++)
    {
        /*******************************************************************************
        * Description  : ������������λ�ã��ɼ��ߵ͵�ƽ
                        �ߵ�ƽ�������أ�90%��ɼ���
                        �͵�ƽ���½��أ�10%��ɼ���
                        �ڸߡ��͵�ƽ���ɼ�һ�κ�lockflgʵ�֣����Ž����������ݲɼ���
        * Author       : 2018/3/29 ������, by redmorningcn
        *******************************************************************************/
        p_wr = Ctrl.ch.test[i].p_wr_vol;

        
        if(( Ctrl.ch.test[i].station.fall_90)   && lockflg[i] == 0 )    
        {
            if(GPIO_ReadInputDataBit(port[i],pin[i]) == RESET){       //ȷ�����ڸ�λ
                lockflg[i] = 1;                                      //�����ź�
                
                Ctrl.ch.test[i].voltage[p_wr].ch_low_voltage =   Get_ADC(ADC_Channel_10+i);
            }
        }
        
        if((Ctrl.ch.test[i].station.raise_10) && lockflg[i]  )
        //if(Ctrl.ch.test[i].station.raise_10 && lockflg[i]  )
        {
            if(GPIO_ReadInputDataBit(port[i],pin[i]) == SET){      //ȷ�����ڵ�λ
                
                lockflg[i] = 0;                                       //�����ź�,����ȡ��͵�ƽ����ִ��
                
                Ctrl.ch.test[i].voltage[p_wr].ch_hig_voltage = Get_ADC(ADC_Channel_10+i);
                
                /*******************************************************************************
                * Description  : ȡ�����ƽ��дָ��++
                * Author       : 2018/3/29 ������, by redmorningcn
                *******************************************************************************/
                if(i == 1)  //ȡ�����ƽ
                    Ctrl.ch.test[i].voltage[p_wr].vcc_hig_voltage = Get_ADC(ADC_Channel_10+2);
                
                p_wr++;
                
                Ctrl.ch.test[i].p_wr_vol = p_wr % CH_VOLTAGE_BUF_SIZE;
            }
        }
        
        /*******************************************************************************
        * Description  : ����ߵ͵�ƽ
        * Author       : 2018/3/29 ������, by redmorningcn
        *******************************************************************************/
        p_wr = Ctrl.ch.test[i].p_wr_vol;
        p_rd = Ctrl.ch.test[i].p_rd_vol;

        if(     ( p_wr > p_rd) &&  (p_wr > p_rd+ calcnum)           
           ||   ( p_wr < p_rd) &&  (p_wr + CH_VOLTAGE_BUF_SIZE > p_rd+calcnum)           
            )  
        {
            int32       voh,vol,vcc;  
            
            nocalatimes     = 0;                                                //
            systime         = Ctrl.sys.time;
            
            /*******************************************************************************
            * Description  : ��10�����У���ȥ���ֵ����Сֵ����ȡƽ��
            * Author       : 2018/3/29 ������, by redmorningcn
            *******************************************************************************/
            //����͵�ƽ
            u16   buftmp[CALC_NUM_VOL];
            
            for(u8 j=0;j < calcnum;j++){
                buftmp[j] = Ctrl.ch.test[i].voltage[(p_rd + j)%CH_VOLTAGE_BUF_SIZE].ch_low_voltage;
            }
            vol = App_GetFilterValue(buftmp, buftmp, calcnum, calcnum/divisor, 0);
            
                
            //����ߵ�ƽ
            for(u8 j=0;j < 10;j++){
                buftmp[j] = Ctrl.ch.test[i].voltage[(p_rd + j)%CH_VOLTAGE_BUF_SIZE].ch_hig_voltage;
            }
            voh = App_GetFilterValue(buftmp, buftmp, calcnum, calcnum/divisor, 0);
            

            //���㹩���Դ
            if(i == 1)
            {
                //����ߵ�ƽ
                for(u8 j=0;j < 10;j++){
                    buftmp[j] = Ctrl.ch.test[i].voltage[(p_rd + j)%CH_VOLTAGE_BUF_SIZE].vcc_hig_voltage;
                }
                vcc = App_GetFilterValue(buftmp, buftmp, calcnum, calcnum/divisor, 0);
                
                Ctrl.ch.vcc_vol     = (vcc * Ctrl.calitab.VccVol.line / CALI_LINE_BASE)       + Ctrl.calitab.VccVol.Delta;
            }
            
            /**************************************************************
            * Description  : ����ֵ����
            * Author       : 2018/5/30 ������, by redmorningcn
            */
            Ctrl.ch.para[i].Vol = (vol * Ctrl.calitab.CaliBuf[i+1].line / CALI_LINE_BASE) + Ctrl.calitab.CaliBuf[i+1].Delta;
            Ctrl.ch.para[i].Voh = (voh * Ctrl.calitab.CaliBuf[i+1].line / CALI_LINE_BASE) + Ctrl.calitab.CaliBuf[i+1].Delta;
            
            /*******************************************************************************
            * Description  : ������ָ��
            * Author       : 2018/3/29 ������, by redmorningcn
            *******************************************************************************/
            Ctrl.ch.test[i].p_rd_vol = (p_rd + calcnum) % CH_VOLTAGE_BUF_SIZE;
        }
    }
    
    /**************************************************************
    * Description  : �����ʱ��δ���вɼ��ź�VCC��
    * Author       : 2018/5/30 ������, by redmorningcn
    */
    if(Ctrl.sys.time  > systime + 1000){
        systime = Ctrl.sys.time;
        nocalatimes++;
        
        if(nocalatimes >20 ){                           //���źţ��������òο���ѹ
            nocalatimes = 0;
            int32 vcc = Get_ADC(ADC_Channel_10+2);      //�ɼ���ѹ
            if(vcc < 1550)                              //С��15V
                Ctrl.ch.vcc_vol     = (vcc * Ctrl.calitab.VccVol.line / CALI_LINE_BASE) + Ctrl.calitab.VccVol.Delta;
            else
                Ctrl.ch.vcc_vol     = Ctrl.sys.ref_limitvol_min;    
        }
    }
}


/*******************************************************************************
* Description  : �ٶ��źŵ��쳣�ж�
* Author       : 2018/4/13 ������, by redmorningcn
*******************************************************************************/
void    app_ch_judge(void)
{
    static  uint32  tick;
    static  uint32  errcnt[2];
    uint8    i;
    static  int pluse[2];
        
    if(Ctrl.sys.time > tick + 300 || Ctrl.sys.time < tick)    //300ms�ж�һ��
    {
        tick = Ctrl.sys.time;
            
        /**************************************************************
        * Description  : ���ź��ж�
        * Author       : 2018/7/18 ������, by redmorningcn
        */
        for(i = 0; i < 2;i++)
        {
            if(pluse[i] !=  Ctrl.ch.test[i].pulse_cnt)
            {
                errcnt[i] = 0;
                
                Ctrl.ch.para[i].status.nopluse = 0;            //�������ź�        
            }
            else
            {   
                if(Ctrl.ch.para[i].status.nopluse == 0){       //����̖
                    errcnt[i]++;
                    
                    if(errcnt[i] > 3)                           //�쳣������(�ñ�ʶλ���������㣬�������ź�)
                    {
                        Ctrl.ch.para[i].fail     = 0;
                        Ctrl.ch.para[i].freq     = 0;
                        Ctrl.ch.para[i].period   = 0;
                        Ctrl.ch.para[i].raise    = 0;
                        Ctrl.ch.para[i].ratio    = 0;
                        Ctrl.ch.para[i].status.flags    = 0;
                        Ctrl.ch.para[i].status.nopluse  = 1;    //�������ź�
                        Ctrl.ch.para[i].Voh      = 0;
                        Ctrl.ch.para[i].Vol      = 0;
                        
                        Ctrl.ch.ch1_2phase   = 0;
                    }
                }
            }
            pluse[i] = Ctrl.ch.test[i].pulse_cnt;
        }
    }
}


/*******************************************************************************
* 				end of file
*******************************************************************************/
