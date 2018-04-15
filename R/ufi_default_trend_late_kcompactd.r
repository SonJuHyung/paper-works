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

name_png<-paste("ufi_result_",workload,"_local_",left_size,"G_used_",frag,"_",version,"",option,sep="")
name_data<-paste("ufi_result_",workload,"_local_",left_size,"G_used_",frag,"_",version,"",option,sep="")

name_result<-paste(name_png,".png",sep="")
name_data<-paste(name_data,".txt",sep="")

path<-getwd()
path_result<-paste(path,"/plot/ufi/",name_result,sep="")
path_data<-paste(path,"/../data/ufi/",name_data,sep="")

#          0          1  
#       free page    ufi
#         black      red
colors=c("black","#FF3030")
legneds=c("unused free pages","unusable free space index")
print("done !");

# reading data  
print(paste("reading ", path_data,"..."));
data_frame<-read.table(path_data, sep=",", header=F)

GB_to_KB=1048576
y_freepage_max_real<-max(data_frame$V2)
y_freepage_max<-GB_to_KB*30
y_height_max<-1.1

print(paste("free page max : ", y_freepage_max))
print("done !")

# plotting ...
print(paste("plotting ",path_result,"..."))

#png(path_result, width=3400,height=height_max*10,unit="px")
png(path_result, width=1000,height=1000,unit="px")

par(mar=c(5,7,3,7)) # default : c(5.1, 4.1, 4.1, 2.1) / margin : down, left, up, right
par(mgp=c(3,1.2,0)) # default : c(3,1,0) / position : title, label index number, line

# free page plotting
plot(data_frame$V2,xlab="",ylab="",ylim=c(0,y_freepage_max),lty=2,pch=4,col=colors[1],type="b",axes=FALSE,xaxs='r',yaxs='i') # xaxs : 'i':no margin, 'r':4% margin 
mtext("unused free pages",side=2,line=5,cex=2,col=colors[1])
y_bar_raw<-seq(from=0,to=y_freepage_max,by=GB_to_KB*4)
y_bar<-paste(y_bar_raw/GB_to_KB,"GB",sep = " ")
axis(2,at=y_bar_raw,labels=y_bar,col=colors[1],cex.axis=1.5,tck=-0.01)

box()
par(new=TRUE)

# ufi plotting
plot(data_frame$V1,xlab="",ylab="",ylim=c(0,y_height_max),lty=1,pch=2,col=colors[2],type="b",axes=FALSE,xaxs='r',yaxs='i') # xaxs : 'i':no margin, 'r':4% margin 
mtext("unusable free space index",side=4,line=5,cex=2,col=colors[2])
axis(4,ylim=c(0,y_height_max),las=1,cex.axis=1.5,tck=-0.01,col.axis=colors[2],col=colors[2])

pre_successed=data_frame$V3[1]
cur_successed=0 
count=0 
i=1
lty_dc=4
col_dc="#FF3030"

# DC plotting
for( cur_successed in data_frame$V3  ){ 
    
    if(pre_successed != cur_successed){
        pre_successed=cur_successed 
        count=count+1
        abline(v=i,lty=lty_dc,col=col_dc)
        text=paste(count,"-th DC",sep="")
        text(i,1.05,labels=text,cex=1.5,col=col_dc)

#         if(count != 3)
#             text(i+7,1.05,labels=text,cex=1.5,col=col_dc)
#         else
#             text(i-7,1.05,labels=text,cex=1.5,col=col_dc)

#         if(count < 10){
#             if(count == 7 || count == 9)
#                 text(i-7,1.02,labels=text,cex=1.2,col=col_dc)
#             else
#                 text(i-7,1.05,labels=text,cex=1.2,col=col_dc)
#         }else{
#             text(i-9,1.05,labels=text,cex=1.2,col=col_dc)      
#         }

#         if(count < 10){
#             text(i-7,1.05,labels=text,cex=1.2,col=col_dc)
#         }else{
#             text(i-9,1.05,labels=text,cex=1.2,col=col_dc)      
#         }


    } 
    i=i+1
}
print(paste("compact count : ",count))

pre_try=data_frame$V4[1]
cur_try=0 
count=0 
i=1
lty_kd=6
col_kcompactd="Blue"
first=0
#col_kcompactd="Yellow"

# kcompactd wakeup plotting
for( cur_successed in data_frame$V4  ){ 
   
    if(cur_successed == -1){
        count=count+1
#        abline(v=i,lty=lty_kd,col=col_kcompactd)
        arrows(i,0.05,i,0,length=0.25,lty=6,lwd=2,col=col_kcompactd)

#         if(i < 10){
#             text=paste(count,"th-KD",sep="")
#             text(i+30,0.09,labels=text,cex=1.2,col=col_kcompactd)
#             text=paste("wakeup fail",sep="")
#             text(i+30,0.07,labels=text,cex=1.2,col=col_kcompactd)
# 
#         }else if(first == 0){
#             first=1
#             text=paste("2~6 th-KD",sep="")
#             text(i+20,0.09,labels=text,cex=1.2,col=col_kcompactd)
#             text=paste("wakeup fail",sep="")
#             text(i+20,0.07,labels=text,cex=1.2,col=col_kcompactd)
# 
#         }
    } 
    i=i+1
}


#legend(x=13,y=1.08,legneds, cex=2, pch=c(4,2),col=colors)

print("done !")

