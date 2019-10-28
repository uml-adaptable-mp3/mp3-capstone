#! /usr/bin/perl

while(<>) {
    chop if (/[\n\r]$/);
    $line = $_;

    if ($line =~ /^(.*)\/\/.*$/) {
	$line = $1;
    }

    if ($line =~ /^(.*)\/\*.*\*\/(.*)$/) {
	$line = $1 . $2;
    }

    if ($line =~ /^\#define\s+(\S+)\s+0x([fF][c-fC-F]\S\S)\s*$/) {
	printf("r %s %s\n", uc($2),$1);
    } elsif ($line =~ /^\#define\s+(\S+)_REG_BITS\s+(\d+)\s*$/) {
	printf("w %X %s\n", $2, $1);
    } elsif ($line =~ /^\#define\s+(\S+)_B\s+(\d+)\s*$/) {
	printf("b %X %s\n", $2, $1);
    }
}
