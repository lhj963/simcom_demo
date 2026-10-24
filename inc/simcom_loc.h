 /** 
* @file         simcom_loc.h 
* @brief        SIMCom  Location API
* @author       xuxiaohui
* @date         2021/6/29
* @version      V1.0.0 
* @par Copyright (c):  2021 SIMCom Wireless Solutions Co.,Ltd, All Rights Reserved
*/ 
#ifndef SIMCOM_LOC_H
#define SIMCOM_LOC_H

#include "simcom_sdk_common.h"
#include "simcom_sdk_indication.h"


/** Milliseconds since January 1, 1970 */
typedef int64_t GpsUtcTime;

#define false   0
#define true    1

#ifndef bool
#define bool int
#endif


/** Location info. WGS-84 coordinate system*/
typedef struct {
     /** set to sizeof(GpsLocation) */
    uint32_t          size;
    /** Contains GpsLocationFlags bits. */
    uint16_t        flags;
    /** Represents latitude in degrees. */
    double          latitude;
    /** Represents longitude in degrees. */
    double          longitude;
    /** Represents altitude in meters above the WGS 84 reference
     * ellipsoid. */
    double          altitude;
    /** Represents speed in meters per second. */
    float           speed;
    /** Represents heading in degrees. */
    float           bearing;
    /** Represents expected accuracy in meters. */
    float           accuracy;
    /** Timestamp for the location fix. */
    GpsUtcTime      timestamp;
    /** Y-M-D H-M-S*/
    uint16_t  time[6];

    uint32_t noSate;

    char status;
} simcom_loc_info;

typedef enum
{
    FAILD             = 0,
    SUCCESSFUL        = 1,
    UPDATING          = 2,
    UPDATING_OK_BUT_DEVICES_ERR = 3
}DYNANIC_STATUS;

typedef enum
{
    GNSS_INFO_OUTPUT_CB          = 0,
    NMEA_DATA_OUTPUT_NMEA_PORT        = 1,
}NMEA_DATA_OUTPUT_PORT;
#ifdef __SIMCOM_ASR_5311__
typedef enum
{
    GNSS_MODE_GPS = 1,
    GNSS_MODE_BD,
    GNSS_MODE_GPS_BD,
    GNSS_MODE_GLO,
    GNSS_MODE_GPS_GLO,
    GNSS_MODE_BD_GLO,
    GNSS_MODE_GPS_BD_GLO,
} GNSS_MODE;
#else
typedef enum
{
    GNSS_MODE_GPS_SBAS_QZSS = 1,
    GNSS_MODE_BDS,
    GNSS_MODE_GPS_GLONASS_GALILEO_SBAS_QZSS,    //If the gnss chip firmware version is earlier than 3.4.0.0 this mode is not support
    GNSS_MODE_GPS_BDS_GALILEO_SBAS_QZSS,
} GNSS_MODE;
#endif

typedef enum
{
    NMEA          = 0,
    LOCATION_INFO = 1,
}NMEA_LOC_INFO_SWITCH;


/*struct gps_context {
	struct ubus_context *ubus;
	struct gps_ril_context *ril;
	struct gps_hsm_context *hsm;
	struct ubus_subscriber sms_ubus_subscriber;

	 //gps state machine 
	enum e_gps_state state;

	int pipe[2]; // UBUS pipe for incoming requests from hsm 
	struct threadqueue queue; // response queue shared with hsm 
	pthread_mutex_t lock;
	struct uloop_fd ubus_fd;

};*/

/**
* @brief
*   Initialize Location function
* @param
*   [in] loc_ind_cb: Receive Location info callback
* @return
*   0  : SUCCESS \n
*  -1  : FAIL
* @note
*   The function must be called before all other loc api
*/
int simcom_loc_init(simcom_event_ind_cb loc_ind_cb, int gnss_num);

/**
* @brief
*   deinit Location function
* @param
*   NONE
* @return
*   0  : SUCCESS \n
*  -1  : FAIL
* @note
*   NONE
*/
void simcom_loc_deinit();


/**
* @brief
*   Cold Start Location
* @param
*   NONE
* @return 
*   0  : SUCCESS \n
*  -1  : FAIL
* @note
*
*/
int simcom_loc_coldstart();
/**
* @brief
*   Hot Start Location
* @param
*   NONE
* @return 
*   0  : SUCCESS \n
*  -1  : FAIL
* @note
*
*/
int simcom_loc_hotstart();

/**
* @brief
*   Stop Location engine
* @param
*   NONE
* @return
*   0  : SUCCESS \n
*  -1  : FAIL
* @note
*   NONE
*/
int simcom_loc_stop();

void simcom_loc_agnss_enable();

void simcom_loc_agnss_disable();

int simcom_loc_poweroff(void);

void simcom_loc_dynamic_upgrade_disable();

int simcom_loc_get_version_info(char* firminfo);

/*****************************************************************************
* Function Name : get_handle
* Description   : Gets a handle to the GPS serial port
* Input         : handle (Control handle)
* Output        : Null
* Return        : handle(The value of handle)
* Auther        : linhuanzhong
* Date          : 2022.4.25
*****************************************************************************/
int get_handle(int *handle);

/*****************************************************************************
* Function Name : get_dynamic_status
* Description   : check dynamic status 
* Input         : Null
* Output        : Null
* Return        : typedef enum
                  {
                    FAILD             = 0,
                    SUCCESSFUL        = 1,
                    UPDATING          = 2
                  }DYNANIC_STATUS;
* Auther        : rui.zhu
* Date          : 2023.06.30
*****************************************************************************/
DYNANIC_STATUS get_dynamic_status();

/*****************************************************************************
* Function Name : simcom_loc_send_gnss_cmd
* Description   : send gnss cmd 
* Input         : gnss cmd
* Output        : Null
* Return        : 0 success other faild
* Auther        : rui.zhu
* Date          : 2023.07.12
*****************************************************************************/
int simcom_loc_send_gnss_cmd(char *cmd);

/*****************************************************************************
* Function Name : Sunsea_Gnss_Init
* Description   : Get Location function initialized with handle 
* Input         : None
* Output        : None
* Return        : 0 succes    -1 Fail
*****************************************************************************/
int Sunsea_Gnss_Init();

/*****************************************************************************
* Function Name : Sunsea_Gnss_SetPositionMode
* Description   : Set navigation parameters 
* Input         : navigation parameters
* Output        : None
* Return        : 0 succes    -1 Fail
*****************************************************************************/
int Sunsea_Gnss_SetPositionMode(GNSS_MODE pt_mode);

/*****************************************************************************
* Function Name : Sunsea_Gnss_Start
* Description   : Open ASR GNSS 5311 dynamic load this chip
* Input         : None
* Output        : None
* Return        : 0 succes    -1 Fail
*****************************************************************************/
int Sunsea_Gnss_Start();

/*****************************************************************************
* Function Name : Sunsea_Gnss_Deinit
* Description   : Deinit ASR GNSS 5311 
* Input         : None
* Output        : None
* Return        : 0 succes    -1 Fail
*****************************************************************************/
int Sunsea_Gnss_Deinit();

/*****************************************************************************
* Function Name : Sunsea_Gnss_Output_Channel_Switch
* Description   : choose channel to output nmea data 
* Input         : channel
                  0:callback   1 Usb nmea port
* Output        : None
* Return        : 0 succes    -1 Fail
*****************************************************************************/
int Sunsea_Gnss_Output_Channel_Switch(NMEA_DATA_OUTPUT_PORT output_port);

/*****************************************************************************
* Function Name : Sunsea_Gnss_AgpsReqData
* Description   : get agps data and input to gnss
* Input         : None
* Output        : None
* Return        : 0 succes    -1 Fail
*****************************************************************************/
int Sunsea_Gnss_AgpsReqData();

/*****************************************************************************
* Function Name : Sunsea_Gnss_Nmea_Info_Switch
* Description   : choose callback return whitch data 
* Input         : switch_data
                  0:nmea    1:loc_info
* Output        : None
* Return        : 0 succes    -1 Fail
*****************************************************************************/
int Sunsea_Gnss_Nmea_Info_Switch(NMEA_LOC_INFO_SWITCH switch_data);



#endif /* SIMCOM_LOC_H */
