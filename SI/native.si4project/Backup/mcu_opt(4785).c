
#include "mcu_opt.h"
#include <unistd.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>
#include "libcardlan_StandardBus_util.h"

#if defined(ANDROID_CODE_DEBUG)
#define TARGET_ANDROID
#elif defined(NDK_CODE_DEBUG)
#define TARGET_DEBUG
#endif

#if defined(TARGET_ANDROID)
#ifndef LOG_TAG
#define LOG_TAG __FILE__
#endif
#include <android/log.h>
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define  LOGD(...)  __android_log_print(ANDROID_LOG_DEBUG,LOG_TAG,__VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)
#elif defined(TARGET_DEBUG)
#include <stdio.h>
#define  LOGI    printf
#define  LOGD    printf
#define  LOGE    printf
#else 
#define  LOGI    
#define  LOGD    
#define  LOGE
#endif


#define LOGI printf


#define IRQ "/sys/class/gpio/gpio59/value"

static const char       *device  =    "/dev/spidev2.0";
static unsigned char     mode   =    0;
static unsigned char     bits   =    8;
static unsigned int      speed  =    10000000;
//static unsigned int      speed  =    5000000;

static unsigned short    delay  =    0;
static int fd = -1;

static int spi_send_cmd_4bytes(char* tx, char* rx);


static void menu_print(unsigned char *buffer, int length)
{
    int i;

    for (i = 0; i < length; i++)
    {
        LOGI("%02X ", *(buffer + i));
        if ((i + 1) % 8 == 0) LOGI(" ");
        if ((i + 1) % 16 == 0) LOGI("\n");
    }
    LOGI("\n");
}




static int spi_send_cmd_4bytes(char* tx, char* rx)
{
    int ret = 0;
    int i;
    
    struct spi_ioc_transfer tr = 
    {
        .tx_buf = (unsigned long)tx,
        .rx_buf = (unsigned long)rx,
        .len = 4,
        .delay_usecs = delay,
        .speed_hz = speed,
        .bits_per_word = bits,
    };

    ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
    if (ret < 1)
    {
        LOGI("can't send spi message\n");
        return -1;
    }

    //LOGI("------tx :%02X %02X %02X %02X ---> ",tx[0],tx[1],tx[2],tx[3]);  
    //LOGI("rx: %02X %02X %02X %02X\n",rx[0],rx[1],rx[2],rx[3]);
    return 0;
}

static int spi_send(char* tx, int tx_len)
{
    int remain            = 0;
    int i                  = 0;
    char send_buff[4]    = {0};
    char recv_buff[4]    = {0};

    if(NULL == tx)
    {
        return -1;
    }

    /* calc remain */
    remain = tx_len % 4;
    
    for(i = 0; i < tx_len / 4; i++)
    {
        memcpy(send_buff, tx + ( 4 * i ), 4);
        spi_send_cmd_4bytes(send_buff, recv_buff);
    }

    if(0 != remain)
    {
        memset(send_buff, 0, sizeof(send_buff));
        memcpy(send_buff, tx+(4*i), remain);
        spi_send_cmd_4bytes(send_buff, recv_buff);
    }
    return 0;
}

int spi_recv_len(char* rx, int rx_len)
{
    char get_data_len[]    = {0x02, 0x10, 0x03, 0xFF};
    char get_data[]        = {0xFF, 0xFF, 0xFF, 0xFF};
    char recv_buff[4]     = {0};
    int  count            = 0;
    int  i                = 0;

    if(NULL == rx)
    {
        return -1;
    }
    
    spi_send(get_data_len, sizeof(get_data_len));
    spi_send_cmd_4bytes(get_data, recv_buff);
    memcpy(rx, recv_buff, 4);

    return 0;
}

void spi_recv_data(char* rx, int rx_len)
{
    char spi_get_data_cmd[]     = {0x02, 0x20, 0x03, 0xFF};
    char get_data[]             = {0xFF, 0xFF, 0xFF, 0xFF};
    char recv_buff[4]     = {0};
    int  count            = 0;
    int  i                = 0;

    if(NULL == rx)
        return;

    /* calc remain */
    if(0 != rx_len%4)
        count = rx_len/4 + 1;
    else
        count = rx_len/4;

    for(i=0; i<count; i++)
    {
        spi_send_cmd_4bytes(get_data, recv_buff);
        memcpy(rx+(4*i), recv_buff, 4);
    }
}


int init_mcu_spi(void)
{
    int ret = 0;

    if(fd > 0)
    {
        return 0;
    }

    fd = open(device, O_RDWR);
    if (fd < 0)
    {
        LOGI("can't open device");
        return -1;
    }
    /* Set spi mode W */
    ret = ioctl(fd, SPI_IOC_WR_MODE, &mode);
    if (ret == -1)
    {     
        LOGI("can't set spi mode");
        fd = -1;
        return -2;
    }
    /* Set spi mode R */
    ret = ioctl(fd, SPI_IOC_RD_MODE, &mode);
    if (ret == -1)
    {
        LOGI("can't get spi mode");
        fd = -1;
        return -3;
    }
    
    /* Set bits per word W */
    ret = ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &bits);
    if (ret == -1)
    {
        LOGI("can't set bits per word");
        fd = -1;
        return -4;
    }
    
    /* Set bits per word R */
    ret = ioctl(fd, SPI_IOC_RD_BITS_PER_WORD, &bits);
    if (ret == -1)
    {
        LOGI("can't get bits per word");
        fd = -1;
        return -5;
    }
    
    /* Set max speed hz W */
    ret = ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
    if (ret == -1)
    {
        LOGI("can't set max speed hz");
        fd = -1;
        return -6;
    }
    
    /* Set max speed hz R */
    ret = ioctl(fd, SPI_IOC_RD_MAX_SPEED_HZ, &speed);
    if (ret == -1)
    {
        LOGI("can't get max speed hz");
        fd = -1;
        return -7;
    }
    
    LOGI("sip 2 mode: %d\n", mode);
    LOGI("sip 2 Bits per word: %d\n", bits);
    LOGI("sip 2 Max speed: %d Hz (%d KHz)\n", speed, speed/1000);
    return 0;
}


int  mcu_cmd_read_and_write(unsigned char device,unsigned char action,unsigned char* send_data, unsigned char send_cmd_len[4], char* recv_data, unsigned char recv_cmd_len[4])
{

    char send[256]  = {0};
  
    int  ret        = 0;
    int  len        = 0;


    send[0] = 0x02;
    send[1] = device;
    send[2] = action;
    memcpy(&send[3],send_cmd_len,4);
    send[7] = 0x03; 
    len = 8;
    if(send_data != NULL && (send_cmd_len[0] + send_cmd_len[1] + send_cmd_len[2] + send_cmd_len[3]) > 0)
    {
        memcpy(&send[8], send_data,  send_cmd_len[0] + send_cmd_len[1] + send_cmd_len[2] + send_cmd_len[3]);
    }
    len += send_cmd_len[0] + send_cmd_len[1] + send_cmd_len[2] + send_cmd_len[3];
     
    LOGI("[ %s %d ]send ->\n",__FUNCTION__,__LINE__);
    menu_print(send,len);
    spi_send(send,len);

    /* Wait For Irq */
    int  data_len = 0;
    int  i   = 100;
    int  fd;
    char irq = 0;
    while(i)
    {
       /* Open Irq */
       fd = open(IRQ, O_RDONLY, 0644);
       ret = read(fd, &irq, 1);    
       //printf("[%d] IRQ = %d\n",ret, irq);
       if('1' == irq)
       {
           close(fd);
           break;
       }
       i--;
       usleep(20000);
       close(fd);
    }

    if(i == 0)
    {
        LOGI("[ %s %d ]get mcu respond time out \n",__FUNCTION__,__LINE__);
        return -1;
    }


    unsigned char recv_buff[4];
    spi_recv_len(recv_buff, 4);
    memcpy(recv_cmd_len,recv_buff,4);
    data_len = recv_buff[0];
    LOGI("[%s %d]time = %d ms data_len : %d \n",__FUNCTION__,__LINE__,(100 - i) * 20,data_len);
    /*get buff len*/
    {
        char recv_buff[256]       = {0};
        unsigned char spi_get_data_cmd[]     = {0x02, 0x20, 0x03, 0xFF};
        spi_send_cmd_4bytes(spi_get_data_cmd, recv_buff);
        spi_recv_data(recv_data, data_len);

        LOGI("[ %s %d ]recv ->\n",__FUNCTION__,__LINE__);
        menu_print(recv_data,data_len);
    }

    return 0;
}

