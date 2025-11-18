#ifndef GRIDS_H
#define GRIDS_H

#include <QString>


float grids_dist(float lat1, float lon1, float lat2, float lon2);
void grids_maidenhead2latlon ( QString maidenhead, float *lat, float *lon);

#endif // GRIDS_H
