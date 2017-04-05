
int build_index(char *inputfilename, char *preset, int sc, int bc, int ec, int sc2, int bc2, int ec2, char *delimiter, char *meta_char, int line_skip, int force){

  if(force==0){
    char *fnidx = calloc(strlen(inputfilename) + 5, 1);
    strcat(strcpy(fnidx, inputfilename), ".px2");
    struct stat stat_px2, stat_input;
     if (stat(fnidx, &stat_px2) == 0){  // file exists.
         // Before complaining, check if the input file isn't newer. This is a common source of errors,
         // people tend not to notice that pairix failed
         stat(inputfilename, &stat_input);
         if ( stat_input.st_mtime <= stat_px2.st_mtime ) return(-4);
     }
     free(fnidx);
  }
  if ( bgzf_is_bgzf(inputfilename)!=1 ) return(-3);
  else {
    ti_conf_t conf;
    if (strcmp(preset, "") == 0 && sc == 0 && bc == 0){
      int l = strlen(inputfilename);
      int strcasecmp(const char *s1, const char *s2);
      if (l>=7 && strcasecmp(inputfilename+l-7, ".gff.gz") == 0) conf = ti_conf_gff;
      else if (l>=7 && strcasecmp(inputfilename+l-7, ".bed.gz") == 0) conf = ti_conf_bed;
      else if (l>=7 && strcasecmp(inputfilename+l-7, ".sam.gz") == 0) conf = ti_conf_sam;
      else if (l>=7 && strcasecmp(inputfilename+l-7, ".vcf.gz") == 0) conf = ti_conf_vcf;
      else if (l>=10 && strcasecmp(inputfilename+l-10, ".psltbl.gz") == 0) conf = ti_conf_psltbl;
      else if (l>=9 && strcasecmp(inputfilename+l-9, ".pairs.gz") == 0) conf = ti_conf_pairs;
      else return(-5); // file extension not recognized and no preset specified
    }
    else if (strcmp(preset, "") == 0 && sc != 0 && bc != 0){
      conf.sc = sc;
      conf.bc = bc;
      conf.ec = ec;
      conf.sc2 = sc2;
      conf.bc2 = bc2;
      conf.ec2 = ec2;
      conf.delimiter = (delimiter)[0];
      conf.meta_char = (int)((meta_char)[0]);
      conf.line_skip = line_skip;
    }
    else if (strcmp(preset, "gff") == 0) conf = ti_conf_gff;
    else if (strcmp(preset, "bed") == 0) conf = ti_conf_bed;
    else if (strcmp(preset, "sam") == 0) conf = ti_conf_sam;
    else if (strcmp(preset, "vcf") == 0 || strcmp(preset, "vcf4") == 0) conf = ti_conf_vcf;
    else if (strcmp(preset, "psltbl") == 0) conf = ti_conf_psltbl;
    else if (strcmp(preset, "pairs") == 0) conf = ti_conf_pairs;
    else if (strcmp(preset, "merged_nodups") == 0) conf = ti_conf_merged_nodups;
    else if (strcmp(preset, "old_merged_nodups") == 0) conf = ti_conf_old_merged_nodups;
    else return(-2);  // wrong preset

    return ti_index_build(inputfilename, &conf);  // -1 if failed
  }
}



