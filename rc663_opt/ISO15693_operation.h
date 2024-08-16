#ifndef __ISO15693_OPERATION_H__
#define __ISO15693_OPERATION_H__

void ISO15693_Opt_halt(void);
int ISO15693_Opt_Read_Multiple(unsigned char block,unsigned char *receive_buf);
int ISO15693_Opt_read_signal_block(unsigned char block,unsigned char receive_buf[128]);
int ISO15693_Opt_WriteData(unsigned char  *Send,unsigned char Slen);
int ISO15693_Opt_CardReset(char *data,unsigned char *plen);

#endif  //__ISO15693_OPERATION_H__


