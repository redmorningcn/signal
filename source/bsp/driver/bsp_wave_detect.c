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
* Description  : 全局时间累积。真实时间 time = strSys.time * 65536 + TIM_GetCounter  
                              再乘以单周期时间。65536/72000000
* Author       : 2018/3/13 星期二, by redmorningcn
                 2018/11/12 将中断函数处理直接写成寄存器处理，提高响应速度
*******************************************************************************/
void TIM8_OVER_IRQHandler(void)
{
//	if(TIM_GetITStatus(TIM8,TIM_IT_Update)!=RESET)  //计数器溢出中断
//	{
//		TIM_ClearITPendingBit(TIM8,TIM_IT_Update);  //清除中断标志
//        Ctrl.sys.time++;                           //系统是时间累加
//	}
    
    if(TIM8->SR & TIM_IT_Update != (uint16_t)RESET){       //计数器溢出中断
        TIM8->SR = (uint16_t)~TIM_IT_Update;                //清除中断标志
        Ctrl.sys.time++;             
        //Ctrl.sys.num++;    
        //系统是时间累加
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
	TIM_BaseInitStructure.TIM_Period    = 7200-1;                   //计数器自动重装值
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
    
    /**************************************************************
    * Description  : 设置中断优先级
    * Author       : 2018/7/17 星期二, by redmorningcn
    */
    NVIC_InitTypeDef NVIC_InitStructure; 
    NVIC_InitStructure.NVIC_IRQChannel = TIM8_UP_IRQn;  //
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0 ;    // 抢占优先级为0 
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;          // 子优先级位0 
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;             // IRQ通道使能

    NVIC_Init(&NVIC_InitStructure);                             // 根据上面指定的参数初始化NVIC寄存器    
    
    Ctrl.sys.time = 0;                                          // 系统时间置0
}


void TIM2_IRQHandler(void)
{

    if(TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET)
    {
         //interrupt_rtc();//可以使你自己定义的执行函数
        switch (Ctrl.sys.CHtime %4) 
        {       
          case 0:
                  BSP_LED_On(1);
                  BSP_LED_Off(2);
                  BSP_LED_On(3);
                  BSP_LED_Off(4);
                  BSP_LED_On(5);
                  BSP_LED_Off(6);
                  break;
          case 1:
                  BSP_LED_On(1);
                  BSP_LED_On(2);
                  BSP_LED_On(3);
                  BSP_LED_On(4);
                  BSP_LED_On(5);
                  BSP_LED_On(6);
                  break;
          case 2:
                  BSP_LED_Off(1);
                  BSP_LED_On(2);
                  BSP_LED_Off(3);
                  BSP_LED_On(4);
                  BSP_LED_Off(5);
                  BSP_LED_On(6);
                  break;
              
          case 3:
                  BSP_LED_Off(1);
                  BSP_LED_Off(2);
                  BSP_LED_Off(3);
                  BSP_LED_Off(4);
                  BSP_LED_Off(5);
                  BSP_LED_Off(6);
                  break;
        }
        Ctrl.sys.CHtime++;
    }
     TIM_ClearITPendingBit(TIM2, TIM_FLAG_Update);
}


/*******************************************************************************
 * 作　　 者：       redmoringcn
 * 创建日期：        2022-02-17
 * 修    改： 
 * 修改日期：       
 *  中断时间Tout= ((arr+1)*(psc+1))/Tclk；
 *   feq:  Tclk  / ((arr+1)*(psc+1)) 
 *   脉冲产生频率 = feq  / 4
 *******************************************************************************/
void TIM2_Change_Freq(void)
{
    TIM_TimeBaseInitTypeDef    TIM_TimeBaseStructure;

    u16  period = 0;
    u16  prescaler = 0;
    u32  calcfreq = 0;
    u32  Tclk = Ctrl.sys.cpu_freq;
    
    if(Ctrl.sys.paraflg.ChangeFreqflg == 1){
      Ctrl.sys.paraflg.ChangeFreqflg = 0;
      
      Ctrl.sys.setfrq = (Ctrl.sys.setSpeed * 1000 * Ctrl.sys.PluseNum)/ (3.14 * Ctrl.sys.LocoDim * 3.6); 
      calcfreq = Ctrl.sys.setfrq * 4;                     //脉冲中断频率 = 设置频率 * 4
      
      u8 i = 0;
      while(i < 6){
        Ctrl.sys.speedCH[i++] = Ctrl.sys.setfrq;                //暂定速度值等于频率
      }
        
      if(Ctrl.sys.setfrq > 1000){
        prescaler = 72;
      }else if (Ctrl.sys.setfrq > 100){
        prescaler = 720;
      }else {
        prescaler = 7200;
      }
      period  = Tclk/(prescaler * calcfreq);
      if (period < 1){
        period = 1;
      }
    
      TIM_TimeBaseStructure.TIM_Period    = period - 1;          //2000 - 1;
      TIM_TimeBaseStructure.TIM_Prescaler = prescaler -1;
      TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
      TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;

      TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);
    }
}
/*******************************************************************************
 * 名    称：        pwm_init_Time2()
 * 功    能：        定时器2波形产生初始化
 * 入口参数：
 * 出口参数：        无
 * 作　　 者：       redmoringcn
 * 创建日期：        2021-11-30
 * 修    改： 
 * 修改日期：       
 *  中断时间Tout= ((arr+1)*(psc+1))/Tclk；
 *   feq:  Tclk  / ((arr+1)*(psc+1)) 
 *******************************************************************************/
void TIM2_Int_Init(void)
{
    TIM_TimeBaseInitTypeDef    TIM_TimeBaseStructure;
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

    TIM_DeInit(TIM2);

    TIM_TimeBaseStructure.TIM_Period = 10 - 1;//2000 - 1;
    TIM_TimeBaseStructure.TIM_Prescaler = (7200 - 1);
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;

    TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);

    TIM_ClearFlag(TIM2, TIM_FLAG_Update);
    TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);

    /***********************************************
    * 描述：设置中断入口函数及中断优先级
    */
    BSP_IntVectSet(BSP_INT_ID_TIM2, TIM2_IRQHandler);
    BSP_IntEn(BSP_INT_ID_TIM2);
    
    TIM_Cmd(TIM2, ENABLE);
    Ctrl.sys.CHtime = 0;
}


///*******************************************************************************
//* Description  : 配置定时器外部捕获
//                定时器的双边捕获有bug，不能产生。在中断服务程序中改变触发边缘，实现
//                双边捕获功能。
//                
//* Author       : 2018/3/16 星期五, by redmorningcn
//*******************************************************************************/
//void Timer8_Configuration(void)
//{
//	GPIO_InitTypeDef GPIO_InitStructure;
////	TIM_TimeBaseInitTypeDef	TIM_BaseInitStructure;
//	TIM_ICInitTypeDef TIM_ICInitStruct;
//	
//	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM8,ENABLE);//使能定时器时钟
//	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC,ENABLE);//使能IO口时钟
//	
//	GPIO_InitStructure.GPIO_Pin =        GPIO_Pin_6
//                                        |GPIO_Pin_7  
//                                        |GPIO_Pin_8
//                                        |GPIO_Pin_9;
//                           
//	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;//下拉输入
//	GPIO_Init(GPIOC,&GPIO_InitStructure);//初始化
//	
//	//初始化TIM8输入捕获参数
//	TIM_ICInitStruct.TIM_Channel = TIM_Channel_1;//输入端映射到TI1
//    
//	TIM_ICInitStruct.TIM_ICPolarity = TIM_ICPolarity_Rising;//上升沿捕获
//    //TIM_ICInitStruct.TIM_ICPolarity = TIM_ICPolarity_BothEdge;//上升沿捕获
//    
//	TIM_ICInitStruct.TIM_ICSelection = TIM_ICSelection_DirectTI;//映射到TI1上
//	TIM_ICInitStruct.TIM_ICPrescaler = TIM_ICPSC_DIV1;//输入不分频
//	TIM_ICInitStruct.TIM_ICFilter = 0x00;//输入不滤波
//	TIM_ICInit(TIM8,&TIM_ICInitStruct);//初始化TIM8
//	
//	TIM_ICInitStruct.TIM_Channel = TIM_Channel_2;//输入端映射到TI2
//	TIM_ICInitStruct.TIM_ICPolarity = TIM_ICPolarity_Rising;//上升沿捕获
//	TIM_ICInitStruct.TIM_ICSelection = TIM_ICSelection_DirectTI;//映射到TI2上
//	TIM_ICInitStruct.TIM_ICPrescaler = TIM_ICPSC_DIV1;//输入不分频
//	TIM_ICInitStruct.TIM_ICFilter = 0x00;//输入不滤波
//	TIM_ICInit(TIM8,&TIM_ICInitStruct); //初始化TIM8
//	
//	TIM_ICInitStruct.TIM_Channel = TIM_Channel_3;//输入端映射到TI3
//	TIM_ICInitStruct.TIM_ICPolarity = TIM_ICPolarity_Rising;//上升沿捕获
//	TIM_ICInitStruct.TIM_ICSelection = TIM_ICSelection_DirectTI;//映射到TI3上
//	TIM_ICInitStruct.TIM_ICPrescaler = TIM_ICPSC_DIV1;//输入不分频
//	TIM_ICInitStruct.TIM_ICFilter = 0x00;//输入不滤波
//	TIM_ICInit(TIM8,&TIM_ICInitStruct);//初始化TIM8
//	
//	TIM_ICInitStruct.TIM_Channel = TIM_Channel_4;//输入端映射到TI4
//	TIM_ICInitStruct.TIM_ICPolarity = TIM_ICPolarity_Rising;//上升沿捕获
//	TIM_ICInitStruct.TIM_ICSelection = TIM_ICSelection_DirectTI;//映射到TI4上
//	TIM_ICInitStruct.TIM_ICPrescaler = TIM_ICPSC_DIV1;//输入不分频
//	TIM_ICInitStruct.TIM_ICFilter = 0x00;//输入不滤波
//	TIM_ICInit(TIM8,&TIM_ICInitStruct);//初始化TIM8
//	
//    /**************************************************************
//    * Description  : 设置中断优先级
//    * Author       : 2018/7/17 星期二, by redmorningcn
//    */
//    NVIC_InitTypeDef NVIC_InitStructure; 
//    NVIC_InitStructure.NVIC_IRQChannel = TIM8_CC_IRQn;  //
//    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0 ;// 抢占优先级为0 
//    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;      // 子优先级位0 
//    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;         //IRQ通道使能
//
//    NVIC_Init(&NVIC_InitStructure);                         //根据上面指定的参数初始化NVIC寄存器    
//    
//	TIM_ITConfig(TIM8,TIM_IT_Update|TIM_IT_CC1|TIM_IT_CC2|TIM_IT_CC3|TIM_IT_CC4,ENABLE);//不允许更新中断 CC1IE捕获中断
//	
//	BSP_IntVectSet(TIM8_CC_IRQn, TIM8_CC_IRQHandler);
//	BSP_IntEn(TIM8_CC_IRQn);
//	
//	TIM_Cmd(TIM8,ENABLE);                                   //开启定时器8
//}

///*******************************************************************************
//* Description  : 配置定时器PWM
//。
//                
//* Author       : 2021/11/02 星期五, by redmorningcn
//*******************************************************************************/
//void PWM_Cfg(float dutyfactor1,float dutyfactor2,float dutyfactor3,float dutyfactor4)
//{
//	GPIO_InitTypeDef GPIO_InitStructure;
////	TIM_TimeBaseInitTypeDef	TIM_BaseInitStructure;
//	TIM_ICInitTypeDef TIM_ICInitStruct;
//    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
//	
//    RCC_APB2PeriphClockCmd(RCC_APB1Periph_TIM3,ENABLE);//使能定时器时钟
//    //预分频系数为0，即不进行预分频，此时TIMER的频率为72MHzre.TIM_Prescaler =0;
//    TIM_TimeBaseStructure.TIM_Prescaler = 0;
//    //设置计数溢出大小，每计20000个数就产生一个更新事件
//    TIM_TimeBaseStructure.TIM_Period = 7200 - 1;
//    //设置时钟分割
//    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
//    //设置计数器模式为向上计数模式
//    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
//
//    //将配置应用到TIM3中
//    TIM_TimeBaseInit(TIM3,&TIM_TimeBaseStructure);    
//    
//	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC,ENABLE);//使能IO口时钟
//    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);
//	
//	GPIO_InitStructure.GPIO_Pin =        GPIO_Pin_6
//                                        |GPIO_Pin_7  
//                                        |GPIO_Pin_8
//                                        |GPIO_Pin_9;
//                       
//    GPIO_PinRemapConfig(GPIO_FullRemap_TIM3, ENABLE);
//    
//    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;//推挽输出
//	GPIO_Init(GPIOC,&GPIO_InitStructure);//初始化
//	
//    TIM_OCInitTypeDef TIM_OCInitStructure;
//      //设置缺省值
//    TIM_OCStructInit(&TIM_OCInitStructure);
//      
//      //TIM3的CH1输出
//    TIM_OCInitStructure.TIM_OCMode       = TIM_OCMode_PWM1;             //设置是PWM模式还是比较模式 
//    TIM_OCInitStructure.TIM_OutputState  = TIM_OutputState_Enable;      //比较输出使能，使能PWM输出到端口
//    TIM_OCInitStructure.TIM_OCPolarity   = TIM_OCPolarity_High;         //设置极性是高还是低
//      //设置占空比，占空比=(CCRx/ARR)*100%或(TIM_Pulse/TIM_Period)*100%
//    TIM_OCInitStructure.TIM_Pulse = dutyfactor1 * 7200 / 100;
//    TIM_OC1Init(TIM3, &TIM_OCInitStructure);
//      
//      //TIM3的CH2输出
//    TIM_OCInitStructure.TIM_OCMode       = TIM_OCMode_PWM1; //设置是PWM模式还是比较模式 
//    TIM_OCInitStructure.TIM_OutputState  = TIM_OutputState_Enable; //比较输出使能，使能PWM输出到端口
//    TIM_OCInitStructure.TIM_OCPolarity   = TIM_OCPolarity_High; //设置极性是高还是低
//      //设置占空比，占空比=(CCRx/ARR)*100%或(TIM_Pulse/TIM_Period)*100%
//    TIM_OCInitStructure.TIM_Pulse = dutyfactor2 * 7200 / 100;
//    TIM_OC2Init(TIM3, &TIM_OCInitStructure);
//      
//   
//      //TIM3的CH3输出
//    TIM_OCInitStructure.TIM_OCMode       = TIM_OCMode_PWM1; //设置是PWM模式还是比较模式 
//    TIM_OCInitStructure.TIM_OutputState  = TIM_OutputState_Enable; //比较输出使能，使能PWM输出到端口
//    TIM_OCInitStructure.TIM_OCPolarity   = TIM_OCPolarity_High; //设置极性是高还是低
//      //设置占空比，占空比=(CCRx/ARR)*100%或(TIM_Pulse/TIM_Period)*100%
//    TIM_OCInitStructure.TIM_Pulse = dutyfactor3 * 7200 / 100;
//    TIM_OC3Init(TIM3, &TIM_OCInitStructure);
//      
//      
//      //TIM3的CH4输出
//    TIM_OCInitStructure.TIM_OCMode       = TIM_OCMode_PWM1; //设置是PWM模式还是比较模式 
//    TIM_OCInitStructure.TIM_OutputState  = TIM_OutputState_Enable; //比较输出使能，使能PWM输出到端口
//    TIM_OCInitStructure.TIM_OCPolarity   = TIM_OCPolarity_High; //设置极性是高还是低
//      //设置占空比，占空比=(CCRx/ARR)*100%或(TIM_Pulse/TIM_Period)*100%
//    TIM_OCInitStructure.TIM_Pulse = dutyfactor4 * 7200 / 100;
//    TIM_OC4Init(TIM3, &TIM_OCInitStructure);
//            
//      //使能输出状态
//     TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
//      
//    //设置TIM3的PWM输出为使能
//     TIM_CtrlPWMOutputs(TIM3,ENABLE);
//     TIM_Cmd(TIM3,ENABLE);
//
//}



void GPIO_Cfg(void)
{
    
    GPIO_InitTypeDef GPIO_InitStructure;
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC|RCC_APB2Periph_GPIOA|RCC_APB2Periph_AFIO,ENABLE);
      
   //全部映射，将TIM3_CH2映射到PB5
   //根据STM32中文参考手册2010中第第119页可知：
   //当没有重映射时，TIM3的四个通道CH1，CH2，CH3，CH4分别对应PA6，PA7,PB0,PB1
   //当部分重映射时，TIM3的四个通道CH1，CH2，CH3，CH4分别对应PB4，PB5,PB0,PB1
   //当完全重映射时，TIM3的四个通道CH1，CH2，CH3，CH4分别对应PC6，PC7,PC8,PC9
   //也即是说，完全重映射之后，四个通道的PWM输出引脚分别为PC6，PC7,PC8,PC9，我们用到了通道1和通道2，所以对应引脚为PC6，PC7,PC8,PC9，我们用到了通道1和通道2，所以对应引脚为
    GPIO_PinRemapConfig(GPIO_FullRemap_TIM3, ENABLE);
      
       //部分重映射的参数
       //GPIO_PinRemapConfig(GPIO_PartialRemap_TIM3, ENABLE);
    
    //设置PC6、PC7、PC8、PC9为复用输出，输出4路PWM
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_6|GPIO_Pin_7|GPIO_Pin_8|GPIO_Pin_9;
    GPIO_Init(GPIOC,&GPIO_InitStructure);
}


void TIM_Cfg(void)
{
     //定义结构体
     TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;

       //重新将Timer设置为缺省值
       TIM_DeInit(TIM3);
       //采用内部时钟给TIM2提供时钟源
       //TIM_InternalClockConfig(TIM3);
     
     //预分频系数为0，即不进行预分频，此时TIMER的频率为72MHzre.TIM_Prescaler =0;
       TIM_TimeBaseStructure.TIM_Prescaler = 0;
     //设置计数溢出大小，每计20000个数就产生一个更新事件
       TIM_TimeBaseStructure.TIM_Period = 7200 - 1;
       //设置时钟分割
       TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
       //设置计数器模式为向上计数模式
       TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
       
       //将配置应用到TIM2中
       TIM_TimeBaseInit(TIM3,&TIM_TimeBaseStructure);
       //清除溢出中断标志
       //TIM_ClearFlag(TIM2, TIM_FLAG_Update);
       //禁止ARR预装载缓冲器
       //TIM_ARRPreloadConfig(TIM2, DISABLE);
       //开启TIM2的中断
       //TIM_ITConfig(TIM2,TIM_IT_Update,ENABLE);
    
}




/*******************************************************************************
* 函 数 名         : PWM波产生配置函数
* 函数功能         : PWM_Cfg
* 输    入         : dutyfactor 占空比数值，大小从0.014到100
* 输    出         : 无
*******************************************************************************/
void PWM_Cfg(float dutyfactor1,float dutyfactor2,float dutyfactor3,float dutyfactor4)
{
    TIM_OCInitTypeDef TIM_OCInitStructure;
      //设置缺省值
      TIM_OCStructInit(&TIM_OCInitStructure);
      
      //TIM3的CH1输出
      TIM_OCInitStructure.TIM_OCMode       = TIM_OCMode_PWM1; //设置是PWM模式还是比较模式 
      TIM_OCInitStructure.TIM_OutputState  = TIM_OutputState_Enable; //比较输出使能，使能PWM输出到端口
      TIM_OCInitStructure.TIM_OCPolarity   = TIM_OCPolarity_High; //设置极性是高还是低
      //设置占空比，占空比=(CCRx/ARR)*100%或(TIM_Pulse/TIM_Period)*100%
      TIM_OCInitStructure.TIM_Pulse = dutyfactor1 * 7200 / 100;
      TIM_OC1Init(TIM3, &TIM_OCInitStructure);
      TIM_OC1PreloadConfig(TIM3, TIM_OCPreload_Enable);      
      
      
      //TIM3的CH2输出
      TIM_OCInitStructure.TIM_OCMode       = TIM_OCMode_PWM1; //设置是PWM模式还是比较模式 
      TIM_OCInitStructure.TIM_OutputState  = TIM_OutputState_Enable; //比较输出使能，使能PWM输出到端口
      TIM_OCInitStructure.TIM_OCPolarity   = TIM_OCPolarity_High; //设置极性是高还是低
      //设置占空比，占空比=(CCRx/ARR)*100%或(TIM_Pulse/TIM_Period)*100%
      TIM_OCInitStructure.TIM_Pulse = dutyfactor2 * 7200 / 100;
      TIM_OC2Init(TIM3, &TIM_OCInitStructure);
      //TIM_OC2PreloadConfig(TIM3, TIM_OCPreload_Enable);      

   
      //TIM3的CH3输出
      TIM_OCInitStructure.TIM_OCMode       = TIM_OCMode_PWM1;           //设置是PWM模式还是比较模式 
      TIM_OCInitStructure.TIM_OutputState  = TIM_OutputState_Enable;    //比较输出使能，使能PWM输出到端口
      TIM_OCInitStructure.TIM_OCPolarity   = TIM_OCPolarity_High;       //设置极性是高还是低
      //设置占空比，占空比=(CCRx/ARR)*100%或(TIM_Pulse/TIM_Period)*100%
      TIM_OCInitStructure.TIM_Pulse = dutyfactor3 * 7200 / 100;
      TIM_OC3Init(TIM3, &TIM_OCInitStructure);
      //TIM_OC3PreloadConfig(TIM3, TIM_OCPreload_Enable);      
      
      //TIM3的CH4输出
      TIM_OCInitStructure.TIM_OCMode       = TIM_OCMode_PWM1;           //设置是PWM模式还是比较模式 
      TIM_OCInitStructure.TIM_OutputState  = TIM_OutputState_Enable;    //比较输出使能，使能PWM输出到端口
      TIM_OCInitStructure.TIM_OCPolarity   = TIM_OCPolarity_High;       //设置极性是高还是低
      //设置占空比，占空比=(CCRx/ARR)*100%或(TIM_Pulse/TIM_Period)*100%
      TIM_OCInitStructure.TIM_Pulse = dutyfactor4 * 7200 / 100;
      TIM_OC4Init(TIM3, &TIM_OCInitStructure);
      //TIM_OC4PreloadConfig(TIM3, TIM_OCPreload_Enable);      
     
      
      //使能输出状态
      TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
     
      //TIM_ARRPreloadConfig(TIM3,ENABLE);
    
      TIM_Cmd(TIM3,ENABLE);
      
    //设置TIM3的PWM输出为使能
      //TIM_CtrlPWMOutputs(TIM3,ENABLE);
}
//
///*******************************************************************************
// * 名    称：        pwm_init_Time4()
// * 功    能：        定时器1波形产生初始化
// * 入口参数：
// * 出口参数：        无
// * 作　　 者：       redmoringcn
// * 创建日期：        2021-11-30
// * 修    改： 
// * 修改日期：        STM32定时器1和定时器8为高级定时器，设置方法和通用定时器稍有区别
// *******************************************************************************/
//void pwm_init_Time4(void)
//{
//    GPIO_InitTypeDef GPIO_InitStructure;
//
//    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
//
//    TIM_OCInitTypeDef TIM_OCInitStructure;
//
//    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE); 
//    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4,ENABLE); 
//    //RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);
//    TIM_DeInit(TIM4);
//    
//    GPIO_InitStructure.GPIO_Pin     =GPIO_Pin_6|GPIO_Pin_7|GPIO_Pin_8; 
//    GPIO_InitStructure.GPIO_Speed   =GPIO_Speed_50MHz; 
//    GPIO_InitStructure.GPIO_Mode    =GPIO_Mode_AF_PP;
//    GPIO_Init(GPIOB,&GPIO_InitStructure);
//
//    TIM_TimeBaseInitStructure.TIM_Period = 7200; //PWM 72000/900=8Khz
//    TIM_TimeBaseInitStructure.TIM_Prescaler = 1000;
//    TIM_TimeBaseInitStructure.TIM_ClockDivision = 0; 
//    TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up; 
//    TIM_TimeBaseInit(TIM4, & TIM_TimeBaseInitStructure);
//
////GPIO_PinRemapConfig(GPIO_FullRemap_TIM3,ENABLE); //管脚映射到LED
//
//    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM2; 
//    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
//    TIM_OCInitStructure.TIM_OutputNState = TIM_OutputState_Disable;
//    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
//    TIM_OCInitStructure.TIM_Pulse = 3600;
//    
//    TIM_OC1Init(TIM4, &TIM_OCInitStructure);
//    TIM_OC1PreloadConfig(TIM4, TIM_OCPreload_Enable);
//    
//    TIM_OC2Init(TIM4, &TIM_OCInitStructure);
//    TIM_OC2PreloadConfig(TIM4, TIM_OCPreload_Enable);
//
//    TIM_OC3Init(TIM4, &TIM_OCInitStructure);
//    TIM_OC3PreloadConfig(TIM4, TIM_OCPreload_Enable);
//
//    TIM_OC4Init(TIM4, &TIM_OCInitStructure);
//    TIM_OC4PreloadConfig(TIM4, TIM_OCPreload_Enable);
//    
//    TIM_ARRPreloadConfig(TIM4,ENABLE);
//    
//    TIM_Cmd(TIM4,ENABLE);
//}



/*******************************************************************************
 * 名    称：        pwm_init_Time1()
 * 功    能：        定时器1波形产生初始化
 * 入口参数：
 * 出口参数：        无
 * 作　　 者：       redmoringcn
 * 创建日期：        2021-11-30
 * 修    改： 
 * 修改日期：        STM32定时器1和定时器8为高级定时器，设置方法和通用定时器稍有区别
 *******************************************************************************/
void pwm_init_Time1(void)
{
    GPIO_InitTypeDef GPIO_InitStructure2;         
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;        
    TIM_OCInitTypeDef TIM_OCInitStructure;        
    TIM_BDTRInitTypeDef TIM_BDTRInitStructure;
    //第一步：配置时钟                 
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOB|RCC_APB2Periph_TIM1,ENABLE);
    //第二步，配置goio口          
    GPIO_InitStructure2.GPIO_Pin = GPIO_Pin_8|GPIO_Pin_9|GPIO_Pin_10|GPIO_Pin_11;         
    GPIO_InitStructure2.GPIO_Speed=GPIO_Speed_50MHz;         
    GPIO_InitStructure2.GPIO_Mode=GPIO_Mode_AF_PP;                 //设置为复用浮空输出         
    GPIO_Init(GPIOA,&GPIO_InitStructure2);        

    //第三步，定时器基本配置         
    TIM_TimeBaseStructure.TIM_Period=1000-1;                       // 自动重装载寄存器的值        
    TIM_TimeBaseStructure.TIM_Prescaler=72-1;                      // 时钟预分频数        
    TIM_TimeBaseStructure.TIM_ClockDivision=TIM_CKD_DIV1;          // 采样分频        
    TIM_TimeBaseStructure.TIM_CounterMode=TIM_CounterMode_Up;      //向上计数        
    //TIM_TimeBaseStructure.TIM_RepetitionCounter=0;               //重复寄存器，用于自动更新pwm占空比                       
    TIM_TimeBaseInit(TIM1, &TIM_TimeBaseStructure);

    //第四步pwm输出配置         
    TIM_OCInitStructure.TIM_OCMode=TIM_OCMode_PWM2;                //设置为pwm1输出模式         
    TIM_OCInitStructure.TIM_Pulse=500;                              //设置占空比时间         
    TIM_OCInitStructure.TIM_OCPolarity=TIM_OCPolarity_Low;          //设置输出极性         
    TIM_OCInitStructure.TIM_OutputState=TIM_OutputState_Enable;     //使能该通道输出         

    //下面几个参数是高级定时器才会用到，通用定时器不用配置         
    // TIM_OCInitStructure.TIM_OCNPolarity=TIM_OCNPolarity_High;        //设置互补端输出极性         
    // TIM_OCInitStructure.TIM_OutputNState=TIM_OutputNState_Enable;//使能互补端输出         
    TIM_OCInitStructure.TIM_OCIdleState=TIM_OCIdleState_Reset;        //死区后输出状态         
    TIM_OCInitStructure.TIM_OCNIdleState=TIM_OCNIdleState_Reset;//死区后互补端输出状态         
    TIM_OC1Init(TIM1,&TIM_OCInitStructure); 
    //按照指定参数初始化           
    TIM_OC2Init(TIM1,&TIM_OCInitStructure); 
    //按照指定参数初始化           
    TIM_OC3Init(TIM1,&TIM_OCInitStructure); 
    //按照指定参数初始化           
    TIM_OC4Init(TIM1,&TIM_OCInitStructure); 
    //按照指定参数初始化           
    //第五步，死区和刹车功能配置，高级定时器才有的，通用定时器不用配置         
    TIM_BDTRInitStructure.TIM_OSSRState = TIM_OSSRState_Disable;//运行模式下输出
    TIM_BDTRInitStructure.TIM_OSSIState = TIM_OSSIState_Disable;//空闲模式下输出选择          
    TIM_BDTRInitStructure.TIM_LOCKLevel = TIM_LOCKLevel_OFF;         //锁定设置        
    TIM_BDTRInitStructure.TIM_DeadTime = 0x90;                                         //死区时间设置         
    TIM_BDTRInitStructure.TIM_Break = TIM_Break_Disable;                 //刹车功能使能         
    TIM_BDTRInitStructure.TIM_BreakPolarity = TIM_BreakPolarity_High;//刹车输入极性        
    TIM_BDTRInitStructure.TIM_AutomaticOutput = TIM_AutomaticOutput_Enable;//自动输出使能          
    TIM_BDTRConfig(TIM1,&TIM_BDTRInitStructure);

    //第六步，使能端的打开 
    TIM_OC1PreloadConfig(TIM1, TIM_OCPreload_Enable);  //使能TIMx在CCR1上的预装载寄存器         
    TIM_OC2PreloadConfig(TIM1, TIM_OCPreload_Enable);  //使能TIMx在CCR1上的预装载寄存器         
    TIM_OC3PreloadConfig(TIM1, TIM_OCPreload_Enable);  //使能TIMx在CCR1上的预装载寄存器         
    TIM_OC4PreloadConfig(TIM1, TIM_OCPreload_Enable);  //使能TIMx在CCR1上的预装载寄存器         

    TIM_ARRPreloadConfig(TIM1, ENABLE);                //使能TIMx在ARR上的预装载寄存器         
    TIM_Cmd(TIM1,ENABLE);                              //打开TIM

    //下面这句是高级定时器才有，输出PWM必须打开  
    TIM_CtrlPWMOutputs(TIM1,ENABLE);    
}



/*******************************************************************************
 * 名    称：        pwm_init_Time5()
 * 功    能：        定时器2波形产生初始化
 * 入口参数：
 * 出口参数：        无
 * 作　　 者：       redmoringcn
 * 创建日期：        2021-11-30
 * 修    改： 
 * 修改日期：       
 *******************************************************************************/
void pwm_init_Time5(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;

    TIM_OCInitTypeDef TIM_OCInitStructure;

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5,ENABLE); 
    //RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);
    TIM_DeInit(TIM5);
    
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE); 
    
    GPIO_InitStructure.GPIO_Pin     =GPIO_Pin_1|GPIO_Pin_2; 
    GPIO_InitStructure.GPIO_Speed   =GPIO_Speed_50MHz; 
    GPIO_InitStructure.GPIO_Mode    =GPIO_Mode_AF_PP;
    GPIO_Init(GPIOA,&GPIO_InitStructure);        
    
    TIM_TimeBaseInitStructure.TIM_Period = 7200; //PWM 72000/900=8Khz
    TIM_TimeBaseInitStructure.TIM_Prescaler = 10;
    TIM_TimeBaseInitStructure.TIM_ClockDivision = 0; 
    TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up; 
    TIM_TimeBaseInit(TIM5, & TIM_TimeBaseInitStructure);


    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM2; 
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStructure.TIM_OutputNState = TIM_OutputState_Disable;
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
    TIM_OCInitStructure.TIM_Pulse = 3600;
    

    //TIM_OC1Init(TIM5, &TIM_OCInitStructure);
    //TIM_OC1PreloadConfig(TIM5, TIM_OCPreload_Enable);
    
    TIM_OC2Init(TIM5, &TIM_OCInitStructure);
    TIM_OC2PreloadConfig(TIM5, TIM_OCPreload_Enable);

    TIM_OC3Init(TIM5, &TIM_OCInitStructure);
    TIM_OC3PreloadConfig(TIM5, TIM_OCPreload_Enable);

    //TIM_OC4Init(TIM5, &TIM_OCInitStructure);
    //TIM_OC4PreloadConfig(TIM5, TIM_OCPreload_Enable);
    

    TIM_ARRPreloadConfig(TIM5,ENABLE);
    
    TIM_Cmd(TIM5,ENABLE);
}

/*******************************************************************************
 * 名    称：        pwm_init_Time2()
 * 功    能：        定时器2波形产生初始化
 * 入口参数：
 * 出口参数：        无
 * 作　　 者：       redmoringcn
 * 创建日期：        2021-11-30
 * 修    改： 
 * 修改日期：       
 *******************************************************************************/
void pwm_init_Time2(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;

    TIM_OCInitTypeDef TIM_OCInitStructure;

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2,ENABLE); 
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);
    TIM_DeInit(TIM2);
    
    //引脚复用
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE); 
    
    GPIO_PinRemapConfig(GPIO_FullRemap_TIM2,ENABLE);    //管脚映射到
    
    GPIO_InitStructure.GPIO_Pin     =GPIO_Pin_10|GPIO_Pin_11; 
    GPIO_InitStructure.GPIO_Speed   =GPIO_Speed_50MHz; 
    GPIO_InitStructure.GPIO_Mode    =GPIO_Mode_AF_PP;

    GPIO_Init(GPIOB,&GPIO_InitStructure);    
    
    TIM_TimeBaseInitStructure.TIM_Period = 7200;        //PWM 72000/900=8Khz
    TIM_TimeBaseInitStructure.TIM_Prescaler = 10;
    TIM_TimeBaseInitStructure.TIM_ClockDivision = 0; 
    TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up; 
    TIM_TimeBaseInit(TIM2, & TIM_TimeBaseInitStructure);

    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM2; 
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStructure.TIM_OutputNState = TIM_OutputState_Disable;
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
    TIM_OCInitStructure.TIM_Pulse = 3600;
    

    //TIM_OC1Init(TIM2, &TIM_OCInitStructure);
    //TIM_OC1PreloadConfig(TIM2, TIM_OCPreload_Enable);
    
    //TIM_OC2Init(TIM2, &TIM_OCInitStructure);
    //TIM_OC2PreloadConfig(TIM2, TIM_OCPreload_Enable);

    TIM_OC3Init(TIM2, &TIM_OCInitStructure);
    TIM_OC3PreloadConfig(TIM2, TIM_OCPreload_Enable);

    TIM_OC4Init(TIM2, &TIM_OCInitStructure);
    TIM_OC4PreloadConfig(TIM2, TIM_OCPreload_Enable);

    TIM_ARRPreloadConfig(TIM2,ENABLE);
    
    TIM_Cmd(TIM2,ENABLE);
}

/*******************************************************************************
 * 名    称：        pwm_init_Time4()
 * 功    能：        定时器2波形产生初始化
 * 入口参数：
 * 出口参数：        无
 * 作　　 者：       redmoringcn
 * 创建日期：        2021-11-30
 * 修    改： 
 * 修改日期：       
 *******************************************************************************/
void pwm_init_Time4(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;

    TIM_OCInitTypeDef TIM_OCInitStructure;


    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4,ENABLE); 
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);
    TIM_DeInit(TIM4);
    
    //引脚复用
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE); 
    
    GPIO_PinRemapConfig(GPIO_Remap_TIM4 , DISABLE); //这个不重映射功能函数
    //GPIO_PinRemapConfig(GPIO_FullRemap_TIM4,ENABLE); //管脚映射到
    
    GPIO_InitStructure.GPIO_Pin     =GPIO_Pin_6|GPIO_Pin_7; 
    GPIO_InitStructure.GPIO_Speed   =GPIO_Speed_50MHz; 
    GPIO_InitStructure.GPIO_Mode    =GPIO_Mode_AF_PP;

    GPIO_Init(GPIOB,&GPIO_InitStructure);    
    
    TIM_TimeBaseInitStructure.TIM_Period = 7200; //PWM 72000/900=8Khz
    TIM_TimeBaseInitStructure.TIM_Prescaler = 10;
    TIM_TimeBaseInitStructure.TIM_ClockDivision = 0; 
    TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up; 
    TIM_TimeBaseInit(TIM4, & TIM_TimeBaseInitStructure);


    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM2; 
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStructure.TIM_OutputNState = TIM_OutputState_Disable;
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
    TIM_OCInitStructure.TIM_Pulse = 3600;
    

    TIM_OC1Init(TIM4, &TIM_OCInitStructure);
    TIM_OC1PreloadConfig(TIM4, TIM_OCPreload_Enable);
    
    TIM_OC2Init(TIM4, &TIM_OCInitStructure);
    TIM_OC2PreloadConfig(TIM4, TIM_OCPreload_Enable);

    //TIM_OC3Init(TIM4, &TIM_OCInitStructure);
    //TIM_OC3PreloadConfig(TIM4, TIM_OCPreload_Enable);

    //TIM_OC4Init(TIM4, &TIM_OCInitStructure);
    //TIM_OC4PreloadConfig(TIM4, TIM_OCPreload_Enable);
    

    TIM_ARRPreloadConfig(TIM4,ENABLE);
    
    TIM_Cmd(TIM4,ENABLE);
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
    
    
    TIM2_Int_Init();              //启动全局定时器

    //pwm_init_Time2();
    //pwm_init_Time5();
    //pwm_init_Time4();

////    
    //PWM_Cfg(20,40,50,80);       //设置四路的占空比
    //Timer8_Configuration();     //初始化运算参数，初始外部捕获功能
}


/*******************************************************************************
* 				end of file
*******************************************************************************/
