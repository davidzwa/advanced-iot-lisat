#include "tdoaAlgorithm.h"
#include <math.h> 

// Function : 
void TDOA_direction_estimation(int TDOA[2], float dir[2])
{
    // TDOA = array of time differences
    // dir = direction array (estimate array will be saved there)

    const float inverse[2][2] = {{4.000000000000001, 7.999999999999998}, {0.0, 0.0}};
    float mag;

    dir[0] = dir[1] = 0;

    // matrix multiplication
    for (int i = 0; i < 2; i++)
    {
        dir[0] += inverse[0][i] * TDOA[i];
        dir[1] += inverse[1][i] * TDOA[i];
    }

    // normalize vector
    mag = sqrt(dir[0] * dir[0] + dir[1] * dir[1]);
    dir[0] = dir[0] / mag;
    dir[1] = dir[1] / mag;
}
