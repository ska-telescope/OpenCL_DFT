// Copyright 2019 Compucon New Zealand

// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:

// 1. Redistributions of source code must retain the above copyright notice,
// this list of conditions and the following disclaimer.

// 2. Redistributions in binary form must reproduce the above copyright
// notice, this list of conditions and the following disclaimer in the
// documentation and/or other materials provided with the distribution.

// 3. Neither the name of the copyright holder nor the names of its
// contributors may be used to endorse or promote products derived from this
// software without specific prior written permission.

// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.

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
