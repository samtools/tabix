#!/usr/bin/perl

$nlines = shift @ARGV;
$i=0;
$chr=1;
$chr2=1;
$pos=int(rand(1000));
$pos2=int(rand(1000));
$nlines2 = sqrt($nlines);
$MAXCHR=25;
$MAXPOS=536000000;
while($i<$nlines){
  print "lalala\tchr$chr\t$pos\tchr$chr2\t$pos2\t+\t+\n";
  if(rand($nlines/10)<=1 && $chr<$MAXCHR){
     $chr++;
     $chr2=$chr;
     $pos=int(rand(1000));
     $pos2=int(rand(1000));
  }
  if(rand($nlines/50)<=1 && $chr2<$MAXCHR){
     $chr2++;
     $pos2=int(rand(1000));
  }
  $pos+=int(rand(3)) if $pos<$MAXPOS;
  $pos2+=int(rand(3)) if $pos2<$MAXPOS;
  $i++;
}
