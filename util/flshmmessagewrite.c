#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <flshm.h>

char * hex_to_bin(char * hex, unsigned int * len) {
	unsigned int len_hex = strlen(hex);
	if (len_hex % 2) {
		len_hex--;
	}
	unsigned int bin_len = len_hex / 2;
	char * bin = malloc(bin_len);
	for (unsigned int i = 0; i < bin_len; i++) {
		unsigned int c;
		sscanf(hex + (i * 2), "%02x", &c);
		bin[i] = c;
	}
	* len = bin_len;
	return bin;
}

int main(int argc, char ** argv) {

	char usage[] =
		"%s "
		"tick "
		"name "
		"host "
		"version "
		"sandboxed "
		"https "
		"sandbox "
		"swfv "
		"filepath "
		"amfv "
		"method "
		"size "
		"data\n";

	if (argc < 13) {
		printf(usage, argv[0]);
		return EXIT_FAILURE;
	}

	uint32_t tick = 0;
	char * name = NULL;
	char * host = NULL;
	flshm_version version = FLSHM_VERSION_1;
	bool sandboxed = false;
	bool https = false;
	uint32_t sandbox = 0;
	uint32_t swfv = 0;
	char * filepath = NULL;
	flshm_amf amfv = FLSHM_AMF0;
	char * method = NULL;
	uint32_t size = 0;
	void * data = NULL;

	if (!sscanf(argv[1], "%u", &tick) || !tick) {
		printf("ERROR: tick: %s\n", argv[1]);
		return EXIT_FAILURE;
	}
	name = argv[2];
	host = argv[3];
	version = argv[4][0] - (uint8_t)'0';
	sandboxed = argv[5][0] != '0';
	https = argv[6][0] != '0';
	if (!sscanf(argv[7], "%u", &sandbox)) {
		printf("ERROR: sandbox: %s\n", argv[7]);
		return EXIT_FAILURE;
	}
	if (!sscanf(argv[8], "%u", &swfv)) {
		printf("ERROR: swfv: %s\n", argv[8]);
		return EXIT_FAILURE;
	}
	filepath = argv[9];
	if (!sscanf(argv[10], "%u", &amfv)) {
		printf("ERROR: amfv: %s\n", argv[10]);
		return EXIT_FAILURE;
	}
	method = argv[11];
	data = hex_to_bin(argv[12], &size);

	flshm_message * message = malloc(sizeof(flshm_message));
	message->tick = tick;
	message->name = name;
	message->host = host;
	message->version = version;
	message->sandboxed = sandboxed;
	message->https = https;
	message->sandbox = sandbox;
	message->swfv = swfv;
	message->filepath = filepath;
	message->amfv = amfv;
	message->method = method;
	message->size = size;
	message->data = data;

	flshm_info * info = flshm_open(false);

	if (!info) {
		printf("FAILED: flshm_open\n");
		return EXIT_FAILURE;
	}

	int ret = EXIT_SUCCESS;

	// Lock memory, to avoid race conditions.
	flshm_lock(info);

	if (!flshm_message_write(info, message)) {
		printf("FAILED: flshm_message_write\n");
		ret = EXIT_FAILURE;
	}

	// Unlock memory.
	flshm_unlock(info);

	flshm_close(info);

	return ret;
}
