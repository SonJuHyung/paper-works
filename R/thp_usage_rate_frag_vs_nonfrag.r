#!/usr/bin/env Rscript 
#
# huge page usage percent in fragmented vs non-fragmented environment
# host THP rate, guest THP rate
#

# parsing command line 
print("setting argument ...");
args=commandArgs(trailingOnly=TRUE)

if(length(args) != 2){
    stop("expr type missing !!")
}

workload<-args[1]
version<-args[2]
name_png<-paste("hpage_usage_result_",workload,"_",version,sep="")
name_result<-paste(name_png,".png",sep="")
path<-getwd()
path_result<-paste(path,"/plot/latency/",name_result,sep="")

#               1                    2  
#              frag                nonfrag 
#          host      guest      host      guest
#          red      lightred    blue    lightblue
#colors=c("#FF3030","#FF8282","#009ACD","#7FCCE6")
#colors=c("#009ACD","#7FCCE6","#FF3030","#FF8282")
colors=c("black","lightgrey")

legneds=c("fragmented","non-fragmented")

print("done !");

# reading data  
print(paste("setting data ..."));

nonfrag_host=99.2
nonfrag_guest=99.99
frag_host=65.7
frag_guest=99.9

guest <- c(nonfrag_guest, frag_guest)
host <- c(nonfrag_host, frag_host)
hpage_usage <- rbind(guest, host)
args<-c("non-fragmented","fragmented")

print("done !")

# plotting ...
print(paste("plotting ",path_result,"..."))

#png(path_result, width=3400,height=height_max*10,unit="px")
png(path_result, width=1000,height=1000,unit="px")

unit<-20
height_max<-100

par(mar=c(3,10,3,3)) # default : c(5.1, 4.1, 4.1, 2.1) / margin : down, left, up, right
par(mgp=c(7,1.2,0)) # default : c(3,1,0) / position : title, line label, line
#plot(data_frame_nonfrag$V2,xlab="frag : execute VM2_redis(10G) after VM1_mongodb(20G) \n default : execute VM2_redis(10G)",ylab="latency(ms)",xlim=c(0,width_max),ylim=c(0,height_max),pch=2,col=colors[2],type="o",xaxt='n',yaxt='n',cex.lab=2.5,xaxs='i',yaxs='r',lwd=1.5,cex=2) # xaxs : 'i':no margin, 'r':4% margin 
barplot(hpage_usage, beside=TRUE, ylim=c(0,120),col=colors,ylab="huge page ratio(%)",names.arg=args,yaxt='n',cex.lab=2.5,cex.axis=2.5,cex.names=2.5, xlim=c(0,4),space=c(0,0.5),width=c(0.8,0.8))

# x_bar_raw<-seq(from=0,to=width_max,by=unit)
y_bar_raw<-seq(from=0,to=120,by=unit)
# axis(1,at=x_bar_raw,labels=TRUE,cex.axis=1.5,tck=-0.01)
axis(2,at=y_bar_raw,labels=TRUE,cex.axis=1.5,tck=-0.01)

legneds=c("guest huge page","host huge page")
legend(x=2.5,y=118,legneds, cex=2, pch=c(16,16),col=colors)


print("done !")

