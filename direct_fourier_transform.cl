#pragma OPENCL EXTENSION cl_khr_fp64 : enable
typedef __global struct {double x,y,z;} double_3;
typedef __global struct {double x,y;} double_2;

__kernel void DFT_OpenCL(__global double_3* visibility, __global double_2* visIntensity, int visCount, __global double_3* sources, int sourceCount)
{
	int visibilityIndex = get_global_id(0);

	if(visibilityIndex >= visCount)
		return;

	double term = 0.0;
	double w_correction = 0.0;
	double image_correction = 0.0;
	double theta = 0.0;
	double src_correction = 0.0;
        
	const double two_PI = 3.14159265358979323846 + 3.14159265358979323846;

	// For all sources
	for(int s = 0; s < sourceCount; ++s)
	{
		term = 0.5 * (sources[s].x * sources[s].x + sources[s].y * sources[s].y);
		w_correction = -term;
		image_correction = 1.0 - term;
		src_correction = sources[s].z / image_correction;

		theta = (visibility[visibilityIndex].x * sources[s].x + visibility[visibilityIndex].y * sources[s].y + visibility[visibilityIndex].z * w_correction) * two_PI;

		visIntensity[visibilityIndex].x += cos(theta) * src_correction;
		visIntensity[visibilityIndex].y += -sin(theta) * src_correction;
	}
}
