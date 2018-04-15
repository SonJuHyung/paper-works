#!/bin/bash 

#cat $1 | awk '{ split($0,parse1,":"); funcname=parse1[2]; split(parse1[3],parse2," "); split(parse2[2],_pfn,"="); pfn=_pfn[2]; split(parse2[3],_order,"="); order=_order[2]; split(parse2[4],_mtype,"="); mtype=_mtype[2]; printf("%s,%s,%s\n",funcname,pfn,order,mtype);  }'
cat $1 | awk '{ split($0,parse1,"son__alloc_pages_nodemask:"); split(parse1[2],parse2,","); name=parse2[2]; order=parse2[3]; freepage=parse2[4]; ufi=parse2[5]; printf("%s,%s,%s,%s\n",name,order,freepage,ufi);  }'
