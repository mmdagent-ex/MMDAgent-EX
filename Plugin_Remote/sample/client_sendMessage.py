#
# a client script, sending a message to MMDAgent-EX running as server
#

import socket

server = ("127.0.0.1", 39392)
buffer_size = 4096

tcp_client = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
tcp_client.connect(server)
tcp_client.send(b"MESSAGE|aaa|bbb")
tcp_client.close()
