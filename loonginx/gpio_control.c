#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include "sensor_adc_alarm.h"

#define MAP_SIZE 0x10000
#define REG_BASE 0x1fe00000
#define GPIO_EN 0x500
#define GPIO_OUT 0x510

unsigned char *map_base = NULL;
int dev_fd;

// GPIO��ʼ��
int gpio_init(void) {
    dev_fd = open("/dev/mem", O_RDWR | O_SYNC);
    if (dev_fd < 0) {
        perror("open /dev/mem failed");
        return -1;
    }
    
    map_base = (unsigned char *)mmap(0, MAP_SIZE, PROT_READ | PROT_WRITE, 
                                    MAP_SHARED, dev_fd, REG_BASE);
    if (map_base == MAP_FAILED) {
        perror("mmap failed");
        close(dev_fd);
        return -1;
    }
    
    return 0;
}

// GPIOʹ��
int gpio_enable(int gpio_num, int val) {
    int offset, gpio_move;
    
    if (gpio_num > 31) {
        offset = 4;
        gpio_move = gpio_num - 32;
    } else {
        offset = 0;
        gpio_move = gpio_num;
    }
    
    if (val == 0) {
        // ����Ϊ����
        *(volatile unsigned int *)(map_base + GPIO_EN + offset) |= (1 << gpio_move);
    } else {
        // ����Ϊ���
        *(volatile unsigned int *)(map_base + GPIO_EN + offset) &= ~(1 << gpio_move);
    }
    
    return 0;
}

// GPIOд����
int gpio_write(int gpio_num, int val) {
    int offset, gpio_move;
    
    if (gpio_num > 31) {
        offset = 4;
        gpio_move = gpio_num - 32;
    } else {
        offset = 0;
        gpio_move = gpio_num;
    }
    
    if (val == 1) {
        // ����ߵ�ƽ
        *(volatile unsigned int *)(map_base + GPIO_OUT + offset) |= (1 << gpio_move);
    } else {
        // ����͵�ƽ
        *(volatile unsigned int *)(map_base + GPIO_OUT + offset) &= ~(1 << gpio_move);
    }
    
    return 0;
}

// �ر�GPIO
int gpio_close(void) {
    if (map_base) {
        munmap(map_base, MAP_SIZE);
    }
    if (dev_fd >= 0) {
        close(dev_fd);
    }
    return 0;
}