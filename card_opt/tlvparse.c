/*****************************************************************************
* 文件描述：PBOC3.0-TLV Parse
* 应用平台：linux
* 创建时间：20150918
* 作者：RedXu
*****************************************************************************/
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include "tlvparse.h"

static void TLV_print_buffer(unsigned     char *buffer, int length)
{
    int i;

    for (i = 0; i < length; i++)
    {
        printf("%02X ", *(buffer + i));
        if ((i + 1) % 8 == 0) printf(" ");
        if ((i + 1) % 16 == 0) printf("\n");
    }
    printf("\n");
}


/**
 * 检查bit位是否为真
 * @param  value [待检测值]
 * @param  bit   [1~8位 低~高]
 * @return       [1 真 0 假]
 */
static inline int CheckBit(unsigned char value, int bit)
{
	unsigned char bitvalue[8] = {0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80};

	if((bit >= 1)&&(bit <= 8))
	{
		 if(value & bitvalue[bit-1])
		 	return(1);
		 else
		 	return(0);
	}
	else
	{
		printf("FILE: %s LINE: %d -- 传入的函数参数错误! bit=[%d]\n", 
			__FILE__, __LINE__, bit);
		return(-1);
	}
}

/**
 * Create TLVNode
 * @return  [description]
 */
static struct TLVNode* TLV_CreateNode(void)
{
	struct TLVNode* node = (struct TLVNode *)malloc(sizeof(*node));
  	if(node == NULL)
	{
		return NULL;
	}
	memset(node,0,sizeof(*node));
	return node;
}

/**
 * Free TLVNode Memory
 * @param node [node]
 */
void TLV_Free(struct TLVNode* node)
{
	int i;
	if(node == NULL)
	{
		return;
	}

	if(node->Value)
		free(node->Value);

	for(i=0;i<node->SubCount;i++)
	{
		TLV_Free(node->Sub[i]);
		node->Sub[i] = NULL;
	}

	if(node->Next)
	{
		TLV_Free(node->Next);
		node->Next = NULL;
	}

	free(node);
}

/**
 * 打印TLV子结点信息(深度优先)
 * @param parent [description]
 * @param step   [description]
 */
static void TLV_DebugSubNode(struct TLVNode* parent,int step)
{
	int i,j;
	for(i=0;i<parent->SubCount;i++)
	{
		struct TLVNode* sub = parent->Sub[i];
		printf("---[%d]SUBTAG=%04X LEN=[%d] ADDR=[%p]\n",step,sub->Tag,sub->Length,sub);
        TLV_print_buffer(sub->Value, sub->Length);
		printf("[%d]MoreFlag=%d SubFlag=%d\n",step,sub->MoreFlag,sub->SubFlag);

		if(sub->SubFlag == 1)
		{
			TLV_DebugSubNode(sub,step+1);
		}
	}
}

/**
 * DEBUG TLVNode
 * @param node [description]
 */
void TLV_Debug(struct TLVNode* node)
{
    if(node == NULL)
    {
        return;
    }
	int i;
	printf("\n************************BEGIN TLV DEBUG************************\n");
	printf("TAG=%04X LEN=[%d] ADDR=[%p]\n",node->Tag,node->Length,node);
    TLV_print_buffer(node->Value, node->Length);
	printf("MoreFlag=%d SubFlag=%d\n",node->MoreFlag,node->SubFlag);
	if(node->SubFlag == 1)
	{
		TLV_DebugSubNode(node,1);
	}
	if(node->Next)
	{
		printf("Next=[%p]\n",node->Next);
	}
	printf("************************END TLV DEBUG************************\n");
}

/**
 * 解析一条数据到TLVNode结构
 * @param  buf  [description]
 * @param  size [description]
 * @param  sflag [description]
 * @return      [description]
 */
static struct TLVNode* TLV_Parse_One(unsigned char* buf,int size)
{
	int index = 0;
	int i;
	uint16_t tag1,tag2,tagsize;
	uint16_t len,lensize;
	unsigned char* value;
	struct TLVNode* node = TLV_CreateNode();
    if(node == NULL)
    {
        return NULL;
    }
	tag1 = tag2 = 0;
	tagsize = 1;
	tag1 = buf[index++];

	if((tag1 & 0x1f) == 0x1f)
	{
		tagsize++;
		tag2 = buf[index++];
		//tag2 b8 must be 0!
	}
	if(tagsize == 1)
		node->Tag = tag1;
	else
		node->Tag = (tag1<<8) + tag2;
	node->TagSize = tagsize;

	//SubFlag
	node->SubFlag = CheckBit(tag1,6);

	//L zone
	len = 0;
	lensize = 1;
	len = buf[index++];
	if(CheckBit(len,8) == 0)
	{
		node->Length = len;
	}
	else
	{
		lensize = len & 0x7f;
		len = 0;
		for(i=0;i<lensize;i++)
		{
			//不确定长度是否是小端编码?
			len += (uint16_t)buf[index++] << (i*8);
		}
		//fix lensize
		lensize++;
	}
	node->Length = len;
	node->LengthSize = lensize;

	//V zone
	value = (unsigned char *)malloc(len);
    if(value == NULL)
    {
        return NULL;
    }
	memcpy(value,buf+index,len);
	node->Value = value;
	index += len;

	if(index < size)
	{
		node->MoreFlag = 1;
	}
	else if(index == size)
	{
		node->MoreFlag = 0;
	}
	else
	{
		printf("Parse Error! index=%d size=%d\n",index,size);
        return NULL;
	}

	return node;
}


/**
 * [TLV_Parse_More description]
 * @param  parent [description]
 * @return        [1 more 0 over]
 */
static int TLV_Parse_SubNodes(struct TLVNode* parent)
{
	//check have more
	int sublen = 0;
	int i;

	//have no subtag!
	if(parent->SubFlag == 0)
		return 0;

	for(i=0;i<parent->SubCount;i++)
	{
		sublen += (parent->Sub[i]->TagSize + parent->Sub[i]->Length + parent->Sub[i]->LengthSize);
	}

	if(sublen < parent->Length)
	{
		struct TLVNode* subnode = TLV_Parse_One(parent->Value+sublen,parent->Length-sublen);
		parent->Sub[parent->SubCount++] = subnode;
		return subnode->MoreFlag;
	}
	else
	{
		return 0;
	}
}


/**
 * 解析子段内嵌数据(广度优先)
 * @param parent [description]
 */
static void TLV_Parse_Sub(struct TLVNode* parent)
{
	int i;
	if(parent->SubFlag != 0)
	{
		//parse sub nodes.
		while(TLV_Parse_SubNodes(parent) != 0)
		{

		}
		
		for(i=0;i<parent->SubCount;i++)
		{
			if(parent->Sub[i]->SubFlag != 0)
			{
				TLV_Parse_Sub(parent->Sub[i]);
			}
		}
	}
}


/**
 * 解析数据,生成TLV结构
 * @param  buf  [数据]
 * @param  size [数据长度]
 * @return      [TLVNode]
 */
TLVNode_T* TLV_Parse(unsigned char* buf,int size)
{
	struct TLVNode* node = TLV_Parse_One(buf,size);
    if(node == NULL)
    {
        return NULL;
    }
	TLV_Parse_Sub(node);
	return node;
}

/**
 * 拷贝一个节点到另一个结点(不包含Next)
 * @param dst [description]
 * @param src [description]
 */
static void TLV_Copy(struct TLVNode* dst,struct TLVNode* src)
{
	int i;
	memcpy(dst,src,sizeof(*src));
	dst->Value = (unsigned char*)malloc(src->Length);
	memcpy(dst->Value,src->Value,src->Length);

	for(i=0;i<src->SubCount;i++)
	{
		dst->Sub[i] = TLV_CreateNode();
		TLV_Copy(dst->Sub[i],src->Sub[i]);
	}
}

/**
 * 合并src结构到target
 * @param target [目标TLVNode]
 * @param src    [源TLVNode]
 */
void TLV_Merge(struct TLVNode* target,struct TLVNode* src)
{
	int i;
	int found = 0;
	struct TLVNode* tmpnode = target;
	assert(target != NULL);
	while(tmpnode)
	{
		//只比较一级tag是否相同,子tag必须是不同的
		if(tmpnode->Tag == src->Tag)
		{
			found = 1;
			break;
		}

		if(tmpnode->Next)
			tmpnode = tmpnode->Next;
		else
			break;
	}

	if(found == 0)
	{
		tmpnode->Next = TLV_CreateNode();
		TLV_Copy(tmpnode->Next,src);
	}
	else
	{
		for(i=0;i<src->SubCount;i++)
		{
			tmpnode->Sub[tmpnode->SubCount] = TLV_CreateNode();
			TLV_Copy(tmpnode->Sub[tmpnode->SubCount],src->Sub[i]);
			tmpnode->SubCount++;
			tmpnode->SubFlag = 1;
		}
	}
}
/**
 * 在node中查找tag.
 * @param  node [TLVNode]
 * @param  tag  [tag标签]
 * @return      [NULL - 未找到]
 */
int TLV_Find(  TLVNode_T* node,uint16_t tag,TLVNode_T **node_list)
{
  	int i,num = 0;

    if(node == NULL || node_list == NULL)
    {
        return 0;
    }
    
    if(node->Tag == tag)
	{
    	node_list[0] = node;
		return 1;
	}

    
    TLVNode_T* tmpnode;
	for(i = 0; i < node->SubCount ; i++)
	{
		num += TLV_Find(node->Sub[i],tag,&node_list[num]);
    }
  
	return num;
}


/**
 * 在node中查找tag.
 * @param  node [TLVNode]
 * @param  tag  [tag标签]
 * @return      [NULL - 未找到]
 */
TLVNode_T* TLV_Find_one(struct TLVNode* node,uint16_t tag)
{
	int i;
	struct TLVNode* tmpnode;
    if(node == NULL)
    {
        return NULL;
    }
    if(node->Tag == tag)
	{
		return node;
	}
	for(i=0;i<node->SubCount;i++)
	{
		tmpnode = NULL;
		tmpnode = TLV_Find_one(node->Sub[i],tag);
		if(tmpnode != NULL)
			return tmpnode;
	}
	if(node->Next)
	{
		tmpnode = NULL;
		tmpnode = TLV_Find_one(node->Next,tag);
		if(tmpnode != NULL)
			return tmpnode;
	}

	return NULL;
}

