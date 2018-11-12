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
* Description  : ��Լ�ж�ʱ�䣬������ֻ��ֵ��
                 1��ȡϵͳ��ʱ��ʱ��sys.time
                 2��ȡ�������������ʱ��cnt�����������ʵ��ʱ��Ϊsys.time * 65536 + cnt;
                 3�������źż�¼������ʱ�䣬���β���ʱ��ȷ�� 
                    ��������10%λ��ʱ�䣻
                    ��������90%λ��ʱ�䣻
                    �½�����10%λ��ʱ�䣻
                    �½�����90%λ��ʱ�䡣
* Author       : 2018/3/16 ������, by redmorningcn
*******************************************************************************/
void TIM8_CC_IRQHandler(void)
{

    uint16      cnt;
    u32         time;                               //ʱ����� sys.time * 65536+TIM_CNT     
    
    //cnt  = TIM_CNT;
    time = Ctrl.sys.time;                           //ʱ����� sys.time * 65536+TIM_CNT     
	
	if(TIM8->SR&0x04)                               //CH2�����ж� 
	{
        cnt = TIM8->CCR2;

        //if(GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_7) == SET)      //��λ��90%������������,,,�������εĴ�0~10%
        if((GPIOC->IDR & GPIO_Pin_7) != (uint32_t)Bit_RESET)
        {     
            TIM_OC2PolarityConfig(TIM8,TIM_ICPolarity_Falling); //����Ϊ�½��ز���			

            Ctrl.ch.test[0].time[Ctrl.ch.test[0].p_write].hig_up_time  = time;     
            Ctrl.ch.test[0].time[Ctrl.ch.test[0].p_write].hig_up_cnt   = cnt;   
            
            //Ctrl.ch.test[0].pluse_status = CH_RAISE_90_STATUS;    //�����أ�90%
        }
        //else if(GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_7) == RESET)                                                   //��λ��90%���½��ش���������������90%��10%
        else if((GPIOC->IDR & GPIO_Pin_7) == (uint32_t)Bit_RESET)
        {    
            TIM_OC2PolarityConfig(TIM8,TIM_ICPolarity_Rising);      //����Ϊ�����ز���

            Ctrl.ch.test[0].time[Ctrl.ch.test[0].p_write].hig_down_time   = time;     
            Ctrl.ch.test[0].time[Ctrl.ch.test[0].p_write].hig_down_cnt    = cnt;   
            
            Ctrl.ch.test[0].pulse_cnt++;                             //���ڽ��������ں���

            Ctrl.ch.test[0].pluse_status = CH_FALL_90_STATUS;       //�½��أ�90%
        }
        
        //TIM_ClearITPendingBit(TIM8,TIM_IT_CC2);                     //����жϱ�־
        TIM8->SR = (uint16_t)~TIM_IT_CC2;

	}
    
	if(TIM8->SR&0x02)								                //CC1�����ж� 
	{

        cnt = TIM8->CCR1;
        //if(GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_6) == SET)          //��λ��10%���������������������ε�10%~90%
        if((GPIOC->IDR & GPIO_Pin_6) != (uint32_t)Bit_RESET)
        {  
            TIM_OC1PolarityConfig(TIM8,TIM_ICPolarity_Falling);     //����Ϊ�½��ز���			

            Ctrl.ch.test[0].time[Ctrl.ch.test[0].p_write].low_up_time    =  time;     
            Ctrl.ch.test[0].time[Ctrl.ch.test[0].p_write].low_up_cnt     =  cnt;    
            
            Ctrl.ch.test[0].pluse_status = CH_RAISE_10_STATUS;       //�����أ�10%

        }
        //else if(GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_6) == RESET)                                              //��λ��10%�����½��������������ε�100%~90
        else if((GPIOC->IDR & GPIO_Pin_6) == (uint32_t)Bit_RESET)
        {           
            TIM_OC1PolarityConfig(TIM8,TIM_ICPolarity_Rising);  //����Ϊ�����ز���

            Ctrl.ch.test[0].time[Ctrl.ch.test[0].p_write].low_down_time    =  time;     
            Ctrl.ch.test[0].time[Ctrl.ch.test[0].p_write].low_down_cnt     =  cnt; 
            
            Ctrl.ch.test[0].p_write           =      Ctrl.ch.test[0].pulse_cnt 
                % CH_TIMEPARA_BUF_SIZE;
            
            //Ctrl.ch.test[0].pluse_status = CH_FALL_10_STATUS;       //�½��أ�10%
        }
        
        //TIM_ClearITPendingBit(TIM8,TIM_IT_CC1);//����жϱ�־
        TIM8->SR = (uint16_t)~TIM_IT_CC1;

	}    
	
	if(TIM8->SR&0x10)//CH4�����ж� ��CH4�������ж��м�¼��ֵ��Ϊ�����ε�������ʱ���
	{
        
        cnt =  TIM8->CCR4;
        //if(GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_9) == SET)      //��λ��90%������������
        if((GPIOC->IDR & GPIO_Pin_9) != (uint32_t)Bit_RESET)
        {     
            TIM_OC4PolarityConfig(TIM8,TIM_ICPolarity_Falling);//����Ϊ�½��ز���			

            Ctrl.ch.test[1].time[Ctrl.ch.test[1].p_write].hig_up_time  = time;     
            Ctrl.ch.test[1].time[Ctrl.ch.test[1].p_write].hig_up_cnt   = cnt;   
            
            //Ctrl.ch.test[1].pluse_status = CH_RAISE_90_STATUS;       //�����أ�90%
        }
        //else  if(GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_9) == RESET)                                                  //��λ��90%���½��ش���
        else if(( GPIOC->IDR & GPIO_Pin_9) == (uint32_t)Bit_RESET)
        {           
            TIM_OC4PolarityConfig(TIM8,TIM_ICPolarity_Rising);//����Ϊ�����ز���		

            Ctrl.ch.test[1].time[Ctrl.ch.test[1].p_write].hig_down_time   = time;     
            Ctrl.ch.test[1].time[Ctrl.ch.test[1].p_write].hig_down_cnt    = cnt; 
            
            Ctrl.ch.test[1].pulse_cnt++;                             //���ڽ��������ں���

            Ctrl.ch.test[1].pluse_status = CH_FALL_90_STATUS;       //�½��أ�90%
        }
        
        //TIM_ClearITPendingBit(TIM8,TIM_IT_CC4);//����жϱ�־
        TIM8->SR = (uint16_t)~TIM_IT_CC4;
	}
    
	if(TIM8->SR&0x08)//CH3�����ж�  
	{
        cnt = TIM8->CCR3;
        //if(GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_8) == SET)      //��λ��10%������������
        if(( GPIOC->IDR & GPIO_Pin_8) != (uint32_t)Bit_RESET)
        {  
            TIM_OC3PolarityConfig(TIM8,TIM_ICPolarity_Falling); //����Ϊ�½��ز���			

            Ctrl.ch.test[1].time[Ctrl.ch.test[1].p_write].low_up_time    =  time;     
            Ctrl.ch.test[1].time[Ctrl.ch.test[1].p_write].low_up_cnt     =  cnt;    
            
            Ctrl.ch.test[1].pluse_status = CH_RAISE_10_STATUS;       //�����أ�10%
        }
        //else  if(GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_8) == RESET)                                                  //��λ��10%�����½�����
        else if(( GPIOC->IDR & GPIO_Pin_8) == (uint32_t)Bit_RESET)
        {     
            TIM_OC3PolarityConfig(TIM8,TIM_ICPolarity_Rising);  //����Ϊ�����ز���		

            Ctrl.ch.test[1].time[Ctrl.ch.test[1].p_write].low_down_time    =  time;     
            Ctrl.ch.test[1].time[Ctrl.ch.test[1].p_write].low_down_cnt     =  cnt;  
            
            
            Ctrl.ch.test[1].p_write             =     Ctrl.ch.test[1].pulse_cnt 
                                                    % CH_TIMEPARA_BUF_SIZE;
            
            //Ctrl.ch.test[1].pluse_status = CH_FALL_10_STATUS;        //�½��أ�10%
        }
            
        //TIM_ClearITPendingBit(TIM8,TIM_IT_CC3);                 //����жϱ�־
        TIM8->SR = (uint16_t)~TIM_IT_CC3;

	}
}


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
        TIM8->SR = (uint16_t)~TIM_IT_Update;        //����жϱ�־
        Ctrl.sys.time++;                            //ϵͳ��ʱ���ۼ�
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
	TIM_BaseInitStructure.TIM_Period = 65535;                   //�������Զ���װֵ
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
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;             //IRQͨ��ʹ��

    NVIC_Init(&NVIC_InitStructure);                             //��������ָ���Ĳ�����ʼ��NVIC�Ĵ���    
    
    Ctrl.sys.time = 0;                                          //ϵͳʱ����0
}



/*******************************************************************************
* Description  : ���ö�ʱ���ⲿ����
                ��ʱ����˫�߲�����bug�����ܲ��������жϷ�������иı䴥����Ե��ʵ��
                ˫�߲����ܡ�
                
* Author       : 2018/3/16 ������, by redmorningcn
*******************************************************************************/
void Timer8_Configuration(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
//	TIM_TimeBaseInitTypeDef	TIM_BaseInitStructure;
	TIM_ICInitTypeDef TIM_ICInitStruct;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM8,ENABLE);//ʹ�ܶ�ʱ��ʱ��
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC,ENABLE);//ʹ��IO��ʱ��
	
	GPIO_InitStructure.GPIO_Pin =        GPIO_Pin_6
                                        |GPIO_Pin_7  
                                        |GPIO_Pin_8
                                        |GPIO_Pin_9;
                           
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;//��������
	GPIO_Init(GPIOC,&GPIO_InitStructure);//��ʼ��
	
	//��ʼ��TIM8���벶�����
	TIM_ICInitStruct.TIM_Channel = TIM_Channel_1;//�����ӳ�䵽TI1
    
	TIM_ICInitStruct.TIM_ICPolarity = TIM_ICPolarity_Rising;//�����ز���
    //TIM_ICInitStruct.TIM_ICPolarity = TIM_ICPolarity_BothEdge;//�����ز���
    
	TIM_ICInitStruct.TIM_ICSelection = TIM_ICSelection_DirectTI;//ӳ�䵽TI1��
	TIM_ICInitStruct.TIM_ICPrescaler = TIM_ICPSC_DIV1;//���벻��Ƶ
	TIM_ICInitStruct.TIM_ICFilter = 0x00;//���벻�˲�
	TIM_ICInit(TIM8,&TIM_ICInitStruct);//��ʼ��TIM8
	
	TIM_ICInitStruct.TIM_Channel = TIM_Channel_2;//�����ӳ�䵽TI2
	TIM_ICInitStruct.TIM_ICPolarity = TIM_ICPolarity_Rising;//�����ز���
	TIM_ICInitStruct.TIM_ICSelection = TIM_ICSelection_DirectTI;//ӳ�䵽TI2��
	TIM_ICInitStruct.TIM_ICPrescaler = TIM_ICPSC_DIV1;//���벻��Ƶ
	TIM_ICInitStruct.TIM_ICFilter = 0x00;//���벻�˲�
	TIM_ICInit(TIM8,&TIM_ICInitStruct);//��ʼ��TIM8
	
	TIM_ICInitStruct.TIM_Channel = TIM_Channel_3;//�����ӳ�䵽TI3
	TIM_ICInitStruct.TIM_ICPolarity = TIM_ICPolarity_Rising;//�����ز���
	TIM_ICInitStruct.TIM_ICSelection = TIM_ICSelection_DirectTI;//ӳ�䵽TI3��
	TIM_ICInitStruct.TIM_ICPrescaler = TIM_ICPSC_DIV1;//���벻��Ƶ
	TIM_ICInitStruct.TIM_ICFilter = 0x00;//���벻�˲�
	TIM_ICInit(TIM8,&TIM_ICInitStruct);//��ʼ��TIM8
	
	TIM_ICInitStruct.TIM_Channel = TIM_Channel_4;//�����ӳ�䵽TI4
	TIM_ICInitStruct.TIM_ICPolarity = TIM_ICPolarity_Rising;//�����ز���
	TIM_ICInitStruct.TIM_ICSelection = TIM_ICSelection_DirectTI;//ӳ�䵽TI4��
	TIM_ICInitStruct.TIM_ICPrescaler = TIM_ICPSC_DIV1;//���벻��Ƶ
	TIM_ICInitStruct.TIM_ICFilter = 0x00;//���벻�˲�
	TIM_ICInit(TIM8,&TIM_ICInitStruct);//��ʼ��TIM8
	
    /**************************************************************
    * Description  : �����ж����ȼ�
    * Author       : 2018/7/17 ���ڶ�, by redmorningcn
    */
    NVIC_InitTypeDef NVIC_InitStructure; 
    NVIC_InitStructure.NVIC_IRQChannel = TIM8_CC_IRQn;  //
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0 ;// ��ռ���ȼ�Ϊ0 
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;      // �����ȼ�λ0 
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;         //IRQͨ��ʹ��

    NVIC_Init(&NVIC_InitStructure);                         //��������ָ���Ĳ�����ʼ��NVIC�Ĵ���    
    
	TIM_ITConfig(TIM8,TIM_IT_Update|TIM_IT_CC1|TIM_IT_CC2|TIM_IT_CC3|TIM_IT_CC4,ENABLE);//����������ж� CC1IE�����ж�
	
	BSP_IntVectSet(TIM8_CC_IRQn, TIM8_CC_IRQHandler);
	BSP_IntEn(TIM8_CC_IRQn);
	
	TIM_Cmd(TIM8,ENABLE);//������ʱ��8
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
    
    Timer8_Configuration();     //��ʼ�������������ʼ�ⲿ������
}


/*******************************************************************************
* 				end of file
*******************************************************************************/
