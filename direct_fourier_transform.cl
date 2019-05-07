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
