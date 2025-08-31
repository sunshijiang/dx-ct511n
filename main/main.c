#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"
#include "esp_log.h"

#define UART_PORT       UART_NUM_1
#define UART_TX_PIN     21   // ESP32-C3 TX → 模块 RX
#define UART_RX_PIN     20   // ESP32-C3 RX → 模块 TX
#define UART_BAUD       115200

static const char *TAG = "MAIN";

/**
 * 发送 AT 命令到模组
 */
static void send_at_cmd(const char *cmd)
{
    uart_write_bytes(UART_PORT, cmd, strlen(cmd));
    uart_write_bytes(UART_PORT, "\r\n", 2);
    ESP_LOGI(TAG, "Send: %s", cmd);
}

/**
 * UART 初始化
 */
static void uart_init(void)
{
    const uart_config_t uart_config = {
        .baud_rate = UART_BAUD,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
    };

    ESP_ERROR_CHECK(uart_driver_install(UART_PORT, 2048, 0, 0, NULL, 0));
    ESP_ERROR_CHECK(uart_param_config(UART_PORT, &uart_config));
    ESP_ERROR_CHECK(uart_set_pin(UART_PORT, UART_TX_PIN, UART_RX_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));

    ESP_LOGI(TAG, "UART init done");
}

/**
 * 主任务
 */
void app_main(void)
{
    uart_init();

    // 配置 APN
    send_at_cmd("AT+QICSGP=1,1,\"cmnbiot\",\"\",\"\"");
    vTaskDelay(pdMS_TO_TICKS(1000));

    // 打开网络
    send_at_cmd("AT+NETOPEN");
    vTaskDelay(pdMS_TO_TICKS(2000));

    // MQTT 配置
    send_at_cmd("AT+MCONFIG=\"DX-CT511N\",\"dx-ct511\",\"191910\",0,0");
    vTaskDelay(pdMS_TO_TICKS(1000));

    // 连接 MQTT 服务器
    send_at_cmd("AT+MIPSTART=122.51.175.127,1883,3");
    vTaskDelay(pdMS_TO_TICKS(2000));

    send_at_cmd("AT+MCONNECT=1,60");
    vTaskDelay(pdMS_TO_TICKS(2000));

    // 订阅主题
    send_at_cmd("AT+MSUB=\"/sun/mqtt/\",0");
    vTaskDelay(pdMS_TO_TICKS(1000));

    // 发布测试消息
    send_at_cmd("AT+MPUB=\"/sun/mqtt/\",0,0,\"test\"");
    vTaskDelay(pdMS_TO_TICKS(1000));

    ESP_LOGI(TAG, "MQTT setup done");
}
