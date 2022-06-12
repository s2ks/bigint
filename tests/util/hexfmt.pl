#!/bin/perl

#Input a hexadecimal number and a size paramterer and output a c style array that this project's
#BIGINT implementation can interpret.

my $in = @ARGV[0];
my $size = @ARGV[1] or die "provide a size parameter";
$in =~ s/^0x//g;
$in = scalar reverse $in;

while(length($in) < $size * 2) {
	$in .= "0";
}

my @in = split("", $in);
my @out;

my $digit = "";

for my $x (@in) {
	$digit = $x . $digit;
	if(length($digit) == 2) {
		push(@out, "0x" . $digit);
		$digit = "";
	}
}

print "{" . join(", ", @out) . "}", "\n";
