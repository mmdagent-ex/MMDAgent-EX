#!/usr/bin/perl

use IO::Socket;

$sock = new IO::Socket::INET(
    PeerAddr => "localhost",
    PeerPort => 39392,
    Proto => "tcp");

die "IO::Socket: $!" unless $sock;

print "connected\n";

while (<$sock>) {
    print;
}

close($sock);
