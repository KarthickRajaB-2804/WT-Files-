/*--------------------------Includes--------------------------*/


/*--------------------------Extern Variables--------------------------*/

extern enum fault_status DRV_COM_FAULT, FAULT_OA, DRV_OVERTEMP_WARNING, DRV_OVERTEMP_SHUTDOWN, VDS_OCP, UNDER_VOLT_LOCKOUT, CHARGEPUMP_UNDER_VOLT, GATE_DRIVE_FAULT ;
extern enum fault_status SEN_OCA, SEN_OCB, SEN_OCC, VDS_OCP_HA, VDS_OCP_LA, VDS_OCP_HB, VDS_OCP_LB, VDS_OCP_HC, VDS_OCP_LC, VGS_HA, VGS_LA, VGS_HB, VGS_LB, VGS_HC, VGS_LC ;

/*--------------------------User Function Prototypes--------------------------*/

void DRV_INIT(void);
uint16_t DRV_Read_Data (uint8_t drv_reg);
void DRV_Write_Data (uint8_t drv_reg, uint16_t drv_value);
void DRV_Read_Fault(void);
void DRV_Fault_Clear(void);
void DRV_Calibration(void);
void DRV_FAULT (void);
void DRV_Lock (void);
void DRV_UnLock (void);
void CLR_DRV_FAULT_COUNT(void);
void DRV_FAULT_HANDLER (void);
void DRV_TIMED_CLEAR_FAULTS (void);

#ifndef FAULT_DEF_ENUM
#define FAULT_DEF_ENUM
enum fault_status {NIL = 0U, FAULT = 1U};
#endif

/*--------------------------User Defines--------------------------*/

#define FAULT_RETRY_COUNT       3       // No. of times Before the Fault is considered
#define FAULT_CLR_TIME          5000    // Time to retain the faults count in MilliSeconds

/*-------------FAULT_STATUS_REG_1-------------*/
#define FAULT_POS_BIT                   10
#define VDS_OCP_POS_BIT                 9
#define GATE_DRIVE_FAULT_POS_BIT        8
#define UNDER_VOLT_LOCKOUT_POS_BIT      7
#define OVER_TEMP_SHUTDOWN_POS_BIT      6
#define VDS_OCP_HA_POS_BIT              5
#define VDS_OCP_LA_POS_BIT              4
#define VDS_OCP_HB_POS_BIT              3
#define VDS_OCP_LB_POS_BIT              2
#define VDS_OCP_HC_POS_BIT              1
#define VDS_OCP_LC_POS_BIT              0

#define FAULT_MASK                      1024
#define VDS_OCP_MASK                    512
#define GATE_DRIVE_FAULT_MASK           256
#define UNDER_VOLT_LOCKOUT_MASK         128
#define OVER_TEMP_SHUTDOWN_MASK         64
#define VDS_OCP_HA_MASK                 32
#define VDS_OCP_LA_MASK                 16
#define VDS_OCP_HB_MASK                 8
#define VDS_OCP_LB_MASK                 4
#define VDS_OCP_HC_MASK                 2
#define VDS_OCP_LC_MASK                 1

/*-------------FAULT_STATUS_REG_2-------------*/
#define SENSE_OC_A_POS_BIT              10
#define SENSE_OC_B_POS_BIT              9
#define SENSE_OC_C_POS_BIT              8
#define OVER_TEMP_WARNING_POS_BIT       7
#define CHARGEPUMP_UNDER_VOLT_POS_BIT   6
#define VGS_HA_POS_BIT                  5
#define VGS_LA_POS_BIT                  4
#define VGS_HB_POS_BIT                  3
#define VGS_LB_POS_BIT                  2
#define VGS_HC_POS_BIT                  1
#define VGS_LC_POS_BIT                  0

#define SENSE_OC_A_MASK                 1024
#define SENSE_OC_B_MASK                 512
#define SENSE_OC_C_MASK                 256
#define OVER_TEMP_WARNING_MASK          128
#define CHARGEPUMP_UNDER_VOLT_MASK      64
#define VGS_HA_MASK                     32
#define VGS_LA_MASK                     16
#define VGS_HB_MASK                     8
#define VGS_LB_MASK                     4
#define VGS_HC_MASK                     2
#define VGS_LC_MASK                     1

/*-------------REGISTER_ADDRESS-------------*/
#define FAULT_STATUS_REG 	        0x00
#define VGS_STATUS_REG 		        0x01
#define DRV_CTRL_REG 		        0x02
#define GATE_DRVHS_REG 		        0x03
#define GATE_DRVLS_REG 		        0x04
#define OCP_CTRL_REG 		        0x05
#define CSA_CTRL_REG 		        0x06

/*-------------DRV_CONTROL_REGISTER_(0x02)-------------*/
#define OCP_ACT 			1	        //All three half-bridges are shutdown in response to VDS_OCP and SEN_OCP
#define DIS_GDUV			0 	        //VCP and VGLS under voltage lockout fault is enabled
#define DIS_GDF				0 	        //Gate drive fault is enabled
#define OTW_REP				0 	        //OTW is reported on nFAULT and the FAULT bit
#define PWM_MODE		 	0               // use Decimal TO set the values 6x PWM Mode
#define PWM_COM				0 	        //1x PWM mode uses synchronous rectification
#define PWM_DIR				0	        //In 1x PWM mode this bit is ORed with the INHC (DIR) input
#define COAST				0	        //Write a 1 to this bit to put all MOSFETs in the Hi-Z state
#define BRAKE				0	        //Write a 1 to this bit to turn on all three low-side MOSFETs
#define CLR_FLT				1	        //Write a 1 to this bit to clear latched fault bits

//#define DRV_CTRL_REG_DATA 	1153
#define DRV_CTRL_REG_DATA 	((OCP_ACT<<10)|(DIS_GDUV<<9)|(DIS_GDF<<8)|(OTW_REP<<7)|(PWM_MODE<<5)|(PWM_COM<<4)|(PWM_DIR<<3)|(COAST<<2)|(BRAKE<<1)|(CLR_FLT))

/*-------------GATE_DRIVE_HS_REGISTER_(0x03)-------------*/
#define LOCK				3           //Write 011b to this register to unlock all registers.
#define IDRIVEP_HS			4          //260mA
#define IDRIVEN_HS			4          //520mA

#define LOCK_MASK                       255U       // 0 for BIT - 10,9,8----> REST 0 - 7 = 1
#define LOCK_VALUE                      1536U      // bits-->{10,9,8} = {1,1,0}
#define UNLOCK_VALUE                    768U            // bits-->{10,9,8} = {0,1,1}

//#define GATE_DRVHS_REG_DATA             1023
#define GATE_DRVHS_REG_DATA 	(LOCK<<8)|(IDRIVEP_HS<<4)|(IDRIVEN_HS)

/*-------------GATE_DRIVE_LS_REGISTER_(0x04)-------------*/
#define CBC				0	        //For VDS_OCP and SEN_OCP, the fault is cleared after RETRY
#define TDRIVE				3	        //4000-ns peak gate-current drive time
#define IDRIVEP_LS			4	        //260mA
#define IDRIVEN_LS			4	        //520mA

//#define GATE_DRVLS_REG_DATA 2047
#define GATE_DRVLS_REG_DATA (CBC<<10)|(TDRIVE<<8)|(IDRIVEP_LS<<4)|(IDRIVEN_LS)

/*-------------OCP_CONTROL_REGISTER_(0x05)-------------*/
#define TRETRY 				0		//VDS_OCP and SEN_OCP retry time is 8 ms
#define DEAD_TIME 			0	        //VDS_OCP and SEN_OCP retry time is 8 ms
#define OCP_MODE 			2	        //Over current causes an automatic retrying fault
#define OCP_DEG 			3	        //Over current deglitch of 4 µs
#define VDS_LVL				15	        //1.88 V

//#define OCP_CTRL_REG_DATA 	319
#define OCP_CTRL_REG_DATA 	(TRETRY<<10)|(DEAD_TIME<<8)|(OCP_MODE<<6)|(OCP_DEG<<4)|(VDS_LVL)

/*-------------CSA_CONTROL_REGISTER_(0x06)-------------*/
#define CSA_FET 			0		//Sense amplifier positive input is SPx
#define VREF_DIV 			1		//0 - Sense amplifier reference voltage is VREF (unidirectional mode) 1 - Bidirectional mode
#define LS_REF 				0		//VDS_OCP for the low-side MOSFET is measured across SHx to SPx
#define CSA_GAIN 			0	        //40-V/V shunt amplifier gain
#define DIS_SEN 			1		//Sense overcurrent fault is disabled
#define CSA_CAL_A 			0		//Normal sense amplifier C operation
#define CSA_CAL_B 			0		//Normal sense amplifier C operation
#define CSA_CAL_C 			0		//Normal sense amplifier C operation
#define SEN_LVL 			3	        //Sense OCP 1 V
#define I_SENSE_GAIN                    5.0f            //Current Sense Gain as per CSA_GAIN bit

//#define CSA_CTRL_REG_DATA 	515
#define CSA_CTRL_REG_DATA 	(CSA_FET<<10)|(VREF_DIV<<9)|(LS_REF<<8)|(CSA_GAIN<<6)|(DIS_SEN<<5)|(CSA_CAL_A<<4)|(CSA_CAL_B<<3)|(CSA_CAL_C<<2)|(SEN_LVL)


#define SWITCH_OFF_MOSFET 	1156
#define SWITCH_ON_MOSFET 	1152

#define DRV_ENABLE    HAL_GPIO_WritePin(DRV_EN_GPIO_Port,DRV_EN_Pin,GPIO_PIN_SET)
#define DRV_DISABLE   HAL_GPIO_WritePin(DRV_EN_GPIO_Port,DRV_EN_Pin,GPIO_PIN_RESET)
#define DRV_AUTO_CAL  DRV_Calibration()