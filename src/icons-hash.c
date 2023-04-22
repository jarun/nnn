/*
 * simple program which outputs a hash-table of `icons_ext` with low collusion.
 * the hash function is case-insensitive, it also doesn't hash beyond the
 * length of the longest extension.
 */

#include <stddef.h>
#include <stdint.h>
#include <inttypes.h>

#define GOLDEN_RATIO_32  UINT32_C(2654442313) /* golden ratio for 32bits: (2^32) / 1.61803 */
#define GOLDEN_RATIO_64  UINT64_C(0x9E3793492EEDC3F7)
#define ICONS_TABLE_SIZE 8 /* size in bits. 8 = 256 */

#ifndef TOUPPER
	#define TOUPPER(ch)     (((ch) >= 'a' && (ch) <= 'z') ? ((ch) - 'a' + 'A') : (ch))
#endif

/* all of this is just for the static hash-table generation. only the hash
 * function gets included in `nnn` binary.
 */
#ifdef ICONS_GENERATE

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include "icons.h"

/* like assert, but always sticks around during generation. */
#define ENSURE(X) do { \
	if (!(X)) { \
		fprintf(stderr, "%s:%d: `%s`\n", __FILE__, __LINE__, #X); \
		abort(); \
	} \
} while (0)
#define ARRLEN(X) (sizeof(X) / sizeof((X)[0]))
#define MAX(A, B) ((A) > (B) ? (A) : (B))
#define HGEN_ITERARATION (1ul << 13)
#define ICONS_PROBE_MAX_ALLOWED 6
#define ICONS_MATCH_MAX (512)

#if 0 /* for logging some interesting info to stderr */
	#define log(...)  fprintf(stderr, "[INFO]: " __VA_ARGS__)
#else
	#define log(...) ((void)0)
#endif

static uint32_t icon_ext_hash(const char *s);

/* change ICONS_TABLE_SIZE to increase the size of the table */
static struct icon_pair table[1u << ICONS_TABLE_SIZE];
static uint8_t seen[ARRLEN(table)];
/* arbitrarily picked starting position. change if needed.
 * but ensure they're above 1 and prefer prime numbers.
 */
static uint32_t hash_start = 7;
static uint32_t hash_mul = 251; /* unused as of now */

/*
 * use robin-hood insertion to reduce the max probe length
 */
static void
rh_insert(const struct icon_pair item, uint32_t idx, uint32_t n)
{
	ENSURE(n != 0);
	for (uint32_t tries = 0; tries < ARRLEN(table); ++tries, ++n) {
		if (seen[idx] < n) {
			struct icon_pair tmp_item = table[idx];
			uint32_t tmp_n = seen[idx];

			ENSURE(n < (uint8_t)-1);
			table[idx] = item;
			seen[idx] = n;

			if (tmp_n != 0) /* the slot we inserted to wasn't empty */
				rh_insert(tmp_item, idx, tmp_n);
			return;
		}
		idx = (idx + 1) % ARRLEN(table);
	}
	ENSURE(0 && "unreachable");
}

enum { PROBE_MAX, PROBE_TOTAL, PROBE_CNT };
static unsigned int *
table_populate(unsigned int p[static PROBE_CNT])
{
	memset(seen, 0x0, sizeof seen);
	memset(table, 0x0, sizeof table);
	for (size_t i = 0; i < ARRLEN(icons_ext); ++i) {
		if (icons_ext[i].icon[0] == '\0') /* skip empty entries */
			continue;
		uint32_t h = icon_ext_hash(icons_ext[i].match);
		rh_insert(icons_ext[i], h, 1);
	}

	p[PROBE_MAX] = p[PROBE_TOTAL] = 0;
	for (size_t i = 0; i < ARRLEN(seen); ++i) {
		p[PROBE_MAX] = MAX(p[PROBE_MAX], seen[i]);
		p[PROBE_TOTAL] += seen[i];
	}
	return p;
}

/* permuted congruential generator */
static uint32_t
pcg(uint64_t *state)
{
	uint64_t oldstate = *state;
	*state *= GOLDEN_RATIO_64;
	uint32_t r = (oldstate >> 59);
	uint32_t v = (oldstate ^ (oldstate >> 18)) >> 27;
	return (v >> (-r & 31)) | (v << r);
}

int
main(void)
{
	ENSURE(ARRLEN(icons_ext) <= ARRLEN(table));
	ENSURE(ICONS_TABLE_SIZE < 16);
	ENSURE(1u << ICONS_TABLE_SIZE == ARRLEN(table));
	ENSURE((GOLDEN_RATIO_32 & 1) == 1); /* must be odd */
	ENSURE((GOLDEN_RATIO_64 & 1) == 1); /* must be odd */
	ENSURE(hash_start > 1);
	ENSURE(hash_mul > 1);
	/* ensure power of 2 hashtable size which allows compiler to optimize
	 * away mod (`%`) operations
	 */
	ENSURE((ARRLEN(table) & (ARRLEN(table) - 1)) == 0);

	unsigned int max_probe = (unsigned)-1;
	uint32_t best_hash_start = 0, best_hash_mul = 0, best_total_probe = 9999;
	uint64_t hash_start_rng = hash_start, hash_mul_rng = hash_mul;

	for (size_t i = 0; i < HGEN_ITERARATION; ++i) {
		unsigned *p = table_populate((unsigned [PROBE_CNT]){0});
		if (p[PROBE_MAX] < max_probe ||
		    (p[PROBE_MAX] == max_probe && p[PROBE_TOTAL] < best_total_probe))
		{
			max_probe = p[PROBE_MAX];
			best_total_probe = p[PROBE_TOTAL];
			best_hash_start = hash_start;
			best_hash_mul = hash_mul;
		}
		hash_start = pcg(&hash_start_rng);
		hash_mul = pcg(&hash_mul_rng);
	}
	ENSURE(max_probe < ICONS_PROBE_MAX_ALLOWED);
	hash_start = best_hash_start;
	hash_mul = best_hash_mul;
	{
		unsigned *p = table_populate((unsigned [PROBE_CNT]){0});
		ENSURE(p[PROBE_MAX] == max_probe);
		ENSURE(p[PROBE_TOTAL] == best_total_probe);
	}

	/* sanity check */
	double nitems = 0;
	unsigned int total_probe = 0;
	for (size_t i = 0; i < ARRLEN(icons_ext); ++i) {
		if (icons_ext[i].icon[0] == 0)
			continue;
		uint32_t found = 0, h = icon_ext_hash(icons_ext[i].match);
		for (uint32_t k = 0; k < max_probe; ++k) {
			uint32_t z = (h + k) % ARRLEN(table);
			++total_probe;
			if (table[z].match && strcasecmp(icons_ext[i].match, table[z].match) == 0) {
				found = 1;
				break;
			}
		}
		ENSURE(found);
		++nitems;
	}
	ENSURE(total_probe == best_total_probe);

	size_t match_max = 0, icon_max = 0;
	for (size_t i = 0; i < ARRLEN(icons_name); ++i) {
		match_max = MAX(match_max, strlen(icons_name[i].match) + 1);
		icon_max = MAX(icon_max, strlen(icons_name[i].icon) + 1);
	}
	for (size_t i = 0; i < ARRLEN(icons_ext); ++i) {
		match_max = MAX(match_max, strlen(icons_ext[i].match) + 1);
		icon_max = MAX(icon_max, strlen(icons_ext[i].icon) + 1);
	}
	icon_max = MAX(icon_max, strlen(dir_icon.icon) + 1);
	icon_max = MAX(icon_max, strlen(exec_icon.icon) + 1);
	icon_max = MAX(icon_max, strlen(file_icon.icon) + 1);
	ENSURE(icon_max < ICONS_MATCH_MAX);

	const char *uniq[ARRLEN(icons_ext)] = {0};
	size_t uniq_head = 0;
	for (size_t i = 0; i < ARRLEN(icons_ext); ++i) {
		if (icons_ext[i].icon[0] == 0)
			continue;
		int isuniq = 1;
		for (size_t k = 0; k < uniq_head; ++k) {
			if (strcasecmp(uniq[k], icons_ext[i].icon) == 0) {
				isuniq = 0;
				break;
			}
		}
		if (isuniq) {
			ENSURE(uniq_head < ARRLEN(uniq));
			uniq[uniq_head++] = icons_ext[i].icon;
		}
	}
	ENSURE(uniq_head < (unsigned char)-1);

	log("load-factor:  %.2f (%u/%zu)\n", (nitems * 100.0) / (double)ARRLEN(table),
	    (unsigned int)nitems, ARRLEN(table));
	log("max_probe  : %6u\n", max_probe);
	log("total_probe: %6u\n", total_probe);
	log("uniq icons : %6zu\n", uniq_head);
	log("no-compact : %6zu bytes\n", ARRLEN(table) * icon_max);
	log("compaction : %6zu bytes\n", uniq_head * icon_max + ARRLEN(table));
	log("hash_start : %6" PRIu32 "\n", hash_start);
	log("hash_mul   : %6" PRIu32 "\n", hash_mul);

	printf("#ifndef INCLUDE_ICONS_GENERATED\n");
	printf("#define INCLUDE_ICONS_GENERATED\n\n");

	printf("/*\n * NOTE: This file is automatically generated.\n");
	printf(" * DO NOT EDIT THIS FILE DIRECTLY.\n");
	printf(" * Use `icons.h` to customize icons\n */\n\n");

	printf("#define hash_start  UINT32_C(%" PRIu32 ")\n", hash_start);
	printf("#define hash_mul    UINT32_C(%" PRIu32 ")\n\n", hash_mul);
	printf("#define ICONS_PROBE_MAX %uu\n", max_probe);
	printf("#define ICONS_MATCH_MAX %zuu\n\n", match_max);
	printf("#define ICONS_STR_MAX %zuu\n\n", icon_max);

	printf("struct icon_pair { const char match[ICONS_MATCH_MAX]; "
	       "const char icon[ICONS_STR_MAX]; unsigned char color; };\n\n");

	printf("static const char icons_ext_uniq[%zu][ICONS_STR_MAX] = {\n", uniq_head);
	for (size_t i = 0; i < uniq_head; ++i)
		printf("\t\"%s\",\n", uniq[i]);
	printf("};\n\n");

	printf("static const struct {\n\tconst char match[ICONS_MATCH_MAX];\n"
	       "\tunsigned char idx;\n\tunsigned char color;\n} icons_ext[%zu] = {\n",
	        ARRLEN(table));
	for (size_t i = 0; i < ARRLEN(table); ++i) {
		if (table[i].icon == NULL || table[i].icon[0] == '\0') /* skip empty entries */
			continue;
		size_t k;
		for (k = 0; k < uniq_head; ++k) {
			if (strcasecmp(table[i].icon, uniq[k]) == 0)
				break;
		}
		ENSURE(k < uniq_head);
		printf("\t[%3zu] = {\"%s\", %zu, %hhu },\n",
		       i, table[i].match, k, table[i].color);
	}
	printf("};\n\n");

	printf("#endif /* INCLUDE_ICONS_GENERATED */\n");
}

#else
	#define ENSURE(X) ((void)0)
#endif /* ICONS_GENERATE */

#if defined(ICONS_GENERATE) || defined(ICONS_ENABLED)
static uint32_t
icon_ext_hash(const char *str)
{
	uint32_t i, hash = hash_start;
	enum { wsz = sizeof hash * CHAR_BIT, z = wsz - ICONS_TABLE_SIZE, r = 5 };

	/* just an xor-rotate hash. in general, this is a horrible hash
	 * function but for our specific input it works fine while being
	 * computationally cheap.
	 */
	for (i = 0; i < ICONS_MATCH_MAX && str[i] != '\0'; ++i) {
		hash ^= TOUPPER((unsigned char)str[i]);
		hash  = (hash >> (wsz - r)) | (hash << r);
	}

	/* finalizer: https://probablydance.com/2018/06/16 */
	hash ^= (hash >> z);
	hash *= GOLDEN_RATIO_32;

	hash >>= z;
	ENSURE(hash < ARRLEN(table));

	return hash;
}
#endif
