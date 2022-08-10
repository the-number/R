// 20220810 (C) Gunter Liszewski -*- mode: pov; -*-
// spline example
#version 3.7;
#include "colors.inc"
 global_settings { assumed_gamma 1.0 }

camera { location <0,2,-10> look_at 0 }
light_source { <-5,30,-10> 1 }

#macro Side( Colour, P1, P2, P3, P4)
polygon {  4, P1, P2, P3, P4
  texture{ finish  { specular 0.6 }
  	   pigment { colour Colour }
	   normal  { agate 0.25 scale 1/2 } } }
#end
#declare F = Side(Red,     <0,0,0>, <0,1,0>, <1,1,0>, <1,0,0>)
#declare B = Side(Green,   <0,0,1>, <0,1,1>, <1,1,1>, <1,0,1>)
#declare U = Side(Blue,    <0,1,0>, <0,1,1>, <1,1,1>, <1,1,0>)
#declare D = Side(Cyan,    <0,0,0>, <0,0,1>, <1,0,1>, <1,0,0>)
#declare L = Side(Magenta, <0,0,0>, <0,0,1>, <0,1,1>, <0,1,0>)
#declare R = Side(Yellow,  <1,0,0>, <1,1,0>, <1,1,1>, <1,0,1>)

#declare cubelet = union {
  object { F }
  object { U }
  object { R }
  object { B }
  object { D }
  object { L }
  translate <-0.5, -0.5, -0.5>
}

#declare that_cube = union { 
// <X,Y,Z> = <right,up,back> , we progress in a circular motion
// front and left and up
  object { cubelet rotate <0,0,-90>
                   translate <-1.05,1.05,-1.05> } // corner
  object { cubelet rotate <0,0,-90>
                   translate <0,    1.05,-1.05> } // edge
// front and up and right
  object { cubelet rotate <0,0,-90> 
                   translate <1.05, 1.05,-1.05> } // corner
  object { cubelet rotate <0,0,-90>
                   translate <1.05, 0,   -1.05> } // edge
// down and right
  object { cubelet rotate <0,0,-90>
                   translate <1.05,-1.05,-1.05> } // corner
  object { cubelet rotate <0,0,-90>
                   translate <0,   -1.05,-1.05> } // edge
// down and left
  object { cubelet rotate <0,0,-90>
                   translate <-1.05,-1.05,-1.05> } // corner
  object { cubelet rotate <0,0,-90>
                   translate <-1.05,0,    -1.05> } // edge  
// front centre
  object { cubelet rotate <0,0,-90>
                   translate <0,0,-1.05> }        // front centre
// back-right-up
  object { cubelet translate <1.05,  1.05,1.05> } // corner
  object { cubelet translate <0,     1.05,1.05> } // edge
// back-down-right
  object { cubelet translate <1.05, -1.05,1.05> } // corner
  object { cubelet translate <0,    -1.05,1.05> } // edge
// back-left
  object { cubelet translate <-1.05, 1.05,1.05> } // corner
  object { cubelet translate <-1.05, 0,   1.05> } // edge
// back-left-down
  object { cubelet translate <-1.05, -1.05,1.05> } // corner
  object { cubelet translate <1.05,  0,    1.05> } // edge  
// back centre
  object { cubelet translate <0,    0,   1.05> } // back centre

// slice in the middle of front and left
// up-left
  object { cubelet translate <-1.05,1.05,0> } // corner
  object { cubelet translate <0,    1.05,0> } // edge
// up-right
  object { cubelet translate <1.05, 1.05, 0> } // corner
  object { cubelet translate <1.05, 0,    0> }  // edge
// down-right
  object { cubelet translate <1.05,-1.05,0> }  // corner
  object { cubelet translate <0,   -1.05,0> }  // edge
// down-left
  object { cubelet translate <-1.05,-1.05,0> } // corner
  object { cubelet translate <-1.05,0,  0> }   // edge
// cube invisible centre
  object { cubelet }
}

#macro Mirror( Colour )
  box { <0,0,0>, <10,4,4>
    pigment { colour Colour } finish { reflection 1 } }
#end

// The things in this picture
union {
  object { Mirror( Blue ) rotate y*87 translate <-10,0,0> }
  object { that_cube }
  object { cubelet rotate y*-30 translate <-4,0,-4> }
  translate <3,0,1> rotate <131,122,133> }
