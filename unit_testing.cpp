#include <cstdlib>
#include <cstdio>
#include <gtest/gtest.h>

#include "direct_fourier_transform.h"
#include "direct_fourier_transform.c"

// Test performs DFT on a fixed set of sources, produces a set of visibilities using said sources
// and compares these visibilities against a set of correct visibilities for these sources.
// A threshold is used to determine acceptance as equality is not an ideal assertion for
// non-integer numbers due to rounding error.
//**************************************//
//      UNIT TESTING FUNCTIONALITY      //
//**************************************//

TEST(DFTTest, VisibilitiesApproximatelyEqual)
{
	double threshold = 1e-5; // 0.00001
	double difference = unit_test_generate_approximate_visibilities();
	ASSERT_LE(difference, threshold); // diff <= threshold
}

int main(int argc, char **argv) {
	testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
