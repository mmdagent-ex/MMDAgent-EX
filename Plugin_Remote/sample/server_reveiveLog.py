#
# a server script, receiving log from connected MMDAgent-EX client
#

import socket

server = ("127.0.0.1", 60001)
listen_num = 5

tcp_server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
tcp_server.bind(server)
tcp_server.listen(listen_num)

while True:
   client, address = tcp_server.accept()
   print("[*] connected: {}".format(address))
   while True:
      try:
         rcvmsg = client.recv(4096)
         print("[*] received: {}".format(rcvmsg))
      except Exception as e:
         print(e)
   client.close()

