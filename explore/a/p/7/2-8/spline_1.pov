// 20220809 (C) Gunter Liszewski -*- mode: pov; -*-
// spline
#version 3.7;
#include "colors.inc"
 global_settings { assumed_gamma 1.0 }

#macro Foo( Bar, Val )
  #declare Y = Bar(Val).y;
#end

#declare myspline = 
  spline { 
    1, <4,5> 
    3, <5,5> 
    5, <6,5>
    }

Foo(myspline, 2)

#debug str(Y,5,5)
#debug "\n"
