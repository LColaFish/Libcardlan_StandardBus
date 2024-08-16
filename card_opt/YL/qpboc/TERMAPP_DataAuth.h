#ifndef __TERMAPP_DATAAUTH_H__
#define __TERMAPP_DATAAUTH_H__

typedef struct {
    uchar RID[5];			//ע���Ӧ���ṩ�̱�ʶ
    uchar CAPKI;			//��֤���Ĺ�Կ����
    unsigned int Exponent;	//��֤���Ĺ�Կָ��
    uchar ModulLen;			//��֤���Ĺ�Կģ����
    uchar *pModul;			//��֤���Ĺ�Կģ
} CAPK_STRUCT2;

extern CAPK_STRUCT2 CAPK_list[];

#endif //#ifndef __TERMAPP_DATAAUTH_H__
