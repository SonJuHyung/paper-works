#!/usr/bin/env Rscript 
# 
# default : vm_mongo -> vm_redis 's UFI
# compact : vm_mongo -> vm_redis 's UFI but compact in 10s interval

# parsing command line 
print("setting argument ...");
args=commandArgs(trailingOnly=TRUE)

if(length(args) != 3){
    stop("expr type missing !!")
}

hp_type<-args[1]
interval<-args[2]
version<-args[3]

#name_png<-paste("pbstat_result_redis_",hp_type,"_",interval,"_",version,sep="")
#name_png<-paste("pbstat_result_mongodb_",hp_type,"_",interval,"_",version,sep="")
name_png<-paste("pbstat_result_redis_",hp_type,"_",interval,"_",version,sep="")
#name_file<-paste("pbstat_result_redis_",hp_type,"_",interval,"_",version,sep="")
#name_file<-paste("pbstat_result_mongodb_",hp_type,"_",interval,"_",version,sep="")
name_file<-paste("pbstat_result_redis_",hp_type,"_",interval,"_",version,sep="") 

name_png<-paste(name_png,".png",sep="")
name_file<-paste(name_file,".txt",sep="")
#name_file_mongodb<-paste(name_mongodb,".txt",sep="")
#name_file_cassandra<-paste(name_cassandra,".txt",sep="")

path<-getwd()
path_png<-paste(path,"/plot/pbstat/",name_png,sep="")
path_file<-paste(path,"/../data/pbstat/",name_file,sep="")
#path_file_mongodb<-paste(path,"/../data/pbstat/",name_file_mongodb,sep="")
#path_file_cassandra<-paste(path,"/../data/pbstat/",name_file_cassandra,sep="")

#          0       1  
#        default  compact
colors=c("blue","green","yellow","red")
#legneds=c("without compaction","with compaction")
print("done !");

# reading data  
print(paste("reading ..."));
data_frame<-read.table(path_file, sep=",", header=F)
#data_frame_mongodb<-read.table(path_file_mongodb, sep=",", header=F)
#data_frame_cassandra<-read.table(path_file_cassandra, sep=",", header=F)

colnames(data_frame) <- c("nid","zone","white","blue","green","yellow","red","umov","iso_mg","iso_fr")
width_max<-nrow(data_frame)
height_max<-15360

print(paste("width(addr max) : ", width_max))
print(paste("height(scan count) : ", height_max))
print("done !")

head(data_frame)

#### plotting ...
print(paste("plotting ",path_png,"..."))

#png(path_result, width=3400,height=height_max*10,unit="px")
png(path_png, width=1000,height=1000,unit="px")

par(mar=c(5,5,3,3)) # default : c(5.1, 4.1, 4.1, 2.1) / margin : down, left, up, right
par(mgp=c(3,1,0)) # default : c(3,1,0) / position : title, line label, line 
# par(mar=c(10,10,3,3)) # default : c(5.1, 4.1, 4.1, 2.1) / margin : down, left, up, right
# par(mgp=c(7,1.2,0)) # default : c(3,1,0) / position : title, line label, line

# xlab_default=paste("without compaction : execute ",workload,"(",workload_size,"G)"," after mongodb ","(",free_size,"G free)",sep="");
# xlab_compact=paste("with compaction : execute ",workload,"(",workload_size,"G)", ,"(",free_size,"G free)",sep="");
# xlab_text=paste(xlab_vm,"\n",xlab_local)
#xlab_text="redis"
#xlab_text="mongodb"
xlab_text="redis(15G)"
ylab_text="page block count"

#plot(data_frame_redis$blue,xlab=xlab_text,ylab=ylab_text,xlim=c(0,width_max+1),ylim=c(0,height_max),pch=2,col=colors[1],type="o",xaxt='n',yaxt='n',cex.lab=2.5,xaxs='i',yaxs='i',lwd=1.5,cex=2) # xaxs : 'i':no margin, 'r':4% margin 
plot(data_frame$blue,xlab=xlab_text,ylab=ylab_text,xlim=c(0,width_max),ylim=c(0,height_max),pch=2,col=colors[1],type="l",cex.lab=2.5,xaxs='i',yaxs='i',lwd=1.5,cex=2) # xaxs : 'i':no margin, 'r':4% margin 

# x_bar_raw<-seq(from=0,to=width_max,by=2)
# y_bar_raw<-seq(from=0,to=1,by=0.1)
# 
# axis(1,at=x_bar_raw,labels=TRUE,cex.axis=1.5,tck=-0.01)
# axis(2,at=y_bar_raw,labels=TRUE,cex.axis=1.5,tck=-0.01)
# 
par(new=T) 
plot(data_frame$green,xlab=xlab_text,ylab=ylab_text,xlim=c(0,width_max),ylim=c(0,height_max),pch=2,col=colors[2],type="l",cex.lab=2.5,xaxs='i',yaxs='i',lwd=1.5,cex=2) # xaxs : 'i':no margin, 'r':4% margin 
par(new=T) 
plot(data_frame$yellow,xlab=xlab_text,ylab=ylab_text,xlim=c(0,width_max),ylim=c(0,height_max),pch=2,col=colors[3],type="l",cex.lab=2.5,xaxs='i',yaxs='i',lwd=1.5,cex=2) # xaxs : 'i':no margin, 'r':4% margin 
par(new=T) 
plot(data_frame$red,xlab=xlab_text,ylab=ylab_text,xlim=c(0,width_max),ylim=c(0,height_max),pch=2,col=colors[4],type="l",cex.lab=2.5,xaxs='i',yaxs='i',lwd=1.5,cex=2) # xaxs : 'i':no margin, 'r':4% margin 

# 
# plot(data_frame_redis$V3,xlab="",ylab="",xlim=c(0,width_max+1),ylim=c(0.3,height_max+0.1),pch=0,col=colors[2],type="o",xaxt='n',yaxt='n',lwd=1.5,cex=2,xaxs='i',yaxs='i') # xaxs : 'i':no margin, 'r':4% margin 
# 
# legend(x=20,y=1.18,legneds, cex=2, pch=c(2,0),col=colors)
# 
# print("done !")

