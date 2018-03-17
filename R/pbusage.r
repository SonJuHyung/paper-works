#!/usr/bin/env Rscript 

# parsing command line 
print("setting argument ...");
args=commandArgs(trailingOnly=TRUE)

if(length(args) != 2){
    stop("expr type missing !!")
}

workload<-args[1]
version<-args[2]
name<-paste("pbstate_result_",workload,"_",version,sep="")
name_file<-paste(name,".txt",sep="")
name_result<-paste(name,".png",sep="")

path<-getwd()
path_file<-paste(path,"/../data/",name_file,sep="")
path_result<-paste(path,"/plot/",name_result,sep="")

#         free   unmov   mov    recm     hato    iso      inv
colors=c("White","Red","Green","Yellow","White","white","Gray")
print("done !");

# reading data  
print(paste("reading ", path_file,"..."));
data_frame<-read.table(path_file, sep=",", header=F)
width_max<-max(data_frame$V1)
height_max<-nrow(data_frame[data_frame$V1==0,])

print(paste("width(addr max) : ", width_max))
print(paste("height(scan count) : ", height_max))
print("done !")

# plotting ...
print(paste("plotting ",path_result,"..."))
# png_height= round(3400*height_max/150)
# if(png_height < 200)
#     png_height = 200 

#png(path_result, width=3400,height=height_max*10,unit="px")
png(path_result, width=3400,height=500,unit="px")

par(mar=c(10,5,3,3)) # default : c(5.1, 4.1, 4.1, 2.1) / margin : down, left, up, right
par(mgp=c(6,2,0)) # default : c(3,1,0) / position : title, line label, line
plot(1,type="n",xlab="physical page block status ",ylab="",xlim=c(1,width_max),ylim=c(1,height_max+1), xaxt='n',yaxt='n',cex.lab=4,xaxs='i')

x_bar_raw<-seq(from=0,to=width_max,by=2048)
x_bar<-paste(x_bar_raw/512,"GB",sep = " ")
#y_bar<-seq(from=0,to=height_max,1)

axis(1,at=x_bar_raw,labels=x_bar,cex.axis=2,tck=-0.01)
#axis(2,at=y_bar,labels=TRUE,cex.axis=2,tck=-0.01)

color_index=0
x_pos=0 
y_pos=0
index=0

free=umov=mov=reclm=high=iso=inv=0
for( i in data_frame$V1  ){ 
    if(i == 0)
        y_pos=y_pos+1 

    x_pos=i+1
    color_index=data_frame$V2[index+1]
#     if(color_index == 0)
#         free=free+1
#     if(color_index == 1)
#         umov=umov+1
#     if(color_index == 2)
#         mov=mov+1
#     if(color_index == 3) {
#         reclm=reclm+1
#     }
#     if(color_index == 4)
#         high=high+1
#     if(color_index == 5)
#         iso=iso+1
#     if(color_index == 6)
#         inv=inv+1
    points(x_pos,y_pos,pch=1,cex=0.5,col=colors[color_index+1])  
      index=index+1 

}

# print(paste("free : ",free,sep=" "))
# print(paste("unmovable : ",umov,sep=" "))
# print(paste("movable : ",mov,sep=" "))
# print(paste("reclaimable : ",reclm,sep=" "))
# print(paste("highatomic : ",high,sep=" "))
# print(paste("isolate : ",iso,sep=" "))
# print(paste("invalid : ",inv,sep=" "))
print("done !")

