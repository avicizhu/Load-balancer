# Load-balancer
Load-balancer with least-connection algo


Progress record:

7.31

1. add weighted Least-connection algorithm

7.28

1. design 3 test cases and collecting simulation data
2. add servers with different performances case

7.21

1. change p2p bandwidth to 4800Mbps, TOR to Core link bandwidth to 100Mbps

7.20

1. add congestion window tracing , tracing file would be named as “lb-server-*.cwnd"
2. change TCP client to Bulksend mode, each client sends 655350 Bytes as fast as traffic allow
3. New Reno is selected as congestion control protocol

7.16

1. expand server number to 48 per server rack
2. optimised coding style, changed data structure for recording server status

7.14
1. TOR switch is added between servers and router

7.9

1. problem 7.8.2 is fixed by a work around 
2. problem 7.8.3 is fixed by chaining channel parameter between LB and client
3. trying to setup a TOR switch between servers and LB

7.8

1. LB would search a server with minimum connection number for new request, then add the connection information to established connection list
2. Server would notifies LB when connection is established, but there is a gap between the first SYN package been forwarded by LB and the server status been changed in LB, any new request arrivals in this gap would be forwarded based on old server status. Need to be fixed
3. Server cannot accept more than 3 connection simultaneously, need to be fixed

7.7

1. Creating an additional list to record established connection information , recording source IP/Port and associated server’s IP/Port, so that packets belong to established connection will not be influenced by server’s status
2. Need to delete the connection recored once a connection is finished

7.6 

1. creating an additional socket on each server at the beginning of the simulation, sending connection number to the load balancer in UDP packet, this socket is never closed
2. Receive UDP packet and record the connection number on the load balancer 

6.29

1. Replace UDP client&server with TCP in simulation, since TCP is connection-oriented 
2. Retrieve connection numbers on each server by accessing its socket list, each socket is a connection 
