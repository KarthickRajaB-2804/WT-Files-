/*--------------------------System Includes--------------------------*/

#include "main.h"
#include "string.h"
#include "spi.h"
#include "tim.h"

/*--------------------------User Includes--------------------------*/

#include "DRV8323.h"
//#include "Finite_State_Machine.h"
//#include "ADC_Functions.h"
/* ===== Forward declarations ===== */
void LATCHED_FAULT(void);


/*--------------------------Extern Defines--------------------------*/

enum fault_status DRV_COM_FAULT, FAULT_OA, DRV_OVERTEMP_WARNING, DRV_OVERTEMP_SHUTDOWN, VDS_OCP, UNDER_VOLT_LOCKOUT, CHARGEPUMP_UNDER_VOLT, GATE_DRIVE_FAULT = NIL;
enum fault_status SEN_OCA, SEN_OCB, SEN_OCC, VDS_OCP_HA, VDS_OCP_LA, VDS_OCP_HB, VDS_OCP_LB, VDS_OCP_HC, VDS_OCP_LC, VGS_HA, VGS_LA, VGS_HB, VGS_LB, VGS_HC, VGS_LC = NIL;
uint8_t VDS_OCP_COUNT, GATE_DRIVE_FAULT_COUNT, UNDER_VOLT_LOCKOUT_COUNT, DRV_OVERTEMP_SHUTDOWN_COUNT, DRV_OVERTEMP_WARNING_COUNT, CHARGEPUMP_UNDER_VOLT_COUNT, SEN_OCA_COUNT, SEN_OCB_COUNT, SEN_OCC_COUNT = 0;





/*--------------------------global Defines--------------------------*/

uint8_t DRV_error_check;
uint8_t flt_chk = 0;
uint16_t DRV_send_data, DRV_receive_data;
uint16_t DRV_Fault_Status_1=0,DRV_Fault_Status_2=0;
volatile uint32_t DRV_CNT_RST_TIM = 0;
volatile uint8_t  DRV_CNT_RST_TIM_EN = 0;

/*--------------------------Callback Functions--------------------------*/


/*--------------------------User Functions--------------------------*/

void DRV_INIT(void)
{
	DRV_ENABLE;
        DRV_AUTO_CAL;
        DRV_UnLock();
	DRV_Write_Data(DRV_CTRL_REG,DRV_CTRL_REG_DATA);
	DRV_Write_Data(GATE_DRVHS_REG,GATE_DRVHS_REG_DATA);
	DRV_Write_Data(GATE_DRVLS_REG,GATE_DRVLS_REG_DATA);
	DRV_Write_Data(OCP_CTRL_REG,OCP_CTRL_REG_DATA);
	DRV_Write_Data(CSA_CTRL_REG,CSA_CTRL_REG_DATA);
        DRV_Lock();
}

uint16_t DRV_Read_Data (uint8_t drv_reg)
{
    uint16_t read_reg = (((uint16_t)drv_reg)<<11) | 0x8000;
    HAL_SPI_TransmitReceive(&hspi3, (uint8_t*)&read_reg, (uint8_t*)&DRV_receive_data, 1, HAL_MAX_DELAY);
    return(DRV_receive_data);
}

void DRV_Write_Data (uint8_t drv_reg, uint16_t drv_value)
{
    uint8_t DRV_error_count = 0;
    static uint16_t check_val=0,value_req_test=0;
    if (drv_value < 2048)													  			// Total Value is only 11 bits max 2047
    {
    	DRV_send_data = 0x7FFF & ((((uint16_t)drv_reg)<<11) | (((uint16_t)drv_value))); // Data Appending
        value_req_test = DRV_send_data & 2047;									        // Required Value while reading the written register
        while(1)
        {
            HAL_SPI_Transmit(&hspi3, (uint8_t*)&DRV_send_data, 1, HAL_MAX_DELAY);			  		// Send Data
            check_val = DRV_Read_Data (drv_reg);						  		  		// Read the Sent Data
            if(check_val == value_req_test)									  			// Check the read data with the required value
            {
              DRV_COM_FAULT = NIL;
              break;
            }
            else
            {
              if(drv_reg == DRV_CTRL_REG)
              {
                if(check_val == value_req_test - 1)
                {
                  DRV_COM_FAULT = NIL;
                  break;
                }
              }
              DRV_error_count++;

              if(DRV_error_count >= FAULT_RETRY_COUNT)											  	// Error Counts Allowed
              {
                DRV_COM_FAULT = FAULT;
                DRV_FAULT ();    							//Call For Error
                break;
              }
            }
        }
    }
    else
    {
    	DRV_error_check = 1; 															// Call for invalid Value Error
    }
}

void DRV_Lock (void)
{
  uint16_t Read_LOCK_Register = DRV_Read_Data(GATE_DRVHS_REG);
  Read_LOCK_Register = Read_LOCK_Register & LOCK_MASK;
  Read_LOCK_Register = Read_LOCK_Register | LOCK_VALUE;
  DRV_Write_Data(GATE_DRVHS_REG,Read_LOCK_Register);
  
  
}

void DRV_UnLock (void)
{
  uint16_t Read_UNLOCK_Register = DRV_Read_Data(GATE_DRVHS_REG);
  Read_UNLOCK_Register = Read_UNLOCK_Register & LOCK_MASK;
  Read_UNLOCK_Register = Read_UNLOCK_Register | UNLOCK_VALUE;
  DRV_Write_Data(GATE_DRVHS_REG,Read_UNLOCK_Register);
}

void DRV_Read_Fault(void)
{
  DRV_Fault_Status_1 = DRV_Read_Data(FAULT_STATUS_REG);
  DRV_Fault_Status_2 = DRV_Read_Data(VGS_STATUS_REG);        
}

void DRV_Fault_Parser (void)
{
  FAULT_OA              = (enum fault_status) ((DRV_Fault_Status_1 & FAULT_MASK)                        >> FAULT_POS_BIT);
  VDS_OCP               = (enum fault_status) ((DRV_Fault_Status_1 & VDS_OCP_MASK)                      >> VDS_OCP_POS_BIT);
  GATE_DRIVE_FAULT      = (enum fault_status) ((DRV_Fault_Status_1 & GATE_DRIVE_FAULT_MASK)             >> GATE_DRIVE_FAULT_POS_BIT);
  UNDER_VOLT_LOCKOUT    = (enum fault_status) ((DRV_Fault_Status_1 & UNDER_VOLT_LOCKOUT_MASK)           >> UNDER_VOLT_LOCKOUT_POS_BIT);
  DRV_OVERTEMP_SHUTDOWN = (enum fault_status) ((DRV_Fault_Status_1 & OVER_TEMP_SHUTDOWN_MASK)           >> OVER_TEMP_SHUTDOWN_POS_BIT);
  DRV_OVERTEMP_WARNING  = (enum fault_status) ((DRV_Fault_Status_2 & OVER_TEMP_WARNING_MASK)            >> OVER_TEMP_WARNING_POS_BIT);
  CHARGEPUMP_UNDER_VOLT = (enum fault_status) ((DRV_Fault_Status_2 & CHARGEPUMP_UNDER_VOLT_MASK)        >> CHARGEPUMP_UNDER_VOLT_POS_BIT);
  SEN_OCA               = (enum fault_status) ((DRV_Fault_Status_2 & SENSE_OC_A_MASK)                   >> SENSE_OC_A_POS_BIT);
  SEN_OCB               = (enum fault_status) ((DRV_Fault_Status_2 & SENSE_OC_B_MASK)                   >> SENSE_OC_B_POS_BIT);
  SEN_OCC               = (enum fault_status) ((DRV_Fault_Status_2 & SENSE_OC_C_MASK)                   >> SENSE_OC_C_POS_BIT);
  
  if (GATE_DRIVE_FAULT == FAULT)
  {
    VGS_HA              = (enum fault_status) ((DRV_Fault_Status_2 & VGS_HA_MASK)                       >> VGS_HA_POS_BIT);
    VGS_LA              = (enum fault_status) ((DRV_Fault_Status_2 & VGS_LA_MASK)                       >> VGS_LA_POS_BIT);
    VGS_HB              = (enum fault_status) ((DRV_Fault_Status_2 & VGS_HB_MASK)                       >> VGS_HB_POS_BIT);
    VGS_LB              = (enum fault_status) ((DRV_Fault_Status_2 & VGS_LB_MASK)                       >> VGS_LB_POS_BIT);
    VGS_HC              = (enum fault_status) ((DRV_Fault_Status_2 & VGS_HC_MASK)                       >> VGS_HC_POS_BIT);
    VGS_LC              = (enum fault_status) ((DRV_Fault_Status_2 & VGS_LC_MASK)                       >> VGS_LC_POS_BIT);
  }
  
  if (VDS_OCP == FAULT)
  {
    VDS_OCP_HA          = (enum fault_status) ((DRV_Fault_Status_1 & VDS_OCP_HA_MASK)                   >> VDS_OCP_HA_POS_BIT);
    VDS_OCP_LA          = (enum fault_status) ((DRV_Fault_Status_1 & VDS_OCP_LA_MASK)                   >> VDS_OCP_LA_POS_BIT);
    VDS_OCP_HB          = (enum fault_status) ((DRV_Fault_Status_1 & VDS_OCP_HB_MASK)                   >> VDS_OCP_HB_POS_BIT);
    VDS_OCP_LB          = (enum fault_status) ((DRV_Fault_Status_1 & VDS_OCP_LB_MASK)                   >> VDS_OCP_LB_POS_BIT);
    VDS_OCP_HC          = (enum fault_status) ((DRV_Fault_Status_1 & VDS_OCP_HC_MASK)                   >> VDS_OCP_HC_POS_BIT);
    VDS_OCP_LC          = (enum fault_status) ((DRV_Fault_Status_1 & VDS_OCP_LC_MASK)                   >> VDS_OCP_LC_POS_BIT);
  }
}

void DRV_Fault_Clear(void)
{
	uint16_t DRV_read_ctrl_reg = DRV_Read_Data(DRV_CTRL_REG);							//Read the Existing values in the DRV_CTRL_REG
	DRV_Write_Data(DRV_CTRL_REG,(DRV_read_ctrl_reg | 0x0001));							//The LSB is set to 1b to clear the faults
}

void DRV_Calibration(void)
{
  HAL_GPIO_WritePin (CAL_GPIO_Port,CAL_Pin,GPIO_PIN_SET);
  HAL_Delay(1);
  HAL_GPIO_WritePin(CAL_GPIO_Port,CAL_Pin,GPIO_PIN_RESET); 
}

void DRV_TIMED_CLEAR_FAULTS (void)
{
  if (DRV_CNT_RST_TIM > FAULT_CLR_TIME)
  {
    DRV_CNT_RST_TIM_EN = 0;
    DRV_CNT_RST_TIM    = 0;
    CLR_DRV_FAULT_COUNT ();
  }
}

void DRV_FAULT_HANDLER (void)
{
  if(FAULT_OA == FAULT)
  {
    DRV_CNT_RST_TIM_EN = 1;
  }
  /*_______________________COUNTING THE NO. of FAULT OCCURANCE_______________________*/
  if (VDS_OCP == FAULT)
  {
    VDS_OCP_COUNT++;
    VDS_OCP = NIL;
  }
  if (GATE_DRIVE_FAULT == FAULT)
  {
    GATE_DRIVE_FAULT_COUNT++;
    GATE_DRIVE_FAULT = NIL;
  }
  if (UNDER_VOLT_LOCKOUT == FAULT)
  {
    UNDER_VOLT_LOCKOUT_COUNT++;
    UNDER_VOLT_LOCKOUT = NIL;
  }
  if (DRV_OVERTEMP_SHUTDOWN == FAULT)
  {
    DRV_OVERTEMP_SHUTDOWN_COUNT++;
    DRV_OVERTEMP_SHUTDOWN = NIL;
  }
  if (DRV_OVERTEMP_WARNING == FAULT)
  {
    DRV_OVERTEMP_WARNING_COUNT++;
    DRV_OVERTEMP_WARNING = NIL;
  }
  if (CHARGEPUMP_UNDER_VOLT == FAULT)
  {
    CHARGEPUMP_UNDER_VOLT_COUNT++;
    CHARGEPUMP_UNDER_VOLT = NIL;
  }
  if (SEN_OCA == FAULT)
  {
    SEN_OCA_COUNT++;
    SEN_OCA = NIL;
  }
    if (SEN_OCB == FAULT)
  {
    SEN_OCB_COUNT++;
    SEN_OCB = NIL;
  }
    if (SEN_OCC == FAULT)
  {
    SEN_OCC_COUNT++;
    SEN_OCC = NIL;
  }
  
    /*_______________________CHECKING FOR NO. OF FAULTS_______________________*/
  
  if (VDS_OCP_COUNT >= FAULT_RETRY_COUNT)
  {
    VDS_OCP = FAULT;
  }
  
  if (GATE_DRIVE_FAULT_COUNT >= FAULT_RETRY_COUNT)
  {
    GATE_DRIVE_FAULT = FAULT;
  }
  
  if (UNDER_VOLT_LOCKOUT_COUNT >= FAULT_RETRY_COUNT)
  {
    UNDER_VOLT_LOCKOUT = FAULT;
  }
  
  if (DRV_OVERTEMP_SHUTDOWN_COUNT >= FAULT_RETRY_COUNT)
  {
    DRV_OVERTEMP_SHUTDOWN = FAULT;
  }
  
  if (DRV_OVERTEMP_WARNING_COUNT >= FAULT_RETRY_COUNT)
  {
    DRV_OVERTEMP_WARNING = FAULT;
  }
  
  if (CHARGEPUMP_UNDER_VOLT_COUNT >= FAULT_RETRY_COUNT)
  {
    CHARGEPUMP_UNDER_VOLT = FAULT;
  }
  
  if (SEN_OCA_COUNT >= FAULT_RETRY_COUNT)
  {
    SEN_OCA = FAULT;
  }
  
  if (SEN_OCB_COUNT >= FAULT_RETRY_COUNT)
  {
    SEN_OCB = FAULT;
  }
  
  if (SEN_OCC_COUNT >= FAULT_RETRY_COUNT)
  {
    SEN_OCC = FAULT;
  }
  
/*_______________________LATCHING FOR EXCESSIVE NO. OF FAULTS_______________________*/
  
    if (VDS_OCP == FAULT)
  {
    LATCHED_FAULT ();
  }
  if (GATE_DRIVE_FAULT == FAULT)
  {
    LATCHED_FAULT ();
  }
  if (UNDER_VOLT_LOCKOUT == FAULT)
  {
    LATCHED_FAULT ();
  }
  if (DRV_OVERTEMP_SHUTDOWN == FAULT)
  {
    LATCHED_FAULT ();
  }
  if (DRV_OVERTEMP_WARNING == FAULT)
  {
    LATCHED_FAULT ();
  }
  if (CHARGEPUMP_UNDER_VOLT == FAULT)
  {
    LATCHED_FAULT ();
  }
  if (SEN_OCA == FAULT)
  {
    LATCHED_FAULT ();
  }
    if (SEN_OCB == FAULT)
  {
    LATCHED_FAULT ();
  }
    if (SEN_OCC == FAULT)
  {
    LATCHED_FAULT ();
  }
  if (DRV_COM_FAULT == FAULT)
  {
    LATCHED_FAULT ();
  }
  
  FAULT_OA = NIL;               // Clearing the OVERALL FAULT BIT INCASE OF NON ALARMING STATUS
}

void CLR_DRV_FAULT_COUNT (void)
{
  VDS_OCP_COUNT                    = 0;
  GATE_DRIVE_FAULT_COUNT           = 0;
  UNDER_VOLT_LOCKOUT_COUNT         = 0;
  DRV_OVERTEMP_SHUTDOWN_COUNT      = 0;
  DRV_OVERTEMP_WARNING_COUNT       = 0;
  CHARGEPUMP_UNDER_VOLT_COUNT      = 0;
  SEN_OCA_COUNT                    = 0;
  SEN_OCB_COUNT                    = 0;
  SEN_OCC_COUNT                    = 0;
}



void DRV_FAULT (void)
{
  DRV_Read_Fault();
  DRV_Fault_Parser();
  TIM1->CCR1 = 0;
  TIM1->CCR2 = 0;
  TIM1->CCR3 = 0;
  HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_1);
  HAL_TIMEx_PWMN_Stop(&htim1, TIM_CHANNEL_1);
  HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_2);
  HAL_TIMEx_PWMN_Stop(&htim1, TIM_CHANNEL_2);
  HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_3);
  HAL_TIMEx_PWMN_Stop(&htim1, TIM_CHANNEL_3);  
  HAL_TIM_Base_Stop_IT(&htim1);
  DRV_FAULT_HANDLER ();
  DRV_UnLock();
  DRV_Fault_Clear();
  DRV_Lock();
  //while(1);
  //Motor_Stop ();
}
void LATCHED_FAULT (void)
{
  DRV_DISABLE;
  DRV_CNT_RST_TIM_EN = 0;
  DRV_CNT_RST_TIM    = 0;
  while (1);
}
