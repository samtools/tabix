
void build_index(char **pinputfilename, char **ppreset, int *psc, int *pbc, int *pec, int *psc2, int *pbc2, int *pec2, char **pdelimiter, char **pmeta_char, int *pline_skip, int *pforce, int *pflag){

  if(*pforce==0){
    char *fnidx = calloc(strlen(*pinputfilename) + 5, 1);
    strcat(strcpy(fnidx, *pinputfilename), ".px2");
    struct stat stat_px2, stat_input;
     if (stat(fnidx, &stat_px2) == 0){  // file exists.
         // Before complaining, check if the input file isn't newer. This is a common source of errors,
         // people tend not to notice that pairix failed
         stat(*pinputfilename, &stat_input);
         if ( stat_input.st_mtime <= stat_px2.st_mtime ) *pflag=-4;
     }
     free(fnidx);
  }
  if(*pflag != -4){
    if ( bgzf_is_bgzf(*pinputfilename)!=1 ) *pflag = -3;
    else {
      ti_conf_t conf;
      if (strcmp(*ppreset, "") == 0 && *psc == 0 && *pbc == 0){
        int l = strlen(*pinputfilename);
        int strcasecmp(const char *s1, const char *s2);
        if (l>=7 && strcasecmp(*pinputfilename+l-7, ".gff.gz") == 0) conf = ti_conf_gff;
        else if (l>=7 && strcasecmp(*pinputfilename+l-7, ".bed.gz") == 0) conf = ti_conf_bed;
        else if (l>=7 && strcasecmp(*pinputfilename+l-7, ".sam.gz") == 0) conf = ti_conf_sam;
        else if (l>=7 && strcasecmp(*pinputfilename+l-7, ".vcf.gz") == 0) conf = ti_conf_vcf;
        else if (l>=10 && strcasecmp(*pinputfilename+l-10, ".psltbl.gz") == 0) conf = ti_conf_psltbl;
        else if (l>=9 && strcasecmp(*pinputfilename+l-9, ".pairs.gz") == 0) conf = ti_conf_pairs;
        else *pflag = -5; // file extension not recognized and no preset specified
      }
      else if (strcmp(*ppreset, "") == 0 && *psc != 0 && *pbc != 0){
        conf.sc = *psc;
        conf.bc = *pbc;
        conf.ec = *pec;
        conf.sc2 = *psc2;
        conf.bc2 = *pbc2;
        conf.ec2 = *pec2;
        conf.delimiter = (*pdelimiter)[0];
        conf.meta_char = (int)((*pmeta_char)[0]);
        conf.line_skip = *pline_skip;
      }
      else if (strcmp(*ppreset, "gff") == 0) conf = ti_conf_gff;
      else if (strcmp(*ppreset, "bed") == 0) conf = ti_conf_bed;
      else if (strcmp(*ppreset, "sam") == 0) conf = ti_conf_sam;
      else if (strcmp(*ppreset, "vcf") == 0 || strcmp(*ppreset, "vcf4") == 0) conf = ti_conf_vcf;
      else if (strcmp(*ppreset, "psltbl") == 0) conf = ti_conf_psltbl;
      else if (strcmp(*ppreset, "pairs") == 0) conf = ti_conf_pairs;
      else if (strcmp(*ppreset, "merged_nodups") == 0) conf = ti_conf_merged_nodups;
      else if (strcmp(*ppreset, "old_merged_nodups") == 0) conf = ti_conf_old_merged_nodups;
      else *pflag = -2;  // wrong preset

      if (*pflag != -2 && *pflag != -5 ) *pflag= ti_index_build(*pinputfilename, &conf);  // -1 if failed
    }
  }
}



