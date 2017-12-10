default: sender_udp.c receiver_udp.c receiver_tcp.c sender_tcp.c
	gcc sender_udp.c -lssl -lcrypto -o sender_udp
	gcc receiver_udp.c -lssl -lcrypto -o receiver_udp
	gcc sender_tcp.c -lssl -lcrypto -o sender_tcp
	gcc receiver_tcp.c -lssl -lcrypto -o receiver_tcp
	mkdir received
clean:
	if [ -f sender_tcp ]; then rm sender_tcp; fi
	if [ -f receiver_tcp ]; then rm receiver_tcp; fi
	if [ -f sender_udp ]; then rm sender_udp; fi
	if [ -f receiver_udp ]; then rm receiver_udp; fi
	if [ -d received ]; then rm -rf received; fi
	if [ -f dir.zip ]; then rm -f dir.zip; fi 

