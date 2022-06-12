#!/bin/perl
#Input a c style array in the form of: {0xa, 0xb, 0xc, 0xd} and output the hexadecimal value
#that this project's BIGINT implementation interprets it as.

my $in = $ARGV[0];
$in =~ s/[,{}]|0x//g;

my @in = reverse split(" ", $in);

print "0x", join("", @in), "\n";
