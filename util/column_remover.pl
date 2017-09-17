#!/usr/bin/perl
use Getopt::Long;
my $content_only;
&GetOptions( 'd|do-not-fix-header' => \$content_only);
if(@ARGV<2) { print "usage: $0 [--do-no-fix-header|-d] <input_pairs(ungzipped_or_stream)> <col1> [<col2> [...]]\ncol1, col2, ... should be the column names (e.g. 'frag1') if -d option is NOT used. With the -d option, column index (0-based) should be used instead and only a single column can be removed at a time.\n"; exit; }
$in_file = shift @ARGV;
@cols_to_remove = @ARGV;

$delimiter="\t";


if(!defined $content_only){
  open IN, $in_file or die "Can't open input file $in_file\n";
  while(<IN>){
    if(/^#columns/) { 
        chomp;
        @colnames = split/\s/;
        shift @colnames;
        @ind_to_remove=();
        for my $c (@cols_to_remove) {
          my $k=0;
          my %colindex = map { $_, $k++ } @colnames;
          push @ind_to_remove, $colindex{$c};
          splice @colnames, $colindex{$c}, 1;
        }
        print "#columns: @colnames\n"; 
        print STDERR @ind_to_remove;
    }
    elsif(/^#/) { print; }  # all other headers
    else{
        chomp;
        my @line = split/$delimiter/; 
        for my $i (@ind_to_remove) {
          splice @line, $i, 1;
        }
        $"=$delimiter;
        print "@line\n";
    }
  }
  close IN;
}
else {
  if(@cols_to_remove > 1) { die "Only a single column can be removed for -d option\n"; }
  open IN, $in_file or die "Can't open input file $in_file\n";
  while(<IN>){
    if(/^#/){ print; }
    else{
        chomp;
        my @line = split/$delimiter/; 
        splice @line, $cols_to_remove[0], 1;
        $"=$delimiter;
        print "@line\n";
    }
  }
  close IN;

}
