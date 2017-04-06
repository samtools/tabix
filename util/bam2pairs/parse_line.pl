#!/usr/bin/perl
use Getopt::Long;
my $pos_is_5end=0;
&GetOptions( '5|5end' => sub { $pos_is_5end=1 } );

if(@ARGV<1){ &print_usage(); exit(); }
$input = shift @ARGV;
open IN, "$input" or die "Can't open input";
while(<>){

  next if /^@/; # skip header in case it exists
 
  @x=(split/\t/)[0,2,3,6,7,1]; # get the columns
  @seq=(split/\t/)[9,10]; # sequences
  
  # if the two chromosomes are identical, bam represents the second mate as '='. Replace this with the actual chromosome name.
  if($x[3] eq "="){ $x[3]=$x[1]; }
  
  # extract strand information from the flag column
  if($x[5]&16){ $s1="-"; $x[2]+=$pos_is_5end?length($seq[0])-1:0; } else { $s1="+"; }
  if($x[5]&32){ $s2="-"; $x[4]+=$pos_is_5end?length($seq[1])-1:0; } else { $s2="+"; }
  
  pop @x; #remove the flag column
  
  $"="\t";
  if($x[1] lt $x[3] || ($x[1] eq $x[3] && $x[2]<$x[4]) ){  ## upper triangle filtering
    print "@x\t$s1\t$s2\n";  
  }

}
close IN;

sub print_usage {
  print "usage: $0 [-5] <input_sam> > <out_pairs>\n";
  print "  -5 : position is 5-end, not left-most position (default left-most).\n";
}

