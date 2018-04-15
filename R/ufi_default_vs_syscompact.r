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

workload<-args[1]
work_size<-args[2]
version<-args[3]
name_png<-paste("frag_result_",workload,"_local_","G_used_default_vs_syscompact_",version,sep="")
name_default<-paste("ufi_result_",workload,"_local_",work_size,"G_used_default_",version,sep="")
name_compact<-paste("ufi_result_",workload,"_local_",work_size,"G_used_compact_",version,sep="") 

name_result<-paste(name_png,".png",sep="")
name_file_default<-paste(name_default,".txt",sep="")
name_file_compact<-paste(name_compact,".txt",sep="")

path<-getwd()
path_result<-paste(path,"/plot/ufi/",name_result,sep="")
path_file_default<-paste(path,"/../data/ufi/",name_file_default,sep="")
path_file_compact<-paste(path,"/../data/ufi/",name_file_compact,sep="")

#          0       1  
#        default  compact
colors=c("black","#FF3030")
legneds=c("without compaction","with compaction")
print("done !");

# reading data  
print(paste("reading ", path_file_default,"..."));
data_frame_default<-read.table(path_file_default, sep=",", header=F)
data_frame_compact<-read.table(path_file_compact, sep=",", header=F)

width_max<-max(data_frame_default$V1)
height_max<-1.1

print(paste("width(addr max) : ", width_max))
print(paste("height(scan count) : ", height_max))
print("done !")

# plotting ...
print(paste("plotting ",path_result,"..."))

#png(path_result, width=3400,height=height_max*10,unit="px")
png(path_result, width=1000,height=1000,unit="px")

par(mar=c(10,10,3,3)) # default : c(5.1, 4.1, 4.1, 2.1) / margin : down, left, up, right
par(mgp=c(7,1.2,0)) # default : c(3,1,0) / position : title, line label, line 
# par(mar=c(10,10,3,3)) # default : c(5.1, 4.1, 4.1, 2.1) / margin : down, left, up, right
# par(mgp=c(7,1.2,0)) # default : c(3,1,0) / position : title, line label, line

# xlab_default=paste("without compaction : execute ",workload,"(",workload_size,"G)"," after mongodb ","(",free_size,"G free)",sep="");
# xlab_compact=paste("with compaction : execute ",workload,"(",workload_size,"G)", ,"(",free_size,"G free)",sep="");
# xlab_text=paste(xlab_vm,"\n",xlab_local)
xlab_text="execute redis(15G) after mongodb(15G)"
ylab_text="unusable free space index"

plot(data_frame_default$V2,xlab=xlab_text,ylab=ylab_text,xlim=c(0,width_max+1),ylim=c(0.3,height_max+0.1),pch=2,col=colors[1],type="o",xaxt='n',yaxt='n',cex.lab=2.5,xaxs='i',yaxs='i',lwd=1.5,cex=2) # xaxs : 'i':no margin, 'r':4% margin 

x_bar_raw<-seq(from=0,to=width_max,by=2)
y_bar_raw<-seq(from=0,to=1,by=0.1)

axis(1,at=x_bar_raw,labels=TRUE,cex.axis=1.5,tck=-0.01)
axis(2,at=y_bar_raw,labels=TRUE,cex.axis=1.5,tck=-0.01)

par(new=T)

plot(data_frame_compact$V3,xlab="",ylab="",xlim=c(0,width_max+1),ylim=c(0.3,height_max+0.1),pch=0,col=colors[2],type="o",xaxt='n',yaxt='n',lwd=1.5,cex=2,xaxs='i',yaxs='i') # xaxs : 'i':no margin, 'r':4% margin 

legend(x=20,y=1.18,legneds, cex=2, pch=c(2,0),col=colors)

print("done !")

