/*******************************************************************************
* Description  : ADC��ʼ��
                1���ṩ1����ʼ�������������ʼ��ָ�˿ڼ�����ģʽ
                2���ṩ1��ADC��ѹ��ȡ����������ָ����ADͨ������ȡָ��ֵ����λmV��
* Author       : 2018/4/12 ������, by redmorningcn
*******************************************************************************/



/*******************************************************************************
* INCLUDES
*/
#include "bsp_adc.h"


/*******************************************************************************
* Description  : ADC����ģʽ����
* Author       : 2018/4/12 ������, by redmorningcn
*******************************************************************************/
void ADC_Mode_Config(void)
{
	ADC_InitTypeDef ADC_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC|RCC_APB2Periph_ADC1,ENABLE);//ʹ��ADC1ʱ��ͨ��
	RCC_ADCCLKConfig(RCC_PCLK2_Div6);                   //���÷�Ƶ����6 72M/6=12M������ܳ���14M
	
	GPIO_InitStructure.GPIO_Pin =       GPIO_Pin_0
                                    |   GPIO_Pin_1
                                    |   GPIO_Pin_2
                                    |   GPIO_Pin_3
                                    |   GPIO_Pin_4
                                    |   GPIO_Pin_5;
    
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;       //ģ������
	GPIO_Init(GPIOC,&GPIO_InitStructure);               //��ʼ��
	
	ADC_DeInit(ADC1);                                   //��λADC1��������ADC1��ȫ���Ĵ������Ϊȱʡֵ
	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;  //ADC����ģʽ
	ADC_InitStructure.ADC_ScanConvMode = DISABLE;       //��ͨ��ģʽ
	ADC_InitStructure.ADC_ContinuousConvMode = DISABLE; //����ת��
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;//ת�����������
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;//ADC�����Ҷ���
	ADC_InitStructure.ADC_NbrOfChannel = 1;             //˳�����ת����ͨ����
	
	ADC_Init(ADC1,&ADC_InitStructure);                  //����ָ���Ĳ�����ʼ��ADC
	ADC_Cmd(ADC1,ENABLE);                               //ʹ��ADC1
	ADC_ResetCalibration(ADC1);                         //������λУ׼
	while(ADC_GetResetCalibrationStatus(ADC1));         //�ȴ���λ����
	ADC_StartCalibration(ADC1);                         //����ADУ׼
	while(ADC_GetCalibrationStatus(ADC1));              //�ȴ�У׼����
}


/*******************************************************************************
* Description  : ��ȡADCֵ������ת�����̲���Ӳ�ȴ������������
* Author       : 2018/1/24 ������, by redmorningcn
*******************************************************************************/
uint16_t Get_ADC(uint8_t ch)//chΪͨ����
{
    //����ָ��ADC�Ĺ�����ͨ�����������ǵ�ת��˳��Ͳ���ʱ��
    uint32  tmp32;
    
	ADC_RegularChannelConfig(ADC1,ch,1,ADC_SampleTime_7Cycles5);//ͨ��1����������Ϊ7.5����+12.5
	ADC_SoftwareStartConvCmd(ADC1,ENABLE);//ʹ��ָ����ADC1�����ת������
    /*******************************************************************************
    * Description  : �ȴ�ADCת��
    * Author       : 2018/3/29 ������, by redmorningcn
    *******************************************************************************/
	while(!ADC_GetFlagStatus(ADC1,ADC_FLAG_EOC));   //�ȴ�ת������
    
    tmp32 = ADC_GetConversionValue(ADC1);           //ȡת��ֵ
    
    tmp32 = (tmp32 * 3300)/4096;                    //���ز�����ѹֵ����λmV
    
	return  tmp32;
}

/*******************************************************************************
* Description  : ��Ϣ��ƽ����Դ��ƽ����ʼ��
  *     @arg ADC_Channel_10: ͨ��1�źŷ�ֵ���
  *     @arg ADC_Channel_11: ͨ��2�źŷ�ֵ��� 
  *     @arg ADC_Channel_12: ͨ��1��Դ��ֵ���
  *     @arg ADC_Channel_12: ͨ��2��Դ��ֵ���
* Author       : 2018/4/12 ������, by redmorningcn
*******************************************************************************/
void Bsp_ADC_Init(void)
{
	ADC_Mode_Config();
}

/*******************************************************************************
 *              end of file                                                    *
 *******************************************************************************/
