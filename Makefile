apps = service_manager bctest_client bctest_server

all : $(apps)

service_manager : service_manager.o binder.o
	arm-linux-gcc -o $@ $^

bctest_client : bctest_client.o binder.o
	arm-linux-gcc -o $@ $^

bctest_server : bctest_server.o binder.o
	arm-linux-gcc -o $@ $^

%.o : %.c
	arm-linux-gcc -DBINDER_IPC_32BIT=1 -I ./include -c -o $@ $<

clean :
	rm -rf *.o $(apps)
