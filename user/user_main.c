#include "ets_sys.h"
#include "osapi.h"
#include "gpio.h"
#include "os_type.h"
#include "user_config.h"
#include "user_interface.h"
#include "espconn.h"
#include "mem.h"
#include "driver/uart.h"
#include "driver/i2c_master.h"
#include "driver/adc.h"

//#define user_procTaskPrio 0
//#define user_procTaskQueueLen 1

os_event_t * user_procTaskConnect;
os_event_t * user_procTaskSerial;

//struct espconn* pCon;

//static volatile os_timer_t polling_timer;
static void loop();
static void readserial();
bool connected;
char temp[64];
uint16 adc;


void polling_timer_elapsed(void *arg) {
	return;
}

static void ICACHE_FLASH_ATTR tcpclient_discon_cb(void *arg)
{

	 struct espconn *pespconn = (struct espconn *)arg;
	os_free(pespconn->proto.tcp);
	os_free(pespconn);
	os_printf("disconnected\r\n");
	system_deep_sleep(1000*1000*60);
	//espconn_connect(pespconn);

}

void ICACHE_FLASH_ATTR tcpclient_recv(void *arg, char *pdata, unsigned short len)
{
	struct espconn *pespconn = (struct espconn *) arg;

  	os_printf(pdata);
  	os_printf("\r\n");

  	//os_sprintf(temp,"?o=%s&d=30\r\n",pdata);
	//espconn_sent(pespconn, temp, 64);
}

static void ICACHE_FLASH_ATTR tcpclient_sent_cb(void *arg)
{
	struct espconn *pespconn = (struct espconn *) arg;
	os_printf("\r\nSEND OK\r\n");
	espconn_disconnect(pespconn);

}


static void ICACHE_FLASH_ATTR tcpclient_connect_cb(void *arg) {
	struct espconn *pespconn = (struct espconn *) arg;

	os_printf("tcp client connect\r\n");
	os_printf("pespconn %p\r\n", pespconn);
	espconn_regist_disconcb(pespconn, tcpclient_discon_cb);
	//espconn_regist_recvcb(pespconn, tcpclient_recv); ////////
	espconn_regist_sentcb(pespconn, tcpclient_sent_cb); ///////



	adc=system_adc_read();
	os_sprintf(temp,"GET /new_value.php?o=rabat&d=%d\r\n",adc & 0x3FF);
	espconn_sent(pespconn, temp, 40);

}

static void ICACHE_FLASH_ATTR tcpclient_recon_cb(void *arg, sint8 errType) {
	struct espconn *pespconn = (struct espconn *) arg;

	os_printf("at_tcpclient_recon_cb %p\r\n", arg);
}


static void ICACHE_FLASH_ATTR readserial(){


}



//Main code function
static void ICACHE_FLASH_ATTR readSensor() {
static struct espconn *pCon;
//pTcpServer->proto.tcp->local_port=8888;

	while (wifi_station_get_connect_status()!= STATION_GOT_IP )
		os_delay_us(250000);

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
		espconn_regist_connectcb(pCon, tcpclient_connect_cb);
		espconn_regist_reconcb(pCon, tcpclient_recon_cb);
		//espconn_regist_time(pCon,500,0);
		espconn_connect(pCon);

	return;
}

//Init function
void user_init(void) {
	char ssid[32] = "freebox_GGHTOO";
	char password[64] = "iizxabpfiizxabpfiizxabpf";
	struct station_config stationConf;
//Set station mode
	wifi_set_opmode( STATION_MODE);
	//wifi_set_phy_mode(PHY_MODE_11N);
//Set ap settings
	os_memcpy(&stationConf.ssid, ssid, 32);
	os_memcpy(&stationConf.password, password, 32);
	wifi_station_set_config(&stationConf);

//Disarm timer
//	os_timer_disarm(&polling_timer);

//Setup timer
//	os_timer_setfn(&polling_timer, (os_timer_func_t *) polling_timer_elapsed,NULL);

//Set up the timer, (timer, milliseconds, 1=cycle 0=once)
//	os_timer_arm(&polling_timer, 1000, 1);

//Start os task
//	system_os_task(loop, 0, user_procTaskConnect,1);
//	system_os_task(readserial,1,user_procTaskSerial,1);
//	system_os_post(0, 0, 0);
readSensor();
//system_deep_sleep(10*1000*1000);

}
