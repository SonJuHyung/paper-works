#!/usr/bin/env Rscript 
#
# huge page usage percent in fragmented vs non-fragmented environment
# host THP rate, guest THP rate
#

# parsing command line 
print("setting argument ...");
args=commandArgs(trailingOnly=TRUE)

if(length(args) != 4){
    stop("expr type missing !!")
}

workload<-args[1]
size<-args[2]
frag<-args[3]
version<-args[4]
name_result<-paste("compact_node_result_pageblock_scan_percentage_",workload,"_local_",size,"G_used_",frag,"_",version,sep="")
name_data<-paste("compact_node_result_pageblock_scan_percentage_",workload,"_local_",size,"G_used_",frag,"_",version,sep="")

name_png<-paste(name_result,".png",sep="")
name_file<-paste(name_data,".txt",sep="")

path<-getwd()
path_result<-paste(path,"/plot/compact_node/",name_png,sep="")
path_file<-paste(path,"/../data/compact_node/",name_file,sep="")

print("done !");

# reading data  
print(paste("reading ", path_file,"..."));
data_frame<-read.table(path_file, sep=",", header=F) 

# test data
#scanned_pb<-c(2288,1007,482,378,243,90,127,219,254,204,154,225,254,151,199,65,69,272,249,266,195,20,312,185,107,96,316,2298,52, 566)
#obtained_pb<-c(147 ,84 ,29 ,21 ,12 ,4 ,6 ,13 ,12 ,12 ,7 ,12 ,12 1,6 ,10 ,1 ,2 ,9 ,10 ,12 ,4 ,1 ,14 ,6 ,2 ,3 ,17 ,146 ,10 ,30)
#raw_pb<-c(2288,147,1007,84,482,29,378,21 ,243,12,90,4,127,6,219,13,254,12,204,12,154,7,225,12,254,12,151,6,199,10,65,1,69,2,272,9,249,10,266,12,195,4,20,1,312,14,185,6,107,2,96,3,316,17,2298,146,52,10,566,30)
raw_pb<-c(147,2141,
          84,923,
          29,453,
          21,357,
          12,231,
          4,86,
          6,121,
          13,206,
          12,242,
          12,192,
          7,147,
          12,113,
          12,142,
          6,145,
          10,189,
#          1,64,
#          2,67,
          9,283,
          10,239,
          12,254,
          4,191,
#          1,19,
          14,298,
          6,179,
          2,105,
          3,93,
          17,299,
          146,2152,
          10,42,
          30,536)

#matrix_pb<-as.matrix(data_frame,2,30)
matrix_pb<-matrix(raw_pb,2,30)
print(matrix_pb)
print("done !")

# plotting ...
print(paste("plotting ",path_result,"..."))

png(path_result, width=3000,height=1000,unit="px")

# par(mar=c(3,10,3,3)) # default : c(5.1, 4.1, 4.1, 2.1) / margin : down, left, up, right
# par(mgp=c(7,1.2,0)) # default : c(3,1,0) / position : title, line label, line
#plot(data_frame_nonfrag$V2,xlab="frag : execute VM2_redis(10G) after VM1_mongodb(20G) \n default : execute VM2_redis(10G)",ylab="latency(ms)",xlim=c(0,width_max),ylim=c(0,height_max),pch=2,col=colors[2],type="o",xaxt='n',yaxt='n',cex.lab=2.5,xaxs='i',yaxs='r',lwd=1.5,cex=2) # xaxs : 'i':no margin, 'r':4% margin 
#barplot(data_matrix, beside=FALSE,col=colors,ylab="huge page ratio(%)",names.arg=args,yaxt='n',cex.lab=2.5,cex.axis=2.5,cex.names=2.5, xlim=c(0,4),space=c(0,0.5),width=c(0.8,0.8))
bb<-barplot(matrix_pb,ylim=c(0,2600),cex.axis=4, xaxt='n',yaxt='n',space=c(0.5,0.5))
x_bar<-seq(from=0,to=2500,by=1000)
axis(1,at=x_bar,labels=TRUE,cex.axis=1.5,tck=-0.01)

legends=c(6.4,8.3,6.0,5.6,4.9,4.4,4.7,5.9,4.7,5.9,4.5,5.3,4.7,4.0,5.0,1.5,2.9,3.3,4.0,4.5,2.1,5.0,4.5,3.2,1.9,3.1,5.4,6.4,19.2, 5.3)

#text(bb,matrix_pb[1,]+50,labels=paste("(",legends,"%)",sep=""),cex=4)
text(bb,colSums(matrix_pb)+100,labels=paste("",legends,"%",sep=""),cex=3)
text(bb,colSums(matrix_pb)+220,labels=matrix_pb[1,]+matrix_pb[2,],cex=3)

# # x_bar_raw<-seq(from=0,to=width_max,by=unit)
# y_bar_raw<-seq(from=0,to=120,by=unit)
# # axis(1,at=x_bar_raw,labels=TRUE,cex.axis=1.5,tck=-0.01)
# axis(2,at=y_bar_raw,labels=TRUE,cex.axis=1.5,tck=-0.01)
# 
# legneds=c("guest huge page","host huge page")
# legend(x=2.5,y=118,legneds, cex=2, pch=c(16,16),col=colors)
# 
# 
# print("done !")

