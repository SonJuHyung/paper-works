#!/usr/bin/env Rscript 

# parsing command line 
print("setting argument ...");
args=commandArgs(trailingOnly=TRUE)

if(length(args) != 3){
    stop("expr type missing !!")
}

workload<-args[1]
size<-args[2]
version<-args[3]
name_png<-paste("lat_result_",workload,"_vm_",size,"G_",version,sep="")
name_nonfrag<-paste("lat_result_",workload,"_vm_",size,"G_nonfrag_",version,sep="")
name_frag<-paste("lat_result_",workload,"_vm_",size,"G_frag_",version,sep="")

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
#colors=c("#FF3030","#009ACD")
colors=c("#FF3030","Black")

legneds=c("fragmented","non-fragmented")
print("done !");

# reading data  
print(paste("reading ", path_file_nonfrag,"..."));
data_frame_nonfrag<-read.table(path_file_nonfrag, sep=",", header=F)
data_frame_frag<-read.table(path_file_frag, sep=",", header=F)

width_max<-max(data_frame_frag$V1)
height_max<-max(data_frame_frag$V3)

print(paste("width(addr max) : ", width_max))
print(paste("height(scan count) : ", height_max))
print("done !")

# plotting ...
print(paste("plotting ",path_result,"..."))

#png(path_result, width=3400,height=height_max*10,unit="px")
png(path_result, width=1000,height=1200,unit="px")


unit_x<-10
if(size == 5){
unit_y<-5
}else if(size == 7){
unit_y<-10
}else if(size == 9){
    unit_y<-100
}


par(mar=c(10,10,3,3)) # default : c(5.1, 4.1, 4.1, 2.1) / margin : down, left, up, right
par(mgp=c(7,1.2,0)) # default : c(3,1,0) / position : title, line label, line 
xlab_text<-paste("frag : execute VM2_redis(",size,"G) after VM1_mongodb(19G) \n default : execute VM2_redis(",size,"G) only",sep="")
plot(data_frame_nonfrag$V3,xlab=xlab_text,ylab="latency(ms)",xlim=c(0,width_max),ylim=c(0,height_max),pch=2,col=colors[2],type="o",xaxt='n',yaxt='n',cex.lab=2.5,xaxs='i',yaxs='r',lwd=1.5,cex=2) # xaxs : 'i':no margin, 'r':4% margin 
x_bar_raw<-seq(from=0,to=width_max,by=unit_x)
y_bar_raw<-seq(from=0,to=height_max,by=unit_y)

axis(1,at=x_bar_raw,labels=TRUE,cex.axis=1.5,tck=-0.01)
axis(2,at=y_bar_raw,labels=TRUE,cex.axis=1.5,tck=-0.01)

par(new=T)

plot(data_frame_frag$V3,xlab="",ylab="",xlim=c(0,width_max+1),ylim=c(0.6,height_max),pch=0,col=colors[1],type="o",xaxt='n',yaxt='n',lwd=1.5,cex=2,xaxs='i',yaxs='r') # xaxs : 'i':no margin, 'r':4% margin 

legend(x=5,y=height_max*0.97,legneds, cex=2, pch=c(0,2),col=colors)

print("done !")

