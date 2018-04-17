/*******************************************************************************
* Description  : 脉冲信息处理。
                 记录脉冲信号产生的时间信息。通过外部捕获及定时器配合实现
* Author       : 2018/4/12 星期四, by redmorningcn
*******************************************************************************/


/*******************************************************************************
* INCLUDES
*/
#include <bsp_wave_detect.h>



/*******************************************************************************
* Description  : 节约中断时间，处理函数只赋值。
                 1、取系统定时器时间sys.time
                 2、取捕获计算器产生时间cnt，捕获产生的实际时间为sys.time * 65536 + cnt;
                 3、方波信号记录，如下时间，波形参数时间确定 
                    上升沿在10%位置时间；
                    上升沿在90%位置时间；
                    下降沿在10%位置时间；
                    下降沿在90%位置时间。
* Author       : 2018/3/16 星期五, by redmorningcn
*******************************************************************************/
void TIM8_CC_IRQHandler(void)
{

    uint16      cnt;
    u32         time;                                //时间等于 sys.time * 65536+TIM_CNT     
    
    //cnt  = TIM_CNT;
    time = sys.time;                                //时间等于 sys.time * 65536+TIM_CNT     
	
	if(TIM8->SR&0x04)								//CH2捕获中断 
	{
        cnt = TIM8->CCR2;

        if(GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_7) == SET)      //高位（90%）的上升触发
        {     
            TIM_OC2PolarityConfig(TIM8,TIM_ICPolarity_Falling); //设置为下降沿捕获			

            ch.test[0].time[ch.test[0].p_write].hig_up_time  = time;     
            ch.test[0].time[ch.test[0].p_write].hig_up_cnt   = cnt;   
            
            ch.test[0].pluse_status = CH_RAISE_90_STATUS;       //上升沿，90%
        }
        else                                                    //高位（90%）下降沿触发
        {    
            TIM_OC2PolarityConfig(TIM8,TIM_ICPolarity_Rising);  //设置为上升沿捕获

            ch.test[0].time[ch.test[0].p_write].hig_down_time   = time;     
            ch.test[0].time[ch.test[0].p_write].hig_down_cnt    = cnt;   
            
            ch.test[0].pluse_status = CH_FALL_90_STATUS;       //下降沿，90%
        }
        
        TIM_ClearITPendingBit(TIM8,TIM_IT_CC2);                 //清除中断标志
	}
    
	if(TIM8->SR&0x02)								            //CC1捕获中断 
	{

        cnt = TIM8->CCR1;
        if(GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_6) == SET)      //低位（10%）的上升触发
        {  
            TIM_OC1PolarityConfig(TIM8,TIM_ICPolarity_Falling);//设置为下降沿捕获			

            ch.test[0].time[ch.test[0].p_write].low_up_time    =  time;     
            ch.test[0].time[ch.test[0].p_write].low_up_cnt     =  cnt;    
            
            ch.test[0].pluse_status = CH_RAISE_10_STATUS;       //上升沿，10%

        }else                                                   //低位（10%）的下降触发
        {           
            TIM_OC1PolarityConfig(TIM8,TIM_ICPolarity_Rising);  //设置为上升沿捕获

            ch.test[0].time[ch.test[0].p_write].low_down_time    =  time;     
            ch.test[0].time[ch.test[0].p_write].low_down_cnt     =  cnt; 
            
            ch.test[0].pulse_cnt++;                             //周期结束放置在后面
            ch.test[0].p_write           =      ch.test[0].pulse_cnt 
                % CH_TIMEPARA_BUF_SIZE;
            
            ch.test[0].pluse_status = CH_FALL_10_STATUS;       //下降沿，10%
        }
        
        TIM_ClearITPendingBit(TIM8,TIM_IT_CC1);//清除中断标志
	}    
  
	

	if(TIM8->SR&0x10)//CH4捕获中断 在CH4上升沿中断中记录的值即为两波形的上升沿时间差
	{
        
        cnt =  TIM8->CCR4;
        if(GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_9) == SET)      //高位（90%）的上升触发
        {     
            TIM_OC4PolarityConfig(TIM8,TIM_ICPolarity_Falling);//设置为下降沿捕获			

            ch.test[1].time[ch.test[1].p_write].hig_up_time  = time;     
            ch.test[1].time[ch.test[1].p_write].hig_up_cnt   = cnt;   
            
            ch.test[1].pluse_status = CH_RAISE_90_STATUS;       //上升沿，90%
        }
        else                                                    //高位（90%）下降沿触发
        {           
            TIM_OC4PolarityConfig(TIM8,TIM_ICPolarity_Rising);//设置为上升沿捕获		

            ch.test[1].time[ch.test[1].p_write].hig_down_time   = time;     
            ch.test[1].time[ch.test[1].p_write].hig_down_cnt    = cnt; 
            
            ch.test[1].pluse_status = CH_FALL_90_STATUS;       //下降沿，90%
        }
        
        TIM_ClearITPendingBit(TIM8,TIM_IT_CC4);//清除中断标志

	}
    
	if(TIM8->SR&0x08)//CH3捕获中断  
	{
        cnt = TIM8->CCR3;
        if(GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_8) == SET)      //低位（10%）的上升触发
        {  
            TIM_OC3PolarityConfig(TIM8,TIM_ICPolarity_Falling); //设置为下降沿捕获			

            ch.test[1].time[ch.test[1].p_write].low_up_time    =  time;     
            ch.test[1].time[ch.test[1].p_write].low_up_cnt     =  cnt;    
            
            ch.test[1].pluse_status = CH_RAISE_10_STATUS;       //上升沿，10%
        }
        else                                                    //低位（10%）的下降触发
        {     
            TIM_OC3PolarityConfig(TIM8,TIM_ICPolarity_Rising);  //设置为上升沿捕获		

            ch.test[1].time[ch.test[1].p_write].low_down_time    =  time;     
            ch.test[1].time[ch.test[1].p_write].low_down_cnt     =  cnt;  
            
            
            ch.test[1].pulse_cnt++;                             //周期结束放置在后面
            ch.test[1].p_write             =        ch.test[1].pulse_cnt 
                                                    % CH_TIMEPARA_BUF_SIZE;
            
            ch.test[1].pluse_status = CH_FALL_10_STATUS;        //下降沿，10%
        }
            
        TIM_ClearITPendingBit(TIM8,TIM_IT_CC3);                 //清除中断标志
	}
}


/*******************************************************************************
* Description  : 全局时间累积。真实时间 time = strSys.time * 65536 + TIM_GetCounter  
                              再乘以单周期时间。
* Author       : 2018/3/13 星期二, by redmorningcn
*******************************************************************************/
void TIM8_OVER_IRQHandler(void)
{
	if(TIM_GetITStatus(TIM8,TIM_IT_Update)!=RESET)  //计数器溢出中断
	{
		TIM_ClearITPendingBit(TIM8,TIM_IT_Update);  //清除中断标志
        sys.time++;                                 //系统是时间累加
	}
}

/*******************************************************************************
* Description  : 全局时钟，为所有信号提供统一时间标准
* Author       : 2018/3/13 星期二, by redmorningcn
*******************************************************************************/
void Timer8_Iint(void)
{
	TIM_TimeBaseInitTypeDef	TIM_BaseInitStructure;
//	TIM_ICInitTypeDef TIM_ICInitStruct;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM8,ENABLE);         //使能定时器时钟
	//初始化定时器8
	TIM_BaseInitStructure.TIM_Period = 65535;                   //计数器自动重装值
	TIM_BaseInitStructure.TIM_Prescaler = 0;                    //不分频
	TIM_BaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;     //时钟不分割
	TIM_BaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up; //向上计数
	TIM_BaseInitStructure.TIM_RepetitionCounter = 0;            //重复计数设置
    
	TIM_TimeBaseInit(TIM8,&TIM_BaseInitStructure);              //初始化时钟
    
	TIM_ClearFlag(TIM8, TIM_FLAG_Update);                       //清楚中断标志位
	TIM_ITConfig(TIM8,TIM_IT_Update,ENABLE);                    //不允许更新中断
	TIM_Cmd(TIM8,ENABLE);
    
	BSP_IntVectSet(TIM8_UP_IRQn, TIM8_OVER_IRQHandler);
	BSP_IntEn(TIM8_UP_IRQn);
    
    sys.time = 0;                                               //系统时间置0
}



/*******************************************************************************
* Description  : 配置定时器外部捕获
                定时器的双边捕获有bug，不能产生。在中断服务程序中改变触发边缘，实现
                双边捕获功能。
                
* Author       : 2018/3/16 星期五, by redmorningcn
*******************************************************************************/
void Timer8_Configuration(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
//	TIM_TimeBaseInitTypeDef	TIM_BaseInitStructure;
	TIM_ICInitTypeDef TIM_ICInitStruct;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM8,ENABLE);//使能定时器时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC,ENABLE);//使能IO口时钟
	
	GPIO_InitStructure.GPIO_Pin =        GPIO_Pin_6
                                        |GPIO_Pin_7  
                                        |GPIO_Pin_8
                                        |GPIO_Pin_9;
                           
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;//下拉输入
	GPIO_Init(GPIOC,&GPIO_InitStructure);//初始化
	
	//初始化TIM8输入捕获参数
	TIM_ICInitStruct.TIM_Channel = TIM_Channel_1;//输入端映射到TI1
    
	TIM_ICInitStruct.TIM_ICPolarity = TIM_ICPolarity_Rising;//上升沿捕获
    //TIM_ICInitStruct.TIM_ICPolarity = TIM_ICPolarity_BothEdge;//上升沿捕获
    
	TIM_ICInitStruct.TIM_ICSelection = TIM_ICSelection_DirectTI;//映射到TI1上
	TIM_ICInitStruct.TIM_ICPrescaler = TIM_ICPSC_DIV1;//输入不分频
	TIM_ICInitStruct.TIM_ICFilter = 0x00;//输入不滤波
	TIM_ICInit(TIM8,&TIM_ICInitStruct);//初始化TIM8
	
	TIM_ICInitStruct.TIM_Channel = TIM_Channel_2;//输入端映射到TI2
	TIM_ICInitStruct.TIM_ICPolarity = TIM_ICPolarity_Rising;//上升沿捕获
	TIM_ICInitStruct.TIM_ICSelection = TIM_ICSelection_DirectTI;//映射到TI2上
	TIM_ICInitStruct.TIM_ICPrescaler = TIM_ICPSC_DIV1;//输入不分频
	TIM_ICInitStruct.TIM_ICFilter = 0x00;//输入不滤波
	TIM_ICInit(TIM8,&TIM_ICInitStruct);//初始化TIM8
	
	TIM_ICInitStruct.TIM_Channel = TIM_Channel_3;//输入端映射到TI3
	TIM_ICInitStruct.TIM_ICPolarity = TIM_ICPolarity_Rising;//上升沿捕获
	TIM_ICInitStruct.TIM_ICSelection = TIM_ICSelection_DirectTI;//映射到TI3上
	TIM_ICInitStruct.TIM_ICPrescaler = TIM_ICPSC_DIV1;//输入不分频
	TIM_ICInitStruct.TIM_ICFilter = 0x00;//输入不滤波
	TIM_ICInit(TIM8,&TIM_ICInitStruct);//初始化TIM8
	
	TIM_ICInitStruct.TIM_Channel = TIM_Channel_4;//输入端映射到TI4
	TIM_ICInitStruct.TIM_ICPolarity = TIM_ICPolarity_Rising;//上升沿捕获
	TIM_ICInitStruct.TIM_ICSelection = TIM_ICSelection_DirectTI;//映射到TI4上
	TIM_ICInitStruct.TIM_ICPrescaler = TIM_ICPSC_DIV1;//输入不分频
	TIM_ICInitStruct.TIM_ICFilter = 0x00;//输入不滤波
	TIM_ICInit(TIM8,&TIM_ICInitStruct);//初始化TIM8
	
	TIM_ITConfig(TIM8,TIM_IT_Update|TIM_IT_CC1|TIM_IT_CC2|TIM_IT_CC3|TIM_IT_CC4,ENABLE);//不允许更新中断 CC1IE捕获中断
	
	BSP_IntVectSet(TIM8_CC_IRQn, TIM8_CC_IRQHandler);
	BSP_IntEn(TIM8_CC_IRQn);
	
	TIM_Cmd(TIM8,ENABLE);//开启定时器8
}

/*******************************************************************************
* Description  : 通道采样初始化
                初始化全局定时器和采样通道的外部中断；
                以及运行的全局变量；
* Author       : 2018/3/13 星期二, by redmorningcn
*******************************************************************************/
void    init_ch_timepara_detect(void)
{
    Timer8_Iint();              //启动全局定时器
    
    Timer8_Configuration();     //初始化运算参数，初始外部捕获功能
}


/*******************************************************************************
* 				end of file
*******************************************************************************/
