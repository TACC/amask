
.PHONY: all clean

all:
	cd src && $(MAKE)
clean:
	cd src && $(MAKE) clean
cleanall:
	cd src && $(MAKE) cleanall
