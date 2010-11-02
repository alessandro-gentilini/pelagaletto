deck10<-read.csv(file="log10.txt",sep=",",head=T)
summary(deck10)

deck20<-read.csv(file="log20.txt",sep=",",head=T)
summary(deck20)
h20<-hist(deck20$turned_cards,breaks=c(1:max(deck20$turned_cards)),plot=F)
plot(h20$density,log="x",type="b")
abline(a=0,b=0)