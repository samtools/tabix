package Tabix;

use strict;
use warnings;

require Exporter;

our @ISA = qw/Exporter/;
our @EXPORT = qw//;

our $VERSION = '0.1.6';

require XSLoader;
XSLoader::load('Tabix', $VERSION);

1;
__END__
