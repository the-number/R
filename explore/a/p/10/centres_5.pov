// 20220821 (C) Gunter Liszewski -*- mode: pov; -*-
// Rubik cube's six centre cubelets
#version 3.7;
#include "colors.inc"
 global_settings { assumed_gamma 1.0 }

camera { location <0,2,-14> look_at <0,0,2> }
light_source { <-30,30,-10> 1 }

#macro Side( Colour, P1, P2, P3, P4)
  polygon {  5, P1 P2 P3 P4 P1
    texture{ finish  { specular 0.6 }
      pigment { colour Colour }
      normal  { agate 0.25 scale 1/2 }}}
#end
// We want a cubelet of unit square facelets
#declare F = Side(Red,     <-1,-1,-1>, <-1,1,-1>, <1,1,-1>, <1,-1,-1>)
#declare B = Side(Green,   <-1,-1,1>, <-1,1,1>, <1,1,1>, <1,-1,1>)
#declare U = Side(Blue,    <-1,1,-1>, <-1,1,1>, <1,1,1>, <1,1,-1>)
#declare D = Side(Cyan,    <-1,-1,-1>, <-1,-1,1>, <1,-1,1>, <1,-1,-1>)
#declare L = Side(Magenta, <-1,-1,-1>, <-1,-1,1>, <-1,1,1>, <-1,1,-1>)
#declare R = Side(Yellow,  <1,-1,-1>, <1,1,-1>, <1,1,1>, <1,-1,1>)

#macro Spin(a,b)
  #switch (a)
    #case (0) 
      rotate z*(b=2?0:(b=3?180:(b=4?-90:(b=5?90:0))))
    #break
    #case (1)
      rotate y*180
      rotate z*(b=2?0:(b=3?180:(b=4?90:(b=5?-90:0))))
    #break
    #case (2) 
      rotate x*-90 
      rotate z*(b=0?180:(b=1?0:(b=4?-90:(b=5?90:0))))      
    #break
    #case (3) 
      rotate x*90
      rotate z*(b=0?0:(b=1?180:(b=4?-90:(b=5?90:0))))      
    #break
    #case (4) 
      rotate y*-90
      rotate z*(b=0?90:(b=1?-90:(b=2?0:(b=3?180:0))))      
    #break
    #case (5) 
      rotate y*90
      rotate z*(b=0?-90:(b=1?90:(b=2?0:(b=3?180:0))))      
    #break
  #end
#end

#macro Cubelet(a,b)
  union {
  object { F }
  object { U }
  object { R }
  object { B }
  object { D }
  object { L }
    Spin(a,b)
  }
#end
#declare standard_cubelet = Cubelet(0,2);

#macro edge(a,b)
  #local X=0;
  #local Y=0;
  #local Z=0;
  #switch (a)
    #case (0) #local X=-1;
    #break
    #case (1) #local X=1;
    #break
    #case (2) #local Y=1;
    #break
    #case (3) #local Y=-1;
    #break
  #else
    #debug "What is it on this edge A?"
  #end
  #switch (b)
    #case (2) #local Y=1;
    #break
    #case (3) #local Y=-1;
    #break
    #case (4) #local Z=-1;
    #break
    #case (5) #local Z=1;
    #break
  #else
    #debug "What about this face on edge B?"
  #end
  object { Cubelet(0,2) translate <2*X,2*Y,2*Z> }
#end

#macro corner(a,b,c)
  #local X=1; // front, or FUR, or 025
  #local Y=1; //        up
  #local Z=1; //        right
  #switch (a)
    #case (0) #local X=-1;
    #break
    #case (1) #local X=1;
    #break
  #else
    #debug "What is it this corner A?"
  #end
  #switch (b)
    #case (2) #local Y=1;
    #break
    #case (3) #local Y=-1;
    #break
    #case (4) #local Z=-1;
    #break
    #case (5) #local Z=1;
    #break
  #else
    #debug "What about corner B?"
  #end
    #switch (c)
    #case (2) #local Y=1;
    #break
    #case (3) #local Y=-1;
    #break
    #case (4) #local Z=-1;
    #break
    #case (5) #local Z=1;
    #break
  #else
    #debug "Where it the face C?"
  #end
  object { standard_cubelet translate <2*X,2*Y,2*Z> }
#end

#macro centre(a,b,c)
  #local X=0;
  #local Y=0;
  #local Z=0;
  #switch (a)
    #case (0) #local X=-1;
    #break
    #case (1) #local X=1;
    #break
    #case (2) #local Y=1;
    #break
    #case (3) #local Y=-1;
    #break
    #case (4) #local Z=-1;
    #break
    #case (5) #local Z=1;
    #break
  #else
    #debug "What is it at this centre A?"
  #end
  object {  Cubelet(b,c)
    translate <X*2,Y*2,Z*2> }  
#end

#declare standard_edges = union {
// y*0 x*0
  edge(0,2) // front up
  edge(0,3) //       down
  edge(0,4) //       left
  edge(0,5) //       right
// y*180  
  edge(1,2) // back  up
  edge(1,3) //       down
  edge(1,4) //       left
  edge(1,5) //       right
// x*-90  
  edge(2,4) // up    left
  edge(2,5) //       right
// x*90  
  edge(3,4) // down  left
  edge(3,5) //       right
} // standard_edges
#declare standard_corners = union {
// y*0
  corner(0,4,2) // front left up
  corner(0,2,5) //       up right
  corner(0,5,3) //       right down
  corner(0,3,4) //       down left
// y*180
  corner(1,5,2) // back right up
  corner(1,2,4) //      up left
  corner(1,4,3) //      left down
  corner(1,3,5) //      down right
} // standard_corners

#macro centres(a,b)
  union {
// y*0 x*0
  centre(0,a,b) // front
  centre(1,a,b) // back
  centre(2,a,b) // up
  centre(3,a,b) // down
  centre(4,a,b) // left
  centre(5,a,b) // right
} // centres
#end
#declare standard_centres = centres(0,2);

#macro Mirror( Colour )
  box { <0,0,0>, <10,4.5,0.3>
    pigment { colour Colour } finish { reflection 1 } }
#end

//--------------------------------------------------
// reorientation macro, from abyss.pov in 3.7 scenes
//--------------------------------------------------
#macro mOrient(P1,P2)
#local yV1=vnormalize(P2-P1);
#local xV1=vnormalize(vcross(yV1,z));
#local zV1=vcross(xV1,yV1);
                matrix <xV1.x,xV1.y,xV1.z,yV1.x,yV1.y,yV1.z,zV1.x,zV1.y,zV1.z,P1.x,P1.y,P1.z>
#end

#macro Matrix()
  matrix
<   
//  1,0,0, 0,1,0, 0,0,1, 0,0,0 // additive zero matrix-transform (implied column 0,0,0,1)
// our experiments
    // 1.3,0.6,0,    0,1,0, 0,0,1,   -1,0,0
  1.4, 0.6,   0, // 0, --- 3x3 rotation matrix
 -1.3,   1,   0, // 0, --- (V*T)_x=v_x*t_00 + v_y*t_10 + v_z*t_20 + t_30
    0,   0, 1.6, // 0, --- (V*T)_y=v_x*t_01 + v_y*t_11 + v_z*t_21 + t_31
                 //    --- (V*T)_z=v_x*t_02 + v_y*t_12 + v_z*t_22 + t_32
   -1,   0,   0  // 1, --- translation row vector
// POV-ray on the calculation of the matrix-transformed object
// https://www.povray.org/documentation/3.7.0/r3_3.html#r3_3_1_12_4
>
#end

// The things in this picture
union {
  object { Mirror( <0,0.1,0.1> ) rotate y*87 translate <-9,0,3> }

  #declare here=<-5,0,2>;
  #macro next(P)
    #declare here=here+P; scale 0.5 rotate y*-19 translate here
  #end

  #declare the_centre = union {
    object { centres(2,0) next(<0,0,0>) }
  }
//  the_centre

  #declare the_standard_centre = union {
    object { standard_centres next(<3,0,-6>) }
  }
//  the_standard_centre

  // rotating a centre

  #declare this_F = centre(0,0,2);
  this_F 

  #declare this_B = centre(1,0,2);
  this_B
  
  #declare this_L = centre(4,0,2);
  this_L
  
  #declare P = array [3][3] { { 1,2,3 }, { 4,5,6} , {7,8,9} };

  // #declare M1 = < 0,0,1, 0,1,0, -1,0,0, 0,0,0 >;
  // #declare M2 = < 0,0,1, 0,1,0, -1,0,0, 0,0,0 >; 
  // #declare M3 = M1*M2;
  // #debug str(matrix < 0,0,1, 0,1,0, -1,0,0, 0,0,0 >)

   #declare T = transform { rotate x*90 };
   #macro X180()
      matrix < 1,0,0, 0,-1,0, 0,0,-1, 0,0,0>
   #end
   // object { Cubelet(0,2) transform { rotate x*90 } }
   // object { Cubelet(0,2)  matrix < 1,0,0, 0,-1,0, 0,0,-1, 0,0,0>  }
   object { Cubelet(0,2) X180() }
  
  translate <3,0,1> 
  rotate <131,122,133> 
}
