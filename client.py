import socket
import string
import select
import sys

ip = 'localhost'

# Create a UDP socket
s_udp = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

server_address = (ip, 10000)
joinSessList = dict()
tcpSock = list()


while True:

    print '------------ CHAT MENU ------------'
    print "\n1. Create a chat session\n2. Join a chat session\n3. Submit a message\n4. Leave the chat session"
    print "5. Exit\n\nEnter your selection: "

    sel = '0'

    if len(joinSessList.values()):
        tcpSock = joinSessList.values()

    tcpSock.append(sys.stdin)

    readL, _, _ = select.select(tcpSock, [], [])

    tcpSock.remove(sys.stdin)

    for item in readL:

        if item in tcpSock:
            message = item.recv(4096)
            print joinSessList.keys()[joinSessList.values().index(item)],':',message

        else:
            sel = sys.stdin.readline()
            sel = sel[0]

    readL = []

    if sel == '1':

        print "Enter the name for the chat session(max 8 characters):"
        name = raw_input()
        if name in joinSessList:
            print 'session',name, 'already exist'
            continue

        name = 'create ' + name
        sent = s_udp.sendto(name, server_address)
        message, address = s_udp.recvfrom(4096)

        if message == '-1':
            print "Session already exist"
            continue

        s_tcp = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

        # connect to remote host
        s_tcp.connect((ip, int(message)))

        joinSessList[name.split(' ',1)[1]] = s_tcp

        print 'A new chat session',name.split(' ',1)[1],'has been created and you have joined the session'

    elif sel == '2':

        name = raw_input("Enter the name of the chat session to join(max 8 characters):")
        if name in joinSessList:
            print 'Already joined the session',name
            continue

        name = 'join ' + name
        sent = s_udp.sendto(name, server_address)
        message, address = s_udp.recvfrom(4096)

        if message == '-1':
            print name.split(' ',1)[1], 'doesn\'t exist'
            continue

        s_tcp = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

        # connect to remote host
        s_tcp.connect((ip, int(message)))

        joinSessList[name.split(' ',1)[1]] = s_tcp

        print 'Joined the session',name.split(' ',1)[1]

    elif sel == '3':
        print 'Select the session to which you want to submit the message'
        print joinSessList.keys()
        sess = raw_input('selection:')
        sock = joinSessList[sess]
        message = raw_input('Enter the message you want to submit:')
        sock.send(message)

    elif sel == '4':
        print 'Select the session you want to leave'
        print joinSessList.keys()
        sess = raw_input('selection:')
        sock = joinSessList[sess]

    elif sel == '5':
        for value in joinSessList:
            value.close()

    print '\n'
