#!/usr/bin/perl
# old merged_nodups.txt to 4dn-pairs converter 
if(@ARGV<0) { print "usage: $0 merged_nodups.txt outprefix\nRequires sort and bgzip\n"; exit; }
$infile = shift @ARGV;
$outfile_prefix = shift @ARGV;

# writing to text pairs
open OUT, ">$outfile_prefix.pairs" or die "Can't write to $outfile\n";
print OUT "## pairs format v1.0\n";
print OUT "#sorted: chr1-chr2-pos1-pos2\n";
print OUT "#shape: upper triangle\n";
print OUT "#columns: readID chr1 pos1 chr2 pos2 strand1 strand2 frag1 frag2\n";
open IN, "$infile" or die "Can't open $infile\n";
while(<IN>){
  chomp;
  my ($ri,$c1,$p1,$c2,$p2,$s1,$s2,$f1,$f2) = (split/\s/)[0,2,3,6,7,1,5,4,8];
  $s1 = $s1==0?'+':'-';
  $s2 = $s2==0?'+':'-';
  if($c1 gt $c2 || ($c1 eq $c2 && $p1>$p2)) { # flip
    print OUT "$ri\t$c2\t$p2\t$c1\t$p1\t$s2\t$s1\t$f2\t$f1\n";
  } else {
    print OUT "$ri\t$c1\t$p1\t$c2\t$p2\t$s1\t$s2\t$f1\t$f2\n";
  }
}

close IN;
close OUT;

# sorting
system("grep '^#' $outfile_prefix.pairs > $outfile_prefix.bsorted.pairs; grep -v '^#' $outfile_prefix.pairs | sort -k2,2 -k4,4 -k3,3g -k5,5g >> $outfile_prefix.bsorted.pairs");

# bgzipping
system("bgzip -f $outfile_prefix.bsorted.pairs");

# pairix indexing
system("pairix -s2 -b3 -e3 -d4 -u5 -f $outfile_prefix.bsorted.pairs.gz");

# removing text file
# system("rm -f $outfile_prefix.pairs");


