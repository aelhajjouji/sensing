#include "ets_sys.h"
#include "osapi.h"
#include "gpio.h"
#include "os_type.h"
#include "user_config.h"
#include "user_interface.h"
#include "espconn.h"
#include "mem.h"
#define user_procTaskPrio 0
#define user_procTaskQueueLen 1

os_event_t user_procTaskQueue[user_procTaskQueueLen];
//struct espconn* pCon;
static volatile os_timer_t polling_timer;
static void loop();
bool connected;
char temp[64];

void polling_timer_elapsed(void *arg) {
	return;
}

static void ICACHE_FLASH_ATTR at_tcpclient_discon_cb(void *arg)
{
	struct espconn *pespconn = (struct espconn *) arg;
	os_printf("disconnected\r\n");
	espconn_connect(pespconn);

}
void ICACHE_FLASH_ATTR at_tcpclient_recv(void *arg, char *pdata, unsigned short len)
{
	struct espconn *pespconn = (struct espconn *) arg;

  	os_printf(pdata);
  	os_printf("\r\n");
	system_deep_sleep(1000*1000*10);
  	//os_sprintf(temp,"?o=%s&d=30\r\n",pdata);
	//espconn_sent(pespconn, temp, 64);
}
static void ICACHE_FLASH_ATTR at_tcpclient_sent_cb(void *arg)
{
os_printf("\r\nSEND OK\r\n");
}

static void ICACHE_FLASH_ATTR at_tcpclient_connect_cb(void *arg) {
	struct espconn *pespconn = (struct espconn *) arg;

	os_printf("tcp client connect\r\n");
	os_printf("pespconn %p\r\n", pespconn);
	espconn_regist_disconcb(pespconn, at_tcpclient_discon_cb);
	espconn_regist_recvcb(pespconn, at_tcpclient_recv); ////////
	espconn_regist_sentcb(pespconn, at_tcpclient_sent_cb); ///////

	os_sprintf(temp,"GET /new_value.php?o=rabat&d=%d\r\n",system_adc_read());
	espconn_sent(pespconn, temp, 40);

}

static void ICACHE_FLASH_ATTR at_tcpclient_recon_cb(void *arg, sint8 errType) {
	struct espconn *pespconn = (struct espconn *) arg;

	os_printf("at_tcpclient_recon_cb %p\r\n", arg);
}

//Main code function
static void ICACHE_FLASH_ATTR loop() {
static struct espconn *pCon;
//pTcpServer->proto.tcp->local_port=8888;

	if (wifi_station_get_connect_status()== STATION_GOT_IP&& connected==FALSE) {
		uint32_t ip = 0;
		os_printf("connected\r\n");
		connected = TRUE;
		pCon = (struct espconn *) os_zalloc(sizeof(struct espconn));
		pCon->type = ESPCONN_TCP;
		pCon->state = ESPCONN_NONE;
		ip = ipaddr_addr("192.168.0.11");
		pCon->proto.tcp = (esp_tcp *) os_zalloc(sizeof(esp_tcp));
		pCon->proto.tcp->local_port = espconn_port();
		pCon->proto.tcp->remote_port = 80;

		os_memcpy(pCon->proto.tcp->remote_ip, &ip, 4);
		espconn_regist_connectcb(pCon, at_tcpclient_connect_cb);
		espconn_regist_reconcb(pCon, at_tcpclient_recon_cb);
		espconn_regist_time(pCon,500,0);
		espconn_connect(pCon);
	}
	else
		system_os_post(user_procTaskPrio, 0, 0);

	return;
}

//Init function
void user_init(void) {
	char ssid[32] = "freebox_GGHTOO";
	char password[64] = "iizxabpfiizxabpfiizxabpf";
	struct station_config stationConf;
//Set station mode
	wifi_set_opmode( STATION_MODE);
//Set ap settings
	os_memcpy(&stationConf.ssid, ssid, 32);
	os_memcpy(&stationConf.password, password, 32);
	wifi_station_set_config(&stationConf);

//Disarm timer
	os_timer_disarm(&polling_timer);

//Setup timer
	os_timer_setfn(&polling_timer, (os_timer_func_t *) polling_timer_elapsed,
	NULL);

//Set up the timer, (timer, milliseconds, 1=cycle 0=once)
	os_timer_arm(&polling_timer, 1000, 1);

//Start os task
	system_os_task(loop, user_procTaskPrio, user_procTaskQueue,
	user_procTaskQueueLen);
	system_os_post(user_procTaskPrio, 0, 0);

//system_deep_sleep(10*1000*1000);

}
