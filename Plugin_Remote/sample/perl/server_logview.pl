#!/usr/bin/perl

use IO::Socket;
use IO::Select;

$sock = new IO::Socket::INET(
    LocalPort => 39392,
    Proto => "tcp",
    Listen => 1,
    ReuseAddr => 1);

die "IO::Socket: $!" unless $sock;

$selector = new IO::Select($sock);

die "IO::Select: $!" unless $selector;

print STDERR "ready\n";

while (@ready = $selector->can_read) {
    foreach $fh (@ready) {
	if ($fh == $sock) {
	    $new = $sock->accept;
	    $selector->add($new);
	    print STDERR "client connected\n";
	} else {
	    $input = <$fh>;
	    if (defined($input)) {
		&process($input);
	    } else {
		$selector->remove($fh);
		$fh->close;
		print STDERR "client disconnected\n";
	    }
	}
    }
}
close($sock);

sub process {
    my ($instr) = @_;
    my ($outstr);
    my ($f);

    #### input
    print $instr;

    #### some processing

    # $outstr = "<<" . $instr . ">>";

    #### send outstr to clients
    #### (be careful to avoid loop, output will echo back as input again!)
    
    #foreach $f ($selector->handles) {
    #	print $f $outstr;
    #}
}
