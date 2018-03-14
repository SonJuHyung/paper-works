#!/usr/bin/env Rscript 

# parsing command line 
print("setting argument ...");
args=commandArgs(trailingOnly=TRUE)

if(length(args) != 1){
    stop("expr type missing !!")
}

version<-args[1]
name<-paste("pbstate_result_",version,sep="")
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
height_max<-nrow(data_frame[data_frame$V1==width_max,])

print(paste("width(addr max) : ", width_max))
print(paste("height(scan count) : ", height_max))
print("done !")

# plotting ...
print(paste("plotting ",path_result,"..."))
png(path_result, width=2440,height=500,unit="px")
par(mar=c(10,5,3,3)) # default : c(5.1, 4.1, 4.1, 2.1) / margin : down, left, right, up
par(mgp=c(6,1,0)) # default : c(3,1,0) / position : title, line label, line

#plot(1,type="n",xlab="test x",ylab="test y",main="physical page blocks ",xlim=c(0,width_max+1),ylim=c(0,height_max), cex.axis=4,cex.main=4)
plot(1,type="n",xlab="physical page block status ",ylab="",xlim=c(0,width_max+1),ylim=c(0,height_max), cex.axis=4,xaxt='n',yaxt='n',cex.lab=4)

x_bar_raw<-seq(from=0,to=width_max,by=1024)
#x_bar<-x_bar_raw/512
x_bar<-paste(x_bar_raw/512,"GB",sep = " ")
y_bar<-seq(from=0,to=height_max,1)

axis(1,at=x_bar_raw,labels=x_bar,cex.axis=2,tck=-0.01)
axis(2,at=y_bar,labels=TRUE,cex.axis=2,tck=-0.01)
#text(x_bar_raw,labels=x_bar_char,)

print("test")
pb_state=0
x_pos=0
y_pos=0
color_index=0

for( i in data_frame$V1  ){

    x_pos=data_frame$V1[i]
    pb_state=data_frame$V2[i]

    if(length(x_pos) == 0){
        x_pos=0
        y_pos=y_pos+1
    }
    x_pos=x_pos+1
    if(length(pb_state) == 0){
        #    print("free")
        pb_state=1

    }else{
        pb_state=pb_state+1
        #switch(pb_state,
        #       print(paste(pb_state," unmovable")),
        #       print(paste(pb_state," movable")),
        #       print(paste(pb_state," reclaimable")),
        #       print(paste(pb_state," highatomic")),
        #       print(paste(pb_state," isolate")),
        #       print(paste(pb_state," invalid")))
    }
    points(x_pos,y_pos,pch=15,cex=1,col=colors[pb_state])
}

print("done !")

