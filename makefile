TARGET=zmmlc

all:
	cd src; make; mv $(TARGET) ../; cd -
clean:
	cd src; make clean; cd -
	cd include; rm -f *~; cd -
	rm -f *~ $(TARGET)
