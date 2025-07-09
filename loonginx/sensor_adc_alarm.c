#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include "sensor_adc_alarm.h"

// �����ֵ����������
void check_thresholds(Sensor_Data *data) {
    int alarm = 0;
    
    // ����¶���ֵ
    if (data->temperature > TEMP_THRESHOLD_HIGH) {
        printf("ALERT: High temperature (%.1f > %.1f)!\n", 
               data->temperature, TEMP_THRESHOLD_HIGH);
        alarm = 1;
    } else if (data->temperature < TEMP_THRESHOLD_LOW) {
        printf("ALERT: Low temperature (%.1f < %.1f)!\n", 
               data->temperature, TEMP_THRESHOLD_LOW);
        alarm = 1;
    }
    
    // ���ʪ����ֵ
    if (data->humidity > HUMI_THRESHOLD_HIGH) {
        printf("ALERT: High humidity (%.1f > %.1f)!\n", 
               data->humidity, HUMI_THRESHOLD_HIGH);
        alarm = 1;
    } else if (data->humidity < HUMI_THRESHOLD_LOW) {
        printf("ALERT: Low humidity (%.1f < %.1f)!\n", 
               data->humidity, HUMI_THRESHOLD_LOW);
        alarm = 1;
    }
    
    // ��������ֵ
    if (data->light > LIGHT_THRESHOLD_HIGH) {
        printf("ALERT: High light (%d > %d)!\n", 
               data->light, LIGHT_THRESHOLD_HIGH);
        alarm = 1;
    } else if (data->light < LIGHT_THRESHOLD_LOW) {
        printf("ALERT: Low light (%d < %d)!\n", 
               data->light, LIGHT_THRESHOLD_LOW);
        alarm = 1;
    }
    
    // ��������������
    if (alarm) {
        buzzer_alarm(200, 3); // 200ms�������3��
    }
}

// ���������ƺ���
void buzzer_alarm(int duration_ms, int times) {
    printf("Buzzer alarm triggered (GPIO41)!\n");
    for (int i = 0; i < times; i++) {
        gpio_write(BUZZER_GPIO, 1); // �򿪷�����
        usleep(duration_ms * 1000);  // ����ʱ��
        gpio_write(BUZZER_GPIO, 0); // �رշ�����
        usleep(duration_ms * 1000);  // ���ʱ��
    }
}
/*
int main() {
    SensorData sensor_data;
    
    // ��ʼ��GPIO
    if (gpio_init() < 0) {
        fprintf(stderr, "GPIO initialization failed\n");
        return -1;
    }
    
    // ���÷�����GPIOΪ���ģʽ
    if (gpio_enable(BUZZER_GPIO, 1) < 0) {
        fprintf(stderr, "Failed to enable buzzer GPIO\n");
        gpio_close();
        return -1;
    }
    
    // ��ѭ��
    while (1) {
        // ͨ��ADC��ȡ����������
        read_sensor_data(&sensor_data);
        
        // �����ֵ
        check_thresholds(&sensor_data);
        
        // �ȴ�2����ٴμ��
        sleep(2);
    }
    
    // ������Դ
    gpio_close();
    return 0;
}
*/