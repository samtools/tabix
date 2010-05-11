package Tabix;

use strict;
use warnings;
use Carp qw/croak/;

require Exporter;

our @ISA = qw/Exporter/;
our @EXPORT = qw/tabix_open tabix_close tabix_read tabix_query tabix_getnames/;

our $VERSION = '0.1.6';

require XSLoader;
XSLoader::load('Tabix', $VERSION);

sub new {
  my $invocant = shift;
  my %args = @_;
  my $fn = $args{-data} || croak("-data argument required");
  my $class = ref($invocant) || $invocant;
  my $self = {};
  bless($self, $class);
  $self->open($fn);
  return $self;
}

sub open {
  my ($self, $fn) = @_;
  $self->close;
  $self->{_fn} = $fn;
  $self->{_} = tabix_open($fn);
}

sub close {
  my $self = shift;
  if ($self->{_}) {
	tabix_close($self->{_});
	delete($self->{_}); delete($self->{_fn});
  }
}

sub DESTROY {
  my $self = shift;
  $self->close;
}

sub query {
  my $self = shift;
  if (@_) {
	return tabix_query($self->{_}, @_);
  } else {
	return tabix_query($self->{_});
  }
}

sub read {
  my $self = shift;
  my $iter = shift;
  return tabix_read($iter);
}

sub getnames {
  my $self = shift;
  return tabix_getnames($self->{_});
}

1;
__END__
