#ifndef NXP_RC531_OPT_H
#define NXP_RC531_OPT_H
#include <stdio.h>


extern int set_spi_fd(int spi_fd);
extern int get_spi_fd(int *spi_fd);
extern int init_spi(void);
extern uint8_t ReadIO(uint8_t address);
extern void WriteIO(uint8_t address, uint8_t value);
extern void deinit_spi(void);


//#define DEBUG_INFO(fmt,...)   printf("\033[1;36m "fmt"\033[0m\n", ##__VA_ARGS__)
//#define DEBUG_ERROR(fmt,...)  printf("\033[1;35m File:%s Function[%s] Line:%05d:"fmt"\033[0m\n", __FILE__, __FUNCTION__, __LINE__, ##__VA_ARGS__)
//
//
//#define DEBUG_SEND(fmt,...)   printf("\033[1;36m "fmt"\033[0m\n", ##__VA_ARGS__)
//#define DEBUG_RECV(fmt,...)   printf("\033[1;35m "fmt"\033[0m\n", ##__VA_ARGS__)

#endif
