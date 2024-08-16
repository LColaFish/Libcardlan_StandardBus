#include "includes.h"

unsigned long long big_data;

void IntToByteArray(long var,uchar * buf,uchar bufLen);
void bcd_to_asc(uchar *asc, uchar *bcd,unsigned int asc_len);
void ASCToBCD(uchar * bcd_buf, uchar * asc_buf, int n);
void BCDToASC(uchar * asc_buf, uchar * bcd_buf, int n);

uchar byte_to_bcd(uchar bchar)
{
    unsigned char bcd_code, btmp;

    btmp = bchar/10;
    bcd_code = btmp*16;
    bcd_code += bchar-btmp*10;
    return(bcd_code);
}

void bcd_to_asc(uchar *asc, uchar *bcd,unsigned int asc_len)
{
    unsigned int i, j, new_len;
    unsigned char bOddFlag, bchar;

    if (asc_len%2)
    {
        new_len = asc_len/2+1;
        bOddFlag = 1;
    }
    else
    {
        new_len = asc_len/2;
        bOddFlag = 0;
    }
    for(i=0,j=0; i<new_len; i++)
    {
        if ((i == 0)&&(bOddFlag))
        {
            bchar = bcd[i]&0x0f;
            asc[j++] = (bchar<=9)?(0x30+bchar):(bchar-10+'A');
        }
        else
        {
            bchar = bcd[i]/16;
            asc[j++] = (bchar<=9)?(0x30+bchar):(bchar-10+'A');
            bchar = bcd[i]&0x0f;
            asc[j++] = (bchar<=9)?(0x30+bchar):(bchar-10+'A');
        }
    }
    asc[j] = 0x0;
}

uchar char_to_bin(uchar bchar)
{
    if ((bchar >= '0')&&(bchar <= '9'))
        return(bchar-'0');
    else
    {
        if ((bchar >= 'A')&&(bchar <= 'F'))
            return(bchar-'A'+10);
        if ((bchar >= 'a')&&(bchar <= 'f'))
            return(bchar-'a'+10);
        else
            return(0);        
    }
}


uchar bin_to_char(uchar bin)
{
    if(bin <= 0x09)
        return (bin+'0');
    else
    {
        if((bin >= 0xA) && (bin<=0xF))
            return (bin + 'A' - 10);
        else
            return NOK;
    }
}

void asc_to_bcd(uchar *bcd, uchar *asc, unsigned int asc_len)
{
    unsigned int i,j;
    unsigned char bOddFlag, bchar, bchar1, bchar2;

    if (asc_len%2)
        bOddFlag = 1;
    else
        bOddFlag = 0;

    for (i=0,j=0; j<asc_len; i++)
    {
        if ((i==0)&&(bOddFlag))
        {
            bchar1 = asc[j++];
            bcd[i] = char_to_bin(bchar1);
        }
        else
        {
            bchar1 = asc[j++];
            bchar = char_to_bin(bchar1);
            bchar *= 16;
            bchar1 = asc[j++];
            bchar2 = char_to_bin(bchar1);
            bcd[i] = bchar + bchar2;
        }
    }
}


void asc_to_bin(uchar *bin, uchar *asc, unsigned int bin_len)
{
    unsigned int i;
    for(i=0;i<bin_len;i++)
    {
        bin[i]=char_to_bin(asc[i*2])*16+char_to_bin(asc[i*2+1]);
    }
}

unsigned char bin_to_bcd(uchar *bcd, u32 bin, uchar len)
{
    long lbin;
    int i,j;
    uchar sbuf[4];

    lbin = 1;
    for(i=0; i<len; i++)
        lbin *= 100;
    if (bin/lbin)
        return(0);
    lbin = bin;
    for(i=0;i<len;i++)
    {
        sbuf[i] = (BYTE)(lbin%100);
        lbin /= 100;
    }
    for(i=0;i<len;i++)
    {
        bcd[i] = byte_to_bcd(sbuf[len-i-1]);

    }
    return(0);
}

long long bcd_to_bin(uchar *bcd, uchar len)
{
    unsigned long long lbin;
    int i,k;
    big_data = 0;
    lbin=0;
    for(i=0; i<len; i++)
    {
        k=(bcd[i]>>4)*10+(bcd[i]&0x0f);
        lbin=lbin*100+k;
    }
    big_data=lbin;
    return lbin;
}


u32 asc_to_ulong(uchar *asc)
{
    char i, blen;
    u32 dwTmp;

    blen = strlen((char *)asc);
    dwTmp = 0;
    for (i=0; i<blen; i++)
    {
        if ((asc[i] < '0') || (asc[i] > '9'))
            return NOK;
        dwTmp = dwTmp*10 + (asc[i] - '0');
    }
    
    return dwTmp;
}


void ConvTimeToStr(uchar *ptTime, uchar *pTimeStr)
{
    bcd_to_asc((uchar *)&pTimeStr[0], (uchar *)&ptTime[1], 2);
    pTimeStr[2] = '/';
    bcd_to_asc((uchar *)&pTimeStr[3], (uchar *)&ptTime[2], 2);
    pTimeStr[5] = '/';
    if (ptTime[0] >= 0x80)
    {
        pTimeStr[6] = '1';
        pTimeStr[7] = '9';
    }
    else
    {
        pTimeStr[6] = '2';
        pTimeStr[7] = '0';
    }
    bcd_to_asc((uchar *)&pTimeStr[8], (uchar *)&ptTime[0], 2);
    pTimeStr[10] = ' ';
    if(ptTime[5]>=0x60) ptTime[3]=0;
    bcd_to_asc((uchar *)&pTimeStr[11], (uchar *)&ptTime[3], 2);
    pTimeStr[13] = ':';
    if(ptTime[5]>=0x60) ptTime[4]=0;
    bcd_to_asc((uchar *)&pTimeStr[14], (uchar *)&ptTime[4], 2);
    pTimeStr[16] = ':';
    if(ptTime[5]>=0x60) ptTime[5]=0;
    bcd_to_asc((uchar *)&pTimeStr[17], (uchar *)&ptTime[5], 2);
}

long ByteArrayToInt(uchar * buf,uchar bufLen)
{
    int i;
    long temp;
    temp=0;
    for(i=0;i<bufLen;i++)
    {
        temp=(temp<<8)+buf[i];
    }
    return temp;
}

uchar abcd_to_asc(uchar abyte)
{
    if (abyte <= 9)
        abyte = abyte + '0';
    else
        abyte = abyte + 'A' - 10;
    return (abyte);
}

uchar aasc_to_bcd(uchar asc)
{
    uchar bcd;

    if ((asc >= '0') && (asc <= '9'))
        bcd = asc - '0';
    else if ((asc >= 'A') && (asc <= 'F'))
        bcd = asc - 'A' + 10;
    else if ((asc >= 'a') && (asc <= 'f'))
        bcd = asc - 'a' + 10;
    else if ((asc > 0x39) && (asc <= 0x3f))
        bcd = asc - '0';
    else {
             bcd = 0x0f;
         }
    return bcd;
}

void ASCToBCD(uchar * bcd_buf, uchar * asc_buf, int n)
{
    int i, j;

    j = 0;

    for (i = 0; i < (n + 1) / 2; i++) {
        bcd_buf[i] = aasc_to_bcd(asc_buf[j++]);
        bcd_buf[i] = ((j >= n) ? 0x00 : aasc_to_bcd(asc_buf[j++]))
            + (bcd_buf[i] << 4);
    }
}


 void BCDToASC(uchar * asc_buf, uchar * bcd_buf, int n)
{
    int i, j;

    j = 0;
    for (i = 0; i < n / 2; i++) {
        asc_buf[j] = (bcd_buf[i] & 0xf0) >> 4;
        asc_buf[j] = abcd_to_asc(asc_buf[j]);
        j++;
        asc_buf[j] = bcd_buf[i] & 0x0f;
        asc_buf[j] = abcd_to_asc(asc_buf[j]);
        j++;
    }
    if (n % 2) {
        asc_buf[j] = (bcd_buf[i] & 0xf0) >> 4;
        asc_buf[j] = abcd_to_asc(asc_buf[j]);
    }
}


void IntToByteArray(long var,uchar * buf,uchar bufLen)
{
    int i;
    long temp;
    temp=var;
    for(i=0;i<bufLen;i++)
    {
        buf[bufLen-1-i]=temp%256;
        temp>>=8;
    }
}
