
.PHONY: all clean cleanall

all:
	cd src && $(MAKE)
clean:
	cd src && $(MAKE) clean
cleanall:
	cd src && $(MAKE) cleanall
install:
	cd src && $(MAKE) install
