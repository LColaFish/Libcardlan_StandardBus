
#ifndef __MCU_OPT_H__
#define __MCU_OPT_H__

int init_mcu_spi(void);
int spi_cmd_send(unsigned char device,unsigned char action,unsigned char data_len[4], unsigned char *data);
int  mcu_cmd_read_and_write(unsigned char device,unsigned char action,unsigned char* send_data, unsigned char send_cmd_len[4], char* recv_data, unsigned char recv_cmd_len[4]);



#endif  __MCU_OPT_H__

