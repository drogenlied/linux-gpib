MOD=gpib

#GPIBINCLUDES =	-I/opt/nigpib-0.8.3
GPIBINCLUDES =	-I/usr/local/include/sys
#GPIBLIB      =  /opt/nigpib-0.8.3/libgpib.a
GPIBLIB      =  /usr/local/lib/libgpib.a
DRIVER       =  USE_NI_GPIB
EXTENSIONDIR =  /usr/lib/php4/20010901



all : $(MOD).so

$(MOD).so: $(MOD).o 
	gcc -shared -L/usr/local/lib -rdynamic -o $(MOD).so \
	$(MOD).o $(GPIBLIB)

$(MOD).o: $(MOD).c
	gcc -D$(DRIVER) -Wall -fpic -DCOMPILE_DL=1 \
	$(GPIBINCLUDES) \
	-I/usr/local/include -I. \
	-I/usr/include/php4 \
	-I//usr/include/php4/main \
	-I//usr/include/php4/Zend \
	-I//usr/include/php4/TSRM \
	-c $(MOD).c

install: $(MOD).so
	cp $(MOD).so $(EXTENSIONDIR)


clean:
	rm -f *~ *.so *.o

