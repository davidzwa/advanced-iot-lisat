#include "tdoaAlgorithm.h"
#include <math.h> 

// Valin
void TDOA_direction_estimation(int TDOA[2], float dir[2])
{
    // TDOA = array of time differences
    // dir = direction array (estimate array will be saved there)

//    [[ 0. -9.]
//     [-9.  7.]
//     [ 9.  7.]]
    const float inverse[2][2] = {{ -0.055555555555555566, 0.05555555555555555},{ 0.03125, 0.031249999999999993}};
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

// CTP
void plane_cutting_direction_estimation(unsigned long TOA[3], float dir[2])
{
    float X_pair[3][2] = {{ -0.49026123963255896, 0.8715755371245493},{ 0.49026123963255896, 0.8715755371245493},{ 1.0, 0.0}};
    float mag;
    int k = 0;

    dir[0] = dir[1] = 0;

    for (int i = 0; i < 3; i++)
    {
        for (int j = i + 1; j < 3; j++)
        {
            if (TOA[i] > TOA[j])
            {
                dir[0] += X_pair[k][0];
                dir[1] += X_pair[k][1];
            }
            else
            {
                dir[0] -= X_pair[k][0];
                dir[1] -= X_pair[k][1];
            }
            k++;
        }
    }

    // normalize vector
    mag = sqrt(dir[0] * dir[0] + dir[1] * dir[1]);
    dir[0] = dir[0] / mag;
    dir[1] = dir[1] / mag;
}
