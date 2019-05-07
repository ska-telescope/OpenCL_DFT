#define PROGRAM_FILE "../direct_fourier_transform.cl"
#define KERNEL_FUNC "DFT_OpenCL"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <float.h>
#include <string.h>
#include <time.h>
//#include <device_launch_parameters.h>
#include "direct_fourier_transform.h"
#ifdef MAC
#include <OpenCL/cl.h>
#else
#include <CL/cl.h>
#endif

/* Find a GPU or CPU associated with the first available platform

The `platform` structure identifies the first platform identified by the
OpenCL runtime. A platform identifies a vendor's installation, so a system
may have an NVIDIA platform and an AMD platform.

The `device` structure corresponds to the first accessible device
associated with the platform. Because the second parameter is
`CL_DEVICE_TYPE_GPU`, this device must be a GPU.
*/
cl_device_id create_device() {

	cl_platform_id platform;
	cl_device_id dev;
	int err;

	/* Identify a platform */
	err = clGetPlatformIDs(1, &platform, NULL);
	if (err < 0) {
		perror("Couldn't identify a platform");
		exit(1);
	}

	// Access a device
	// GPU
	err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &dev, NULL);
	if (err == CL_DEVICE_NOT_FOUND) {
		// CPU
		err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_CPU, 1, &dev, NULL);
	}
	if (err < 0) {
		perror("Couldn't access any devices");
		exit(1);
	}

	return dev;
}

/* Create program from a file and compile it */
cl_program build_program(cl_context ctx, cl_device_id dev, const char* filename) {

	cl_program program;
	FILE *program_handle;
	char *program_buffer, *program_log;
	size_t program_size, log_size;
	int err;

	/* Read program file and place content into buffer */
	program_handle = fopen(filename, "r");
	if (program_handle == NULL) {
		perror("Couldn't find the program file");
		exit(1);
	}
	fseek(program_handle, 0, SEEK_END);
	program_size = ftell(program_handle);
	rewind(program_handle);
	program_buffer = (char*)malloc(program_size + 1);
	program_buffer[program_size] = '\0';
	fread(program_buffer, sizeof(char), program_size, program_handle);
	fclose(program_handle);

	/* Create program from file

	Creates a program from the source code.
	Specifically, the code reads the file's content into a char array
	called program_buffer, and then calls clCreateProgramWithSource.
	*/
	program = clCreateProgramWithSource(ctx, 1,
		(const char**)&program_buffer, &program_size, &err);
	if (err < 0) {
		perror("Couldn't create the program");
		exit(1);
	}
	free(program_buffer);

	/* Build program

	The fourth parameter accepts options that configure the compilation.
	These are similar to the flags used by gcc. For example, you can
	define a macro with the option -DMACRO=VALUE and turn off optimization
	with -cl-opt-disable.
	*/
	err = clBuildProgram(program, 0, NULL, NULL, NULL, NULL);
	if (err < 0) {

		/* Find size of log and print to std output */
		clGetProgramBuildInfo(program, dev, CL_PROGRAM_BUILD_LOG,
			0, NULL, &log_size);
		program_log = (char*)malloc(log_size + 1);
		program_log[log_size] = '\0';
		clGetProgramBuildInfo(program, dev, CL_PROGRAM_BUILD_LOG,
			log_size + 1, program_log, NULL);
		printf("%s\n", program_log);
		free(program_log);
		exit(1);
	}

	return program;
}

void initConfig(Config *config)
{
	/// Number of sources to process
	config->numSources = 1;

	// Number of visibilities per source
	config->numVisibilities = 1;

	// Use fixed sources (not from file)
	config->synthetic_sources = 0;

	// Use fixed visibilities (not from file)
	config->synthetic_visibilities = 0;

	// if using synthetic visibility creation, set this flag to Gaussian distribute random visibility positions
	config->gaussian_distribution_sources = 0;

	// Disregard visibility w coordinate during transformation
	config->force_zero_w_term = 0;

	// Origin of Sources
	config->source_file = "../500_synthetic_sources.csv";

	// Source of Visibilities
	config->vis_src_file = "../sample_10k_vis_input.csv";

	// Destination of Visibilities
	config->vis_file = "../DFT_visibilities.txt";

	// Dimension of Fourier domain grid
	config->grid_size = 1024.0;

	// Fourier domain grid cell size in radians
	config->cell_size = 4.848136811095360e-06;

	// Frequency of visibility uvw terms
	config->frequency_hz = 300000000.0;

	// Scalar for visibility coordinates
	config->uv_scale = config->grid_size * config->cell_size;

	// Range for visibility u coordinates
	config->min_u = -(config->grid_size / 2.0);
	config->max_u = config->grid_size / 2.0;

	// Range for visibility v coordinates
	config->min_v = -(config->grid_size / 2.0);
	config->max_v = config->grid_size / 2.0;

	config->enable_messages = 1;
}

void loadVisibilities(Config *config, Visibility **visibilities, Complex **visIntensity)
{
	if (config->synthetic_visibilities)
	{
		if(config->enable_messages)
			printf(">>> UPDATE: Using synthetic Visibilities...\n\n");

		*visibilities = (Visibility*)calloc(config->numVisibilities, sizeof(Visibility));
		if (*visibilities == NULL)  return;

		*visIntensity = (Complex*)calloc(config->numVisibilities, sizeof(Complex));
		if (*visIntensity == NULL)
		{
			if (*visibilities) free(*visibilities);
			return;
		}

		double gV = 1.0;
		double gU = 1.0;
		//try randomize visibilities in the center of the grid
		for (int i = 0; i < config->numVisibilities; ++i)
		{
			if (config->gaussian_distribution_sources)
			{
				gV = sampleNormal();
				gU = sampleNormal();
			}
			double u = randomInRange(config->min_u, config->max_u) * gU;
			double v = randomInRange(config->min_v, config->max_v) * gV;
			double w = randomInRange(config->min_v / 10.0, config->max_v / 10.0) * gV;
			(*visibilities)[i] = (Visibility) { .u = u / config->uv_scale, .v = v / config->uv_scale, .w = w / config->uv_scale };
		}

		printf("Total vis: %d\n ", config->numVisibilities);

	}
	else // Reading visibilities from file
	{
		if(config->enable_messages)
			printf(">>> UPDATE: Using Visibilities from file...\n\n");

		FILE *file = fopen(config->vis_src_file, "r");
		if (file == NULL)
		{
			printf(">>> ERROR: Unable to locate visibilities file...\n\n");
			return;
		}

		// Reading in the counter for number of visibilities
		fscanf(file, "%d\n", &(config->numVisibilities));

		*visibilities = (Visibility*)calloc(config->numVisibilities, sizeof(Visibility));
		*visIntensity = (Complex*)calloc(config->numVisibilities, sizeof(Complex));

		// File found, but was memory allocated?
		if (*visibilities == NULL || *visIntensity == NULL)
		{
			printf(">>> ERROR: Unable to allocate memory for visibilities...\n\n");
			if (file) fclose(file);
			if (*visibilities) free(*visibilities);
			if (*visIntensity) free(*visIntensity);
			return;
		}

		double u = 0.0;
		double v = 0.0;
		double w = 0.0;
		Complex brightness;
		double intensity = 0.0;

		// Used to scale visibility coordinates from wavelengths to meters

		double wavelength_to_meters = config->frequency_hz / C;

		// Read in n number of visibilities
		for (int vis_indx = 0; vis_indx < config->numVisibilities; ++vis_indx)
		{
			// Read in provided visibility attributes
			// u, v, w, brightness (real), brightness (imag), intensity
			fscanf(file, "%lf %lf %lf %lf %lf %lf\n", &u, &v, &w,
				&(brightness.real), &(brightness.imaginary), &intensity);

			(*visibilities)[vis_indx] = (Visibility) {
				.u = u * wavelength_to_meters,
					.v = v * wavelength_to_meters,
					.w = (config->force_zero_w_term) ? 0.0 : w * wavelength_to_meters
			};
		}

		// Clean up
		fclose(file);
		if(config->enable_messages)
			printf(">>> UPDATE: Successfully loaded %d visibilities from file...\n\n", config->numVisibilities);
	}
}
void loadSources(Config *config, Source **sources)
{
	if (config->synthetic_sources)
	{
		if(config->enable_messages)
			printf(">>> UPDATE: Using synthetic Sources...\n\n");
		*sources = (Source*)calloc(config->numSources, sizeof(Source));
		if (*sources == NULL) return;
		for (int n = 0; n < config->numSources; ++n)
		{
			(*sources)[n] = (Source) {
				.l = randomInRange(config->min_u, config->max_u) * config->cell_size,
					.m = randomInRange(config->min_v, config->max_v) * config->cell_size,
					.intensity = 1.0
			};
		}
		if(config->enable_messages)
			printf(">>> UPDATE: Successfully loaded %d synthetic sources..\n\n", config->numSources);
	}
	else
	{
		if(config->enable_messages)
			printf(">>> UPDATE: Using Sources from file...\n\n");

		FILE *file = fopen(config->source_file, "r");

		// Unable to open file
		if (!file)
		{
			printf(">>> ERROR: Unable to load sources from file...\n\n");
			return;
		}

		fscanf(file, "%d\n", &(config->numSources));
		*sources = (Source*)calloc(config->numSources, sizeof(Source));
		if (*sources == NULL) return;
		double l, m, intensity;
		for (int i = 0; i < config->numSources; ++i)
		{
			fscanf(file, "%lf %lf %lf\n", &l, &m, &intensity);
			(*sources)[i] = (Source) {
				.l = l * config->cell_size,
					.m = m * config->cell_size,
					.intensity = intensity
			};
		}
		fclose(file);
		if(config->enable_messages)
			printf(">>> UPDATE: Successfully loaded %d sources from file..\n\n", config->numSources);

	}
}

void extract_visibilities(Config *config, Source *sources, Visibility *visibilities, 
	Complex *visIntensity, int numVisibilities)
{

	/* OpenCL structures */
	cl_device_id device;
	cl_context context;
	cl_program program;
	cl_kernel kernel;
	cl_command_queue queue;
	cl_int err;
	size_t global_size;

	   /* Data and buffers    */

	cl_int num_groups;


	/* Create device and context

	Creates a context containing only one device — the device structure
	created earlier.
	*/
	device = create_device();
	context = clCreateContext(NULL, 1, &device, NULL, NULL, &err);
	if (err < 0) {
		perror("Couldn't create a context");
		exit(1);
	}

	/* Build program */
	program = build_program(context, device, PROGRAM_FILE);

	/* Create a command queue

	Does not support profiling or out-of-order-execution
	*/
	queue = clCreateCommandQueue(context, device, 0, &err);
	if (err < 0) {
		perror("Couldn't create a command queue");
		exit(1);
	};


	/* Create data buffer

	• `global_size`: total number of work items that will be
	   executed on the GPU (e.g. total size of your array)
	• `local_size`: size of local workgroup. Each workgroup contains
	   several work items and goes to a compute unit

	  Notes:
	• Intel recommends workgroup size of 64-128. Often 128 is minimum to
	get good performance on GPU
	• On NVIDIA Fermi, workgroup size must be at least 192 for full
	utilization of cores
	• Optimal workgroup size differs across applications
	*/

	//Allocating GPU memory for visibility intensity
	cl_mem deviceSources;
	cl_mem deviceVisibilities;
	cl_mem deviceIntensities;

	if(config->enable_messages)
		printf(">>> UPDATE: Allocating GPU MEMORY...\n\n");

	//copy the visibilities to the GPU
	deviceVisibilities = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, numVisibilities * sizeof(double_3), visibilities, &err); // <=====INPUT

	//copy the sources to the GPU.
	deviceSources = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(double_3) * config->numSources, sources, &err); // <=====INPUT

	deviceIntensities = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR, numVisibilities * sizeof(double_2), visIntensity, &err); // <=====OUTPUT

	if (err < 0) {
		perror("Couldn't create a buffer");
		exit(1);
	};

	/* Create a kernel */
	kernel = clCreateKernel(program, KERNEL_FUNC, &err);
	if (err < 0) {
		perror("Couldn't create a kernel");
		exit(1);
	};
	/* Create kernel arguments */

	err = clSetKernelArg(kernel, 0, sizeof(cl_mem), (void *)&deviceVisibilities);
	err |= clSetKernelArg(kernel, 1, sizeof(cl_mem), (void *)&deviceIntensities);
	err |= clSetKernelArg(kernel, 2, sizeof(unsigned int), &numVisibilities);
	err |= clSetKernelArg(kernel, 3, sizeof(cl_mem), (void *)&deviceSources);
	err |= clSetKernelArg(kernel, 4, sizeof(unsigned int), &config->numSources);
	if (err < 0) {
		perror("Couldn't create a kernel argument");
		exit(1);
	}
	if(config->enable_messages)
		printf(">>> UPDATE: Calling DFT GPU Kernel...\n\n");

	/* Enqueue kernel

	   At this point, the application has created all the data structures
	   (device, kernel, program, command queue, and context) needed by an
	   OpenCL host application. Now, it deploys the kernel to a device.

	   Of the OpenCL functions that run on the host, clEnqueueNDRangeKernel
	   is probably the most important to understand. Not only does it deploy
	   kernels to devices, it also identifies how many work-items should
	   be generated to execute the kernel (global_size) and the number of
	   work-items in each work-group (local_size).
	   */

	global_size = numVisibilities;

	err = clEnqueueNDRangeKernel(queue, kernel, 1, NULL, &global_size,
		NULL, 0, NULL, NULL);

	if (err < 0) {
		perror("Couldn't enqueue the kernel");
		exit(1);
	}
	if(config->enable_messages)
		printf(">>> UPDATE: DFT GPU Kernel Completed...\n\n");
	/* Read the kernel's output    */
	err = clEnqueueReadBuffer(queue, deviceIntensities, CL_TRUE, 0, numVisibilities * sizeof(double_2), visIntensity, 0, NULL, NULL); // <=====GET OUTPUT
	if (err < 0) {
		perror("Couldn't read the buffer");
		exit(1);
	}

	if(config->enable_messages)
		printf(">>> UPDATE: Copied Visibility Data back to Host - Completed...\n\n");

	/* Deallocate resources */
	clReleaseKernel(kernel);
	clReleaseMemObject(deviceSources);
	clReleaseMemObject(deviceVisibilities);
	clReleaseMemObject(deviceIntensities);
	clReleaseCommandQueue(queue);
	clReleaseProgram(program);
	clReleaseContext(context);
}

void saveVisibilities(Config *config, Visibility *visibilities, Complex *visIntensity)
{
	// Save visibilities to file
	FILE *file = fopen(config->vis_file, "w");

	// Unable to open file
	if (!file)
	{
		printf(">>> ERROR: Unable to save visibilities to file...\n\n");
		return;
	}
	if(config->enable_messages)
		printf(">>> UPDATE: Writing visibilities to file...\n\n");

	// Record number of visibilities
	fprintf(file, "%d\n", config->numVisibilities);

	// Scalar from meters to wavelengths
	double wavelengthScalar = config->frequency_hz / C;

	// Record individual visibilities
	for (int n = 0; n < config->numVisibilities; ++n)
	{
		// u, v, w, real, imag
		fprintf(file, "%f %f %f %f %f %f\n", visibilities[n].u / wavelengthScalar,
			visibilities[n].v / wavelengthScalar,
			visibilities[n].w / wavelengthScalar,
			visIntensity[n].real,
			visIntensity[n].imaginary,
			1.0);
	}

	// Clean up
	fclose(file);
	if(config->enable_messages)
		printf(">>> UPDATE: Completed writing of visibilities to file...\n\n");
}
double randomInRange(double min, double max)
{
	double range = (max - min);
	double div = RAND_MAX / range;
	return min + (rand() / div);
}

double sampleNormal()
{
	double u = ((double)rand() / RAND_MAX) * 2 - 1;
	double v = ((double)rand() / RAND_MAX) * 2 - 1;
	double r = u*u + v*v;
	if (r <= 0 || r > 1)
		return sampleNormal();
	double c = sqrt(-2 * log(r) / r);
	if ((u * v * c) > 1.0 || (u * v * c) < -1.0)
		return sampleNormal();
	return u * v * c;
}



//      UNIT TEST     

void unit_test_init_config(Config *config)
{
	config->numSources = 1;
	config->numVisibilities = 1;
	config->synthetic_sources = 0;
	config->synthetic_visibilities = 0;
	config->gaussian_distribution_sources = 0;
	config->force_zero_w_term = 0;
	config->source_file = "../unit_test_sources.txt";
	config->vis_src_file = "../unit_test_visibilities.txt";
	config->vis_file = "../unit_test_vis_output.txt";
	config->grid_size = 1024.0;
	config->cell_size = 4.848136811095360e-06;
	config->frequency_hz = 300000000.0;
	config->uv_scale = config->grid_size * config->cell_size;
	config->min_u = -(config->grid_size / 2.0);
	config->max_u = config->grid_size / 2.0;
	config->min_v = -(config->grid_size / 2.0);
	config->max_v = config->grid_size / 2.0;
	config->enable_messages=0;
}

double unit_test_generate_approximate_visibilities(void)
{
	// used to invalidate the unit test
	double error = DBL_MAX;

	Config config;
	unit_test_init_config(&config);

	// Read in test sources
	Source *sources = NULL;
	loadSources(&config, &sources);
	if(sources == NULL)
		return error;

	// Read in test visibilities and process
	FILE *file = fopen(config.vis_src_file, "r");
	if(file == NULL)
	{
		if(sources) free(sources);
		return error;
	}

	fscanf(file, "%d\n", &(config.numVisibilities));

	double u = 0.0;
	double v = 0.0;
	double w = 0.0;
	double intensity = 0.0;
	double difference = 0.0;
	double wavelength_to_meters = config.frequency_hz / C;
	Complex brightness = (Complex) {.real = 0.0, .imaginary = 0.0};
	Complex test_vis_intensity;
	Visibility approx_visibility[1]; // testing one at a time
	Complex approx_vis_intensity[1]; // testing one at a time

	for(int vis_indx = 0; vis_indx < config.numVisibilities; ++vis_indx)
	{
		fscanf(file, "%lf %lf %lf %lf %lf %lf\n", &u, &v, &w, 
			&(brightness.real), &(brightness.imaginary), &intensity);

		test_vis_intensity.real      = brightness.real;
		test_vis_intensity.imaginary = brightness.imaginary;

		approx_visibility[0] = (Visibility) {
			.u = u * wavelength_to_meters,
			.v = v * wavelength_to_meters,
			.w = w * wavelength_to_meters
		};

		approx_vis_intensity[0] = (Complex) {
			.real      = 0.0,
			.imaginary = 0.0
		};

		// Measure one visibility brightness from n sources
		extract_visibilities(&config, sources, approx_visibility, approx_vis_intensity, 1);

		double current_difference = sqrt(pow(approx_vis_intensity[0].real
			-test_vis_intensity.real, 2.0)
			+ pow(approx_vis_intensity[0].imaginary
			-test_vis_intensity.imaginary, 2.0));

		if(current_difference > difference)
			difference = current_difference;
	}

	// Clean up
	fclose(file);
	if(sources) free(sources);

	printf(">>> INFO: Measured maximum difference of evaluated visibilities is %f\n", difference);

	return difference;
}

