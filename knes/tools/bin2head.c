/*

  bin2head
  by thefox//aspekt 2010

  C sucks.

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define VERSION         "v0.1"
#define AUTHOR          "thefox"
#define BYTES_PER_ROW   16

static int process(const char *infile);

int main(int argc, char **argv)
{
    if(argc != 2) {
        printf("bin2head " VERSION " by " AUTHOR "\n");
        printf("usage: %s infile\n", argv[0]);
        return EXIT_FAILURE;
    }

    return process(argv[1]) ? EXIT_SUCCESS : EXIT_FAILURE;
}

int process(const char *infile)
{
    FILE *ifp;
    FILE *ofp;
    char *outfile;
    char *table_name;
    char *p;
    int ch;
    int ext_pos;
    int i;
    int num_bytes;

    ifp = fopen(infile, "rb");
    if(!ifp) {
        printf("couldn't open infile");
        return 0;
    }

    // find the file extension from infile
    ext_pos = strlen(infile);
    for(i = strlen(infile) - 1; i >= 0; --i) {
        if(infile[i] == '.' && i) { // skip if dot is the first character of the filename
            ext_pos = i;
            break;
        } else if(infile[i] == '/' || infile[i] == '\\') {
            break;
        }
    }

    outfile = (char *)calloc(ext_pos + 3, 1);
    strncpy(outfile, infile, ext_pos);
    strcat(outfile, ".h");

    ofp = fopen(outfile, "w");
    free(outfile);
    if(!ofp) {
        printf("couldn't open outfile");
        fclose(ifp);
        return 0;
    }

    table_name = (char *)calloc(ext_pos + 1, 1);
    strncpy(table_name, infile, ext_pos);
    for(p = table_name; *p; ++p) {
        if(!isalnum(*p)) {
            *p = '_';
        }
    }

    fprintf(ofp, "const unsigned char %s[] = {\n", table_name);
    free(table_name);

    num_bytes = 0;
    while((ch = fgetc(ifp)) != EOF) {
        if(num_bytes % BYTES_PER_ROW == 0) {
            fprintf(ofp, "  ");
        }
        fprintf(ofp, "0x%02X,", ch);
        if(num_bytes % BYTES_PER_ROW == BYTES_PER_ROW - 1) {
            fprintf(ofp, "\n");
        }
        ++num_bytes;
    }
    if(num_bytes % BYTES_PER_ROW != 0) {
        fprintf(ofp, "\n");
    }
    fprintf(ofp, "};\n");

    fclose(ifp);
    fclose(ofp);

    return 1;
}
