#!/usr/bin/env Rscript 
# 
# default : vm_mongo -> vm_redis 's UFI
# compact : vm_mongo -> vm_redis 's UFI but compact in 10s interval

# parsing command line 
print("setting argument ...");
args=commandArgs(trailingOnly=TRUE)

if(length(args) != 4){
    stop("expr type missing !!")
}

workload<-args[1]
scan_time<-args[2]
bitmap<-args[3]
version<-args[4]

name_png<-paste("refcount_result_",workload,"_",scan_time,"_",bitmap,"term_",version,sep="")
name_data<-paste("refcount_result_",workload,"_",scan_time,"_",bitmap,"term_",version,sep="")

name_result<-paste(name_png,".png",sep="")
name_file_data<-paste(name_data,".txt",sep="")

path<-getwd()
path_result<-paste(path,"/plot/refcount/",name_result,sep="")
path_file_data<-paste(path,"/../data/refcount/",name_file_data,sep="")

#          0       1  
#        default  compact
colors=c("black","#FF3030")
print("done !");

# reading data  
print(paste("reading ", path_file_data,"..."));
data_frame<-read.table(path_file_data, sep=",", header=F)

migrated_pfn_max<-length(data_frame$V3)
#migrated_pfn_max<-max(data_frame$V2)

print(paste("migrate pages : ", migrated_pfn_max))
print("done !")

width_max<-0
height_max<-0

npfn_0<-nrow(data_frame[data_frame$V3==0,])
npfn_1<-nrow(data_frame[data_frame$V3==1,])
npfn_2<-nrow(data_frame[data_frame$V3==2,])
npfn_3<-nrow(data_frame[data_frame$V3==3,])
npfn_4<-nrow(data_frame[data_frame$V3==4,])
npfn_5<-nrow(data_frame[data_frame$V3==5,])
npfn_6<-nrow(data_frame[data_frame$V3==6,])
npfn_7<-nrow(data_frame[data_frame$V3==7,])
npfn_8<-nrow(data_frame[data_frame$V3==8,])
npfn_9<-nrow(data_frame[data_frame$V3==9,])
npfn_10<-nrow(data_frame[data_frame$V3==10,])
npfn_11<-nrow(data_frame[data_frame$V3==11,])
npfn_12<-nrow(data_frame[data_frame$V3==12,])
npfn_13<-nrow(data_frame[data_frame$V3==13,])
npfn_14<-nrow(data_frame[data_frame$V3==14,])
npfn_15<-nrow(data_frame[data_frame$V3==15,])
npfn_16<-nrow(data_frame[data_frame$V3==16,])

print(paste("0 : ",npfn_0,"/",npfn_0*100/migrated_pfn_max))
print(paste("1 : ",npfn_1*100/migrated_pfn_max))
print(paste("2 : ",npfn_2*100/migrated_pfn_max))
print(paste("3 : ",npfn_3*100/migrated_pfn_max))
print(paste("4 : ",npfn_4*100/migrated_pfn_max))
print(paste("5 : ",npfn_5*100/migrated_pfn_max))
print(paste("6 : ",npfn_6*100/migrated_pfn_max))
print(paste("7 : ",npfn_7*100/migrated_pfn_max))
print(paste("8 : ",npfn_8*100/migrated_pfn_max))
print(paste("9 : ",npfn_9*100/migrated_pfn_max))
print(paste("10 : ",npfn_10*100/migrated_pfn_max))
print(paste("11 : ",npfn_11*100/migrated_pfn_max))
print(paste("12 : ",npfn_12*100/migrated_pfn_max))
print(paste("13 : ",npfn_13*100/migrated_pfn_max))
print(paste("14 : ",npfn_14*100/migrated_pfn_max))
print(paste("15 : ",npfn_15*100/migrated_pfn_max))
print(paste("16 : ",npfn_16*100/migrated_pfn_max))

# # plotting ...
# print(paste("plotting ",path_result,"..."))
# 
# png(path_result, width=1000,height=1000,unit="px")
# 
# par(mar=c(10,10,3,3)) # default : c(5.1, 4.1, 4.1, 2.1) / margin : down, left, up, right
# par(mgp=c(7,1.2,0)) # default : c(3,1,0) / position : title, line label, line 
# xlab_text="execute redis(15G) after mongodb(15G)"
# ylab_text="unusable free space index"
# 
# plot(data_frame_default$V2,xlab=xlab_text,ylab=ylab_text,xlim=c(0,width_max+1),ylim=c(0.3,height_max+0.1),pch=2,col=colors[1],type="o",xaxt='n',yaxt='n',cex.lab=2.5,xaxs='i',yaxs='i',lwd=1.5,cex=2) # xaxs : 'i':no margin, 'r':4% margin 
# 
# x_bar_raw<-seq(from=0,to=width_max,by=2)
# y_bar_raw<-seq(from=0,to=1,by=0.1)
# 
# axis(1,at=x_bar_raw,labels=TRUE,cex.axis=1.5,tck=-0.01)
# axis(2,at=y_bar_raw,labels=TRUE,cex.axis=1.5,tck=-0.01)
# 
# par(new=T)
# 
# plot(data_frame_compact$V3,xlab="",ylab="",xlim=c(0,width_max+1),ylim=c(0.3,height_max+0.1),pch=0,col=colors[2],type="o",xaxt='n',yaxt='n',lwd=1.5,cex=2,xaxs='i',yaxs='i') # xaxs : 'i':no margin, 'r':4% margin 
# 
# legend(x=20,y=1.18,legneds, cex=2, pch=c(2,0),col=colors)
# 
# print("done !")

