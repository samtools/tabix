#!/usr/bin/perl
use Getopt::Long;
my $pos_is_5end=1;
my $chrsizefile;
&GetOptions( 'l|leftmost' => sub { $pos_is_5end=0 },
             'c|chromsize=s' => \$chrsizefile );  ## This defines ordering between mates

if(@ARGV<2){ &print_usage(); exit(); }
my $input = $ARGV[0];
my $prefix = $ARGV[1];
my $outpairs = "$prefix.bsorted.pairs";

my %chr_ord=();
my %chr_size=();
if(defined $chrsizefile){
  &read_chrsizefile($chrsizefile, \%chr_size, \%chr_ord);  ## chr_ord is a hash with chr name as key and order (0,1,2..) as value.
}


open $IN, "samtools view -F12 -h $input |" or die "Can't open input bam file $input\n";

open $OUT, ">$outpairs" or die "Can't write to $outpairs";
&print_headers($OUT, $pos_is_5end, $chrsizefile, @ARGV);

my @SQ=();
my $header_parsed=0;
while(<$IN>){ 

  chomp;
  if(/^\@SQ/){
    push @SQ, $_; 
    next;
  }
  if(!/^\@/ && !$header_parsed){
    &parse_header(\@SQ, \%chr_size, \%chr_ord, $OUT);
    close $OUT; 
    open $OUT, "| sort -k2,2 -k4,4 -k3,3n -k5,5n >>$outpairs";
    $header_parsed=1;
  }
  @x=(split/\t/)[0,2,3,6,7,1]; # get the columns
  @seq=(split/\t/)[9,10]; # sequences
  
  # if the two chromosomes are identical, bam represents the second mate as '='. Replace this with the actual chromosome name.
  if($x[3] eq "="){ $x[3]=$x[1]; }
  
  # extract strand information from the flag column
  if($x[5]&16){ $s1="-"; $x[2]+=$pos_is_5end?length($seq[0])-1:0; } else { $s1="+"; }
  if($x[5]&32){ $s2="-"; $x[4]+=$pos_is_5end?length($seq[1])-1:0; } else { $s2="+"; }
  
  pop @x; #remove the flag column
  
  $"="\t";
  if(%chr_ord){
    next if(!exists $chr_ord{$x[1]} || !exists $chr_ord{$x[3]}); # works as filter as well.
    if($chr_ord{$x[1]} < $chr_ord{$x[3]} || ($chr_ord{$x[1]} == $chr_ord{$x[3]} && $x[2] < $x[4]) ){  ## upper triangle filtering
      print $OUT "@x\t$s1\t$s2\n";  
    }
  } else {
    if($x[1] lt $x[3] || ($x[1] eq $x[3] && $x[2] < $x[4]) ){  ## upper triangle filtering
      print $OUT "@x\t$s1\t$s2\n";  
    }
  }

}
close $IN;
close $OUT;

system("bgzip -f $prefix.bsorted.pairs");
system("pairix -f $prefix.bsorted.pairs.gz");

sub print_usage {
  print "usage: $0 [-l][-c <chromsize_file>] <input_bam> <out_prefix>\n";
  print "  -l : position is left-most position (default 5'end).\n";
  print "  -c|--chromsize : chrom size file is provided to define mate ordering. (default alpha-numeric)\n";
}

sub read_chrsizefile {
  my $chrsizefile = shift @_;
  my $pChr_size = shift @_;
  my $pChr_ord = shift @_;
  open $CHRSIZE, $chrsizefile or die "Can't open chrsize file $chrsizefile";
  my $k=0;  # chr order index
  while(<$CHRSIZE>){
    chomp; 
    my @s = split/\s+/; # any file with chromosome name as first column and size as second column would work. chr name should not have space.
    $pChr_ord->{$s[0]}=$k++;
    $pChr_size->{$s[0]}=$s[1];
  }
  close $CHRSIZE;
}


sub print_headers {
  my $OUT = shift @_;
  my $pos_is_5end = shift @_;
  my $chrsizefile = shift @_;
  my @ARGV = @_;

  print $OUT "## pairs format v1.0\n";
  print $OUT "#sorted: chr1-chr2-pos1-pos2\n";
  print $OUT "#shape: upper triangle\n";
  print $OUT "#columns: readID chr1 pos1 chr2 pos2 strand1 strand2\n";

  my $command_option = sprintf("%s %s", $pos_is_5end?'':'-l', $chrsizefile?"-c $chrsizefile":'');
  print $OUT "#command: bam2pairs $command_option @ARGV\n";
}

sub print_chr_size_header {
  my $OUT = shift @_;
  my $pChr_size = shift @_;
  my $pChr_ord = shift @_;
  for my $chr (sort {$pChr_ord->{$a}<=>$pChr_ord->{$b}} keys %$pChr_ord){
    print $OUT "#chromsize: $chr\t$pChr_size->{$chr}\n";
  }
}

sub parse_header {
  my $pSQ = $_[0];
  my $pChr_size=$_[1];
  my $pChr_ord=$_[2];
  my $fout=$_[3];

  if(scalar %$pChr_ord==0){
    for my $line (@$pSQ) {
      if($line=~/^\@SQ\s+SN:(\S+)\s+LN:(\d+)/){
        my $chrname = $1; my $chrlen = $2;
        $pChr_size->{$chrname}=$chrlen;
      }
    }
    my $k=0;
    %$pChr_ord = map { $_, $k++ } sort {$a cmp $b} keys %$pChr_size;
  }
  &print_chr_size_header($fout, $pChr_size, $pChr_ord);
}

