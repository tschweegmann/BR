all: 
	gcc sender_udp.c -lssl -lcrypto -o sender
	gcc receiver_udp.c -lssl -lcrypto -o receiver
	mkdir received
clean:
	rm receiver sender
	rm -r received
	rm dir.zip
