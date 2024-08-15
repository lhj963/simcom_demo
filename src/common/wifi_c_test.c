#include "wifi_c_test.h"


/*=============================================
	Convert MAC Hex To String

=============================================*/
int convert_MAC_asc_to_str(unsigned char *in_str, char *out_str)
{
	int i = 0;

	for (i = 0; i < 6; i++) {
		if ((in_str[i] >> 4) >= 10) {
			out_str[i * 3] = 'A' + (in_str[i] >> 4) - 0x0A;
		} else
			out_str[i * 3] = '0' + (in_str[i] >> 4);

		if (((in_str[i]) & 0x0F) >= 10) {
			out_str[i * 3 + 1] = 'A' + (in_str[i] & 0x0F) - 0x0A;
		} else
			out_str[i * 3 + 1] = '0' + (in_str[i] & 0x0F);
		out_str[i * 3 + 2] = ':';
	}
	out_str[17] = '\0';
	return 0;
}

/*=============================================
	Convert IP Hex To String
=============================================*/

int convert_ip_asc_to_str(unsigned char *in_str, char *out_str)
{
	int i = 0;
	int index = 0;

	for (i = 0; i < 4; i++) {
		if (in_str[i] >= 100)
			out_str[index++] = '0' + (in_str[i] / 100);
		if (in_str[i] >= 10)
			out_str[index++] = '0' + (in_str[i] % 100) / 10;
		out_str[index++] = '0' + in_str[i] % 10;
		out_str[index++] = '.';
	}
	out_str[index - 1] = '\0';
	return 0;
}


//连头结点一起释放
void client_info_list_destroy(client_info_node_ptr client_info_head_node)
{
    client_info_node_ptr p = client_info_head_node;
    while (p)
    {
        client_info_head_node = client_info_head_node->next;
        free(p);
        //delete(p);
        p = client_info_head_node;
    }
}


//保留头结点，置头结点的next指针为NULL
void client_info_list_empty(client_info_node_ptr client_info_head_node)
{
    client_info_node_ptr p;
    while (client_info_head_node->next)
    {
        p = client_info_head_node->next;
        client_info_head_node->next = p->next;
        free(p);
        //delete(p);
    }
}


void client_info_list_display(client_info_node_ptr client_info_head_node)
{
    client_info_node_ptr p;
    if(client_info_head_node)
    {
      p = client_info_head_node->next;
    }
    else
    {
        return;
    }

    char mac_str[18] = {};
    while (p != NULL)
    {
        memset(mac_str, 0, sizeof(mac_str));
        convert_MAC_asc_to_str(p->mac, mac_str);
       // printf("%s  %s  %s\n",p->name,mac_str,p->ip);
       printf("%-35s",p->name);
       printf("%-20s",mac_str);
       memset(mac_str, 0, sizeof(mac_str));
       convert_ip_asc_to_str(p->ip, mac_str);
       printf("%-16s",mac_str);
       printf("%-2d\n",p->status);
       p = p->next;
    }
}



/*=============================================
   单向链表的释放，连头结点一起释放
=============================================*/
void sta_scan_info_list_destroy(sta_scan_info_node_ptr head)
{
    sta_scan_info_node_ptr p = head;
    while (p)
    {
        head = head->next;
        free(p);
        //delete(p);
        p = head;
    }
}

/*=============================================
   单向链表的释放，保留头结点
=============================================*/
void sta_scan_info_list_empty(sta_scan_info_node_ptr head)
{
    sta_scan_info_node_ptr p;
    while (head->next)
    {
        p = head->next;
        head->next = p->next;
        free(p);
        //delete(p);
    }
}

/*=============================================
   遍历链表，打印bssid,frequecy,sig_level,flag,ssid等信息
=============================================*/
void sta_scan_info_list_display(sta_scan_info_node_ptr head)
{
    sta_scan_info_node_ptr p;
    if(head)
    {
      p = head->next;
    }
    else
    {
        return;
    }

    while (p != NULL)
    {
       printf("%-20s",p->bssid);
       printf("%-6d",p->freq);
       printf("%-6d",p->signal_level);
       printf("%-6d",p->flags);
       printf("%s\n",p->ssid);
       p = p->next;
    }
}




