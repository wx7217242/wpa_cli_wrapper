

PHONY:clean all

all:
	cd ../wpa_supplicant-2.6/wpa_supplicant/ \
	&& make -f Makefile.wrapper clean \
	&& make -f Makefile.wrapper \
	&& cp libwpa_cli_wrapper.a libwpa_cli_wrapper.h ../../wpa_cli_oper/ -f 
	gcc wpa_cli_oper.c smartlist.c bitarray.c compat.c libwpa_cli_wrapper.a


clean:
	rm -rf *.o *core
