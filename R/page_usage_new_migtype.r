#!/usr/bin/env Rscript 

# parsing command line 
print("setting argument ...");
args=commandArgs(trailingOnly=TRUE)

if(length(args) != 2){
    stop("expr type missing !!")
}

workload<-args[1]
version<-args[2]
name<-paste("pbstate_result_",workload,"_",version,sep="")
name_file<-paste(name,".txt",sep="")
name_result<-paste(name,".png",sep="")

path<-getwd()
path_file<-paste(path,"/../data/pbusage/",name_file,sep="")
path_result<-paste(path,"/plot/pbusage/",name_result,sep="")

#           0      1      2       3         4       5        6     7
#         free    unmov    mov    recm     hato    iso      inv   compact
#colors=c("White","Green","Green","Green","White","white","Gray", "Red")
#colors=c("White","#FF3030","#00AA8D","#FFC107","White","white","Gray","#3F51B5")
colors=c("White","Red","Green","Yellow","White","white","Gray","Blue")

print("done !");

# reading data  
print(paste("reading ", path_file,"..."));
data_frame<-read.table(path_file, sep=",", header=F)
width_max<-max(data_frame$V1)+512
height_max<-nrow(data_frame[data_frame$V1==0,])

print(paste("width(addr max) : ", width_max))
print(paste("height(scan count) : ", height_max))
print("done !")

# plotting ...
print(paste("plotting ",path_result,"..."))
# png_height= round(3400*height_max/150)
# if(png_height < 200)
#     png_height = 200 

#png(path_result, width=6000,height=height_max*10,unit="px")
png(path_result, width=6000,height=600,unit="px")
#png(path_result, width=3400*2,height=500*2,unit="px")
#png(path_result, width=3400,height=300,unit="px")


par(mar=c(10,5,3,3)) # default : c(5.1, 4.1, 4.1, 2.1) / margin : down, left, up, right
par(mgp=c(6,2,0)) # default : c(3,1,0) / position : title, line label, line
plot(1,type="n",xlab="physical page block status ",ylab="",xlim=c(1,width_max+2000),ylim=c(1,height_max+1), xaxt='n',yaxt='n',cex.lab=4,xaxs='i')
KB_to_GB=1048576
x_bar_raw<-seq(from=0,to=width_max,by=KB_to_GB)
x_bar<-paste(x_bar_raw/(KB_to_GB)*4,"GB",sep = " ")
#y_bar<-seq(from=0,to=height_max,1)

axis(1,at=x_bar_raw,labels=x_bar,cex.axis=2,tck=-0.01)
#axis(2,at=y_bar,labels=TRUE,cex.axis=2,tck=-0.01)

color_index=0
x_pos=0 
y_pos=0

free=umov=mov=reclm=high=iso=inv=0

pre_x=0
cur_x=0
pre_state=-1
cur_state=-1
j=0
index=1

count=0 

point_cex=0.5
point_pch=20

for( i in data_frame$V1  ){ 
    if(i == 0){
        print(paste(count," th try is done. ",sep=""))
        count=count+1
        cur_x=0
#        i=1
        y_pos=y_pos+1 
    }

    cur_x=i
    cur_state=data_frame$V2[index]

    if(pre_x == (width_max-512)){
#        if(pre_state == 1 || pre_state == 2 || pre_state == 3){
        if(pre_state == 1 || pre_state == 2 || pre_state == 3 || pre_state == 7){
            for(j in (pre_x):(pre_x+512)){
                #            print(paste(j+1,",",y_pos-1,",",pre_state+1,",son",sep=""))            
                points(j+1,y_pos-1,pch=point_pch,cex=point_cex,col=colors[pre_state+1])  
            }
        }
        pre_x=0
    }

    if(pre_x != cur_x){
#        if(pre_state == 1 || pre_state == 2 || pre_state == 3){
        if(pre_state == 1 || pre_state == 2 || pre_state == 3 || pre_state == 7){
            for(j in (pre_x):(cur_x-1)){
                #            print(paste(j+1,",",y_pos,",",pre_state+1,",son",sep=""))
                points(j+1,y_pos,pch=point_pch,cex=point_cex,col=colors[pre_state+1])  
            }
        }
    }

    index=index+1 
    pre_state=cur_state
    pre_x=cur_x
}

if(pre_x == (width_max-512)){
#    if(pre_state == 1 || pre_state == 2 || pre_state == 3){
    if(pre_state == 1 || pre_state == 2 || pre_state == 3 || pre_state == 7){
        for(j in (pre_x):(pre_x+512)){        
            #        print(paste(j+1,",",y_pos,",",pre_state+1,",son",sep=""))
            points(j+1,y_pos,pch=point_pch,cex=point_cex,col=colors[pre_state+1])  
        }
    }
    pre_x=0
}

print("done !")

