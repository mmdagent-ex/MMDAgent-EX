#
# a client script, receiving log from MMDAgent-EX running as server
#

import socket

server = ("127.0.0.1", 39392)
tcp_client = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
tcp_client.connect(server)
while True:
   try:
      rcvmsg = tcp_client.recv(4096)
      print("[*] received: {}".format(rcvmsg))
   except Exception as e:
      print(e)
tcp_client.close()

