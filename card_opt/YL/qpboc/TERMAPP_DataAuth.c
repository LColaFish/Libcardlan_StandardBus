#include <openssl/rsa.h>
#include "includes.h"
#include "common/cardlan_StandardBus_tpye.h"
#include "libcardlan_StandardBus_util.h"


#if defined(ANDROID_CODE_DEBUG)
#define TARGET_ANDROID
#endif
#if defined(NDK_CODE_DEBUG)
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

/*
extern FATFS fs;            // Work area (file system object) for logical drive
extern FIL fsrc, fdst;      // file objects
extern FRESULT res;         // FatFs function common result code
extern UINT br, bw;         // File R/W count
*/

uchar TERMAPP_StaticAuth(void);
uchar TERMAPP_GetCAPK(uchar *RID, uchar index);
uchar TERMAPP_GetIPK(void);
uchar TERMAPP_SDVerify(void);
uchar TERMAPP_DynamicAuth(void);
uchar TERMAPP_GetICCPK(void);
void  TERMAPP_FillICCPK(uchar *ICCPKData, struct ICCPK_RECOVER *recovICCPK);
uchar TERMAPP_ExpireDateVerify(uchar *ExpireDate);
uchar TERMAPP_DynSignVerify(void);
uchar TERMAPP_CheckDataMissDDA(void);
uchar TERMAPP_CheckDataMissSDA(void);
void  TERMAPP_FillSSA(uchar *SSAData, struct SIGN_STAT_APPDATA_RECOVER *recovSSA);
void  TERMAPP_FillIPK(uchar *ipkData, IPK_RECOVER *recovIPK);

uchar RSAPublicDecrypt(uchar *PublicKey, u32 exponent, int keyLength, uchar *encryptnum, uchar *decresult)
{
    RSA *key;
    unsigned char ptext[256];
    unsigned char exp[3];
    int num;
    int keysize = keyLength / 8;

    LOGI("RSAPublicDecrypt() is called.\n");

    exp[0] = (exponent >> 16) & 0xFF;
    exp[1] = (exponent >> 8) & 0xFF;
    exp[2] = exponent & 0xFF;

    key = RSA_new();
    key->n = BN_bin2bn(PublicKey, keysize, key->n);
    key->e = BN_bin2bn(exp, sizeof(exp), key->e);
    key->flags |= RSA_FLAG_NO_CONSTTIME;

    num = RSA_public_decrypt(keysize, encryptnum, decresult, key, RSA_NO_PADDING);

    /*
    LOGI("PKCS#1 v1.5 decryption.\n");

    LOGI("n (%d bytes):\n", keysize);
    menu_print(PublicKey, keysize);

    LOGI("e (%d bytes):\n", sizeof(exp));
    menu_print(exp, sizeof(exp));

    LOGI("cipher msg (%d bytes):\n", keysize);
    menu_print(encryptnum, keysize);

    LOGI("plain text (%d bytes):\n", num);
    if (num > 0)
    {
        menu_print(decresult, num);
    }
    */

    RSA_free(key);

    return 0;
}

void sha1( const unsigned char *input, int ilen, unsigned char *output)
{
    LOGI("sha1() is called.\n");

    EVP_Digest(input, ilen, output, NULL, EVP_sha1(), NULL);
}

int GetTime(char *buffer)
{
    SysTime Time;
    unsigned char buff[6];
    LOGI("GetTime() is called.\n");

    if (buffer == NULL)
    {
        return -1;
    }
    /*get current time*/
    {
        memset(buff,0,sizeof(buff));
        Rd_time (buff);
        Time.year = buff[0];
        Time.month = buff[1];
        Time.day = buff[2];
        Time.hour = buff[3];
        Time.min = buff[4];
        Time.sec = buff[5];
    }

    buffer[0] = Time.year;
    buffer[1] = Time.month;
    buffer[2] = Time.day;
    buffer[3] = Time.hour;
    buffer[4] = Time.min;
    buffer[5] = Time.sec;

    menu_print(buffer, 6);
    return 0;
}

uchar TERMAPP_GetCAPK(uchar *RID, uchar index)
{
    uchar status = NOK;
    int i;

    LOGI("TERMAPP_GetCAPK() is called, index = %d, RID = ", index);
    menu_print(RID, 5);

    i = 0;
    while (CAPK_list[i].pModul)
    {
        if ((index == CAPK_list[i].CAPKI)
            && (!memcmp(RID, CAPK_list[i].RID, 5))) 
        {
            memset(CAPK.RID, sizeof(CAPK.RID), 0);
            memcpy(CAPK.RID, CAPK_list[i].RID, sizeof(CAPK.RID));

            CAPK.CAPKI = CAPK_list[i].CAPKI;
            CAPK.Exponent[0] = (CAPK_list[i].Exponent >> 16) & 0xFF;
            CAPK.Exponent[1] = (CAPK_list[i].Exponent >> 8) & 0xFF;
            CAPK.Exponent[2] = CAPK_list[i].Exponent & 0xFF;
            CAPK.ModulLen = CAPK_list[i].ModulLen;
            memset(CAPK.Modul, sizeof(CAPK.Modul), 0);
            memcpy(CAPK.Modul, CAPK_list[i].pModul, CAPK_list[i].ModulLen);
            status = OK;

            LOGI("TERMAPP_GetCAPK(): CAPK found, i = %d\n", i);
            break;
        }
        i++;
    }

    return status;
}

uchar TERMAPP_DataAuth(unsigned char channle)
{
    uchar i, retCode;
/*
    CardInfo.AIP 对应字节
    字节1
    位8 RFU
    位7 1= 支持SDA
    位6 1= 支持DDA
    位5 1= 支持持卡人验证
    位4 1= 支持终端风险管理
    位3 1= 支持发卡行认证
    位2 1= RFU
    位1 1= 支持CDA
    字节2
    位8 = 0
    位7~1 RFU

*/
    LOGI("TERMAPP_DataAuth() is called.\n");
    LOGI("CardInfo.AIP[0]: \t%02X.\n", CardInfo.AIP[0]);    //7c
    LOGI("TermInfo.TermCapab[2]: \t%02X.\n", TermInfo.TermCapab[2]); //c8
    LOGI("bErrSDATL: \t%02X.\n", bErrSDATL);
    LOGI("bErrAuthData: \t%02X.\n", bErrAuthData);

    if(channle==0)
    {
        if(((CardInfo.AIP[0] & 0x20) == 0x20) && (TermInfo.TermCapab[2] & 0x40) == 0x40) 
        {
            if(bErrSDATL || bErrAuthData) 
            {
                TermInfo.TVR[0] |= 0x08; //Offline Data Authentication was preformed
                TermInfo.TSI[0] |= 0x80;
                return OK;
            }
            retCode = TERMAPP_DynamicAuth();
            if(retCode == err_IccCommand || retCode == err_IccDataFormat || retCode == err_IccReturn || retCode == JY_END || retCode == JY_REFUSE) 
            {
                return JY_REFUSE;
            }
            else if(retCode == OK)
            {
                TermInfo.TSI[0] |= 0x80;        //Offline Data Auth    was    performed
            } 
            else
            {
                TermInfo.TVR[0] |= 0x08;        //Offline dynamic Data Authentication failed
                TermInfo.TSI[0] |= 0x80;        //Offline Data Authentication was performed
            }
        } 
        else if(((CardInfo.AIP[0] & 0x40) == 0x40) && ((TermInfo.TermCapab[2] & 0x80) == 0x80))
        {
            if(bErrSDATL || bErrAuthData)
            {
                TermInfo.TVR[0] |= 0x40;        //Offline Static Data Authentication failed
                TermInfo.TSI[0] |= 0x80;        //set bit 'Offline Data Authentication was performed' bit 1
                return JY_REFUSE;
            }
            retCode = TERMAPP_StaticAuth();
            if(retCode == err_IccCommand || retCode == err_IccDataFormat || retCode == err_IccReturn || retCode == JY_END || retCode == JY_REFUSE) return JY_REFUSE;
            else if(retCode == OK) {
                TermInfo.TSI[0] |= 0x80; //Offline Data Authentication was preformed
                TermInfo.TVR[0] &= 0xBF; //Offline Data Authentication was successful
            } else {
                TermInfo.TVR[0] |= 0x40; //Offline Static Data Authentication failed
                TermInfo.TSI[0] |= 0x80; //set bit 'Offline Data Authentication'
            }
        } 
        else 
        {                        //all dynamic and static authentication and DDA/AC are not supported
            TermInfo.TVR[0] |= 0x80;        //Offline Data Authentication not performed
            TermInfo.TSI[0] &= 0x7F;    //set bit 'Offline data authentication was performed' bit 0.
            return JY_REFUSE;
        }
       
        if(((CardInfo.AIP[0] & 0x40) == 0x40) && ((TermInfo.TermCapab[2] & 0x80) == 0x80))
        {
            if(bErrSDATL || bErrAuthData)
            {
                TermInfo.TVR[0] |= 0x40;        //Offline Static Data Authentication failed
                TermInfo.TSI[0] |= 0x80;        //set bit 'Offline Data Authentication was performed' bit 1
                return JY_REFUSE;
            }
            retCode = TERMAPP_StaticAuth();
            if(retCode == err_IccCommand || retCode == err_IccDataFormat || retCode == err_IccReturn || retCode == JY_END || retCode == JY_REFUSE) return JY_REFUSE;
            else if(retCode == OK) {
                TermInfo.TSI[0] |= 0x80; //Offline Data Authentication was preformed
                TermInfo.TVR[0] &= 0xBF; //Offline Data Authentication was successful
            } else {
                TermInfo.TVR[0] |= 0x40; //Offline Static Data Authentication failed
                TermInfo.TSI[0] |= 0x80; //set bit 'Offline Data Authentication'
            }
        } else {                        //all dynamic and static authentication and DDA/AC are not supported
            TermInfo.TVR[0] |= 0x80;        //Offline Data Authentication not performed
            TermInfo.TSI[0] &= 0x7F;    //set bit 'Offline data authentication was performed' bit 0.
            return JY_REFUSE;
        }
    }
    else if(channle==2)
    {
         if(((CardInfo.AIP[0] & 0x40) == 0x40) && ((TermInfo.TermCapab[2] & 0x80) == 0x80)) {
               if(bErrSDATL || bErrAuthData) {
                   TermInfo.TVR[0] |= 0x40;        //Offline Static Data Authentication failed
                   TermInfo.TSI[0] |= 0x80;        //set bit 'Offline Data Authentication was performed' bit 1
                   return JY_REFUSE;
               }
               retCode = TERMAPP_StaticAuth();
               if(retCode == err_IccCommand || retCode == err_IccDataFormat || retCode == err_IccReturn || retCode == JY_END || retCode == JY_REFUSE) return JY_REFUSE;
               else if(retCode == OK) {
                   TermInfo.TSI[0] |= 0x80; //Offline Data Authentication was preformed
                   TermInfo.TVR[0] &= 0xBF; //Offline Data Authentication was successful
               } else {
                   TermInfo.TVR[0] |= 0x40; //Offline Static Data Authentication failed
                   TermInfo.TSI[0] |= 0x80; //set bit 'Offline Data Authentication'
               }
           } else {                        //all dynamic and static authentication and DDA/AC are not supported
               TermInfo.TVR[0] |= 0x80;        //Offline Data Authentication not performed
               TermInfo.TSI[0] &= 0x7F;    //set bit 'Offline data authentication was performed' bit 0.
               return JY_REFUSE;
           }
        
    }
    return OK;
}

uchar TERMAPP_StaticAuth()
{
    LOGI("TERMAPP_StaticAuth() is called.\n");

    if(TERMAPP_CheckDataMissSDA() != OK) {
        TermInfo.TVR[0] |= 0x20;
        return JY_REFUSE;
    }
    if(CAPK.ModulLen == 0) {
#ifdef BCTCTEST
        if(TERMAPP_testGetCAPK() != OK) {
            if(    TERMAPP_testGetCAPK() != OK)
                return JY_REFUSE;
        }
#else
        if(TERMAPP_GetCAPK(CardInfo.AID, CardInfo.CAPKI) != OK)  {
            return JY_REFUSE;
        }
#endif
    }

    //串口发出调试信息 "认证中心公钥"

    if(IPKModulLen == 0) {
        if(TERMAPP_GetIPK() != OK) {
            return JY_REFUSE;
        }
    }
    if(TERMAPP_SDVerify() != OK) {
        return JY_REFUSE;
    }
    return OK;
}


uchar TERMAPP_GetIPK()
{
    uchar IPKData[266], IPKHash[20];
    uchar revokeList[9];
    uchar buf[9], bytePAN[10], byteIssuID[10];
    IPK_RECOVER recovIPK;
    IPK_REVOKE IPKRevoke;
    u32 Exp = 0;
    u32 filesize, IPKRevokeNum;
    int i, index, fid;


    LOGI("TERMAPP_GetIPK() is called, CardInfo.IPKCertLen=%d\n", CardInfo.IPKCertLen, CAPK.ModulLen);

    if(CardInfo.IPKCertLen != CAPK.ModulLen) {
        return JY_REFUSE;
    }
    #if 1
    Exp = ((CAPK.Exponent[0] << 8) << 8) + (CAPK.Exponent[1] << 8) + CAPK.Exponent[2];
    #else
    Exp = CAPK.Exponent[0] * 256 * 256 + CAPK.Exponent[1] * 256 + CAPK.Exponent[2];
    #endif

    LOGI("TERMAPP_GetIPK(): exp=%d.\n", Exp);

    if(RSAPublicDecrypt(CAPK.Modul, Exp, CAPK.ModulLen * 8, CardInfo.IPKCert, IPKData) != OK) {
        //串口发出调试信息“恢复ICC公钥失败”
        return JY_REFUSE;
    }

    //串口发出调试信息“恢复ICC公钥成功”
    TERMAPP_FillIPK(IPKData, &recovIPK);

    if(recovIPK.DataTrail != 0xBC) {
        return JY_REFUSE;
    }
    if(recovIPK.DataHead != 0x6A) {
        return JY_REFUSE;
    }
    if(recovIPK.CertFormat != 0x02) {
        return JY_REFUSE;
    }

    index = 0;
    for(i = 0; i < CAPK.ModulLen - 22; i++) {
        IPKData[i] = IPKData[i + 1];
    }
    index += CAPK.ModulLen - 22;
    if(recovIPK.IPKLen > CAPK.ModulLen - 36)
    {
        if(ICCDataTable[MV_IPKRem].bExist == 0) {
            TermInfo.TVR[0] |= 0x20;   //ICC data missing
            return JY_REFUSE;
        }
        memcpy((uchar *)&IPKData[index], (uchar *)&CardInfo.IPKRem, CardInfo.IPKRemLen);
        index += CardInfo.IPKRemLen;

    }
    if(CardInfo.IPKExpLen == 0x01) memcpy((uchar *)&IPKData[index], (uchar *)&CardInfo.IPKExp[2], CardInfo.IPKExpLen);
    if(CardInfo.IPKExpLen == 0x03) memcpy((uchar *)&IPKData[index], CardInfo.IPKExp, CardInfo.IPKExpLen);
    index += CardInfo.IPKExpLen;
    memset(IPKHash, 0, 20);
    sha1(IPKData, index, IPKHash);
    if(recovIPK.HashInd == 0x01) 
    {
        if(memcmp(recovIPK.HashResult, IPKHash, 20)) 
        {
            return JY_REFUSE;
        }
    }
    else
    {
        return JY_REFUSE;
    }
    //串口发出调试信息（"HashResultOK",IPKHash,20）
    LOGI("TERMAPP_GetIPK(): HashResultOK.\n");
    menu_print(IPKHash, 20);

    for(i = 0; i < 4; i++) 
    {
        bytePAN[2 * i] = (CardInfo.PAN[i] & 0xF0) >> 4;
        bytePAN[2 * i + 1] = CardInfo.PAN[i] & 0x0F;
        byteIssuID[2 * i] = (recovIPK.IssuID[i] & 0xF0) >> 4;
        byteIssuID[2 * i + 1] = recovIPK.IssuID[i] & 0x0F;
    }

    for(i = 5; i >= 2; i--) 
    {
        if(byteIssuID[i] != 0x0F) 
        {
            if(memcmp(byteIssuID, bytePAN, i + 1)) 
            {
                return JY_REFUSE;
            } 
            else 
            {
                break;
            }
        }
    }
    
    if(i < 2)
    {
        return JY_REFUSE;
    }
    
    if(TERMAPP_ExpireDateVerify(recovIPK.ExpireDate) != OK) 
    {
        return JY_REFUSE;
    }
    //check if IPK is revoked.
    memcpy(buf, CardInfo.AID, 5);
    buf[5] = CardInfo.CAPKI;
    memcpy((uchar *)&buf[6], recovIPK.CertSerial, 3);

    /*
    res = f_mount(0, &fs);
    res = f_open (&fsrc,"REVOKE",FA_READ);
    if(res==OK)
    {
        filesize = fsrc.fsize;
        IPKRevokeNum=filesize/sizeof(IPK_REVOKE);
        for(i=0;i<IPKRevokeNum;i++)
        {
             f_lseek(&fsrc,i*sizeof(IPK_REVOKE));
             res=f_read(&fsrc, revokeList,sizeof(IPK_REVOKE), &br);
             if(!memcmp(buf,revokeList,9))
             {
                 f_close (&fsrc);
                f_mount(0,NULL);
                return JY_REFUSE;
             }
        }
        f_close (&fsrc);
        f_mount(0,NULL);
    }
    f_close (&fsrc);
    f_mount(0,NULL);
    */

    if(recovIPK.IPKAlgoInd != 0x01) return JY_REFUSE;
    IPKModulLen = recovIPK.IPKLen;
    if(recovIPK.IPKLen <= CAPK.ModulLen - 36) 
    {
        memcpy(IPKModul, recovIPK.IPKLeft, recovIPK.IPKLen);
    } 
    else 
    {
        memcpy(IPKModul, recovIPK.IPKLeft, CAPK.ModulLen - 36);
        memcpy((uchar *)&IPKModul[CAPK.ModulLen - 36], CardInfo.IPKRem, recovIPK.IPKLen - CAPK.ModulLen + 36);
    }
    return OK;
}


void TERMAPP_FillIPK(uchar *ipkData, IPK_RECOVER *recovIPK)
{
    uchar i;

    LOGI("TERMAPP_FillIPK() is called.\n");

    recovIPK->DataHead = ipkData[0];
    recovIPK->CertFormat = ipkData[1];
    memcpy(recovIPK->IssuID, (uchar *)&ipkData[2], 4);
    memcpy(recovIPK->ExpireDate, (uchar *)&ipkData[6], 2);
    memcpy(recovIPK->CertSerial, (uchar *)&ipkData[8], 3);
    recovIPK->HashInd = ipkData[11];
    recovIPK->IPKAlgoInd = ipkData[12];
    recovIPK->IPKLen = ipkData[13];
    recovIPK->IPKExpLen = ipkData[14];

    memcpy((uchar *)&recovIPK->IPKLeft, (uchar *)&ipkData[15], CAPK.ModulLen - 36);
    for(i = 0; i < 20; i++)
        recovIPK->HashResult[i] = ipkData[CAPK.ModulLen - 21 + i];
    recovIPK->DataTrail = ipkData[CAPK.ModulLen - 1];

}

uchar TERMAPP_SDVerify()
{
    uchar SSAToSign[512], SSAHash[20];
    unsigned int index;
    u32 Exp = 0;
    uchar SSAData[248];
    struct SIGN_STAT_APPDATA_RECOVER recovSSA;

    LOGI("TERMAPP_SDVerify() is called.\n");

    if(CardInfo.SignStatAppDataLen != IPKModulLen)
        return JY_REFUSE;

    if(CardInfo.IPKExpLen == 0x01) {
        Exp = CardInfo.IPKExp[2];    //2012-4-3
        if(Exp != 0x03) return JY_REFUSE;
    }
    if(CardInfo.IPKExpLen == 0x03) {
        if(memcmp(CardInfo.IPKExp, "\x01\x00\x01", 3) != 0x0) return JY_REFUSE;    //2012-4-3
        else Exp = CardInfo.IPKExp[0] * 256 * 256 + CardInfo.IPKExp[1] * 256 + CardInfo.IPKExp[2];
    }
    if(RSAPublicDecrypt(IPKModul, Exp, IPKModulLen * 8, CardInfo.SignStatAppData, SSAData) != OK)
        return JY_REFUSE;

    TERMAPP_FillSSA(SSAData, &recovSSA);

    if(recovSSA.DataTrail != 0xBC)     //If it is not 'BC' ,static data authentication has failed.
        return JY_REFUSE;
    if(recovSSA.DataHead != 0x6A)         //If it is not '6A' ,static data authentication has failed.
        return JY_REFUSE;
    if(recovSSA.DataFormat != 0x03)  //If it is not '03' ,static data authentication has failed.
        return JY_REFUSE;

    index = 0;
    memcpy(SSAToSign, (uchar *)&SSAData[1], IPKModulLen - 22);
    index += IPKModulLen - 22;

    memcpy((uchar *)&SSAToSign[index], AuthData, AuthDataLen);

    index += AuthDataLen;

    sha1(SSAToSign, index, SSAHash);

    if(recovSSA.HashInd == 0x01) { //SHA-1 algorithm
        if(memcmp(recovSSA.HashResult, SSAHash, 20))
            return JY_REFUSE;
    } else return JY_REFUSE;
    memcpy(CardInfo.DataAuthCode, recovSSA.DataAuthCode, 2);

    return OK;
}

void TERMAPP_FillSSA(uchar *SSAData, struct SIGN_STAT_APPDATA_RECOVER *recovSSA)
{
    uchar i;

    LOGI("TERMAPP_FillSSA() is called.\n");

    recovSSA->DataHead = SSAData[0];
    recovSSA->DataFormat = SSAData[1];
    recovSSA->HashInd = SSAData[2];
    memcpy(recovSSA->DataAuthCode, (uchar *)&SSAData[3], 2);
    memcpy(recovSSA->PadPattern, (uchar *)&SSAData[5], IPKModulLen - 26);
    for(i = 0; i < 20; i++)
        recovSSA->HashResult[i] = SSAData[IPKModulLen - 21 + i];
    recovSSA->DataTrail = SSAData[IPKModulLen - 1];
}

uchar TERMAPP_DynamicAuth()
{
    uchar retCode;

    LOGI("TERMAPP_DynamicAuth() is called.\n");

    if(TERMAPP_CheckDataMissDDA() != OK) 
    {
        TermInfo.TVR[0] |= 0x20;
        return JY_REFUSE;
    }
    if(CAPK.ModulLen == 0)
    {
#ifdef BCTCTEST
        if(TERMAPP_testGetCAPK() != OK) 
        {
            if(TERMAPP_testGetCAPK() != OK)
            {
                return JY_REFUSE;
            }
        }
#else
        if(TERMAPP_GetCAPK(CardInfo.AID, CardInfo.CAPKI) != OK)  {
            return JY_REFUSE;
        }
#endif
        if(IPKModulLen == 0) {
            if(TERMAPP_GetIPK() != OK) {
                return JY_REFUSE;
            }
        }
    }

    if(ICCPKModulLen == 0)
    {
        if(TERMAPP_GetICCPK() != OK)
        {
            return JY_REFUSE;
        }
    }

    if(TERMAPP_DynSignVerify() != OK) {
        return JY_REFUSE;
    }

    //串口发出调试信息"动态数据认证成功"
    LOGI("TERMAPP_DynamicAuth(): DynamicAuth is passed.\n");


    return OK;
}

uchar TERMAPP_GetICCPK()
{
    uchar ICCPKData[248], ICCPKToSign[1024], ICCPKHash[20];
    uchar i, byteRecovPAN[20], byteICCPAN[20];
    u32 Exp = 0;
    unsigned int index;
    struct ICCPK_RECOVER recovICCPK;

    LOGI("TERMAPP_GetICCPK() is called.\n");

    memset(ICCPKToSign, 0, 1024);
    if(CardInfo.ICCPKCertLen != IPKModulLen)
        return JY_REFUSE;
    if(CardInfo.IPKExpLen == 0x01) {
        Exp = CardInfo.IPKExp[2];
        if(Exp != 0x03) return JY_REFUSE;
    }
    if(CardInfo.IPKExpLen == 0x03) {
        if(memcmp(CardInfo.IPKExp, "\x01\x00\x01", 3) != 0x0) return JY_REFUSE;
        #if 1
        else Exp = ((CardInfo.IPKExp[0] << 8) << 8) + (CardInfo.IPKExp[1] << 8) + CardInfo.IPKExp[2];
        #else
        else Exp = CardInfo.IPKExp[0] * 256 * 256 + CardInfo.IPKExp[1] * 256 + CardInfo.IPKExp[2];
        #endif
    }
    if(RSAPublicDecrypt(IPKModul, Exp, IPKModulLen * 8, CardInfo.ICCPKCert, ICCPKData) != OK) {
        return JY_REFUSE;
    }
    TERMAPP_FillICCPK(ICCPKData, &recovICCPK);

    if(recovICCPK.DataTrail != 0xBC) //If it is not 'BC',ICCPK certifate is invalid
        return JY_REFUSE;
    if(recovICCPK.DataHead != 0x6A) //If it is not '6A',ICCPK certifate is invalid
        return JY_REFUSE;
    if(recovICCPK.CertFormat != 0x04) //If it is not '04',ICCPK certifate is invalid
        return JY_REFUSE;

    index = 0;
    memcpy(ICCPKToSign, (uchar *)&ICCPKData[1], IPKModulLen - 22);
    index += IPKModulLen - 22;

    if(recovICCPK.ICCPKLen > IPKModulLen - 42) {
        if(ICCDataTable[MV_ICCPKRem].bExist == 0) {
            TermInfo.TVR[0] |= 0x20;
            return JY_REFUSE;
        }
        memcpy((uchar *)&ICCPKToSign[index], (uchar *)&CardInfo.ICCPKRem, CardInfo.ICCPKRemLen);
        index += CardInfo.ICCPKRemLen;
    }
    if(CardInfo.ICCPKExpLen == 0x01) memcpy((uchar *)&ICCPKToSign[index], (uchar *)&CardInfo.ICCPKExp[2], CardInfo.ICCPKExpLen);
    if(CardInfo.ICCPKExpLen == 0x03) memcpy((uchar *)&ICCPKToSign[index], (uchar *)&CardInfo.ICCPKExp, CardInfo.ICCPKExpLen);
    index += CardInfo.ICCPKExpLen;
    memcpy((uchar *)&ICCPKToSign[index], AuthData, AuthDataLen);
    index += AuthDataLen;

    sha1(ICCPKToSign, index, ICCPKHash);

    if(recovICCPK.HashInd == 0x01) { //SHA-1 algorithm
        if(memcmp(recovICCPK.HashResult, ICCPKHash, 20))
            return JY_REFUSE;
    } else return JY_REFUSE;
    if(memcmp(CardInfo.PAN, recovICCPK.AppPAN, CardInfo.PANLen - 3)) {
        return JY_REFUSE;
    }
    if(TERMAPP_ExpireDateVerify(recovICCPK.ExpireDate) != OK) {
        return JY_REFUSE;
    }
    if(recovICCPK.ICCPKAlgoInd != 0x01) {
        return JY_REFUSE;
    }
    ICCPKModulLen = recovICCPK.ICCPKLen;
    if(recovICCPK.ICCPKLen <= IPKModulLen - 42) {
        memcpy(ICCPKModul, recovICCPK.ICCPKLeft, recovICCPK.ICCPKLen);
    } else {
        memcpy(ICCPKModul, recovICCPK.ICCPKLeft, IPKModulLen - 42);
        memcpy((uchar *)&ICCPKModul[IPKModulLen - 42], CardInfo.ICCPKRem, recovICCPK.ICCPKLen - IPKModulLen + 42);
    }
    return OK;
}

void TERMAPP_FillICCPK(uchar *ICCPKData, struct ICCPK_RECOVER *recovICCPK)
{
    uchar i;

    LOGI("TERMAPP_FillICCPK() is called.\n");

    recovICCPK->DataHead = ICCPKData[0];
    recovICCPK->CertFormat = ICCPKData[1];
    memcpy(recovICCPK->AppPAN, (uchar *)&ICCPKData[2], 10);
    memcpy(recovICCPK->ExpireDate, (uchar *)&ICCPKData[12], 2);
    memcpy(recovICCPK->CertSerial, (uchar *)&ICCPKData[14], 2);
    recovICCPK->HashInd = ICCPKData[17];
    recovICCPK->ICCPKAlgoInd = ICCPKData[18];
    recovICCPK->ICCPKLen = ICCPKData[19];
    recovICCPK->ICCPKExpLen = ICCPKData[20];
    memcpy(recovICCPK->ICCPKLeft, (uchar *)&ICCPKData[21], IPKModulLen - 42);
    for(i = 0; i < 20; i++) {
        recovICCPK->HashResult[i] = ICCPKData[IPKModulLen - 21 + i];
    }
    recovICCPK->DataTrail = ICCPKData[IPKModulLen - 1];
}

uchar TERMAPP_ExpireDateVerify(uchar *ExpireDate)
{
    uchar buf[8], CurDate[3], ExpDate[3];

    LOGI("TERMAPP_ExpireDateVerify() is called.\n");

    GetTime(buf);
    if(buf[0] > 0x49) CurDate[0] = 0x19;
    else CurDate[0] = 0x20;
    memcpy((uchar *)&CurDate[1], buf, 2);

    if(ExpireDate[1] > 0x49) ExpDate[0] = 0x19;
    else ExpDate[0] = 0x20;
    ExpDate[1] = ExpireDate[1];
    ExpDate[2] = ExpireDate[0];

    if(memcmp(CurDate, ExpDate, 3) > 0)
        return JY_REFUSE;
    else
        return OK;
}

uchar TERMAPP_FillSDA(uchar *SDAData, struct SIGN_DYN_APPDATA_RECOVER *recovSDA)
{
    uchar LDD, i;

    LOGI("TERMAPP_FillSDA() is called.\n");

    recovSDA->DataHead = SDAData[0];
    recovSDA->DataFormat = SDAData[1];
    recovSDA->HashInd = SDAData[2];
    recovSDA->ICCDynDataLen = SDAData[3];
    LDD = SDAData[3];
    if(LDD > ICCPKModulLen - 25)
        return JY_REFUSE;
    memcpy(recovSDA->ICCDynData, (uchar *)&SDAData[4], LDD);
    memcpy(recovSDA->PadPattern, (uchar *)&SDAData[LDD + 4], ICCPKModulLen - LDD - 25);
    for(i = 0; i < 20; i++)
        recovSDA->HashResult[i] = SDAData[ICCPKModulLen - 21 + i];
    recovSDA->DataTrail = SDAData[ICCPKModulLen - 1];
    return OK;
}

uchar TERMAPP_DynSignVerify()
{
    uchar SDAToSign[512], SDAHash[20], SDAData[248], i;
    unsigned int index;
    u32 Exp = 0;
    struct SIGN_DYN_APPDATA_RECOVER recovSDA;

    LOGI("TERMAPP_DynSignVerify() is called.\n");

    if(CardInfo.SignDynAppDataLen != ICCPKModulLen)
        {
        LOGI("__%s__,__%d___\n",__func__,__LINE__);
        return JY_REFUSE;
        }
    if(CardInfo.ICCPKExpLen == 0x01) {
        Exp = CardInfo.ICCPKExp[2];
        if(Exp != 0x03) return JY_REFUSE;
    }
    LOGI("__%s__,__%d___\n",__func__,__LINE__);
    if(CardInfo.ICCPKExpLen == 0x03) {
        if(memcmp(CardInfo.ICCPKExp, "\x01\x00\x01", 3) != 0x0) return JY_REFUSE;
        #if 1
        else Exp = ((CardInfo.ICCPKExp[0] << 8) << 8) + (CardInfo.ICCPKExp[1] << 8) + CardInfo.ICCPKExp[2];
        #else
        else Exp = CardInfo.ICCPKExp[0] * 256 * 256 + CardInfo.ICCPKExp[1] * 256 + CardInfo.ICCPKExp[2];
        #endif
    }
    if(RSAPublicDecrypt(ICCPKModul, Exp, ICCPKModulLen * 8, CardInfo.SignDynAppData, SDAData) != OK) {
        LOGI("__%s__,__%d___\n",__func__,__LINE__);    
        return JY_REFUSE;
    }
    LOGI("__%s__,__%d___\n",__func__,__LINE__);
    if(TERMAPP_FillSDA(SDAData, &recovSDA) != OK)
        return JY_REFUSE;
    if(recovSDA.DataTrail != 0xBC) //If it is not 'BC',dynamic data authentication has failed
        return JY_REFUSE;
    if(recovSDA.DataHead != 0x6A) //If it is not '6A',dynamic data authentication has failed
        return JY_REFUSE;
    if(recovSDA.DataFormat != 0x05) //If it is not '05',dynamic data authentication has failed
        return JY_REFUSE;
    LOGI("__%s__,__%d___\n",__func__,__LINE__);
    index = 0;
    memcpy(SDAToSign, (uchar *)&SDAData[1], ICCPKModulLen - 22);
    index += ICCPKModulLen - 22;

    if(ICCDataTable[MV_DDOL].bExist == 1) {
        TERMAPP_DOLProcess(typeDDOL, CardInfo.DDOL, CardInfo.DDOLLen, DDOLData, (uchar *)&DDOLDataLen);
    } else {
        if(TermInfo.TermDDOLLen != 0x03) TermInfo.TermDDOLLen = 0x03;
        TERMAPP_DOLProcess(typeDDOL, TermInfo.TermDDOL, TermInfo.TermDDOLLen, DDOLData, (uchar *)&DDOLDataLen);
    }

    memcpy((uchar *)&SDAToSign[index], DDOLData, DDOLDataLen);
    index += DDOLDataLen;

    sha1(SDAToSign, index, SDAHash);
    if(recovSDA.HashInd == 0x01) { //SHA-1 algorithm
        if(memcmp(recovSDA.HashResult, SDAHash, 20))
            return JY_REFUSE;
    } else    return JY_REFUSE;
    LOGI("__%s__,__%d___\n",__func__,__LINE__);
    CardInfo.ICCDynNumLen = recovSDA.ICCDynData[0];
    memcpy(CardInfo.ICCDynNum, recovSDA.ICCDynData + 1, recovSDA.ICCDynData[0]);
    ICCDataTable[MV_ICCDynNum].bExist = 1;
   
    return OK;
}

uchar TERMAPP_CheckDataMissDDA()
{
    uchar i;

    LOGI("TERMAPP_CheckDataMissDDA() is called.\n");

    LOGI("TERMAPP_CheckDataMissDDA(): ICCDataTable existing elements:\n");
    for(i = 0; i < ICCDataNum; i++)
    {
        if (ICCDataTable[i].bExist) LOGI("%d, ", i);
    }
    LOGI("\n");

    for(i = 0; i < ICCDataNum; i++) 
    {
        if(((ICCDataTable[i].flagM & 0x04) == 0x04) && (ICCDataTable[i].bExist == 0))
        {
            return JY_REFUSE;
        }
    }
    return OK;
}

uchar TERMAPP_CheckDataMissSDA()
{
    uchar i;

    LOGI("TERMAPP_CheckDataMissSDA() is called.\n");

    LOGI("TERMAPP_CheckDataMissSDA(): ICCDataTable existing elements:\n");
    for(i = 0; i < ICCDataNum; i++)
        if (ICCDataTable[i].bExist) LOGI("%d, ", i);
    LOGI("\n");

    for(i = 0; i < ICCDataNum; i++) {
        if(((ICCDataTable[i].flagM & 0x02) == 0x02) && ICCDataTable[i].bExist == 0)
            return JY_REFUSE;
    }
    return OK;
}

void TERMAPP_CreateUnpredictNum()
{
    unsigned short seed = 0, randnum = 0;
    unsigned int unPredict1 = 0;
    unsigned int unPredict2 = 0;
    uchar k, t[8];

    LOGI("TERMAPP_CreateUnpredictNum() is called.\n");

    seed = 0;
    GetTime(t);

    k = t[5];
    seed += ((k & 0xF0) >> 4) * 10 + k & 0x0F;
    k = t[4];
    #if 1
    seed += ((((k & 0xF0) >> 4) * 10 + k & 0x0F) << 2) * 15;
    #else
    seed += (((k & 0xF0) >> 4) * 10 + k & 0x0F) * 60;
    #endif
    k = t[3];
    #if 1
    seed += ((((k & 0xF0) >> 4) * 10 + k & 0x0F) << 4) * 225;
    #else
    seed += (((k & 0xF0) >> 4) * 10 + k & 0x0F) * 3600;
    #endif
    srand(seed);
    rand();
    unPredict1 = rand();
    unPredict2 = rand();
    memcpy((uchar *)&TermInfo.UnpredictNum, (uchar *)&unPredict1, 2);
    memcpy((uchar *)&TermInfo.UnpredictNum[2], (uchar *)&unPredict2, 2);
    TermDataTable[MV_UnpredictNum - TermDataBase].bExist = 1;

}
