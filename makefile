all: ds peer

ds: discoveryServer.o ds_gui_routines.o ds_udp_conn.o list_utility.o
	gcc -Wall discoveryServer.o ds_gui_routines.o ds_udp_conn.o list_utility.o -o ds -lpthread

discoveryServer.o: discoveryServer.c headers.h server/ds_headers.h
	gcc -Wall -c -std=gnu99 discoveryServer.c -lpthread

ds_gui_routines.o: server/ds_gui_routines.c headers.h server/ds_headers.h
	gcc -Wall -c -std=gnu99 server/ds_gui_routines.c

ds_udp_conn.o: server/ds_udp_conn.c headers.h server/ds_headers.h
	gcc -Wall -c -std=gnu99 server/ds_udp_conn.c

list_utility.o: server/list_utility.c headers.h server/ds_headers.h
	gcc -Wall -c -std=gnu99 server/list_utility.c

peer: peer.o peer_gui_routines.o peer_udp_conn.o peer_utilities.o peer_tcp_conn.o
	gcc -Wall peer.o peer_gui_routines.o peer_udp_conn.o peer_utilities.o peer_tcp_conn.o -o peer

peer.o: peer.c headers.h peer_code/peer_headers.h
	gcc -Wall -c -std=gnu99 peer.c

peer_gui_routines.o: peer_code/peer_gui_routines.c headers.h peer_code/peer_headers.h
	gcc -Wall -c -std=gnu99 peer_code/peer_gui_routines.c

peer_udp_conn.o: peer_code/peer_udp_conn.c headers.h peer_code/peer_headers.h
	gcc -Wall -c -std=gnu99 peer_code/peer_udp_conn.c

peer_utilities.o: peer_code/peer_utilities.c headers.h peer_code/peer_headers.h
	gcc -Wall -c -std=gnu99 peer_code/peer_utilities.c

peer_tcp_conn.o: peer_code/peer_tcp_conn.c headers.h peer_code/peer_headers.h
	gcc -Wall -c -std=gnu99 peer_code/peer_tcp_conn.c

clean:
	rm *o ds peer

