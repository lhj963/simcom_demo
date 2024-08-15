#include "ls_gps.h"

void open_gps(void)
{
    int ret = -1;

    //printf("ls_oem_gps_open().\n");
    ret = ls_oem_gps_open();
    if(ret)
        printf("GPS open fail.\n");

    //printf("ls_oem_gps_start().\n");
    ret = ls_oem_gps_start();
    if(ret)
        printf("GPS start fail.\n");
}


void update_gps_info_proc(void)
{
    ls_oem_gps_info *pGps_info = {0};

    pGps_info = ls_oem_get_gps_info();
    //printf("[gps](%f,%f) speed:%f gaodu:%f\r\n",pGps_info->longitude,pGps_info->latitude,pGps_info->speedHorizontal,pGps_info->altitudeWrtEllipsoid);

    if ((pGps_info->longitude > 1) && (pGps_info->latitude > 1))
    {
        set_lon(pGps_info->longitude * 1000000);
        set_lat(pGps_info->latitude * 1000000);

        set_mpu_gps_local_status(1);
    }
    else
    {
        set_mpu_gps_local_status(0);
    }

    set_gps_speed(pGps_info->speedHorizontal * 10);
    set_gps_gaodu(pGps_info->altitudeWrtEllipsoid);
    set_gps_angle(pGps_info->heading);

#if 0
    ret = ls_oem_gps_stop();
    if(ret)
        printf("GPS stop fail.\n");  
#endif
}






