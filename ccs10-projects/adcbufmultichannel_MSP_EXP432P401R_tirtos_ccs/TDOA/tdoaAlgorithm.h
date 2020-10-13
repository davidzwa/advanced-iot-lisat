#ifndef TDOA__H
#define TDOA__H

void TDOA_direction_estimation(int TDOA[2], float outputDirectionVector2D[2]);
void plane_cutting_direction_estimation(unsigned long TOA[3], float dir[2]);

#endif // !TDOA__H
