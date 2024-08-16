

#include "cardlan_devctrl.h"
#include "mcu_opt/psam_opt.h"

static int psam_test(void)
{
    int ret;
    int BaudRate[3] = {9600,38400,115200};

    
    unsigned char PsamNum[6] = {0};
    unsigned char PsamKeyIndex = 0; 

    int i,j;

    for(j = 0 ; j < 3 ; j++)
        for(i = 0 ; i < 3 ; i++)
        {
            ret = InitPsam(j, BaudRate[i]);
            if(ret != 0)
            {
                printf("[ %s %d ]InitPsam error ret : %d!!\n",__FUNCTION__,__LINE__,ret);
                continue;
            }

            ret = GetPsamID_ex(j,PsamNum,&PsamKeyIndex);
            if(ret != 0)
            {
                printf("[ %s %d ]GetPsamID error ret : %d!!\n",__FUNCTION__,__LINE__,ret);
                continue;
            }
            break;
        }
        printf("[ %s %d ]PsamNum :\n",__FUNCTION__,__LINE__);
        menu_print(PsamNum,6);
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


    psam_test();
    
    return 0;
}
