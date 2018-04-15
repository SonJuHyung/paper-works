#!/usr/bin/env Rscript 

# parsing command line 
print("setting argument ...");
args=commandArgs(trailingOnly=TRUE)

if(length(args) != 2){
    stop("expr type missing !!")
}

workload<-args[1]
version<-args[2]
name_png<-paste("ufi_result_performance",workload,"_local_vm_",version,sep="")
name_local<-paste("ufi_result_",workload,"_local_",version,sep="")
name_vm<-paste("ufi_result_",workload,"_vm_",version,sep="")

name_result<-paste(name_png,".png",sep="")
name_file_local<-paste(name_local,".txt",sep="")
name_file_vm<-paste(name_vm,".txt",sep="")

path<-getwd()
path_result<-paste(path,"/plot/ufi/",name_result,sep="")
path_file_local<-paste(path,"/../data/ufi/",name_file_local,sep="")
path_file_vm<-paste(path,"/../data/ufi/",name_file_vm,sep="")

#          0       1  
#        default  compact
#colors=c("#FF3030","#009ACD") 
colors=c("#FF3030","black")

legends_vm=paste(workload,"_vm",sep="")
legends_local=paste(workload,"_local",sep="")

legneds=c(legends_local,legends_vm)
print("done !");

# reading data  
print(paste("reading ", path_file_local,",",path_file_vm,"..."));
data_frame_local<-read.table(path_file_local, sep=",", header=F)
data_frame_vm<-read.table(path_file_vm, sep=",", header=F)

width_max<-nrow(data_frame_vm)
height_max<-1.1

print(paste("width(addr max) : ", width_max))
print(paste("height(scan count) : ", height_max))
print("done !")

print(data_frame_vm)

# plotting ...
print(paste("plotting ",path_result,"..."))

#png(path_result, width=3400,height=height_max*10,unit="px")
png(path_result, width=1000,height=1000,unit="px")

par(mar=c(10,10,3,3)) # default : c(5.1, 4.1, 4.1, 2.1) / margin : down, left, up, right
par(mgp=c(7,1.2,0)) # default : c(3,1,0) / position : title, line label, line 

workload_size=28
free_size=2
xlab_local=paste(workload,"_local : execute ",workload,"(",workload_size,"G)"," in local ","(",free_size,"G free)",sep="");
xlab_vm=paste(workload,"_vm : execute ",workload,"(",workload_size,"G)"," in vm ","(",free_size,"G free)",sep="");
xlab_text=paste(xlab_vm,"\n",xlab_local)

plot(data_frame_local$V1,xlab=xlab_text,ylab="unusable free space index",xlim=c(0,width_max+1),ylim=c(0,height_max),pch=2,col=colors[1],type="o",xaxt='n',yaxt='n',cex.lab=2.5,xaxs='i',yaxs='i',lwd=1.5,cex=2) # xaxs : 'i':no margin, 'r':4% margin 

x_bar_raw<-seq(from=0,to=width_max,by=2)
y_bar_raw<-seq(from=0,to=height_max,by=0.1)

axis(1,at=x_bar_raw,labels=TRUE,cex.axis=1.5,tck=-0.01)
axis(2,at=y_bar_raw,labels=TRUE,cex.axis=1.5,tck=-0.01)

par(new=T)

plot(data_frame_vm$V1,xlab="",ylab="",xlim=c(0,width_max+1),ylim=c(0,height_max),pch=0,col=colors[2],type="o",xaxt='n',yaxt='n',lwd=1.5,cex=2,xaxs='i',yaxs='i') # xaxs : 'i':no margin, 'r':4% margin 

col_compact="#FF3030"
legend(x=38,y=1.08,legneds, cex=2, pch=c(2,0),col=colors)
abline(v=15,lty=2,col=col_compact)
text(22,1.05,labels=" compaction start",cex=2,col=col_compact)
print("done !")

