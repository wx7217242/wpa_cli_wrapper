#include "wpa_cli_oper.h"
#include "libwpa_cli_wrapper.h"

#include <stdlib.h>
#include <strings.h>
#include <string.h>

int get_wifi_config(WifiConfig* config)
{
	return -1;
}

smartlist_t* get_connected_wifi_list()
{
	return NULL;
}

static WifiScanItem* get_wifi_scan_item_from_line(const char* line, int line_len)
{
	char* buf = (char*)malloc(line_len + 1);
	memset(buf, 0x00, line_len + 1);
	memcpy(buf, line, line_len);

	const char *delim = "\t";

	char* p = NULL;
	char *saveptr = NULL;

	char fields[5][256] = { 0 };
	int field_count = 0;

	p = strtok_r(buf, delim, &saveptr);
	while (p != NULL)
	{
		strcpy(fields[field_count], p);
		++field_count;
		p = strtok_r(NULL, delim, &saveptr);
	}

	free(buf);
	buf = NULL;

	WifiScanItem* item = NULL;
	if (field_count == 5)
	{
		item = (WifiScanItem*)malloc(sizeof(WifiScanItem));
		if (item == NULL)
		{
			return NULL;
		}
		memset(item, 0x00, sizeof(WifiScanItem));
		field_count = 0;
		strcpy(item->bssid, fields[field_count++]);
		item->frequency = atoi(fields[field_count++]);
		item->signal_level = atoi(fields[field_count++]);
		strcpy(item->flags, fields[field_count++]);
		strcpy(item->ssid, fields[field_count]); // TODO: 16进制转字符串
	}

	return item;
}

smartlist_t* get_wifi_scan_results()
{
	char buf[4096] = { 0 };
	int buf_len = sizeof(buf);
	if (wpa_cli_execute("scan", buf, buf_len) < 0)
	{
		return NULL;
	}

	if (strncasecmp(buf, "OK", 2) != 0)
	{
		return NULL;
	}

	memset(buf, 0x00, buf_len);

	if (wpa_cli_execute("scan_results", buf, buf_len) < 0)
	{
		return NULL;
	}

	smartlist_t* scan_results = smartlist_new();
	if (scan_results == NULL)
	{
		return NULL;
	}

	const char *delim = "\n";

	char* p = NULL;
	char *saveptr = NULL;

	p = strtok_r(buf, delim, &saveptr);
	while (p != NULL)
	{
		WifiScanItem* item = get_wifi_scan_item_from_line(p, strlen(p));
		if (item != NULL)
		{
			smartlist_add(scan_results, item);
		}
		p = strtok_r(NULL, delim, &saveptr);
	}

	return scan_results;
}

static int wpa_cli_comm_exec(const char* cmd)
{
	char buf[256] = { 0 };
	int buf_len = sizeof(buf);

	if (wpa_cli_execute(cmd, buf, buf_len) < 0)
	{
		return -1;
	}
	if (strncasecmp(buf, "OK", 2) != 0)
	{
		return -1;
	}

	return 0;
}

static int add_network(char* network_id, int len)
{
	char buf[256] = { 0 };
	int buf_len = sizeof(buf);

	// add_network
	if (wpa_cli_execute("add_network", buf, buf_len) < 0)
	{
		return -1;
	}

	strcpy(network_id, buf);
	return 0;
}

static int set_ssid(const char* network_id, const char* ssid)
{
	char cmd[256] = { 0 };
	snprintf(cmd, sizeof(cmd), "set_network %s ssid %s", network_id, ssid);

	return wpa_cli_comm_exec(cmd);
}


static int set_password(const char* network_id, int flag, const char* password)
{
	char set_cmd[256] = { 0 };

	int res = -1;

	// 0 wep, 1 wpa, 2 无密码
	switch (flag)
	{
	case 0:
	{
		// set_network 1 key_mgmt NONE
		snprintf(set_cmd, sizeof(set_cmd), "set_network %s key_mgmt NONE", network_id);
		if (wpa_cli_comm_exec(set_cmd) != 0)
		{
			break;
		}

		// > set_network 1 wep_key0 "your ap password"
		snprintf(set_cmd, sizeof(set_cmd), "set_network %s wep_key0 %s", network_id, password);
		res = wpa_cli_comm_exec(set_cmd);
		break;
	}
	case 1:
	{
		// set_network 2 psk "your pre-shared key"
		snprintf(set_cmd, sizeof(set_cmd), "set_network %s psk %s", password);
		res = wpa_cli_comm_exec(set_cmd);
		break;
	}
	case 2:
	{
		// 无密码的wifi
		break;
	}
	default:
	{
		break;
	}
	}

	return res;
}

static int enable_network(const char* network_id)
{
	char cmd[256] = { 0 };
	snprintf(cmd, sizeof(cmd), "enable_network %s", network_id);

	return wpa_cli_comm_exec(cmd);
}

static int disable_network(const char* network_id)
{
	char cmd[256] = { 0 };
	snprintf(cmd, sizeof(cmd), "disable_network %s", network_id);

	return wpa_cli_comm_exec(cmd);
}

static int remove_network(const char* network_id)
{
	char cmd[256] = { 0 };
	snprintf(cmd, sizeof(cmd), "remove_network %s", network_id);

	return wpa_cli_comm_exec(cmd);
}

int connect_wifi(WifiConfig* config)
{
	char network_id[16] = { 0 };

	do
	{
		if (add_network(network_id, sizeof(network_id)) != 0)
		{
			break;
		}

		if (set_ssid(network_id, config->ssid) != 0)
		{
			break;
		}

		if (set_password(network_id, config->flag, config->password) != 0)
		{
			break;
		}

		if (enable_network(network_id) != 0)
		{

			break;
		}

		return 0;

	} while (0);


	if (network_id[0] != '\0')
	{
		// TODO: delete network
		disable_network(network_id);
		remove_network(network_id);
	}

	return -1;
}

int disconnect_wifi(WifiConfig* config)
{
	
	return -1;
}

int get_wifi_status(WifiStatus* status)
{
	return -1;
}
