#ifndef __TWO_CPU_COM_PROTOCOL_H__
#define __TWO_CPU_COM_PROTOCOL_H__
#ifdef __cplusplus
extern "C"
{
#endif

// �����ʹӻ������Է��͵���Ϣ
#define TCCP_FILE_SEND_START_MSG_ID 0x2002	// �����ļ���Ϣ
#define TCCP_SEND_FILE_MSG_ID 0x2000		// �����ļ���Ϣ
#define TCCP_FILE_SEND_FINISH_MSG_ID 0x2001 // �ļ����������Ϣ
#define TCCP_FILE_SEND_NEXT_MSG_ID 0x2003	// �ļ�������һ��

// �ӻ����͵���Ϣ
#define TCCP_SLAVE_SEND_CAN_MSG_ID 0x1000	 // �ӻ�����CAN������Ϣ
#define TCCP_SLAVE_UN_ACK_MSG_ID 0x1001		 // �ӻ���ͨ��Ӧ��
#define TCCP_SLAVE_UPDATA_RES_MSG_ID 0x1002	 // �ӻ��㱨�������
#define TCCP_SLAVE_REPORT_VER_MSG_ID 0x1003	 // �ӻ��㱨�����汾��
#define TCCP_SLAVE_ACK_BEIAN_E_MSG_ID 0x1004 // �ӻ��㱨���ı�����Ϣ
#define TCCP_SLAVE_ACK_BEIAN_M_MSG_ID 0x1005 // �ӻ�Ӧ�����ı�����Ϣ
#define TCCP_SLAVE_ACK_SIGNA_MSG_ID 0x1006	 // �ӻ�Ӧ��ǩ������

// �������͵���Ϣ
#define TCCP_HOST_UN_ACK_MSG_ID 0x0001				// ����ͨ��Ӧ��
#define TCCP_HOST_UPDATA_RES_MSG_ID 0x0002			// �����㱨�������
#define TCCP_HOST_QUERY_SLAVE_CPU_VER_MSG_ID 0x0003 // ������ѯ�ӻ��汾��
#define TCCP_HOST_REQ_BEIAN_E_MSG_ID 0x0004			// �����������ı�����Ϣ
#define TCCP_HOST_REQ_BEIAN_M_MSG_ID 0x0005			// �����������ı�����Ϣ
#define TCCP_HOST_REQ_SIGNA_MSG_ID 0x0006			// ��������ǩ������

#define TCCP_GZ_HEART_BEAT_MSG_ID 0x3000			// ��װ����
#define TCCP_GZ_MASTER_SET_CHANNEL_MSG_ID 0x3002	// ��װ��CPU����ͨ����
#define TCCP_AUTO_TEST_MSG_ID 0x3004				// ����/ֹͣ�Զ�������
#define TCCP_REPORT_DOWNLOAD_PROGRESS_MSG_ID 0x3006 // �㱨�豸�������ؽ���
#define TCCP_ACK_DOWNLOAD_PROGRESS_MSG_ID 0x3007	// ��λ����Ӧ���յ��豸�������ؽ�����Ϣ

#define TCCP_GZ_MASTER_DISP_INFO_MSG_ID 0x3008 // ��λ������ʾ���·���ʾ��Ϣ

#define TCCP_DEVICE_HEART_BEAT_MSG_ID 0x4000  // �豸����
#define TCCP_DEVICE_HEART_BEAT_MSG_ID1 0x4001 // �豸����

#define TCCP_FORWARD_MODE_MSG_ID 0x5000			 // ����/�˳� ת��ģʽ
#define TCCP_GZ_SLAVE_FORWARD_MODE_MSG_ID 0x6000 // ����/�˳� �빤װ�ӻ��Ի�ģʽ

#define TCCP_USART_DOWNLOAD_FILE_MSG_ID 0x7000		// ������������app�ļ�
#define TCCP_USART_DOWNLOAD_BOOT_FILE_MSG_ID 0x7002 // ������������boot�ļ�

#define TCCP_TRANS_CMD_MSG_ID 0x9000	 // ͸���豸ָ��
#define TCCP_ACK_TRANS_CMD_MSG_ID 0x9001 // ��Ӧ͸���豸ָ��

#define TCCP_GZ_SLAVE_SPEAK 0x9002				  // ��װ�ӻ�����������Ϣ��ϢID
#define TCCP_SET_BACKDOOR_HEART_INV_MSG_ID 0x9004 // ����Զ�̹�������ʱ����

#define TCCP_RP_CAN_CMD_MSG_ID 0x9101 // �ϱ�CAN����ָ��

// mcu -> mpu
#define TCCP_MCU_SEND_NET_DATA_MSG_ID 0xA000 // ���������ϴ�ָ��
#define TCCP_MCU_TO_MPU_TEST_MSG_ID 0xA001	 // ���������ϴ�ָ��
#define TCCP_MCU_TO_MPU_PARAM_MSG_ID 0xA002
#define TCCP_MCU_TO_MPU_EXE_LINUX_CMD_MSG_ID 0xA003
#define TCCP_MCU_TO_MPU_STORE_CAN_DATA_MSG_ID 0xA004
#define TCCP_MCU_TO_MPU_PARAM_EXT_MSG_ID 0xA005
#define TCCP_MCU_TO_MPU_STATUS_MSG_ID 0xA006
#define TCCP_MCU_TO_MPU_STORE_CACHE_MSG_ID 0xA007
#define TCCP_MCU_TO_MPU_GET_CACHE_MSG_ID 0xA008
#define TCCP_MCU_TO_MPU_GET_SM2_SIGN_MSG_ID 0xA009
#define TCCP_MCU_TO_MPU_DBG_DISP_MSG_ID 0xA00A
#define TCCP_MCU_TO_MPU_SAVE_LOG_MSG_ID 0xA00B	   // mcu log
#define TCCP_MCU_TO_MPU_SAVE_LOG_HEX_MSG_ID 0xA00C // mcu log
#define TCCP_MCU_TO_MPU_STORE_E_CHANGE_PARAM_MSG_ID 0xA00D
#define TCCP_MCU_TO_MPU_FTP_UPLOAD_FILE_MSG_ID 0xA00E
#define TCCP_MCU_TO_MPU_FTP_UPLOAD_FILE1_MSG_ID 0xA00F
#define TCCP_MCU_TO_MPU_GET_FILE_DATA_MSG_ID 0xA010 // GET FILE DATA
#define TCCP_MCU_TO_MPU_SEND_USER_MSG_MSG_ID 0xA011 // 发送用户消息
#define TCCP_MCU_TO_MPU_EXE_AT_CMD_MSG_ID 0xA012	// AT
#define TCCP_MCU_TO_MPU_SEND_SMS_MSG_ID 0xA013		// send sms

// mpu->mcu
#define TCCP_MPU_ACK_MCU_SEND_NET_DATA_MSG_ID 0xA100 // ���������ϴ�ָ��
#define TCCP_NET_DATA_ARRIVE_MSG_ID 0xA101			 // ���������ϴ�ָ��
#define TCCP_MPU_HEAT_MSG_ID 0xA102					 // MPU����
#define TCCP_MPU_SEND_PARAM_MSG_ID 0xA103
#define TCCP_MPU_SEND_ONLINE_MSG_ID 0xA104
#define TCCP_MPU_SEND_OFFLINE_MSG_ID 0xA105
#define TCCP_MPU_ACK_MCU_MSG_ID 0xA106
#define TCCP_MPU_TO_MCU_PARAM_EXT_MSG_ID 0xA107 // MPU send param
#define TCCP_MPU_TO_MCU_STATUS_MSG_ID 0xA108
#define TCCP_MPU_TO_MCU_CACHE_DATA_ARRIVE_MSG_ID 0xA109
#define TCCP_MPU_TO_MCU_SM2_SIGN_DATA_ARRIVE_MSG_ID 0xA10A
#define TCCP_MPU_TO_MCU_BEIAN_MSGBODY_ARRIVE_MSG_ID 0xA10B
#define TCCP_MPU_TO_MCU_E_PARAM_ARRIVE_MSG_ID 0xA10D
#define TCCP_MPU_TO_MCU_FILE_DATA_ARRIVE_MSG_ID 0xA10E // 文件数据达到
#define TCCP_MPU_TO_MCU_USER_MSG_ARRIVE_MSG_ID 0xA10F  // 用户消息达到
#define TCCP_MPU_TO_MCU_USUAL_ACK_MSG_ID 0xA110		   // mpu usual ack
#define TCCP_MPU_TO_MCU_SMS_ARRIVE_MSG_ID 0xA111	   // sms arrive

#define PARAM_ID_FROM_MCU_SOFTWARE_VERSION 0x0001
#define PARAM_ID_FROM_MCU_MAIN_POWER_VOL 0x0002
#define PARAM_ID_FROM_MCU_BAT_VOL 0x0003
#define PARAM_ID_FROM_MCU_SERVER 0x0004
#define PARAM_ID_FROM_MCU_CHIP_ID 0x0005
#define PARAM_ID_FROM_MCU_POSITION 0x0006

#define PARAM_ID_FROM_MPU_SOFTWARE_VERSION 0x8001
#define PARAM_ID_FROM_MPU_GPS_SPEED 0x8002
#define PARAM_ID_FROM_MPU_SIM_SIGNAL_STRENGTH 0x8003
#define PARAM_ID_FROM_MPU_GAODU 0x8004
#define PARAM_ID_FROM_MPU_ANGLE 0x8005
#define PARAM_ID_FROM_MPU_SERVER0_ADDR 0x8006
#define PARAM_ID_FROM_MPU_SERVER0_PORT 0x8007
#define PARAM_ID_FROM_MPU_SERVER1_ADDR 0x8008
#define PARAM_ID_FROM_MPU_SERVER1_PORT 0x8009
#define PARAM_ID_FROM_MPU_SERVER2_ADDR 0x800A
#define PARAM_ID_FROM_MPU_SERVER2_PORT 0x800B
#define PARAM_ID_FROM_MPU_SERVER3_ADDR 0x800C
#define PARAM_ID_FROM_MPU_SERVER3_PORT 0x800D
#define PARAM_ID_FROM_MPU_VIN 0x800E
#define PARAM_ID_FROM_MPU_IMEI 0x800F
#define PARAM_ID_FROM_MPU_ICCID 0x8010
#define PARAM_ID_FROM_MPU_GET_MCU_PARAM_SW 0x8011
#define PARAM_ID_FROM_MPU_PUBLIC_KEY 0x8012
#define PARAM_ID_FROM_MPU_CHIP_ID 0x8013

#define PARAM_ID_FROM_MPU_SERVER4_ADDR 0x8016
#define PARAM_ID_FROM_MPU_SERVER4_PORT 0x8017
#define PARAM_ID_FROM_MPU_SERVER5_ADDR 0x8018
#define PARAM_ID_FROM_MPU_SERVER5_PORT 0x8019
#define PARAM_ID_FROM_MPU_SERVER6_ADDR 0x801A
#define PARAM_ID_FROM_MPU_SERVER6_PORT 0x801B
#define PARAM_ID_FROM_MPU_SERVER7_ADDR 0x801C
#define PARAM_ID_FROM_MPU_SERVER7_PORT 0x801D
#define PARAM_ID_FROM_MPU_STATUS1 0x801E

	extern unsigned int tccp_pack(unsigned short msgid, void *content, unsigned int contentlen,
								  void *pOutBuf, unsigned int outbuflen);

	/* ��� */
	extern int tccp_unpack(void *pdat, unsigned int len,
						   void (*pCallBackAnalyze)(unsigned int msgid, void *content, unsigned int contentlen),
						   void (*pCallBackError)(void *pdat, unsigned int len));

	extern unsigned int tccp_unpack1(void *pdat, unsigned int len,
									 void (*pCallBackAnalyze)(unsigned int msgid, void *content, unsigned int contentlen),
									 void (*pCallBackError)(void *pdat, unsigned int len));

#ifdef __cplusplus
}
#endif
#endif
