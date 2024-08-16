
#include "card_opt/JTB/cardlan_JTB_operation.h"
#include "cardlan_devctrl.h"


int JTB_Card_test(void)
{
	JTB_CPU_CardInfo JTB_CardInfo;
    int ret;

    /*  交通部卡测试 */
    {
        ret = ReadCardInfor_CPU(&JTB_CardInfo);
        if(ret != 0 )
        {
            printf("[%s %d]  error ret:%d!!\n",__FUNCTION__,__LINE__,ret);
            return -3;
        }

        /* Consume */
        //MonthlyCardConsume(1);
        ret = NormalCardConsume(&JTB_CardInfo,1);
        if(ret != 0 )
        {
            printf("[%s %d]  error ret:%d!!\n",__FUNCTION__,__LINE__,ret);
            return -3;
        }
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
        printf("cardreset fail ret :%d \n",ret);
		return -1;
	}
    printf("cardreset:\n");
    menu_print(Buffer, len);
    JTB_Card_test();
    
    return 0;
}
