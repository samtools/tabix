#!/usr/bin/perl

while(<>){

  @x=(split/\t/)[0,2,3,6,7,1]; # get the columns
  
  # if the two chromosomes are identical, bam represents the second mate as '='. Replace this with the actual chromosome name.
  if($x[3] eq "="){ $x[3]=$x[1]; }
  
  # extract strand information from the flag column
  if($x[5]&16){ $s1="-"; } else { $s1="+"; }
  if($x[5]&32){ $s2="-"; } else { $s2="+"; }
  
  pop @x; #remove the flag column
  
  $"="\t";
  if($x[1] lt $x[3] || ($x[1] eq $x[3] && $x[2]<$x[4]) ){  ## upper triangle filtering
    print "@x\t$s1\t$s2\n";  
  }

}
