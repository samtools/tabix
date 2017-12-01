#!/usr/bin/perl

while(<>){
  if(/^#/) {
    if(!exists $header{$_}){
       $header{$_}=1;
       print;
    }
  } else {
    print;
  }
}
