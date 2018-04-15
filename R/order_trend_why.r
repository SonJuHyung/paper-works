#!/usr/bin/env Rscript 
# 
# default : vm_mongo -> vm_redis 's UFI
# compact : vm_mongo -> vm_redis 's UFI but compact in 10s interval

# parsing command line 
args=commandArgs(trailingOnly=TRUE)

if(length(args) < 4){

    print(length(args))
    stop("expr type missing !!")   
    
}
print("setting argument ...");

workload<-args[1]
left_size<-args[2]
frag<-args[3]
version<-args[4]
if(length(args) == 5){
    option<-paste("_",args[5],sep="")
}else{
    option=""
}

name_png<-paste("order_result_",workload,"_local_",left_size,"G_used_",frag,"_",version,"",option,sep="")
name_data<-paste("order_result_",workload,"_local_",left_size,"G_used_",frag,"_",version,sep="")

name_result<-paste(name_png,".png",sep="")
name_data<-paste(name_data,".txt",sep="")

path<-getwd()
path_result<-paste(path,"/plot/order_trend/",name_result,sep="")
path_data<-paste(path,"/../data/order_trend/",name_data,sep="")

#          0          1  
#       free page    ufi
#         black      red
colors=c("black","#FF3030")
legneds=c("allocated page order(__aloc_pages_nodemask)","unusable free space index")
print("done !");

# reading data  
print(paste("reading ", path_data,"..."));
data_frame<-read.table(path_data, sep=",", header=F)

# plotting ...
print(paste("plotting ",path_result,"..."))

#png(path_result, width=3400,height=height_max*10,unit="px")
png(path_result, width=1000,height=1000,unit="px")

par(mar=c(5,7,3,7)) # default : c(5.1, 4.1, 4.1, 2.1) / margin : down, left, up, right
par(mgp=c(3,1.2,0)) # default : c(3,1,0) / position : title, label index number, line

# order plotting
plot(data_frame$V2,xlab="",ylab="",ylim=c(0,11),pch=4,col=colors[1],type="p",axes=FALSE,xaxs='i',yaxs='r',cex=0.7) # xaxs : 'i':no margin, 'r':4% margin 
mtext("allocated page order(__aloc_pages_nodemask)",side=2,line=5,cex=2,col=colors[1])
y_bar_raw<-seq(from=0,to=11,by=1)
axis(2,at=y_bar_raw,labels=TRUE,col=colors[1],cex.axis=1.5,tck=-0.01)

box()
par(new=TRUE)

# ufi plotting
plot(data_frame$V4,xlab="",ylab="",ylim=c(0,1.1),lty=1,col=colors[2],type="l",axes=FALSE,xaxs='i',yaxs='r',cex=0.7) # xaxs : 'i':no margin, 'r':4% margin 
mtext("unusable free space index",side=4,line=5,cex=2,col=colors[2])
axis(4,ylim=c(0,1.1),las=1,cex.axis=1.5,tck=-0.01,col.axis=colors[2],col=colors[2])

print("done !")

