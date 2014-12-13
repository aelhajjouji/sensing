#include "ets_sys.h"
#include "osapi.h"
#include "gpio.h"
#include "os_type.h"
#include "user_config.h"
#include "user_interface.h"
#include "espconn.h"
#include "mem.h"
//#define user_procTaskPrio 0
//#define user_procTaskQueueLen 1

os_event_t message;
struct espconn* pespconn;
static volatile os_timer_t polling_timer;
static void loop();
bool connected;
char temp[64];


static void ICACHE_FLASH_ATTR at_tcpclient_discon_cb(void *arg)
{
	//struct espconn *pespconn = (struct espconn *) arg;
	pespconn=NULL;
	os_printf("deconnecté\r\n");
	system_deep_sleep(1000*1000*10);

}
void ICACHE_FLASH_ATTR at_tcpclient_recv(void *arg, char *pdata, unsigned short len)
{
	//struct espconn *pespconn = (struct espconn *) arg;

  	os_printf(pdata);
  	os_printf("\r\n");


}
static void ICACHE_FLASH_ATTR at_tcpclient_sent_cb(void *arg)
{
	//struct espconn *pespconn = (struct espconn *) arg;
	os_printf("\r\nSEND OK\r\n");
	espconn_disconnect(pespconn);

}

static void ICACHE_FLASH_ATTR at_tcpclient_connect_cb(void *arg) {
	//struct espconn *pespconn = (struct espconn *) arg;

	os_printf("tcp client connect from port %d to port %d\r\n",pespconn->proto.tcp->local_port,pespconn->proto.tcp->remote_port);
	os_printf("pespconn %p\r\n", pespconn);
	espconn_regist_disconcb(pespconn, at_tcpclient_discon_cb);
	espconn_regist_recvcb(pespconn, at_tcpclient_recv); ////////
	espconn_regist_sentcb(pespconn, at_tcpclient_sent_cb); ///////

	os_sprintf(temp,"GET /new_value.php?o=rabat&d=%d\r\n",system_adc_read());
	espconn_sent(pespconn, temp, 40);

}


//Main code function
static void ICACHE_FLASH_ATTR loop() {


	if (wifi_station_get_connect_status()== STATION_GOT_IP) {
		//static struct espconn *pCon;
		uint32_t ip = 0;
		os_printf("connected\r\n");
		connected = TRUE;
		pespconn = (struct espconn *) os_zalloc(sizeof(struct espconn));
		pespconn->type = ESPCONN_TCP;
		pespconn->state = ESPCONN_NONE;
		ip = ipaddr_addr("192.168.0.11");
		pespconn->proto.tcp = (esp_tcp *) os_zalloc(sizeof(esp_tcp));
		pespconn->proto.tcp->local_port = espconn_port();
		pespconn->proto.tcp->remote_port = 80;

		os_memcpy(pespconn->proto.tcp->remote_ip, &ip, 4);
		espconn_regist_connectcb(pespconn, at_tcpclient_connect_cb);
		//espconn_regist_reconcb(pCon, at_tcpclient_recon_cb);
		//espconn_regist_time(pespconn,50,0);
		espconn_connect(pespconn);
	}
	else
		system_os_post(0, 0, 0);

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



//Start os task
	system_os_task(loop, 0, &message,1);
	system_os_post(0, 0, 0);



}
