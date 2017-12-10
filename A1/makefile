default: sender_udp.c receiver_udp.c 
	gcc sender_udp.c -lssl -lcrypto -o sender_udp
	gcc receiver_udp.c -lssl -lcrypto -o receiver_udp
	mkdir received
clean:
	if [ -f sender_udp ]; then rm sender_udp; fi
	if [ -f receiver_udp ]; then rm receiver_udp; fi
	if [ -d received ]; then rm -rf received; fi
	if [ -f dir.zip ]; then rm -f dir.zip; fi 
