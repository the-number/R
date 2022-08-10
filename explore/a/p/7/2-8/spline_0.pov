// 20220809 (C) Gunter Liszewski -*- mode: pov; -*-
// spline
#version 3.7;
#include "colors.inc"
 global_settings { assumed_gamma 1.0 }

camera { location <0,2,-2> look_at 0 }
light_source { <-5,30,-10> 1 }

#declare MySpline =
spline {
  cubic_spline
    -.25, <0,0,-1>
    0.00, <1,0,0>
    0.25, <0,0,1>
    0.50, <-1,0,0>
    0.75, <0,0,-1>
    1.00, <1,0,0>
    1.25, <0,0,1>
    }

#declare ctr = 0;
#while (ctr < 1)
  sphere {
    MySpline(ctr),.25
    pigment { rgb <1-ctr,ctr,0> }
    }
  #declare ctr = ctr + 0.01;
#end