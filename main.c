#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <sys/time.h>

#include "direct_fourier_transform.h"

int main(int argc, char **argv)
{
	// Seed random from time
	srand(time(NULL));

	Config config;
	initConfig(&config);

	Source *sources = NULL;
	loadSources(&config, &sources);
	if(sources == NULL)
	{	
		printf(">>> ERROR: Source memory was unable to be allocated...\n\n");
		return EXIT_FAILURE;
	}

	Visibility *visibilities = NULL;
	Complex *visIntensity = NULL;
	loadVisibilities(&config, &visibilities, &visIntensity);

	if(visibilities == NULL || visIntensity == NULL)
	{	
		printf(">>> ERROR: Visibility memory was unable to be allocated...\n\n");
		if(sources)      	   free(sources);
		if(visibilities)       free(visibilities);
		if(visIntensity)      free(visIntensity);
		return EXIT_FAILURE;
	}

	extract_visibilities(&config, sources, visibilities, visIntensity, config.numVisibilities);

	// Save visibilities to file
	saveVisibilities(&config, visibilities, visIntensity);

	// Clean up
	if(visibilities)  free(visibilities);
	if(sources)       free(sources);
	if(visIntensity) free(visIntensity);

	printf(">>> INFO: Direct Fourier Transform operations complete, exiting...\n\n");

	return EXIT_SUCCESS;
}
