/*
 * Copyright (c) 2014, Intel Corporation
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *    * Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 *    * Redistributions in binary form must reproduce the above copyright
 *      notice, this list of conditions and the following disclaimer
 *      in the documentation and/or other materials provided with the
 *      distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "parse_dmi.h"
#include "dmi_intel.h"
#include "inc/libdmi.h"

#include <string.h>
#include <stdlib.h>

#define FORMAT_FIELD_MASK    0xE0
#define FORMAT_FIXED_TABLE   0x00
#define FORMAT_KV_TABLE      0x20
#define FORMAT_KTV_TABLE     0x40
#define FIELD_TYPE_BYTE      0x1
#define FIELD_TYPE_WORD      0x2
#define FIELD_TYPE_DWORD     0x3
#define FIELD_TYPE_STRING    0x4

struct intel_0x94 {
	struct dmi_header hdr;
	unsigned char GopVersion;
	unsigned char UCodeVersion;
	unsigned char MRCVersion;
	unsigned char SECVersion;
	unsigned char ULPMCVersion;
	unsigned char PMCVersion;
	unsigned char PUnitVersion;
	unsigned char SoCVersion;
	unsigned char BoardVersion;
	unsigned char FabVersion;
	unsigned char CPUFlavor;
	unsigned char BiosVersion;
	unsigned char PmicVersion;
	unsigned char TouchVersion;
	unsigned char SecureBoot;
	unsigned char BootMode;
	unsigned char SpeedStepMode;
	unsigned char CPUTurboMode;
	unsigned char MaxCState;
	unsigned char GfxTurbo;
	unsigned char S0ix;
	unsigned char RC6;
};

static struct field_desc intel_0x94_desc[] = {
	FIELD_DESC(struct intel_0x94, GopVersion, 0),
	FIELD_DESC(struct intel_0x94, UCodeVersion, 0),
	FIELD_DESC(struct intel_0x94, MRCVersion, 0),
	FIELD_DESC(struct intel_0x94, SECVersion, 0),
	FIELD_DESC(struct intel_0x94, ULPMCVersion, 0),
	FIELD_DESC(struct intel_0x94, PMCVersion, 0),
	FIELD_DESC(struct intel_0x94, PUnitVersion, 0),
	FIELD_DESC(struct intel_0x94, SoCVersion, 0),
	FIELD_DESC(struct intel_0x94, BoardVersion, 0),
	FIELD_DESC(struct intel_0x94, FabVersion, 0),
	FIELD_DESC(struct intel_0x94, CPUFlavor, 0),
	FIELD_DESC(struct intel_0x94, BiosVersion, 0),
	FIELD_DESC(struct intel_0x94, PmicVersion, 0),
	FIELD_DESC(struct intel_0x94, TouchVersion, 0),
	FIELD_DESC(struct intel_0x94, SecureBoot, 0),
	FIELD_DESC(struct intel_0x94, BootMode, 0),
	FIELD_DESC(struct intel_0x94, SpeedStepMode, 0),
	FIELD_DESC(struct intel_0x94, CPUTurboMode, 0),
	FIELD_DESC(struct intel_0x94, MaxCState, 0),
	FIELD_DESC(struct intel_0x94, GfxTurbo, 0),
	FIELD_DESC(struct intel_0x94, S0ix, 0),
	FIELD_DESC(struct intel_0x94, RC6, 0),
};

struct platform_header {
	struct dmi_header hdr;
	unsigned char Platform;
	unsigned char Version;
};

static char *intel_parse_kv_table(struct dmi_header *dmi, char *fieldname) {
	char *value = NULL;
	struct platform_header *hdr = (struct platform_header *) dmi;
	unsigned char table_format = hdr->Version & FORMAT_FIELD_MASK;
	if (table_format > FORMAT_KTV_TABLE) {
		error("Unsupported table version: 0x%x", hdr->Version);
		return NULL;
	}

	if (!strcmp(fieldname, "Platform")) {
		return parse_dmi_field(dmi, (unsigned char *)dmi
			+ offsetof(struct platform_header, Platform), 0);
	}
	else if (!strcmp(fieldname, "Version")) {
		return parse_dmi_field(dmi, (unsigned char *)dmi
			+ offsetof(struct platform_header, Version), 1);
	}
	else if (dmi->length <= sizeof(struct platform_header)) {
		error("No fields defined for table. DMI length: %d\n", dmi->length);
	}
	else if (table_format == FORMAT_KV_TABLE) {
		int entries = (dmi->length - sizeof(struct platform_header)) / 2;
		unsigned char *offset;

		while (entries--) {
			offset = (unsigned char *)dmi + sizeof(struct platform_header)
				+ (entries << 1);

			value = parse_dmi_field(dmi, offset, 0);
			if (value && strcmp(value, fieldname)) {
				free(value);
				continue;
			}
			free(value);

			return parse_dmi_field(dmi, offset + 1, 0);
		}
	}
	else if (table_format == FORMAT_KTV_TABLE) {
		unsigned char *offset = (unsigned char *)dmi + sizeof(struct platform_header);
		unsigned char *last_entry = (unsigned char *)dmi + dmi->length;

		while (offset < last_entry) {
			value = parse_dmi_field(dmi, offset, 0);
			if (value && !strcmp(value, fieldname)) {
				unsigned int type_size = 0;
				++offset;

				free(value);
				if (*offset == 0 || *offset > FIELD_TYPE_STRING) {
					error("Unsupported field type found: 0x%x\n", *offset);
					break;
				}

				if (*offset != FIELD_TYPE_STRING)
					type_size = 1 << (*offset - 1);

				return parse_dmi_field(dmi, ++offset, type_size);
			}
			free(value);

			++offset;
			if (*offset == 0 || *offset > FIELD_TYPE_STRING) {
				error("Unsupported field type found: 0x%x\n", *offset);
				break;
			}
			else
				offset += (*offset == FIELD_TYPE_STRING) ? 1 : 1 << (*offset - 1);

			++offset;
		}
	}

	error("Unknown field %s for table 0x%x\n", fieldname, dmi->type);
	return NULL;
}

char *intel_dmi_parser(struct dmi_header *dmi, char *field)
{
	char *pn;
	switch (dmi->type) {
	case INTEL_SMBIOS:
		pn = dmi_get_product_name();

		if (!pn) {
			error("Cannot get product name\n");
			return NULL;
		}

		if (strncmp(pn, "BROXTON", 7) == 0) {
			free(pn);
			struct platform_header *hdr = (struct platform_header *) dmi;
			char table_format = hdr->Version & FORMAT_FIELD_MASK;
			char *value = parse_dmi_field(dmi, (unsigned char *)dmi
				+ offsetof(struct platform_header, Platform), 0);
			if (!value || strncmp(value, "Broxton-M", 10) ||
			    table_format != FORMAT_KV_TABLE) {
				error("Unsupported Platform: %s; Table Format: 0x%x\n",
					value, table_format);
				free(value);
				return NULL;
			}
			free(value);
			return intel_parse_kv_table(dmi, field);
		}
		else {
			free(pn);
			PARSE_FIELD(intel_0x94, dmi, field);
		}
		break;
	default:
		error("Unsupported Intel table: 0x%x\n", dmi->type);
	}
out:
	return NULL;
}
