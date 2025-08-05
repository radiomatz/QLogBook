#ifndef GRIDS_H
#define GRIDS_H

#include <QString>

class grids
{
public:
    grids();
    float dist(float lat1, float lat2, float lon1, float lon2);
    void maidenhead2latlon ( QString maidenhead, float *lat, float *lon);
};

#endif // GRIDS_H
