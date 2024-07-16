ifdef  PREFIX
TOPDIR=$(PREFIX)
else
TOPDIR=$(PWD)
endif

.PHONY: all clean cleanall

all:
	cd src && $(MAKE)
clean:
	cd src && $(MAKE) clean
cleanall:
	cd src && $(MAKE) cleanall
install:
	cd src && $(MAKE) install
	@echo -e "       Top level install directory is $(TOPDIR)"
	@echo -e "\n -> ** Include $(TOPDIR)/bin in PATH variable."
	@echo -e "\n -> ** Include $(TOPDIR)/lib in LD_LIBRARY_PATH,"
	@echo -e "               only if you instrument your code to use AMASK APIs.\n"
