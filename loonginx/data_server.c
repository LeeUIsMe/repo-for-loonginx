// data_server_serial.c - ���ɴ�����ʾ���������ݷ�����
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <fcntl.h>
#include <termios.h>
#include <signal.h>
#include "serial.h"  // �������ṩ�Ĵ���ͷ�ļ�
#include "sensor_adc_alarm.h"


#define PORT 1234
#define BUFFER_SIZE 128
#define SERIAL_PORT "/dev/ttyS1"
#define SERIAL_BAUDRATE B115200
#define SERIAL_REFRESH_MS 200  // ����ˢ�¼��(ms)

typedef struct {
    float temperature;
    float humidity;
    int light;
    pthread_mutex_t mutex;
} SensorData;




int serial_fd = -1;
int running = 1;
SensorData current_data = {0};
Sensor_Data data={0};

// �źŴ���
void handle_signal(int sig) {
    running = 0;
}

// ��ʼ������
int init_serial() {
    serial_fd = open(SERIAL_PORT, O_RDWR | O_NOCTTY);
    if (serial_fd < 0) {
        perror("open serial");
        return -1;
    }

    // ʹ�����ṩ�Ĵ������ú���
    if (set_port_attr(serial_fd, SERIAL_BAUDRATE, 8, "1", 'N', 10, 0) < 0) {
        fprintf(stderr, "Failed to set serial port attributes\n");
        close(serial_fd);
        return -1;
    }
    
    return 0;
}

// �������ݵ�������
void send_to_serial(const SensorData *data) {
    if (serial_fd < 0) return;
    
    char msg[128];
    int len = snprintf(msg, sizeof(msg), 
           "t0.txt=\"Temp:%.1fC Humi:%.1f%% Light:%d \"\xff\xff\xff", 
            data->temperature, data->humidity, data->light);
    
    // ���������д��
    int ret = write(serial_fd, msg, len);
    if (ret < 0) {
        perror("serial write");
    } else if (ret != len) {
        fprintf(stderr, "Incomplete serial write (%d/%d bytes)\n", ret, len);
    }
}

// ����ˢ���̣߳��򻯰棩
void *serial_thread(void *arg) {
    while (running) {
        // �����������ݣ��̰߳�ȫ��
        pthread_mutex_lock(&current_data.mutex);
        send_to_serial(&current_data);
        pthread_mutex_unlock(&current_data.mutex);
        
        // ����ʱ������ʹ�ø߾��ȼ�ʱ��
        usleep(SERIAL_REFRESH_MS * 1000); // ת��Ϊ΢��
    }
    return NULL;
}

int main() {
    int server_fd, client_fd;
    struct sockaddr_in addr;
    socklen_t addr_len = sizeof(addr);
    char buffer[BUFFER_SIZE];
    pthread_t serial_tid;
    

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
    // ��ʼ��
    signal(SIGINT, handle_signal);
    signal(SIGTERM, handle_signal);
    pthread_mutex_init(&current_data.mutex, NULL);
    
    // ��ʼ������
    if (init_serial() < 0) {
        fprintf(stderr, "Serial initialization failed\n");
        return -1;
    }

    // ��������ˢ���߳�
  /*  if (pthread_create(&serial_tid, NULL, serial_thread, NULL) != 0) {
        perror("pthread_create");
        goto cleanup;
    }
  */
    // ����TCP�׽���
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket");
        goto cleanup;
    }

    // �����׽���ѡ��
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(PORT);
    
    // �󶨲�����
    if (bind(server_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind");
        goto cleanup;
    }
    
    if (listen(server_fd, 1) < 0) {
        perror("listen");
        goto cleanup;
    }
    
    printf("Server started on port %d\n", PORT);

    // ������ѭ��
    while (running) {
        if ((client_fd = accept(server_fd, (struct sockaddr *)&addr, &addr_len)) < 0) {
            if (running) perror("accept");
            continue;
        }
        
        printf("Client connected: %s\n", inet_ntoa(addr.sin_addr));
        
        while (running) {
            int len = read(client_fd, buffer, BUFFER_SIZE-1);
            if (len <= 0) break;
            
            buffer[len] = '\0';
            
            // ����JSON����
            SensorData new_data;
            sscanf(buffer, "{\"t\":%f,\"h\":%f,\"l\":%d}", 
                      &new_data.temperature, &new_data.humidity, &new_data.light);
                printf("Received: %.1fC %.1f%% %dlux\n", 
                      new_data.temperature, new_data.humidity, new_data.light);
            send_to_serial(&current_data);
                // ���¹�������
             //   pthread_mutex_lock(&current_data.mutex);
                current_data = new_data;
                data.temperature=current_data.temperature;
                data.light=current_data.light;
                data.humidity=current_data.humidity;
                
             //   pthread_mutex_unlock(&current_data.mutex);
            
            
                    // �����ֵ
        check_thresholds(&data);
        sleep(1);
        }
            // ������Դ
        gpio_close();
        close(client_fd);
        printf("Client disconnected\n");
    }

cleanup:
    running = 0;
    pthread_join(serial_tid, NULL);
    pthread_mutex_destroy(&current_data.mutex);
    if (serial_fd >= 0) close(serial_fd);
    if (server_fd >= 0) close(server_fd);
    return 0;
}