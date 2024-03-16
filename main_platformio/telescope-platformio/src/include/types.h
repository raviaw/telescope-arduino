#include <string.h>
// Reduction X Axis = 1/ 198
// Reduction Z Axis = 1/ 33

// DEC -> Declination
// RA -> Right Ascension
// ALT -> Altitude
// AZM - Azimuth
typedef struct {
  String name; // name of the target
  
  // Predefined
  double DEC_degrees, DEC_arcmin, DEC_arcsec;
  // Predefined
  double RA_hour, RA_min, RA_sec;
  
  // Calculated
  double DEC_decimal, RA_decimal;   // declination and right ascension
  // Calculated
  double HA_decimal;
  // Calculated
  double ALT_decimal, AZM_decimal;  // altitude and azimuth
} target;
