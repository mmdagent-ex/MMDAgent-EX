#
# a server script, sending a message to connected MMDAgent-EX client
#

import socket

server = ("127.0.0.1", 39392)
listen_num = 5

tcp_server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
tcp_server.bind(server)
tcp_server.listen(listen_num)

while True:
   client, address = tcp_server.accept()
   print("[*] connected: {}".format(address))
   client.send(b"MESSAGE|aaa|bbb")
   client.close()

