/*******************************************************************************
* Description  : ������Ϣ����
                 ��¼�����źŲ�����ʱ����Ϣ��ͨ���ⲿ���񼰶�ʱ�����ʵ��
* Author       : 2018/4/12 ������, by redmorningcn
*******************************************************************************/


/*******************************************************************************
* INCLUDES
*/
#include <bsp_wave_detect.h>



/*******************************************************************************
* Description  : ȫ��ʱ���ۻ�����ʵʱ�� time = strSys.time * 65536 + TIM_GetCounter  
                              �ٳ��Ե�����ʱ�䡣65536/72000000
* Author       : 2018/3/13 ���ڶ�, by redmorningcn
                 2018/11/12 ���жϺ�������ֱ��д�ɼĴ������������Ӧ�ٶ�
*******************************************************************************/
void TIM8_OVER_IRQHandler(void)
{
//	if(TIM_GetITStatus(TIM8,TIM_IT_Update)!=RESET)  //����������ж�
//	{
//		TIM_ClearITPendingBit(TIM8,TIM_IT_Update);  //����жϱ�־
//        Ctrl.sys.time++;                           //ϵͳ��ʱ���ۼ�
//	}
    
    if(TIM8->SR & TIM_IT_Update != (uint16_t)RESET){       //����������ж�
        TIM8->SR = (uint16_t)~TIM_IT_Update;                //����жϱ�־
        Ctrl.sys.time++;             
        //Ctrl.sys.num++;    
        //ϵͳ��ʱ���ۼ�
    }
}


/*******************************************************************************
* Description  : ȫ��ʱ�ӣ�Ϊ�����ź��ṩͳһʱ���׼
* Author       : 2018/3/13 ���ڶ�, by redmorningcn
*******************************************************************************/
void Timer8_Iint(void)
{
	TIM_TimeBaseInitTypeDef	TIM_BaseInitStructure;
//	TIM_ICInitTypeDef TIM_ICInitStruct;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM8,ENABLE);         //ʹ�ܶ�ʱ��ʱ��
	//��ʼ����ʱ��8
	TIM_BaseInitStructure.TIM_Period    = 7200-1;                   //�������Զ���װֵ
	TIM_BaseInitStructure.TIM_Prescaler = 0;                    //����Ƶ
	TIM_BaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;     //ʱ�Ӳ��ָ�
	TIM_BaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up; //���ϼ���
	TIM_BaseInitStructure.TIM_RepetitionCounter = 0;            //�ظ���������
    
	TIM_TimeBaseInit(TIM8,&TIM_BaseInitStructure);              //��ʼ��ʱ��
    
	TIM_ClearFlag(TIM8, TIM_FLAG_Update);                       //����жϱ�־λ
	TIM_ITConfig(TIM8,TIM_IT_Update,ENABLE);                    //����������ж�
	TIM_Cmd(TIM8,ENABLE);
    
	BSP_IntVectSet(TIM8_UP_IRQn, TIM8_OVER_IRQHandler);
	BSP_IntEn(TIM8_UP_IRQn);
    
    /**************************************************************
    * Description  : �����ж����ȼ�
    * Author       : 2018/7/17 ���ڶ�, by redmorningcn
    */
    NVIC_InitTypeDef NVIC_InitStructure; 
    NVIC_InitStructure.NVIC_IRQChannel = TIM8_UP_IRQn;  //
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0 ;    // ��ռ���ȼ�Ϊ0 
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;          // �����ȼ�λ0 
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;             // IRQͨ��ʹ��

    NVIC_Init(&NVIC_InitStructure);                             // ��������ָ���Ĳ�����ʼ��NVIC�Ĵ���    
    
    Ctrl.sys.time = 0;                                          // ϵͳʱ����0
}


void TIM2_IRQHandler(void)
{

    if(TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET)
    {
         //interrupt_rtc();//����ʹ���Լ������ִ�к���
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
 * ������ �ߣ�       redmoringcn
 * �������ڣ�        2022-02-17
 * ��    �ģ� 
 * �޸����ڣ�       
 *  �ж�ʱ��Tout= ((arr+1)*(psc+1))/Tclk��
 *   feq:  Tclk  / ((arr+1)*(psc+1)) 
 *   �������Ƶ�� = feq  / 4
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
      calcfreq = Ctrl.sys.setfrq * 4;                     //�����ж�Ƶ�� = ����Ƶ�� * 4
      
      u8 i = 0;
      while(i < 6){
        Ctrl.sys.speedCH[i++] = Ctrl.sys.setfrq;                //�ݶ��ٶ�ֵ����Ƶ��
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
 * ��    �ƣ�        pwm_init_Time2()
 * ��    �ܣ�        ��ʱ��2���β�����ʼ��
 * ��ڲ�����
 * ���ڲ�����        ��
 * ������ �ߣ�       redmoringcn
 * �������ڣ�        2021-11-30
 * ��    �ģ� 
 * �޸����ڣ�       
 *  �ж�ʱ��Tout= ((arr+1)*(psc+1))/Tclk��
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
    * �����������ж���ں������ж����ȼ�
    */
    BSP_IntVectSet(BSP_INT_ID_TIM2, TIM2_IRQHandler);
    BSP_IntEn(BSP_INT_ID_TIM2);
    
    TIM_Cmd(TIM2, ENABLE);
    Ctrl.sys.CHtime = 0;
}


///*******************************************************************************
//* Description  : ���ö�ʱ���ⲿ����
//                ��ʱ����˫�߲�����bug�����ܲ��������жϷ�������иı䴥����Ե��ʵ��
//                ˫�߲����ܡ�
//                
//* Author       : 2018/3/16 ������, by redmorningcn
//*******************************************************************************/
//void Timer8_Configuration(void)
//{
//	GPIO_InitTypeDef GPIO_InitStructure;
////	TIM_TimeBaseInitTypeDef	TIM_BaseInitStructure;
//	TIM_ICInitTypeDef TIM_ICInitStruct;
//	
//	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM8,ENABLE);//ʹ�ܶ�ʱ��ʱ��
//	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC,ENABLE);//ʹ��IO��ʱ��
//	
//	GPIO_InitStructure.GPIO_Pin =        GPIO_Pin_6
//                                        |GPIO_Pin_7  
//                                        |GPIO_Pin_8
//                                        |GPIO_Pin_9;
//                           
//	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;//��������
//	GPIO_Init(GPIOC,&GPIO_InitStructure);//��ʼ��
//	
//	//��ʼ��TIM8���벶�����
//	TIM_ICInitStruct.TIM_Channel = TIM_Channel_1;//�����ӳ�䵽TI1
//    
//	TIM_ICInitStruct.TIM_ICPolarity = TIM_ICPolarity_Rising;//�����ز���
//    //TIM_ICInitStruct.TIM_ICPolarity = TIM_ICPolarity_BothEdge;//�����ز���
//    
//	TIM_ICInitStruct.TIM_ICSelection = TIM_ICSelection_DirectTI;//ӳ�䵽TI1��
//	TIM_ICInitStruct.TIM_ICPrescaler = TIM_ICPSC_DIV1;//���벻��Ƶ
//	TIM_ICInitStruct.TIM_ICFilter = 0x00;//���벻�˲�
//	TIM_ICInit(TIM8,&TIM_ICInitStruct);//��ʼ��TIM8
//	
//	TIM_ICInitStruct.TIM_Channel = TIM_Channel_2;//�����ӳ�䵽TI2
//	TIM_ICInitStruct.TIM_ICPolarity = TIM_ICPolarity_Rising;//�����ز���
//	TIM_ICInitStruct.TIM_ICSelection = TIM_ICSelection_DirectTI;//ӳ�䵽TI2��
//	TIM_ICInitStruct.TIM_ICPrescaler = TIM_ICPSC_DIV1;//���벻��Ƶ
//	TIM_ICInitStruct.TIM_ICFilter = 0x00;//���벻�˲�
//	TIM_ICInit(TIM8,&TIM_ICInitStruct); //��ʼ��TIM8
//	
//	TIM_ICInitStruct.TIM_Channel = TIM_Channel_3;//�����ӳ�䵽TI3
//	TIM_ICInitStruct.TIM_ICPolarity = TIM_ICPolarity_Rising;//�����ز���
//	TIM_ICInitStruct.TIM_ICSelection = TIM_ICSelection_DirectTI;//ӳ�䵽TI3��
//	TIM_ICInitStruct.TIM_ICPrescaler = TIM_ICPSC_DIV1;//���벻��Ƶ
//	TIM_ICInitStruct.TIM_ICFilter = 0x00;//���벻�˲�
//	TIM_ICInit(TIM8,&TIM_ICInitStruct);//��ʼ��TIM8
//	
//	TIM_ICInitStruct.TIM_Channel = TIM_Channel_4;//�����ӳ�䵽TI4
//	TIM_ICInitStruct.TIM_ICPolarity = TIM_ICPolarity_Rising;//�����ز���
//	TIM_ICInitStruct.TIM_ICSelection = TIM_ICSelection_DirectTI;//ӳ�䵽TI4��
//	TIM_ICInitStruct.TIM_ICPrescaler = TIM_ICPSC_DIV1;//���벻��Ƶ
//	TIM_ICInitStruct.TIM_ICFilter = 0x00;//���벻�˲�
//	TIM_ICInit(TIM8,&TIM_ICInitStruct);//��ʼ��TIM8
//	
//    /**************************************************************
//    * Description  : �����ж����ȼ�
//    * Author       : 2018/7/17 ���ڶ�, by redmorningcn
//    */
//    NVIC_InitTypeDef NVIC_InitStructure; 
//    NVIC_InitStructure.NVIC_IRQChannel = TIM8_CC_IRQn;  //
//    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0 ;// ��ռ���ȼ�Ϊ0 
//    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;      // �����ȼ�λ0 
//    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;         //IRQͨ��ʹ��
//
//    NVIC_Init(&NVIC_InitStructure);                         //��������ָ���Ĳ�����ʼ��NVIC�Ĵ���    
//    
//	TIM_ITConfig(TIM8,TIM_IT_Update|TIM_IT_CC1|TIM_IT_CC2|TIM_IT_CC3|TIM_IT_CC4,ENABLE);//����������ж� CC1IE�����ж�
//	
//	BSP_IntVectSet(TIM8_CC_IRQn, TIM8_CC_IRQHandler);
//	BSP_IntEn(TIM8_CC_IRQn);
//	
//	TIM_Cmd(TIM8,ENABLE);                                   //������ʱ��8
//}

///*******************************************************************************
//* Description  : ���ö�ʱ��PWM
//��
//                
//* Author       : 2021/11/02 ������, by redmorningcn
//*******************************************************************************/
//void PWM_Cfg(float dutyfactor1,float dutyfactor2,float dutyfactor3,float dutyfactor4)
//{
//	GPIO_InitTypeDef GPIO_InitStructure;
////	TIM_TimeBaseInitTypeDef	TIM_BaseInitStructure;
//	TIM_ICInitTypeDef TIM_ICInitStruct;
//    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
//	
//    RCC_APB2PeriphClockCmd(RCC_APB1Periph_TIM3,ENABLE);//ʹ�ܶ�ʱ��ʱ��
//    //Ԥ��Ƶϵ��Ϊ0����������Ԥ��Ƶ����ʱTIMER��Ƶ��Ϊ72MHzre.TIM_Prescaler =0;
//    TIM_TimeBaseStructure.TIM_Prescaler = 0;
//    //���ü��������С��ÿ��20000�����Ͳ���һ�������¼�
//    TIM_TimeBaseStructure.TIM_Period = 7200 - 1;
//    //����ʱ�ӷָ�
//    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
//    //���ü�����ģʽΪ���ϼ���ģʽ
//    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
//
//    //������Ӧ�õ�TIM3��
//    TIM_TimeBaseInit(TIM3,&TIM_TimeBaseStructure);    
//    
//	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC,ENABLE);//ʹ��IO��ʱ��
//    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);
//	
//	GPIO_InitStructure.GPIO_Pin =        GPIO_Pin_6
//                                        |GPIO_Pin_7  
//                                        |GPIO_Pin_8
//                                        |GPIO_Pin_9;
//                       
//    GPIO_PinRemapConfig(GPIO_FullRemap_TIM3, ENABLE);
//    
//    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;//�������
//	GPIO_Init(GPIOC,&GPIO_InitStructure);//��ʼ��
//	
//    TIM_OCInitTypeDef TIM_OCInitStructure;
//      //����ȱʡֵ
//    TIM_OCStructInit(&TIM_OCInitStructure);
//      
//      //TIM3��CH1���
//    TIM_OCInitStructure.TIM_OCMode       = TIM_OCMode_PWM1;             //������PWMģʽ���ǱȽ�ģʽ 
//    TIM_OCInitStructure.TIM_OutputState  = TIM_OutputState_Enable;      //�Ƚ����ʹ�ܣ�ʹ��PWM������˿�
//    TIM_OCInitStructure.TIM_OCPolarity   = TIM_OCPolarity_High;         //���ü����Ǹ߻��ǵ�
//      //����ռ�ձȣ�ռ�ձ�=(CCRx/ARR)*100%��(TIM_Pulse/TIM_Period)*100%
//    TIM_OCInitStructure.TIM_Pulse = dutyfactor1 * 7200 / 100;
//    TIM_OC1Init(TIM3, &TIM_OCInitStructure);
//      
//      //TIM3��CH2���
//    TIM_OCInitStructure.TIM_OCMode       = TIM_OCMode_PWM1; //������PWMģʽ���ǱȽ�ģʽ 
//    TIM_OCInitStructure.TIM_OutputState  = TIM_OutputState_Enable; //�Ƚ����ʹ�ܣ�ʹ��PWM������˿�
//    TIM_OCInitStructure.TIM_OCPolarity   = TIM_OCPolarity_High; //���ü����Ǹ߻��ǵ�
//      //����ռ�ձȣ�ռ�ձ�=(CCRx/ARR)*100%��(TIM_Pulse/TIM_Period)*100%
//    TIM_OCInitStructure.TIM_Pulse = dutyfactor2 * 7200 / 100;
//    TIM_OC2Init(TIM3, &TIM_OCInitStructure);
//      
//   
//      //TIM3��CH3���
//    TIM_OCInitStructure.TIM_OCMode       = TIM_OCMode_PWM1; //������PWMģʽ���ǱȽ�ģʽ 
//    TIM_OCInitStructure.TIM_OutputState  = TIM_OutputState_Enable; //�Ƚ����ʹ�ܣ�ʹ��PWM������˿�
//    TIM_OCInitStructure.TIM_OCPolarity   = TIM_OCPolarity_High; //���ü����Ǹ߻��ǵ�
//      //����ռ�ձȣ�ռ�ձ�=(CCRx/ARR)*100%��(TIM_Pulse/TIM_Period)*100%
//    TIM_OCInitStructure.TIM_Pulse = dutyfactor3 * 7200 / 100;
//    TIM_OC3Init(TIM3, &TIM_OCInitStructure);
//      
//      
//      //TIM3��CH4���
//    TIM_OCInitStructure.TIM_OCMode       = TIM_OCMode_PWM1; //������PWMģʽ���ǱȽ�ģʽ 
//    TIM_OCInitStructure.TIM_OutputState  = TIM_OutputState_Enable; //�Ƚ����ʹ�ܣ�ʹ��PWM������˿�
//    TIM_OCInitStructure.TIM_OCPolarity   = TIM_OCPolarity_High; //���ü����Ǹ߻��ǵ�
//      //����ռ�ձȣ�ռ�ձ�=(CCRx/ARR)*100%��(TIM_Pulse/TIM_Period)*100%
//    TIM_OCInitStructure.TIM_Pulse = dutyfactor4 * 7200 / 100;
//    TIM_OC4Init(TIM3, &TIM_OCInitStructure);
//            
//      //ʹ�����״̬
//     TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
//      
//    //����TIM3��PWM���Ϊʹ��
//     TIM_CtrlPWMOutputs(TIM3,ENABLE);
//     TIM_Cmd(TIM3,ENABLE);
//
//}



void GPIO_Cfg(void)
{
    
    GPIO_InitTypeDef GPIO_InitStructure;
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC|RCC_APB2Periph_GPIOA|RCC_APB2Periph_AFIO,ENABLE);
      
   //ȫ��ӳ�䣬��TIM3_CH2ӳ�䵽PB5
   //����STM32���Ĳο��ֲ�2010�еڵ�119ҳ��֪��
   //��û����ӳ��ʱ��TIM3���ĸ�ͨ��CH1��CH2��CH3��CH4�ֱ��ӦPA6��PA7,PB0,PB1
   //��������ӳ��ʱ��TIM3���ĸ�ͨ��CH1��CH2��CH3��CH4�ֱ��ӦPB4��PB5,PB0,PB1
   //����ȫ��ӳ��ʱ��TIM3���ĸ�ͨ��CH1��CH2��CH3��CH4�ֱ��ӦPC6��PC7,PC8,PC9
   //Ҳ����˵����ȫ��ӳ��֮���ĸ�ͨ����PWM������ŷֱ�ΪPC6��PC7,PC8,PC9�������õ���ͨ��1��ͨ��2�����Զ�Ӧ����ΪPC6��PC7,PC8,PC9�������õ���ͨ��1��ͨ��2�����Զ�Ӧ����Ϊ
    GPIO_PinRemapConfig(GPIO_FullRemap_TIM3, ENABLE);
      
       //������ӳ��Ĳ���
       //GPIO_PinRemapConfig(GPIO_PartialRemap_TIM3, ENABLE);
    
    //����PC6��PC7��PC8��PC9Ϊ������������4·PWM
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_6|GPIO_Pin_7|GPIO_Pin_8|GPIO_Pin_9;
    GPIO_Init(GPIOC,&GPIO_InitStructure);
}


void TIM_Cfg(void)
{
     //����ṹ��
     TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;

       //���½�Timer����Ϊȱʡֵ
       TIM_DeInit(TIM3);
       //�����ڲ�ʱ�Ӹ�TIM2�ṩʱ��Դ
       //TIM_InternalClockConfig(TIM3);
     
     //Ԥ��Ƶϵ��Ϊ0����������Ԥ��Ƶ����ʱTIMER��Ƶ��Ϊ72MHzre.TIM_Prescaler =0;
       TIM_TimeBaseStructure.TIM_Prescaler = 0;
     //���ü��������С��ÿ��20000�����Ͳ���һ�������¼�
       TIM_TimeBaseStructure.TIM_Period = 7200 - 1;
       //����ʱ�ӷָ�
       TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
       //���ü�����ģʽΪ���ϼ���ģʽ
       TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
       
       //������Ӧ�õ�TIM2��
       TIM_TimeBaseInit(TIM3,&TIM_TimeBaseStructure);
       //�������жϱ�־
       //TIM_ClearFlag(TIM2, TIM_FLAG_Update);
       //��ֹARRԤװ�ػ�����
       //TIM_ARRPreloadConfig(TIM2, DISABLE);
       //����TIM2���ж�
       //TIM_ITConfig(TIM2,TIM_IT_Update,ENABLE);
    
}




/*******************************************************************************
* �� �� ��         : PWM���������ú���
* ��������         : PWM_Cfg
* ��    ��         : dutyfactor ռ�ձ���ֵ����С��0.014��100
* ��    ��         : ��
*******************************************************************************/
void PWM_Cfg(float dutyfactor1,float dutyfactor2,float dutyfactor3,float dutyfactor4)
{
    TIM_OCInitTypeDef TIM_OCInitStructure;
      //����ȱʡֵ
      TIM_OCStructInit(&TIM_OCInitStructure);
      
      //TIM3��CH1���
      TIM_OCInitStructure.TIM_OCMode       = TIM_OCMode_PWM1; //������PWMģʽ���ǱȽ�ģʽ 
      TIM_OCInitStructure.TIM_OutputState  = TIM_OutputState_Enable; //�Ƚ����ʹ�ܣ�ʹ��PWM������˿�
      TIM_OCInitStructure.TIM_OCPolarity   = TIM_OCPolarity_High; //���ü����Ǹ߻��ǵ�
      //����ռ�ձȣ�ռ�ձ�=(CCRx/ARR)*100%��(TIM_Pulse/TIM_Period)*100%
      TIM_OCInitStructure.TIM_Pulse = dutyfactor1 * 7200 / 100;
      TIM_OC1Init(TIM3, &TIM_OCInitStructure);
      TIM_OC1PreloadConfig(TIM3, TIM_OCPreload_Enable);      
      
      
      //TIM3��CH2���
      TIM_OCInitStructure.TIM_OCMode       = TIM_OCMode_PWM1; //������PWMģʽ���ǱȽ�ģʽ 
      TIM_OCInitStructure.TIM_OutputState  = TIM_OutputState_Enable; //�Ƚ����ʹ�ܣ�ʹ��PWM������˿�
      TIM_OCInitStructure.TIM_OCPolarity   = TIM_OCPolarity_High; //���ü����Ǹ߻��ǵ�
      //����ռ�ձȣ�ռ�ձ�=(CCRx/ARR)*100%��(TIM_Pulse/TIM_Period)*100%
      TIM_OCInitStructure.TIM_Pulse = dutyfactor2 * 7200 / 100;
      TIM_OC2Init(TIM3, &TIM_OCInitStructure);
      //TIM_OC2PreloadConfig(TIM3, TIM_OCPreload_Enable);      

   
      //TIM3��CH3���
      TIM_OCInitStructure.TIM_OCMode       = TIM_OCMode_PWM1;           //������PWMģʽ���ǱȽ�ģʽ 
      TIM_OCInitStructure.TIM_OutputState  = TIM_OutputState_Enable;    //�Ƚ����ʹ�ܣ�ʹ��PWM������˿�
      TIM_OCInitStructure.TIM_OCPolarity   = TIM_OCPolarity_High;       //���ü����Ǹ߻��ǵ�
      //����ռ�ձȣ�ռ�ձ�=(CCRx/ARR)*100%��(TIM_Pulse/TIM_Period)*100%
      TIM_OCInitStructure.TIM_Pulse = dutyfactor3 * 7200 / 100;
      TIM_OC3Init(TIM3, &TIM_OCInitStructure);
      //TIM_OC3PreloadConfig(TIM3, TIM_OCPreload_Enable);      
      
      //TIM3��CH4���
      TIM_OCInitStructure.TIM_OCMode       = TIM_OCMode_PWM1;           //������PWMģʽ���ǱȽ�ģʽ 
      TIM_OCInitStructure.TIM_OutputState  = TIM_OutputState_Enable;    //�Ƚ����ʹ�ܣ�ʹ��PWM������˿�
      TIM_OCInitStructure.TIM_OCPolarity   = TIM_OCPolarity_High;       //���ü����Ǹ߻��ǵ�
      //����ռ�ձȣ�ռ�ձ�=(CCRx/ARR)*100%��(TIM_Pulse/TIM_Period)*100%
      TIM_OCInitStructure.TIM_Pulse = dutyfactor4 * 7200 / 100;
      TIM_OC4Init(TIM3, &TIM_OCInitStructure);
      //TIM_OC4PreloadConfig(TIM3, TIM_OCPreload_Enable);      
     
      
      //ʹ�����״̬
      TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
     
      //TIM_ARRPreloadConfig(TIM3,ENABLE);
    
      TIM_Cmd(TIM3,ENABLE);
      
    //����TIM3��PWM���Ϊʹ��
      //TIM_CtrlPWMOutputs(TIM3,ENABLE);
}
//
///*******************************************************************************
// * ��    �ƣ�        pwm_init_Time4()
// * ��    �ܣ�        ��ʱ��1���β�����ʼ��
// * ��ڲ�����
// * ���ڲ�����        ��
// * ������ �ߣ�       redmoringcn
// * �������ڣ�        2021-11-30
// * ��    �ģ� 
// * �޸����ڣ�        STM32��ʱ��1�Ͷ�ʱ��8Ϊ�߼���ʱ�������÷�����ͨ�ö�ʱ����������
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
////GPIO_PinRemapConfig(GPIO_FullRemap_TIM3,ENABLE); //�ܽ�ӳ�䵽LED
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
 * ��    �ƣ�        pwm_init_Time1()
 * ��    �ܣ�        ��ʱ��1���β�����ʼ��
 * ��ڲ�����
 * ���ڲ�����        ��
 * ������ �ߣ�       redmoringcn
 * �������ڣ�        2021-11-30
 * ��    �ģ� 
 * �޸����ڣ�        STM32��ʱ��1�Ͷ�ʱ��8Ϊ�߼���ʱ�������÷�����ͨ�ö�ʱ����������
 *******************************************************************************/
void pwm_init_Time1(void)
{
    GPIO_InitTypeDef GPIO_InitStructure2;         
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;        
    TIM_OCInitTypeDef TIM_OCInitStructure;        
    TIM_BDTRInitTypeDef TIM_BDTRInitStructure;
    //��һ��������ʱ��                 
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOB|RCC_APB2Periph_TIM1,ENABLE);
    //�ڶ���������goio��          
    GPIO_InitStructure2.GPIO_Pin = GPIO_Pin_8|GPIO_Pin_9|GPIO_Pin_10|GPIO_Pin_11;         
    GPIO_InitStructure2.GPIO_Speed=GPIO_Speed_50MHz;         
    GPIO_InitStructure2.GPIO_Mode=GPIO_Mode_AF_PP;                 //����Ϊ���ø������         
    GPIO_Init(GPIOA,&GPIO_InitStructure2);        

    //����������ʱ����������         
    TIM_TimeBaseStructure.TIM_Period=1000-1;                       // �Զ���װ�ؼĴ�����ֵ        
    TIM_TimeBaseStructure.TIM_Prescaler=72-1;                      // ʱ��Ԥ��Ƶ��        
    TIM_TimeBaseStructure.TIM_ClockDivision=TIM_CKD_DIV1;          // ������Ƶ        
    TIM_TimeBaseStructure.TIM_CounterMode=TIM_CounterMode_Up;      //���ϼ���        
    //TIM_TimeBaseStructure.TIM_RepetitionCounter=0;               //�ظ��Ĵ����������Զ�����pwmռ�ձ�                       
    TIM_TimeBaseInit(TIM1, &TIM_TimeBaseStructure);

    //���Ĳ�pwm�������         
    TIM_OCInitStructure.TIM_OCMode=TIM_OCMode_PWM2;                //����Ϊpwm1���ģʽ         
    TIM_OCInitStructure.TIM_Pulse=500;                              //����ռ�ձ�ʱ��         
    TIM_OCInitStructure.TIM_OCPolarity=TIM_OCPolarity_Low;          //�����������         
    TIM_OCInitStructure.TIM_OutputState=TIM_OutputState_Enable;     //ʹ�ܸ�ͨ�����         

    //���漸�������Ǹ߼���ʱ���Ż��õ���ͨ�ö�ʱ����������         
    // TIM_OCInitStructure.TIM_OCNPolarity=TIM_OCNPolarity_High;        //���û������������         
    // TIM_OCInitStructure.TIM_OutputNState=TIM_OutputNState_Enable;//ʹ�ܻ��������         
    TIM_OCInitStructure.TIM_OCIdleState=TIM_OCIdleState_Reset;        //���������״̬         
    TIM_OCInitStructure.TIM_OCNIdleState=TIM_OCNIdleState_Reset;//�����󻥲������״̬         
    TIM_OC1Init(TIM1,&TIM_OCInitStructure); 
    //����ָ��������ʼ��           
    TIM_OC2Init(TIM1,&TIM_OCInitStructure); 
    //����ָ��������ʼ��           
    TIM_OC3Init(TIM1,&TIM_OCInitStructure); 
    //����ָ��������ʼ��           
    TIM_OC4Init(TIM1,&TIM_OCInitStructure); 
    //����ָ��������ʼ��           
    //���岽��������ɲ���������ã��߼���ʱ�����еģ�ͨ�ö�ʱ����������         
    TIM_BDTRInitStructure.TIM_OSSRState = TIM_OSSRState_Disable;//����ģʽ�����
    TIM_BDTRInitStructure.TIM_OSSIState = TIM_OSSIState_Disable;//����ģʽ�����ѡ��          
    TIM_BDTRInitStructure.TIM_LOCKLevel = TIM_LOCKLevel_OFF;         //��������        
    TIM_BDTRInitStructure.TIM_DeadTime = 0x90;                                         //����ʱ������         
    TIM_BDTRInitStructure.TIM_Break = TIM_Break_Disable;                 //ɲ������ʹ��         
    TIM_BDTRInitStructure.TIM_BreakPolarity = TIM_BreakPolarity_High;//ɲ�����뼫��        
    TIM_BDTRInitStructure.TIM_AutomaticOutput = TIM_AutomaticOutput_Enable;//�Զ����ʹ��          
    TIM_BDTRConfig(TIM1,&TIM_BDTRInitStructure);

    //��������ʹ�ܶ˵Ĵ� 
    TIM_OC1PreloadConfig(TIM1, TIM_OCPreload_Enable);  //ʹ��TIMx��CCR1�ϵ�Ԥװ�ؼĴ���         
    TIM_OC2PreloadConfig(TIM1, TIM_OCPreload_Enable);  //ʹ��TIMx��CCR1�ϵ�Ԥװ�ؼĴ���         
    TIM_OC3PreloadConfig(TIM1, TIM_OCPreload_Enable);  //ʹ��TIMx��CCR1�ϵ�Ԥװ�ؼĴ���         
    TIM_OC4PreloadConfig(TIM1, TIM_OCPreload_Enable);  //ʹ��TIMx��CCR1�ϵ�Ԥװ�ؼĴ���         

    TIM_ARRPreloadConfig(TIM1, ENABLE);                //ʹ��TIMx��ARR�ϵ�Ԥװ�ؼĴ���         
    TIM_Cmd(TIM1,ENABLE);                              //��TIM

    //��������Ǹ߼���ʱ�����У����PWM�����  
    TIM_CtrlPWMOutputs(TIM1,ENABLE);    
}



/*******************************************************************************
 * ��    �ƣ�        pwm_init_Time5()
 * ��    �ܣ�        ��ʱ��2���β�����ʼ��
 * ��ڲ�����
 * ���ڲ�����        ��
 * ������ �ߣ�       redmoringcn
 * �������ڣ�        2021-11-30
 * ��    �ģ� 
 * �޸����ڣ�       
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
 * ��    �ƣ�        pwm_init_Time2()
 * ��    �ܣ�        ��ʱ��2���β�����ʼ��
 * ��ڲ�����
 * ���ڲ�����        ��
 * ������ �ߣ�       redmoringcn
 * �������ڣ�        2021-11-30
 * ��    �ģ� 
 * �޸����ڣ�       
 *******************************************************************************/
void pwm_init_Time2(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;

    TIM_OCInitTypeDef TIM_OCInitStructure;

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2,ENABLE); 
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);
    TIM_DeInit(TIM2);
    
    //���Ÿ���
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE); 
    
    GPIO_PinRemapConfig(GPIO_FullRemap_TIM2,ENABLE);    //�ܽ�ӳ�䵽
    
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
 * ��    �ƣ�        pwm_init_Time4()
 * ��    �ܣ�        ��ʱ��2���β�����ʼ��
 * ��ڲ�����
 * ���ڲ�����        ��
 * ������ �ߣ�       redmoringcn
 * �������ڣ�        2021-11-30
 * ��    �ģ� 
 * �޸����ڣ�       
 *******************************************************************************/
void pwm_init_Time4(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;

    TIM_OCInitTypeDef TIM_OCInitStructure;


    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4,ENABLE); 
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);
    TIM_DeInit(TIM4);
    
    //���Ÿ���
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE); 
    
    GPIO_PinRemapConfig(GPIO_Remap_TIM4 , DISABLE); //�������ӳ�书�ܺ���
    //GPIO_PinRemapConfig(GPIO_FullRemap_TIM4,ENABLE); //�ܽ�ӳ�䵽
    
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
* Description  : ͨ��������ʼ��
                ��ʼ��ȫ�ֶ�ʱ���Ͳ���ͨ�����ⲿ�жϣ�
                �Լ����е�ȫ�ֱ�����
* Author       : 2018/3/13 ���ڶ�, by redmorningcn
*******************************************************************************/
void    init_ch_timepara_detect(void)
{
    Timer8_Iint();              //����ȫ�ֶ�ʱ��
    
    
    TIM2_Int_Init();              //����ȫ�ֶ�ʱ��

    //pwm_init_Time2();
    //pwm_init_Time5();
    //pwm_init_Time4();

////    
    //PWM_Cfg(20,40,50,80);       //������·��ռ�ձ�
    //Timer8_Configuration();     //��ʼ�������������ʼ�ⲿ������
}


/*******************************************************************************
* 				end of file
*******************************************************************************/
