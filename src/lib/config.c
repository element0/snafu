#include <stdlib.h>
#include <string.h>
#include "config.h"

struct cosmos_program_config *cosmos_program_defaults() {
	struct cosmos_program_config *conf = malloc(sizeof(*conf));
	memset(conf, 0, sizeof(*conf));

	conf->cosmosroot = "/home/raygan/test/cosmos";

	return conf;
}
