#include "consumer.h"
#include<stdio.h>
Consumer :: Consumer(int iid)
{
    id=iid;
}
Consumer :: ~Consumer()
{
    printf("ESTORYING CONSUMER %d\n", id);
}

void Consumer :: consu(int i)
{
     printf("consumer %d thaking product %d\n",id,i);
}
