#include "kstring.h"
#include "bgzf.h"

int tad_readline(BGZF *fp, kstring_t *str)
{
	int c, l = 0;
	str->l = 0;
	while ((c = bgzf_getc(fp)) >= 0 && c != '\n') {
		++l;
		if (c != '\r') kputc(c, str);
	}
	if (c < 0 && l == 0) return -1; // end of file
	return str->l;
}

int main(int argc, char *argv[])
{
	BGZF *fp;
	kstring_t *str = calloc(1, sizeof(kstring_t));
	fp = bgzf_open(argv[1], "r");
	while (tad_readline(fp, str) >= 0)
		printf("%s\n", str->s);
	free(str->s); free(str);
	bgzf_close(fp);
	return 0;
}
