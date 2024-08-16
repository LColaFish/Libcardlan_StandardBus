部分文件介绍
***************************************************************************
Android.mk：
指定静态库libcrypto.a和libssl.a，并执行Android_663.mk和Android_example.mk。
//一般不用改动

Android_example.mk：
编译example内的测试用文件，可供安卓直接运行。
//使用前应确认example内Android.mk，要测试的功能是否被注释（#）
//如编译出错请检查example/*内的Android.mk，LOCAL_SHARED_LIBRARIES后的内容是否与Android_663.mk内的LOCAL_MODULE一致，LOCAL_C_INCLUDED后的路径是否是rc663_opt/

Android_663.mk：
LOCAL_SRC_FILES声明了使用到的源文件
//如需在so库中添加新接口，确认调用的函数所在的源文件是否在里面

LOCAL_C_INCLUDES声明了源文件引用到的头文件所在文件夹

LOCAL_STATIC_LIBRARIES声明调用的静态库

LOCAL_MODULE定义生成的SO库名称

cardlan_devctrl.c：
声明了InitDev函数的定义

libcardlan_mifareplus.c:
功能未查明

libcardlan_StandardBus_util.c：
杂项，主要是一些数据类型转换用的函数和一个显示结构体变量JTB_CardInfo所有成员（也就是进行卡片消费过程中获取的数据，需自行填入）的函数


使用说明
**************************************************************
请确认已配置好NDK，使用build_cmd.bat即可自动完成so库编译。默认生成位置：./libs/armeabi-v7a
