
#include "card_opt/M1/M1_Des_card_operation.h"
#include "cardlan_devctrl.h"


unsigned char CSN[4];

static int mifare_des_card_test(void)
{
    int ret;
    return 0;
}

/* Add For Test */
int main(void)
{
    int ret;
    ret = InitDev();
    if(ret != 0)
    {
        printf("[%s %d]  error ret:%d!!\n",__FUNCTION__,__LINE__,ret);
        return ret;
    }

    char             Buffer[100] = {0};
    unsigned char    len         = 0;
    ret = CardReset(Buffer, &len , 0);
	if(ret != 0x20 )
	{
	    printf("[%s %d]  error ret:0x%02X!!\n",__FUNCTION__,__LINE__,ret);
		return -1;
	}
    printf("cardreset:\n");
    menu_print(Buffer, len);
    memcpy(CSN,Buffer,len);
    mifare_des_card_test();
    
    return 0;
}
