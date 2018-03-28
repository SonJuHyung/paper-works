#!/usr/bin/env Rscript 

# parsing command line 
print("setting argument ...");
args=commandArgs(trailingOnly=TRUE)

if(length(args) != 2){
    stop("expr type missing !!")
}

workload<-args[1]
version<-args[2]
name_png<-paste("lat_result_",workload,"_",version,sep="")
name_nonfrag<-paste("lat_result_",workload,"_nonfrag_",version,sep="")
name_frag<-paste("lat_result_",workload,"_frag_",version,sep="")

name_result<-paste(name_png,".png",sep="")
name_file_nonfrag<-paste(name_nonfrag,".txt",sep="")
name_file_frag<-paste(name_frag,".txt",sep="")

path<-getwd()
path_result<-paste(path,"/plot/latency/",name_result,sep="")
path_file_nonfrag<-paste(path,"/../data/latency/",name_file_nonfrag,sep="")
path_file_frag<-paste(path,"/../data/latency/",name_file_frag,sep="")

#            1         2 
#          frag    nonfrag 
#          red       blue
colors=c("#FF3030","#009ACD")
legneds=c("fragmented","non-fragmented")
print("done !");

# reading data  
print(paste("reading ", path_file_nonfrag,"..."));
data_frame_nonfrag<-read.table(path_file_nonfrag, sep=",", header=F)
data_frame_frag<-read.table(path_file_frag, sep=",", header=F)

width_max<-max(data_frame_frag$V1)
height_max<-max(data_frame_frag$V2)

print(paste("width(addr max) : ", width_max))
print(paste("height(scan count) : ", height_max))
print("done !")

# plotting ...
print(paste("plotting ",path_result,"..."))

#png(path_result, width=3400,height=height_max*10,unit="px")
png(path_result, width=1000,height=1000,unit="px")

unit<-10
par(mar=c(10,10,3,3)) # default : c(5.1, 4.1, 4.1, 2.1) / margin : down, left, up, right
par(mgp=c(7,1.2,0)) # default : c(3,1,0) / position : title, line label, line
plot(data_frame_nonfrag$V2,xlab="frag : execute VM2_redis(10G) after VM1_mongodb(20G) \n default : execute VM2_redis(10G)",ylab="latency(ms)",xlim=c(0,width_max),ylim=c(0,height_max),pch=2,col=colors[2],type="o",xaxt='n',yaxt='n',cex.lab=2.5,xaxs='i',yaxs='r',lwd=1.5,cex=2) # xaxs : 'i':no margin, 'r':4% margin 
x_bar_raw<-seq(from=0,to=width_max,by=unit)
y_bar_raw<-seq(from=0,to=height_max,by=unit)

axis(1,at=x_bar_raw,labels=TRUE,cex.axis=1.5,tck=-0.01)
axis(2,at=y_bar_raw,labels=TRUE,cex.axis=1.5,tck=-0.01)

par(new=T)

plot(data_frame_frag$V2,xlab="",ylab="",xlim=c(0,width_max+1),ylim=c(0.6,height_max),pch=0,col=colors[1],type="o",xaxt='n',yaxt='n',lwd=1.5,cex=2,xaxs='i',yaxs='r') # xaxs : 'i':no margin, 'r':4% margin 

legend(x=5,y=65,legneds, cex=2, pch=c(2,0),col=colors)

print("done !")

