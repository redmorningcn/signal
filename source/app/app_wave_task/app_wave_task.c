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

//210414 �������ģʽ������������Ϊ�������������ۼ��ź��������������ж��Ƿ�����
#define     PLUSE_CNT_DEBUG              1               

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
* Description  : ȡ�źŵ�ƽ���ֵ
* Author       : 2018/3/29 ������, by redmorningcn
*******************************************************************************/
void    app_calc_ch_voltagepara(void)
{
    uint8    i;
    static uint32  systime = 0;
    if(Ctrl.sys.time  > systime + 1000){
        systime  = Ctrl.sys.time;

        for (int8 i = 0;i<6;i++){
            
            Ctrl.sys.vccCH[i]= Get_ADC(ADC_Channel_10+i);      //�ɼ���ѹ  
        }
    }
}



/*******************************************************************************
* 				end of file
*******************************************************************************/
