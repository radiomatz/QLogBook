#include "grids.h"
#include <math.h>
#include <QDebug>


float grids_dist(float lat1, float lon1, float lat2, float lon2) {

/**
 * (c)opyright by Conny Henn - www.DL2FBO.de
 * @Berechnung der Entfernung zwischen zwei Längen- und Breitengraden
 * @license GPL 2.0
 Source: https://afu-base.de/locator-berechnen-2/#Javascript_Berechnung_der_Entfernung_zwischen_zwei_Locator-Codes
 */
    float dist = 0.0;
    float r = 6371.0; // radius of our wonderful planet terra
    const float PI = 3.14159265;

    float latRad1 = lat1 * (PI / 180);
    float lonRad1 = lon1 * (PI / 180);
    float latRad2 = lat2 * (PI / 180);
    float lonRad2 = lon2 * (PI / 180);

    float dlon = lonRad2 - lonRad1;
    float dlat = latRad2 - latRad1;
    float a =
        sin(dlat / 2) * sin(dlat / 2) +
        cos(latRad1) *
        cos(latRad2) *
        sin(dlon / 2) * sin(dlon / 2);
    float c = 2 * atan2(sqrt(a), sqrt(1 - a));
    dist = r * c;
//  with dist := radius given above, there in km
    return(dist);
}

void grids_maidenhead2latlon ( QString maidenhead, float *lat, float *lon) {
    *lat = *lon = 0.0;
    if ( maidenhead.length() < 4 )
       return;
    int dgslon = 12;
    int dgslat = 12;
    char clon = maidenhead.at(0).toLower().toLatin1();
    char clat = maidenhead.at(1).toLower().toLatin1();
    char gslon = maidenhead.at(2).toLower().toLatin1();
    char gslat = maidenhead.at(3).toLower().toLatin1();
    if ( maidenhead.length() > 5 ) {
        dgslon = maidenhead.at(4).toLower().toLatin1() - 'a';
        dgslat = maidenhead.at(5).toLower().toLatin1() - 'a';
    }
    if ( ( clon < 'a' ) || ( clon > 'r') )
        return;
    if ( ( clat < 'a' ) || ( clat > 'r') )
        return;
    if ( (gslon < '0') || ( gslon > '9') )
        return;
    if ( (gslat < '0') || ( gslat > '9') )
        return;

    int ilon = clon - 'a';
    int ilat = clat - 'a';
    int icglon = gslon - '0';
    int icglat = gslat - '0';

    *lon = ilon * 20.0 + icglon * 2.0 + dgslon / 12.0;
    *lon = *lon - 180.0;

    *lat = ilat * 10.0 + icglat + dgslat / 24.0;
    *lat = *lat - 90.0;
}


/**
 * (c)opyright by Conny Henn - www.DL2FBO.de
 * @Berechnung der Entfernung zwischen zwei Längen- und Breitengraden
 * @license GPL 2.0
 */

/*
// Hauptfunktion: Berechnet die Entfernung zwischen zwei geografischen Punkten
function calculateDistance(lat1, lon1, lat2, lon2) {
  var latRad1 = lat1 * (Math.PI / 180);
  var lonRad1 = lon1 * (Math.PI / 180);
  var latRad2 = lat2 * (Math.PI / 180);
  var lonRad2 = lon2 * (Math.PI / 180);

  var dlon = lonRad2 - lonRad1;
  var dlat = latRad2 - latRad1;
  var a =
    Math.sin(dlat / 2) * Math.sin(dlat / 2) +
    Math.cos(latRad1) *
      Math.cos(latRad2) *
      Math.sin(dlon / 2) *
      Math.sin(dlon / 2);
  var c = 2 * Math.atan2(Math.sqrt(a), Math.sqrt(1 - a));
  var distance = 6371 * c; // Radius der Erde in Kilometern

  return distance.toFixed(2); // Runde auf 2 Dezimalstellen
}
*/
