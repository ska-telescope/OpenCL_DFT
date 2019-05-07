#ifndef CONFIG_H_
#define CONFIG_H_

//=========================//
// Algorithm Configurables //
//=========================//

// Speed of light
#ifndef C
	#define C 299792458.0
#endif

//=========================//
//        Structures       //
//=========================//


typedef struct Config {
	int numVisibilities;
	int numSources;
	const char *source_file;
	const char *vis_src_file;
	const char *vis_file;
	int force_zero_w_term;
	int synthetic_sources;
	int synthetic_visibilities;
	int gaussian_distribution_sources;
	double min_u;
	double max_u;
	double min_v;
	double max_v;
	double grid_size;
	double cell_size;
	double uv_scale;
	double frequency_hz;
	int enable_messages;
} Config;

typedef struct Complex {
	double real;
	double imaginary;
} Complex;

typedef struct Source {
	double l;
	double m;
	double intensity;
} Source;

typedef struct Visibility {
	double u;
	double v;
	double w;
} Visibility;

typedef struct {
	double x,y,z;
} double_3;

typedef struct {
	double x,y;
} double_2;

//=========================//
//     Function Headers    //
//=========================//
void initConfig (Config *config);
void loadSources(Config *config, Source **sources);
void loadVisibilities(Config *config, Visibility **visibilities, Complex **visIntensity);
void extract_visibilities(Config *config, Source *sources, Visibility *visibilities, Complex *vis_intensity, int num_visibilities);
void saveVisibilities(Config *config, Visibility *visibilities, Complex *visIntensity);
double randomInRange(double min, double max);
double sampleNormal();
void unit_test_init_config(Config *config);
double unit_test_generate_approximate_visibilities();
#endif /* CONFIG_H_ */
