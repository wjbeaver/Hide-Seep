prefix=/usr
BINDIR=$(prefix)/bin

all : imgcnv


install:
	install imgcnv $(DESTDIR)$(BINDIR)


imgcnv: 
	-mkdir -p generated/obj
	sh build-imgcnv-linux.sh
	#(cd src; qmake imgcnv.pro)
	#(cd src; make)


clean:
	rm -rf generated *.o *~

realclean: clean
	rm -f imgcnv



.FORCE: imgcnv
