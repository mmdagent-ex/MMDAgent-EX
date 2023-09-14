#!/usr/bin/perl

use Cwd 'abs_path';
use Encode;

if ($#ARGV != 1) {
    print "usage: $0 model_alias pmdfile\n";
    exit;
}

$alias = $ARGV[0];
$pmdfile = abs_path($ARGV[1]);
$message = "MODEL_ADD|" . $alias . "|" . $pmdfile;
$message = encode('shiftjis', decode('utf8', $message));

use IO::Socket;
$sock = new IO::Socket::INET(
    PeerAddr => "localhost",
    PeerPort => 39392,
    Proto => "tcp");
die "IO::Socket: $!" unless $sock;
print $sock $message . "\n";
close($sock);
