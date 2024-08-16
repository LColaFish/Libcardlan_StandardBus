
#include "card_opt/YL/YL_card_operation.h"
#include "cardlan_devctrl.h"

int YL_Card_test()
{
    /*  银联卡测试 */
    int channel = 0;
    int ret = 0;
    TERMAPP_QPBOCTermInit(channel);
    YL_CPUCARD_Info_t CardInfo;

    ret = YL_ReadCardInfor_CPU(&CardInfo);    
    if(ret != 0)
    {
        printf("[%s %d]  error ret: %d !!\n",__FUNCTION__,__LINE__,ret);
        return -1;
    }
    return 0;
    ret = TERMAPP_HandleCard(channel,1);
    if(ret != 0)
    {
        printf("[%s %d]  error ret: %d !!\n",__FUNCTION__,__LINE__,ret);
        return -2;
    }
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
	if(ret != 0x20)
	{
		return -1;
	}
    printf("cardreset:\n");
    menu_print(Buffer, len);
    YL_Card_test();
    
    return 0;
}
