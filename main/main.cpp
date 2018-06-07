/* Simple WiFi Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <string>
#include <cstring>
#include <sstream>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event_loop.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "lwip/err.h"
#include "lwip/sys.h"

#include "free_rtos_timer.h"
#include "lwip_tcp_connection.h"
#include "MQTTClient.h"
#include "MQTTConnect.h"


#define ESP_WIFI_SSID      ""
#define ESP_WIFI_PASS      ""

#define MQTT_SERVER_IP     ""
#define MQTT_SERVER_PORT   1883

#define MQTT_DEVICE_NAME   "esp32"
#define MQTT_USER_NAME     ""
#define MQTT_PASSWORD      ""

/* FreeRTOS event group to signal when we are connected*/
static EventGroupHandle_t wifi_event_group;

/* The event group allows multiple bits for each event,
   but we only care about one event - are we connected
   to the AP with an IP? */
const int WIFI_CONNECTED_BIT = BIT0;

static const char *TAG = "PAHO TEST";

static esp_err_t event_handler(void *ctx, system_event_t *event)
{
    switch(event->event_id) {
    case SYSTEM_EVENT_STA_START:
        esp_wifi_connect();
        break;
    case SYSTEM_EVENT_STA_GOT_IP:
        ESP_LOGI(TAG, "got ip:%s",
                 ip4addr_ntoa(&event->event_info.got_ip.ip_info.ip));
        xEventGroupSetBits(wifi_event_group, WIFI_CONNECTED_BIT);
        break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
        esp_wifi_connect();
        xEventGroupClearBits(wifi_event_group, WIFI_CONNECTED_BIT);
        break;
    default:
        break;
    }
    return ESP_OK;
}

void wifi_init()
{
    wifi_event_group = xEventGroupCreate();

    tcpip_adapter_init();
    ESP_ERROR_CHECK(esp_event_loop_init(event_handler, NULL) );

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    wifi_config_t wifi_config;
    memset(&wifi_config, 0, sizeof(wifi_config_t));
    strcpy((char*)wifi_config.sta.ssid, ESP_WIFI_SSID);
    strcpy((char*)wifi_config.sta.password, ESP_WIFI_PASS);

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config) );
    ESP_ERROR_CHECK(esp_wifi_start() );

    ESP_LOGI(TAG, "wifi_init finished.");
}


void mqtt_task(void *pvParameter) {

    LwipTcpConnection tcpConn;

    MQTT::Client<LwipTcpConnection, FreeRtosTimer> client(tcpConn);

    ESP_LOGI(TAG, "Connect to the server");
    while(!tcpConn.connect(MQTT_SERVER_IP, MQTT_SERVER_PORT)) {
        ESP_LOGI(TAG, "Connection failed... Retry in 5 seconds");
        vTaskDelay( 5000 / portTICK_PERIOD_MS );
    }

    MQTTPacket_connectData options = MQTTPacket_connectData_initializer;
    options.MQTTVersion = 3;
    options.clientID.cstring = (char*)MQTT_DEVICE_NAME;
    options.username.cstring = (char*)MQTT_USER_NAME;
    options.password.cstring = (char*)MQTT_PASSWORD;

    ESP_LOGI(TAG, "Open MQTT connection");
    while(0 != client.connect(options)) {
        ESP_LOGI(TAG, "Failed... Retry in 5 seconds");
        vTaskDelay( 5000 / portTICK_PERIOD_MS );
    }

    std::string aliveStr = "ALIVE";
    int dummyTemp = 0;
    while (true) {
        std::stringstream tempSs;
        tempSs << dummyTemp;

        std::string tempStr = tempSs.str();
        dummyTemp++;

        vTaskDelay(1000 / portTICK_PERIOD_MS);

        client.publish("esp/status", (void*)aliveStr.c_str(), aliveStr.size());
        client.publish("esp/temp", (void*)tempStr.c_str(), tempStr.size());
    }

}



extern "C" {
void app_main() {
    //Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    wifi_init();

    vTaskDelay( 5000 / portTICK_PERIOD_MS );

    xTaskCreate(&mqtt_task, "mqtt_task", 4096, NULL, 5, NULL);
}
}