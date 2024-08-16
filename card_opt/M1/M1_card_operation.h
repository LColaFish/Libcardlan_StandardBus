
#ifndef _M1_CARD_OPEATION_H_
#define _M1_CARD_OPEATION_H_


#define WRITE_KEY   1
#define VERIFY_KEY  2
#define READ_ONLY   3
#define WRITE_ONLY  4

int ReadOneSectorDataFromCard(unsigned char *SectorBuf, unsigned long *p_SectorBuf_len,unsigned char SectorNo, unsigned char BlockNo, 
                                                   unsigned char VerifyFlag, unsigned char *key,unsigned char mode);

int WriteOneSertorDataToCard(const unsigned char *SectorBuf, unsigned long SectorBuf_len,unsigned char SectorNo, unsigned char BlockNo, unsigned char VerifyFlag,unsigned char *key,unsigned char mode);
int changOneBlockToMoneyBlock(unsigned char SectorNo, unsigned char BlockNo, unsigned char VerifyFlag,unsigned char *key,unsigned char mode);
int incMoney(int Money, unsigned char SectorNo, unsigned char BlockNo, unsigned char VerifyFlag, unsigned char * key, unsigned char mode);
int decMoney(int Money, unsigned char SectorNo, unsigned char BlockNo, unsigned char VerifyFlag, unsigned char * key, unsigned char mode);
int restoreAndTransfer(unsigned char SectorNo, unsigned char restoreBlockNo, unsigned char transferBlockNo,unsigned char *key,unsigned char mode);




#endif  //_M1_CARD_OPEATION_H_



