
#ifndef __YL_CARD_UTIL_H__
#define __YL_CARD_UTIL_H__

//持卡人有效账号
int YL_find_tag_0x5A(unsigned char *p_src,int length,unsigned char dst[12]);
/*
    按 GB/T 17552 的规定，磁条 2 的数据。不包括起始位、结束位和 LRC（验证码），包括：
    应用主账号（PAN）
    分隔符（“D”）
    失效日期（YYMM）
    服务码
    PIN 验证域
    自定义数据（由支付系统
    定义）
    补 F（如果不是偶数个）
*/
int YL_find_tag_0x57(unsigned char *p_src,int length,unsigned char dst[21]);
/*
    JR/T 0025.15 5.4.1
    GET DATA 命令格式
    字节      值               注释
    CLA     ‘80’ 
    INS     ‘CA’ 
    P1      ‘9F’或‘DF’     ‘9F79’为电子现金余额数据元标签
                          ‘DF79’为第二币种电子现金余额数据元标签  
    P2      ‘79’
    LC      不存在 
    Data    不存在 
    Le      ‘00’ 
    
    电子现金余额查询响应
    字节          值                  注释
    标签（T）       ‘9F79 / DF79’ 
    长度（L）       ‘06’              6 字节长
    数据（V）                         电子现金余额  以应用定义货币表示
    SW1/SW2     ---               状态信息
*/
int YL_get_balance(unsigned long *p_balance);
/*
    JR/T 0025.5 B.7.2
    取数据（GET DATA）命令报文
    编码      值
    CLA     80
    INS     CA
    P1 P2   要访问数据的标签
    Lc      不存在
    数据域     不存在
    Le      00
*/
int YL_get_tag_data(unsigned char P1,unsigned char P2,unsigned char *p_respond_buff,int length);

#endif //__YL_CARD_UTIL_H__

