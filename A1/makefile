all: 
	gcc sender_udp.c -lssl -lcrypto -o sender
	gcc receiver_udp.c -lssl -lcrypto -o receiver
	mkdir received
clean:
	if [ -f sender ]; then rm sender; fi
	if [ -f receiver ]; then rm receiver; fi
	if [ -d received ]; then rm -rf received; fi
	if [ -f dir.zip ]; then rm -f dir.zip; fi 
