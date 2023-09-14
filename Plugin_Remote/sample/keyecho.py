#
# Sample script, enable echoing what key you have pushed in screen
#

import socket
import re

# return first message to be issued at connection establishment
def connectedMessage():
   return "TEXTAREA_ADD|num|6,2|2,0.3,0,1|0,0,0,0.7|1,1,1,1|0,6,-4"

# process incoming message and return message to be sent to MMDAgent-EX
def process(msg):
   matched = re.match('.*KEY\|(.*)$', msg)
   if (matched):
      return "TEXTAREA_SET|num|{}".format(matched.group(1))

# main
server = ("127.0.0.1", 39392)
tcp_client = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
tcp_client.connect(server)
tcp_client.send(connectedMessage().encode())
while True:
   try:
      rcvmsg = tcp_client.recv(4096)
      print("[*] received: {}".format(rcvmsg))
      ret = process(rcvmsg.decode().rstrip())
      if (ret):
         tcp_client.send(ret.encode())
   except Exception as e:
      print(e)
tcp_client.close()
