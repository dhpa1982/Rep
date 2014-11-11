import socket
import select
import sys
import os


ip = 'localhost'

# Create a UDP socket
s_udp = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

# Bind the socket to the port
serverAddress = (ip, 10000)
print 'starting up on %s port %s\n' % serverAddress
s_udp.bind(serverAddress)

sessionPortMap = dict()
tcpPort = 11000

while True:

    message, clientAddress = s_udp.recvfrom(4096)
    print 'recv message:',message
    cmd, data = message.split(' ', 1)

    if cmd =='create':

        if data in sessionPortMap:

            print 'session already running'
            s_udp.sendto('-1', clientAddress)

        else:

            tcpPort += 1
            # Create, bind and listen on a TCP socket
            tcpserverAddress  = (ip, tcpPort)
            s_tcp = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            s_tcp.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
            s_tcp.bind(tcpserverAddress)
            s_tcp.listen(10)

            sessionPortMap[data] = tcpPort


            pid = os.fork()

            if pid == 0:

                # child process
                sockList = list()
                sockList.append(s_tcp)

                while True:
                    rlist, wlist, elist = select.select(sockList, [], [])

                    for sock in rlist:

                        # Connection request from a new client
                        if sock == s_tcp:
                            sock, addr = s_tcp.accept()
                            print 'connection accepted'
                            sockList.append(sock)

                        # message from a joined client
                        else:
                            print 'data received'
                            message = sock.recv(4096)
                            for bsock in sockList:
                                if bsock != s_tcp and bsock != sock:
                                    bsock.send(message)

            elif pid > 0:

                #parent process
                s_udp.sendto(str(tcpPort), clientAddress)

            else:
                print 'Failed creating a new process'

    elif cmd == 'join':

        if data in sessionPortMap:
            message = sessionPortMap[data]
            s_udp.sendto(str(message), clientAddress)
        else:
            s_udp.sendto('-1', clientAddress)
    else:
        print 'unknown cmd'




