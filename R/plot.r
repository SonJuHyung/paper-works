#!/usr/bin/env Rscript 

# parsing command line
args=commandArgs(trailingOnly=TRUE)

if(length(args) != 1){
    stop("expr type missing !!")
}

fsname<-args[1]
name<-paste("postmark_",fsname,"_default_count",sep="")
name_file<-paste(name,".txt",sep="")
name_result<-paste(name,".png",sep="")

path<-getwd()
path_file<-paste(path,"/data/",name_file,sep="")
path_result<-paste(path,"/result/",name_result,sep="")

#           cold                                                                                      hot
colors=c("#3333ff","#3366ff","#3399ff","#33ffff","#33ff99","#33ff33","#ffff33","#ff9933","#ff6633","#ff3333")
#colors=c("#3333ff","#3366ff","3399ff","#33ffff","#33ff99","#33ff33","#99ff33","#ffff33","#ff9933","#ff6633","#ff3333")

# reading data 
print(paste("reading ",name_file,"..."))
data_frame<-read.table(path_file, quote=" ", header=F) 
data_max_blk<-max(data_frame$V1)
data_max_ref<-max(data_frame$V2)
data_min_ref<-min(data_frame$V2)
data_frame<-transform(data_frame,data_norm=(data_frame$V2-min(data_frame$V2))/(max(data_frame$V2)-min(data_frame$V2)))

xlim=4095
print("reading done !")

# sorting...
print("sorting...")
ref_order<-order(-data_frame$V2)

x=0
y=0
i=1
j=1
color_index=10
interval=length(ref_order)/10
interval_sum = interval 
pre=0
ref=0


temp<-sort(data_frame$V2,decreasing=T)
temp2<-unique(temp)
print(temp2)
if(length(temp2) >= 10){
    term=length(temp2)/10
}else{
    term=10/length(temp2)
}
# index 10 
print(paste("term : ",term))
temp3=0
first=0
for( index in ref_order ){
    i=i+1
    data=data_frame$V1[index]
    ref=data_frame$V2[index]

    x=data%%(xlim+1)
    y=data/(xlim+1)

    if(i >= interval_sum){
        if(first == 0){
            temp3=ref
            first=1
        }else{
            if(pre != ref)
                temp3=c(temp3,ref)
        }

        print(ref)
        interval_sum=interval_sum+interval

        pre=ref
    }
}
print(temp3)
frame<-data.frame(temp3)

frame<-transform(frame,norm=(frame$temp3-min(frame$temp3))/(max(frame$temp3)-min(frame$temp3)))
print(frame)

print("sorting done !")

# plotting ...
print(paste("plotting ",name_result,"..."))

png(path_result, width=1440,height=1440,unit="px")
par(mar=c(6,5,9,3))
par(mgp=c(6,2,0))
plot(1,type="n",xlab="",ylab="",main=paste("Block address access pattern ","(",fsname,")",sep=""),xlim=c(0,xlim),ylim=c(0,data_max_blk/xlim)+10, cex.axis=4,xaxt='n',yaxt='n',cex.main=4)
x<-append(seq(from=0,to=4095,by=512),4095,after=512)
y<-seq(from=0,to=data_max_blk,by=64)
axis(1,at=x,labels=TRUE,cex.axis=2,tck=-0.01)
axis(2,at=y,labels=TRUE,cex.axis=2,tck=-0.01)

for( index in ref_order ){
    data=data_frame$V1[index]
    ref=data_frame$V2[index]

    x=data%%(xlim+1)
    y=data/(xlim+1)

    color_index = floor(frame$norm[j]*10)

    if(color_index==0)
        color_index=1 

    if(ref < frame$temp3[j]){ ## HOT 부터
        j=j+1
    }
    points(x,y,pch=4,cex=0.01,col=colors[color_index])
}

print("plotting done !")



