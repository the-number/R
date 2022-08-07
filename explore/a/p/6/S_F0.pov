#version 3.7; // 20220807 (C) Gunter Liszewski -*- mode: povray; -*-
// S_F=(FFbb), F, and the 3x3x3 standard permutation
// POV-ray, to make the Rubik permutations visible, cublets/face 

 #include "colors.inc"
 global_settings { assumed_gamma 1.0 }
 background   { color rgb <0.0025, 0.0025, 0.0025> }
 camera       { location  <50.0, 0.5, -3.0>
                direction 1.5*z
                right     x*image_width/image_height
                // look_at   <0.0, 0.0, 0.0>
                look_at   <0.0, -8.0, -6.0> }
 light_source { <0, 0, 0>
                color rgb <1, 1, 1>
                translate <-5, 5, -5> }
 light_source { <0, 0, 0>
                color rgb <0.25, 0.25, 0.25>
                translate <6, -6, -6> }

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
#declare three = union {
  object { cubelet }
  object { cubelet translate y*-1.05 }
  object { cubelet translate y*+1.05 }
}
#declare face = union {
  object { three }
  object { three translate x*-1.05 }
  object { three translate x*+1.05 }
}
#declare standard_permutation = union {
  object { face }
  object { face translate z*-1.05 }
  object { face translate z*+1.05 }
}
#declare standard_permutation_S_F = union {
  object { face             // front face
           rotate <0,0,-90> // *F
	   rotate <0,0,-90> // *F (again)
  	   translate z*-1.05 }
  object { face }           // slice S_F
  object { face             // back face
  	   rotate <0,0,90>  // *b
  	   rotate <0,0,90>  // *b
  	   translate z*+1.05
	   }
  rotate <0,0,-180>
}
#declare this_cube = union { standard_permutation }
#declare this_cube_S_F = union { standard_permutation_S_F }
/* Start another, aiming to permute the cublets of a face,
   for example to do move F we rotate each of F's cubelets
*/
#declare that_cube = union { 
  // the front slice
  object { cubelet translate <1.05,1.05,-1.05> } // up, front
  object { cubelet translate <1.05,1.05,0> }
  object { cubelet translate <1.05,1.05,1.05> }

  object { cubelet translate <1.05,0,-1.05> } // centre, front
  object { cubelet translate x*1.05 }
  object { cubelet translate <1.05,0,1.05> }

  object { cubelet translate <1.05,-1.05,-1.05> } // down,front
  object { cubelet translate <1.05,-1.05,0> }
  object { cubelet translate <1.05,-1.05,1.05> }
  
  // in the middle
  
  object { cubelet translate <0,1.05,-1.05> } // up,middle
  object { cubelet translate <0,1.05,0> }
  object { cubelet translate <0,1.05,1.05> }

//  object { cubelet translate <0,0,-1.05> } // centre,middle
  object { cubelet }
//  object { cubelet translate <0,0,1.05> } // centre, right

  object { cubelet translate <0,-1.05,-1.05> } // down,middle
  object { cubelet translate <0,-1.05,0> }
  object { cubelet translate <0,-1.05,1.05> }
  
  // the back slice
  
  object { cubelet translate <-1.05,1.05,-1.05> } // up, back
  object { cubelet translate <-1.05,1.05,0> }
  object { cubelet translate <-1.05,1.05,1.05> }

  object { cubelet translate <-1.05,0,-1.05> } // centre, back
  object { cubelet translate <-1.05,0,0> }
  object { cubelet translate <-1.05,0,1.05> }

  object { cubelet translate <-1.05,-1.05,-1.05> } // down,back
  object { cubelet translate <-1.05,-1.05,0> }
  object { cubelet translate <-1.05,-1.05,1.05> }
  
}

// The things in this picture
union {
  object { this_cube_S_F translate <0,0,-1> }
  object { this_cube rotate z*90
	   rotate y*23
           translate <-1.9,0,4> }
  object { this_cube rotate x*-90 rotate z*-90
	   rotate y*-31 translate <-4,0,-1> }
  object { that_cube rotate y*90
           translate <10,0,-10> }
  object { that_cube rotate x*90
           translate <15,0,-13> }
  object { that_cube rotate x*90 rotate y*180
           translate <18,-3,-9> }

  translate <-3,-4,2> rotate <131,144,133> }
