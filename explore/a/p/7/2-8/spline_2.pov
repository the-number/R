// 20220810 (C) Gunter Liszewski -*- mode: pov; -*-
// spline example
#version 3.7;
#include "colors.inc"
 global_settings { assumed_gamma 1.0 }

camera { location <0,2,-2> look_at 0 }
light_source { <-5,30,-10> 1 }

#declare MySpline =
spline {
  cubic_spline
     -.25, <0,-2,-1>
    0.00, <1,-1.7,0>
    0.25, <0,-1.4,1>
    0.50, <-1,-0.9,0>
    0.75, <0,-0.3,-1>
    1.00, <1,0,0>
    1.25, <0,0.4,1>
}

#declare ctr = 0;
#while (ctr < 1)
  sphere {
    MySpline(ctr),.25
    pigment { rgb <1-ctr,ctr,0> }
    }
  #declare ctr = ctr + 0.01;
#end