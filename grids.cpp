#include "grids.h"

grids::grids() {

}

float grids::dist(float lat1, float lat2, float lon1, float lon2) {
    float dLat = lat2 - lat1;
    float dLon = lon2 - lon1;
    float a = pow( sin(dLat / 2.0), 2) + pow( sin(dLon /2.0), 2) * cos(lat1) * cos(lat2);
    float dist = 6378.388 * 2.0 * atan2(sqrt(a), sqrt(1.0-a));
//  mit dist: Entfernung in km
    return(dist);
}

void grids::maidenhead2latlon ( QString maidenhead, float *lat, float *lon) {

}
