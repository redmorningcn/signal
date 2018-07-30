/*******************************************************************************
* Description  : 波形检测结构体定义
* Author       : 2018/4/12 星期四, by redmorningcn
*******************************************************************************/

#ifndef	APP_WAVE_H
#define	APP_WAVE_H

/*******************************************************************************
 * INCLUDES
 */
#include <global.h>
#include <bsp.h>


/*******************************************************************************
 * CONSTANTS
 */

 /*******************************************************************************
 * 				            系统参数相关宏定义                                  *
 *******************************************************************************/

/*******************************************************************************
 * MACROS
 */
//通道时间参数缓冲区大小
#define CH_TIMEPARA_BUF_SIZE    50
#define CH_VOLTAGE_BUF_SIZE     20

//信号位置
#define CH_RAISE_10_STATUS      0x01
#define CH_RAISE_90_STATUS      0x02
#define CH_FALL_90_STATUS       0x04
#define CH_FALL_10_STATUS       0x08


/*******************************************************************************
* Description  : 通道具体指标
* Author       : 2018/3/14 星期三, by redmorningcn
*******************************************************************************/
typedef struct  _strsignalchannelpara{
    uint32              period;                         //周期，  0.00-2000000.00us （0.5Hz）
    uint32              freq;                           //频率，  0-100000hz              
    uint16              raise;                          //上升沿，0.00-50.00us
    uint16              fail;                           //下降沿，0.00-50.00us
    uint16              ratio;                          //占空比，0.00-100.00%
    uint16              Vol;                            //低电平，0.00-30.00V
    uint16              Voh;                            //高电平，0.00-30.00V
    
    union{
        struct{
            u8         lose    :1  ;                   //丢脉冲
            u8         nopluse :1  ;                   //无信号
            u8         rec     :6 ;
            
            s8         acceleration;                   //加速度
        };
        u16             flags;
    }status;                         //通道状态
}strsignalchannelpara;



/*******************************************************************************
 * TYPEDEFS
 */



/*******************************************************************************
* Description  : 通道结构体。其中，通道时间参数辅助运算；
                             通道参数才是通道的具体指标。
* Author       : 2018/3/13 星期二, by redmorningcn
*******************************************************************************/
typedef struct
{
    /*******************************************************************************
    * Description  : 辅助运算
    * Author       : 2018/3/14 星期三, by redmorningcn
    *******************************************************************************/
    struct  {
        //时间指标
        struct  {
            uint32  low_up_time;                            //10%位置，上升沿，中断时间 
            uint32  low_down_time;                          //10%位置，下降沿，中断时间
            uint32  hig_up_time;                            //90%位置，上升沿，中断时间
            uint32  hig_down_time;                          //90%位置，下降沿，中断时间
            uint16  low_up_cnt;
            uint16  low_down_cnt;
            uint16  hig_up_cnt;
            uint16  hig_down_cnt;
        }time[CH_TIMEPARA_BUF_SIZE];
        
        uint16  p_write;                                    //缓冲区读写控制
        uint16  p_read;
        uint32  pulse_cnt;                                  //脉冲个数，判断信号有无
        
        //信号所处位置，高/低
        union  
        {
            struct __pluse_status__ {                      // 信号状态
                uint16  raise_10: 1;                        // 上升位置——10%
                uint16  raise_90: 1;       	                // 上升位置--90%
                uint16  fall_90 : 1;       	                // 下降位置--10
                uint16  fall_10 : 1;  	                    // 其他：未定义
                uint16  res     : 12;  	                    // 其他：未定义
                
                u8      res1    ;                           //预留
                s8      acceleration;                       //加速度
            }station;
            uint32  pluse_status;
        };        
        
        //电平指标
        struct {
            uint16  ch_low_voltage;                         //信号低电平
            uint16  ch_hig_voltage;                         //信号高电平
            uint16  vcc_hig_voltage;                        //通道供电电平
        }voltage[CH_VOLTAGE_BUF_SIZE];
        uint16  p_wr_vol;                                   //电平写，指针
        uint16  p_rd_vol;                                   //电平读，指针
    }test[2];                                               //通道检测内容
    
    
    /*******************************************************************************
    * Description  : 通道具体指标
    * Author       : 2018/3/14 星期三, by redmorningcn
    *******************************************************************************/
    strsignalchannelpara    para[2];
    
    uint32  ch1_2phase;                                     //相位差，0.00-360.00°
    uint16  vcc_vol;                                        //供电电压
    uint16  stand_vol;                                      //参考电压
}strCoupleChannel;


//calibration
/**************************************************************
* Description  : 校准参数
* Author       : 2018/5/22 星期二, by redmorningcn
*/
typedef struct {
    u32     line;       //修正线性度  
    int16   Delta;      //修正偏差
    int16   tmp;        //预留

}strCalibration;


/**************************************************************
* Description  : 修正参数表
* Author       : 2018/5/22 星期二, by redmorningcn
*/
typedef struct{
    union   {
        struct{
            strCalibration  VccVol;         //电平
            strCalibration  Ch1Vol;         //通道1电压
            strCalibration  Ch2Vol;         //通道2电压
        };
        strCalibration      CaliBuf[20];
    };
    
}strCaliTable;


/*******************************************************************************
 * GLOBAL VARIABLES
 */


/*******************************************************************************
 * 				end of file                                                    *
 *******************************************************************************/
#endif	/* GLOBLES_H */
