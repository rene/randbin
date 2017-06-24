/**
 * randbin: A simple tool which 'fuzzifies' files
 * Copyright (C) 2015 Renê de Souza Pinto. All rights reserved.
 *
 * Author: Renê S. Pinto
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */ 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <getopt.h>
#include <libgen.h>

/* Prototypes */
void show_help(const char *prgname, FILE *fp);
int modify_file(char *input, char *outdir, int percent);

int main(int argc, char* const argv[])
{
	int c;
	int longindex;
	const char optstring[] = "hf:o:p:";
	static const struct option longOpts[] = {
		{ "help",   no_argument, NULL, 'h' },
		{ "file",   required_argument, NULL, 'f' },
		{ "outdir", required_argument, NULL, 'o' },
		{ "percent",required_argument, NULL, 'p' },
		{ NULL,     no_argument, NULL, 0 }
	};
	char *input_filename = NULL;
	char *outdir         = NULL;
	int percent = 1;

	/* Parse arguments */
	while((c = getopt_long(argc, argv, optstring, longOpts, &longindex)) != -1) {
		switch(c) {
			case 'h':
				show_help(argv[0], stdout);
				exit(EXIT_SUCCESS);
				break;

			case 'f':
				input_filename = optarg;
				break;

			case 'o':
				outdir = optarg;
				break;

			case 'p':
				percent = atoi(optarg);
				break;

			default:
				break;
		}
	}

	/* Args. validation */
	if (input_filename == NULL) {
		fprintf(stderr, "Input file should be provided.\n");
		show_help(argv[0], stderr);
		return EXIT_FAILURE;
	}
	if (outdir == NULL) {
		fprintf(stderr, "Please, provide output directory destination.\n");
		show_help(argv[0], stderr);
		return EXIT_FAILURE;
	}
	if (percent <= 0) {
		fprintf(stderr, "Percentage should be greater than 0.\n");
		show_help(argv[0], stderr);
		return EXIT_FAILURE;
	}
	
	/* Generate modified files */
	if (modify_file(input_filename, outdir, percent) < 0) {
		fprintf(stderr, "File modification failure.\n");
	}

	return EXIT_SUCCESS;
}


/**
 * @brief: Show program's help
 * @param [in] prgname Program's name
 * @param [in] fp File pointer to write
 */
void show_help(const char *prgname, FILE *fp)
{
	fprintf(fp, "Use: %s [options]\n", prgname);
	fprintf(fp, "Options:\n");
	fprintf(fp, "    -h | --help        Show this help and exit\n");
	fprintf(fp, "    -f | --file        Input file\n");
	fprintf(fp, "    -o | --outdir      Output directory destination\n");
	fprintf(fp, "    -p | --percent     Percentage of mutations (Default: 1)\n");
}

/**
 * @brief Change and generate random bytes into the file
 * @param [in] input Input file
 * @param [in] outdir Output directory destination
 * @param [in] percent Percentage of mutations
 * @return int 0 on success, -1 otherwise
 */
int modify_file(char *input, char *outdir, int percent)
{
	FILE *in, *out;
	char *outfilename;
	char *fname;
	unsigned char *fcontents, *mappos;
	unsigned char newbyte;
	size_t pos, len;
	long fsize, i, nbytes;

	/* Open input files */
	if ((in = fopen(input, "r")) == NULL) {
		perror("fopen()");
		return -1;
	}
	
	/* Get file size */
	fseek(in, 0, SEEK_END);
	fsize = ftell(in);
	fseek(in, 0, SEEK_SET);
	
	/* Create output file */
	fname       = basename(strdup(input));
	len         = strlen(outdir) + strlen(fname) + 2;
	outfilename = malloc(sizeof(char) * len);
	if (outfilename == NULL) {
		perror("malloc()");
		return -1;
	}

	strcpy(outfilename, outdir);
	strcat(outfilename, "/");
	strcat(outfilename, fname);

	/* Open an output file */
	out = fopen(outfilename, "w");
	if (out == NULL) {
		perror("fopen()");
		return -1;
	}
		
	/* Create the seed */
	srand(time(NULL));

	/* Load file to memory */
	fcontents = malloc(sizeof(unsigned char) * fsize);
	if (fcontents == NULL) {
		perror("malloc()");
		free(outfilename);
		fclose(in);
		fclose(out);
		return -1;
	} else {
		/* Load file */
		fread(fcontents, sizeof(unsigned char), fsize, in);
	}

	/* Create random pos map */
	mappos = malloc(sizeof(unsigned char) * fsize);
	memset(mappos, 0x00, fsize);

	/* Mutation */
	nbytes = ((float)fsize * ((float)percent / 100.0));
#ifdef DEBUG
	printf("------------------\n");
	printf("File Size:	%ld\n", fsize);
	printf("Percentage:	%d\n", percent);
	printf("nbytes:	%ld\n", nbytes);
	printf("------------------\n");
#endif
	for (i = 0; i < nbytes; i++) {
		do {
			pos = (rand() % fsize);
		} while(mappos[pos] == 0xFF);
		mappos[pos] = 0xFF;

		do {
			newbyte = (rand() % 255); /* Generate random byte */
			//newbyte = (rand() % 127); /* Generate random byte */
		} while(newbyte == fcontents[pos]);
		fcontents[pos] = newbyte;

#ifdef DEBUG		
		printf("------------------\n");
		printf("pos:     %ld\n", pos);
		printf("newbyte: %c\n", newbyte);
		printf("------------------\n");
#endif
	}

	/* Write output file */
	fwrite(fcontents, sizeof(char), fsize, out);

	/* Messy clean up */
	fclose(in);
	fclose(out);
	free(fcontents);
	free(mappos);
	free(outfilename);
	return 0;
}

