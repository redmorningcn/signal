/*******************************************************************************
* Description  : 计算波形参数
* Author       : 2018/4/12 星期四, by redmorningcn
*******************************************************************************/


/*******************************************************************************
* INCLUDES
*/
#include <app_wave_task.h>
#include <Algorithm.h>

//时间参数计算值过滤
#define     CH_PARA_BUF_SIZE             (10)     
//边沿时间补偿系数
#define     CH_EDGE_CALI                 (1.1)

//210414 定义调试模式，将周期数改为脉冲数，用于累计信号秒冲个数，用于判断是否丢脉冲
#define     PLUSE_CNT_DEBUG              1               

typedef   struct   _strChTimeParaFliter_{
    uint32              period[CH_PARA_BUF_SIZE];                         //周期，  0.00-2000000.00us （0.5Hz）
    uint32              freq[CH_PARA_BUF_SIZE];                           //频率，  0-100000hz              
    uint32              raise[CH_PARA_BUF_SIZE];                          //上升沿，0.00-50.00us
    uint32              fail[CH_PARA_BUF_SIZE];                           //下降沿，0.00-50.00us
    uint32              ratio[CH_PARA_BUF_SIZE];                          //占空比，0.00-100.00%
    uint32              phase[CH_PARA_BUF_SIZE];
    int32               Acceleration[CH_PARA_BUF_SIZE];                   //加速度
}strChTimeParaFliter;

strChTimeParaFliter     lsChTimeFliterBuf[2];
u32                     timetmpbuf[CH_PARA_BUF_SIZE];


/*******************************************************************************
* Description  : 取信号电平检测值
* Author       : 2018/3/29 星期四, by redmorningcn
*******************************************************************************/
void    app_calc_ch_voltagepara(void)
{
    uint8    i;
    static uint32  systime = 0;
    if(Ctrl.sys.time  > systime + 1000){
        systime  = Ctrl.sys.time;

        for (int8 i = 0;i<6;i++){
            
            Ctrl.sys.vccCH[i]= Get_ADC(ADC_Channel_10+i);      //采集电压  
        }
    }
}



/*******************************************************************************
* 				end of file
*******************************************************************************/
