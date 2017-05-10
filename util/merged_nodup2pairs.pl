#!/usr/bin/perl
# merged_nodups.txt to 4dn-pairs converter 
my $split_sort = 0;

use Getopt::Long;
&GetOptions( 's|split_sort=s' => $split_sort );

if(@ARGV<1) { print "usage: $0 [-s|--split_sort <nsplit>] merged_nodups.txt chrom_size outprefix\nRequires sort and bgzip\n"; exit; }
$infile = shift @ARGV;
$chromsizefile = shift @ARGV;
$outfile_prefix = shift @ARGV;

# chromsize file
$k=0;
open CHROMSIZE, $chromsizefile or die "Can't open chromsizefile $chromsizefile";
while(<CHROMSIZE>){
  chomp;
  my ($chr,$size) = split/\s+/;
  $chromsize{$chr}=$size;
  $chromorder{$chr}=$k++;
}
close CHROMSIZE;


# writing to text pairs
open OUT, ">$outfile_prefix.pairs" or die "Can't write to $outfile\n";
print OUT "## pairs format v1.0\n";
print OUT "#sorted: chr1-chr2-pos1-pos2\n";
print OUT "#shape: upper triangle\n";

for my $chr (sort {$chromorder{$a} <=> $chromorder{$b}} keys %chromorder){
  print OUT "#chromsize: $chr $chromsize{$chr}\n";
}

$command_options = $split_sort>0?'-s $split_sort':'';
print OUT "#command: merged_nodup2pairs.pl $command_options @ARGV\n"; 
print OUT "#columns: readID chr1 pos1 chr2 pos2 strand1 strand2 frag1 frag2\n";

my $n=0; # line count
open IN, "$infile" or die "Can't open $infile\n";
while(<IN>){
  chomp;
  my ($ri,$c1,$p1,$c2,$p2,$s1,$s2,$f1,$f2) = (split/\s/)[14,1,2,5,6,0,4,3,7];
  $s1 = $s1==0?'+':'-';
  $s2 = $s2==0?'+':'-';
  if($chromorder{$c1} > $chromorder{$c2} || ($chromorder{$c1} == $chromorder{$c2} && $p1>$p2)) { # flip
    print OUT "$ri\t$c2\t$p2\t$c1\t$p1\t$s2\t$s1\t$f2\t$f1\n";
  } else {
    print OUT "$ri\t$c1\t$p1\t$c2\t$p2\t$s1\t$s2\t$f1\t$f2\n";
  }
  $n++;
}

close IN;
close OUT;

# sorting
system("grep '^#' $outfile_prefix.pairs > $outfile_prefix.bsorted.pairs");

if($split_sort>1) {
  my $L = int($n/$split_sort)+1;
  system("grep -v '^#' $outfile_prefix.pairs | split -l $L - $outfile_prefix.pairs.split");
  for my $i (0..$L-1){
    system("sort -k2,2 -k4,4 -k3,3g -k5,5g $outfile_prefix.pairs.split$i >> $outfile_prefix.bsorted.pairs.split$i");
  }
  system("sort -m -k2,2 -k4,4 -k3,3g -k5,5g $outfile_prefix.bsorted.pairs.split* >> $outfile_prefix.bsorted.pairs"); 
}else{
  system("grep -v '^#' $outfile_prefix.pairs | sort -k2,2 -k4,4 -k3,3g -k5,5g >> $outfile_prefix.bsorted.pairs");
}

# bgzipping
system("bgzip -f $outfile_prefix.bsorted.pairs");

# pairix indexing
system("pairix -f $outfile_prefix.bsorted.pairs.gz");

# removing text file
# system("rm -f $outfile_prefix.pairs");


