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
path_file<-paste(path,"/../data/pbusage_new/",name_file,sep="")
path_result<-paste(path,"/plot/pbusage_new/",name_result,sep="")

#           0      1      2       3         4       5        6     7
#         free    unmov    mov    recm     hato    iso      inv   compact
#colors=c("White","Green","Green","Green","White","white","Gray", "Red")
#colors=c("White","#FF3030","#00AA8D","#FFC107","White","white","Gray","#3F51B5")

#         1,2     3,4,5,6   7,8,9    10
#colors=c("Blue","Yellow","Green", "Red")
colors=c("Blue","Green","Yellow", "Red")

print("done !");

# reading data  
print(paste("reading ", path_file,"..."));
data_frame<-read.table(path_file, sep=",", header=F)

width_max<-max(data_frame$V2)
height_max<-nrow(data_frame[data_frame$V2==1,])

print(paste("width(page block max) : ", width_max))
print(paste("height(scan count) : ", height_max))
print("done !")

# plotting ...
print(paste("plotting ",path_result,"..."))
png(path_result, width=6000,height=600,unit="px")

par(mar=c(10,5,3,3)) # default : c(5.1, 4.1, 4.1, 2.1) / margin : down, left, up, right
par(mgp=c(6,2,0)) # default : c(3,1,0) / position : title, line label, line 

xlab_text = "physical pge block usage status" 

mul=1
mul_sub=0

plot(1,type="n",xlab=xlab_text,ylab="",xlim=c(1,width_max*mul),ylim=c(1,height_max), xaxt='n',yaxt='n',cex.lab=4,xaxs='i',yaxs='r')

MB_to_GB=512
x_bar_raw<-seq(from=0,to=width_max,by=MB_to_GB)
x_bar<-paste(x_bar_raw/MB_to_GB,"GB",sep = " ")
axis(1,at=x_bar_raw,labels=x_bar,cex.axis=2,tck=-0.01)


x_pos=0
y_pos=0
index=1
color_index_raw=0
color_index=0


for( i in data_frame$V2  ){ 

    if(i == 1){
        y_pos=y_pos+1 
    }

    x_pos=i
    color_index_raw=data_frame$V4[index]

    if(color_index_raw != 0){
        if(color_index_raw == 10){
            print("Red")
            color_index=4 
            points(x_pos*mul-mul_sub,y_pos,pch=1,cex=0.1,col=colors[color_index])  
        }
    }else{
        print("Zero")
    }

    index=index+1 
}

x_pos=0
y_pos=0
index=1
color_index_raw=0
color_index=0


for( i in data_frame$V2  ){ 

    if(i == 1){
        y_pos=y_pos+1 
    }

    x_pos=i
    color_index_raw=data_frame$V4[index]

    if(color_index_raw != 0){
        if(7<= color_index_raw && color_index_raw <= 9){
             print("Yellow")
            color_index=3          
            points(x_pos*mul-mul_sub,y_pos,pch=1,cex=0.5,col=colors[color_index])  
        }
    }

    index=index+1 
}

x_pos=0
y_pos=0
index=1
color_index_raw=0
color_index=0


for( i in data_frame$V2  ){ 

    if(i == 1){
        y_pos=y_pos+1 
    }

    x_pos=i
    color_index_raw=data_frame$V4[index]

    if(color_index_raw != 0){
        if(4<= color_index_raw && color_index_raw <= 6){
            print("Green")
            color_index=2
            points(x_pos*mul-mul_sub,y_pos,pch=1,cex=1,col=colors[color_index])  
        }
    }

    index=index+1 
}

x_pos=0
y_pos=0
index=1
color_index_raw=0
color_index=0

for( i in data_frame$V2  ){ 

    if(i == 1){
        y_pos=y_pos+1 
    }

    x_pos=i
    color_index_raw=data_frame$V4[index]

    if(color_index_raw != 0){
        if(1<=color_index_raw && color_index_raw <= 3){
            print("Blue")
            color_index_raw=1
            points(x_pos*mul-mul_sub,y_pos,pch=16,cex=1.5,col=colors[color_index])  
        }
    }

    index=index+1 
}

print("done !")

