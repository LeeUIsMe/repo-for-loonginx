#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include "sensor_adc_alarm.h"

// 检查阈值并触发报警
void check_thresholds(Sensor_Data *data) {
    int alarm = 0;
    
    // 检查温度阈值
    if (data->temperature > TEMP_THRESHOLD_HIGH) {
        printf("ALERT: High temperature (%.1f > %.1f)!\n", 
               data->temperature, TEMP_THRESHOLD_HIGH);
        alarm = 1;
    } else if (data->temperature < TEMP_THRESHOLD_LOW) {
        printf("ALERT: Low temperature (%.1f < %.1f)!\n", 
               data->temperature, TEMP_THRESHOLD_LOW);
        alarm = 1;
    }
    
    // 检查湿度阈值
    if (data->humidity > HUMI_THRESHOLD_HIGH) {
        printf("ALERT: High humidity (%.1f > %.1f)!\n", 
               data->humidity, HUMI_THRESHOLD_HIGH);
        alarm = 1;
    } else if (data->humidity < HUMI_THRESHOLD_LOW) {
        printf("ALERT: Low humidity (%.1f < %.1f)!\n", 
               data->humidity, HUMI_THRESHOLD_LOW);
        alarm = 1;
    }
    
    // 检查光照阈值
    if (data->light > LIGHT_THRESHOLD_HIGH) {
        printf("ALERT: High light (%d > %d)!\n", 
               data->light, LIGHT_THRESHOLD_HIGH);
        alarm = 1;
    } else if (data->light < LIGHT_THRESHOLD_LOW) {
        printf("ALERT: Low light (%d < %d)!\n", 
               data->light, LIGHT_THRESHOLD_LOW);
        alarm = 1;
    }
    
    // 触发蜂鸣器报警
    if (alarm) {
        buzzer_alarm(200, 3); // 200ms间隔，响3次
    }
}

// 蜂鸣器控制函数
void buzzer_alarm(int duration_ms, int times) {
    printf("Buzzer alarm triggered (GPIO41)!\n");
    for (int i = 0; i < times; i++) {
        gpio_write(BUZZER_GPIO, 1); // 打开蜂鸣器
        usleep(duration_ms * 1000);  // 持续时间
        gpio_write(BUZZER_GPIO, 0); // 关闭蜂鸣器
        usleep(duration_ms * 1000);  // 间隔时间
    }
}
/*
int main() {
    SensorData sensor_data;
    
    // 初始化GPIO
    if (gpio_init() < 0) {
        fprintf(stderr, "GPIO initialization failed\n");
        return -1;
    }
    
    // 设置蜂鸣器GPIO为输出模式
    if (gpio_enable(BUZZER_GPIO, 1) < 0) {
        fprintf(stderr, "Failed to enable buzzer GPIO\n");
        gpio_close();
        return -1;
    }
    
    // 主循环
    while (1) {
        // 通过ADC读取传感器数据
        read_sensor_data(&sensor_data);
        
        // 检查阈值
        check_thresholds(&sensor_data);
        
        // 等待2秒后再次检查
        sleep(2);
    }
    
    // 清理资源
    gpio_close();
    return 0;
}
*/