#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <time.h>

#include <linux/stddef.h>
#include <stdlib.h>

#include <parse_dmi.h>
#include <dmi_intel.h>

static const char *test_entries[] = {
	"Platform",
	"Version",
	"IFWI",
	"BIOS",
	"CSE",
	"GOP",
	"MRC",
	"uCode",
	"P-unit",
	"PMC",
	"ISH",
	"SoC",
	"Board ID",
	"Fab ID",
	"CPU Flavor",
	"PMIC",
	"Secure Boot",
	"Boot Mode",
	"Speed Step",
	"CPU Turbo",
	"C-State",
	"Enhanced C-State",
	"MaxPkg C-State",
	"MaxCore C-State",
	"GfxTurbo",
	"S0ix",
	"RC6"
};
static const int entries = (sizeof(test_entries)) / (sizeof(test_entries[0]));

char *dmi_get_product_name() {
	return strdup("BROXTON");
}

unsigned char * load_table_from_file(FILE *file) {
	int size = 0;
	unsigned char *retval;
	unsigned int val;
	while (fscanf(file, " %x", &val) == 1)
		++size;

	retval = (unsigned char *) malloc(size);
	fseek(file, 0, SEEK_SET);
	size = 0;
	while (fscanf(file, " %x", &val) == 1)
		retval[size++] = (unsigned char) val;

	return retval;
}

int main(int __attribute__((unused)) argc, char __attribute__((unused)) **argv) {
	int i = 0;
	FILE * file = fopen ("res/kv_input" , "r");
	if (file == NULL) {
		perror ("Error opening file");
		return -1;
	}

	char *config = load_table_from_file(file);

	for ( ; i < entries; i++) {
		char * entry = intel_dmi_parser((struct dmi_header *)config, test_entries[i]);

		if (entry) {
			fprintf(stderr, "%s :\t\t%s\n", test_entries[i], entry);
			free(entry);
		}
		else
			return -1;
	}

	if (intel_dmi_parser((struct dmi_header *)config, "qwerty") != NULL) {
		fprintf(stderr, "Error, value generated for entry that should not exist\n");
		return -1;
	}

	free(config);

	return 0;
}
