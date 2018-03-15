#!/usr/bin/perl

$"="\t";
while(<>){
  chomp;
  my @line = split/\t/;
  $line[2] = int(rand(1073741822));
  $line[4] = int(rand(1073741822));
  print "@line\n";
}

