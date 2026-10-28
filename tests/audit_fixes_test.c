#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>

#include "func.h"
#include "two_cpu_com_protocol.h"
#include "jmrh_pro.h"
#include "box_firmware.h"

static int g_msgs;
static int g_ota;

static void on_msg(unsigned int msgid, void *content, unsigned int contentlen)
{
	(void)msgid;
	(void)content;
	(void)contentlen;
	g_msgs++;
}

static void on_err(void *pdat, unsigned int len)
{
	(void)pdat;
	(void)len;
}

static void on_ota(OTAParam *p)
{
	(void)p;
	g_ota++;
}

/* Chip decrypt is unavailable in this test. A real build uses dx83 SM4_Decrypt. */
unsigned int sm4_dec1(unsigned char sm4_type, void *psrc, unsigned int srclen, void *pOut, unsigned int outbuflen)
{
	(void)sm4_type;
	(void)psrc;
	(void)srclen;
	(void)pOut;
	(void)outbuflen;
	return 0;
}

static int run_cmd(const char *cmd, const char **out, char *url, unsigned int url_len, char *ack, unsigned int ack_cap, unsigned int *ack_len, int *sync_first)
{
	return mcu_exec_allowlisted_cmd(cmd, (unsigned int)strlen(cmd), out, url, url_len, ack, ack_cap, ack_len, sync_first);
}

static void test_allowlist_rejects_shell(void)
{
	const char *out = 0;
	char url[64];
	char ack[128];
	unsigned int ack_len = 0;
	int sync_first = 0;
	const char *bad[] = {
		"id",
		"reboot;id",
		"wget -O /etc/passwd ftp://h/a.bin",
		"wget -O /data/mcu.bin ftp://h/a.bin;id",
		"wget -O /data/mcu.bin ftp://h/a.bin | sh",
		"wget -O /data/mcu.bin ftp://h/a.bin extra",
		"sh -c /data/mcu.bin ftp://h/a.bin",
	};
	unsigned int i;

	for (i = 0; i < sizeof(bad) / sizeof(bad[0]); i++)
	{
		int rc = run_cmd(bad[i], &out, url, sizeof(url), ack, sizeof(ack), &ack_len, &sync_first);
		if (rc != -1)
		{
			fprintf(stderr, "allowlist accepted: %s (rc=%d)\n", bad[i], rc);
			exit(1);
		}
	}

	sync_first = 1;
	if (run_cmd("reboot", &out, url, sizeof(url), ack, sizeof(ack), &ack_len, &sync_first) != 2 || sync_first != 0)
	{
		fprintf(stderr, "reboot was not allowlisted without running\n");
		exit(1);
	}
	sync_first = 0;
	if (run_cmd("sync; reboot", &out, url, sizeof(url), ack, sizeof(ack), &ack_len, &sync_first) != 2 || sync_first != 1)
	{
		fprintf(stderr, "sync; reboot was not allowlisted\n");
		exit(1);
	}
	sync_first = 0;
	if (run_cmd("sync && reboot", &out, url, sizeof(url), ack, sizeof(ack), &ack_len, &sync_first) != 2 || sync_first != 1)
	{
		fprintf(stderr, "sync && reboot was not allowlisted\n");
		exit(1);
	}
	ack_len = 0;
	if (run_cmd("ps", &out, url, sizeof(url), ack, sizeof(ack), &ack_len, &sync_first) != 0 || ack_len == 0)
	{
		fprintf(stderr, "ps diagnostic was not captured\n");
		exit(1);
	}
}

static void test_safe_ftp_rejects(void)
{
	if (safe_ftp_get("ftp://h/a;id", "u", "p", "/data/test_ota.zip.partial") != -1)
	{
		fprintf(stderr, "safe_ftp accepted shell url\n");
		exit(1);
	}
	if (safe_ftp_get("ftp://h/a.bin", "u;id", "p", "/data/test_ota.zip.partial") != -1)
	{
		fprintf(stderr, "safe_ftp accepted shell user\n");
		exit(1);
	}
	if (safe_ftp_get("http://h/a.bin", "u", "p", "/data/test_ota.zip.partial") != -1)
	{
		fprintf(stderr, "safe_ftp accepted http\n");
		exit(1);
	}
}

static void test_uart_resync(void)
{
	unsigned char buf[128];
	unsigned char pkt[64];
	unsigned int n;
	unsigned int remain;
	unsigned char huge[64];

	memset(buf, 0, sizeof(buf));
	n = tccp_pack(0x0001, "ABC", 3, pkt, sizeof(pkt));
	if (n < 7)
	{
		fprintf(stderr, "pack failed\n");
		exit(1);
	}
	memcpy(buf, pkt, n);
	g_msgs = 0;
	remain = tccp_unpack1(buf, n, on_msg, on_err);
	if ((g_msgs != 1) || (remain != 0))
	{
		fprintf(stderr, "valid frame not consumed msgs=%d remain=%u\n", g_msgs, remain);
		exit(1);
	}

	/* Incomplete frame that still fits must be kept for the next read. */
	memset(buf, 0, sizeof(buf));
	buf[0] = 0xA5;
	buf[1] = 0x5A;
	buf[2] = 0x00;
	buf[3] = 0x01;
	buf[4] = 0x00;
	buf[5] = 0x0A;
	remain = tccp_unpack1(buf, 8, on_msg, on_err);
	if (remain != 8)
	{
		fprintf(stderr, "short frame was not kept remain=%u\n", remain);
		exit(1);
	}

	/* Content length 0xFFFF cannot fit the 20KB reader. */
	memset(huge, 0, sizeof(huge));
	huge[0] = 0xA5;
	huge[1] = 0x5A;
	huge[4] = 0xFF;
	huge[5] = 0xFF;
	remain = tccp_unpack1(huge, sizeof(huge), on_msg, on_err);
	if (remain >= sizeof(huge))
	{
		fprintf(stderr, "oversized frame stalled remain=%u\n", remain);
		exit(1);
	}
}

static void test_jmrh_plaintext_and_cipher(void)
{
	unsigned char plain[64];
	unsigned char cipher[32];
	const char *url = "ftp://h/a";

	memset(plain, 0, sizeof(plain));
	plain[8] = (unsigned char)strlen(url);
	memcpy(plain + 9, url, strlen(url));
	plain[9 + strlen(url)] = 1;
	plain[10 + strlen(url)] = 'u';
	plain[11 + strlen(url)] = 1;
	plain[12 + strlen(url)] = 'p';

	g_ota = 0;
	reg_jmrh_ota_cb_func(on_ota);
	jmrh_analyze_plat_data(plain, sizeof(plain));
	if (g_ota != 1)
	{
		fprintf(stderr, "plaintext ftp:// did not start argv OTA\n");
		exit(1);
	}

	memset(cipher, 0xA5, sizeof(cipher));
	g_ota = 0;
	jmrh_analyze_plat_data(cipher, sizeof(cipher));
	if (g_ota != 0)
	{
		fprintf(stderr, "undecryptable buffer started OTA\n");
		exit(1);
	}
}

static void test_durable_write(void)
{
	char dir[] = "/tmp/auditfixXXXXXX";
	char path[128];
	FILE *fp;
	char got[16];
	size_t n;

	if (mkdtemp(dir) == 0)
	{
		perror("mkdtemp");
		exit(1);
	}
	snprintf(path, sizeof(path), "%s/cfg", dir);
	if (durable_write_file(path, "one", 3) != 0)
	{
		fprintf(stderr, "durable write failed\n");
		exit(1);
	}
	if (durable_write_file(path, "two!", 4) != 0)
	{
		fprintf(stderr, "durable rewrite failed\n");
		exit(1);
	}
	fp = fopen(path, "rb");
	if (fp == 0)
	{
		perror(path);
		exit(1);
	}
	n = fread(got, 1, sizeof(got), fp);
	fclose(fp);
	if ((n != 4) || (memcmp(got, "two!", 4) != 0))
	{
		fprintf(stderr, "durable contents mismatch\n");
		exit(1);
	}
	unlink(path);
	rmdir(dir);
}

static void test_box_name(void)
{
	const char *bad[] = {"../../etc/passwd", "foo;rm", "/usr/lib/libx.so", "..", "lib.so/x", ".hidden", ""};
	unsigned int i;

	for (i = 0; i < sizeof(bad) / sizeof(bad[0]); i++)
	{
		if (box_install_library(bad[i], "A", 1) != -1)
		{
			fprintf(stderr, "box name accepted: %s\n", bad[i]);
			exit(1);
		}
	}
}

int main(void)
{
	test_allowlist_rejects_shell();
	test_safe_ftp_rejects();
	test_uart_resync();
	test_jmrh_plaintext_and_cipher();
	test_durable_write();
	test_box_name();
	printf("audit fix checks passed\n");
	return 0;
}
