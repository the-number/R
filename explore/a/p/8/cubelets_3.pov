// 20220810 (C) Gunter Liszewski -*- mode: pov; -*-
// cube and its cubelets, for  example
#version 3.7;
#include "colors.inc"
 global_settings { assumed_gamma 1.0 }

camera { location <0,2,-10> look_at 0 }
light_source { <-5,30,-10> 1 }

#macro Side( Colour, P1, P2, P3, P4)
  polygon {  5, P1 P2 P3 P4 P1
    texture{ finish  { specular 0.6 }
      pigment { colour Colour }
      normal  { agate 0.25 scale 1/2 }}}
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
  object { cubelet translate <X,Y,Z> }
#end

#macro corner(a,b,c)
//  object { cubelet }
#end

#macro centre(a)
  object { cubelet }
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
} // standard_cube


#declare that_cube = union { 
// <X,Y,Z> = <right,up,back> , we progress in a circular motion
// front and left and up
//  object { cubelet rotate <0,0,-90>
//                   translate <-1.05,1.05,-1.05> } // corner
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

#macro Matrix()
  matrix
<   
//  1,0,0, 0,1,0, 0,0,1, 0,0,0 // additive zero matrix-transform (implied column 0,0,0,1)
// from the 3.7 scenes
// 0.886, 0.5, 0.5,         // the first 3 lines form a rotation matrix
//   0,     1,   0,         // since it is not orthogonal, shearing occurs
// 0.5,     0,  -0.886,
//  -1,     0,   0          // the last 3 values contain the translation
    // our experiments
    // 1.3,0.6,0,    0,1,0, 0,0,1,   -1,0,0
       1.4,0.6,0, -1.3,1,0, 0,0,1.6, -1,0,0
    // POV-ray on the calculation of the matrix-transformed object
    // https://www.povray.org/documentation/3.7.0/r3_3.html#r3_3_1_12_4
>
#end

//--------------------------------------------
// reorientation macro, abyss.pov
//--------------------------------------------
#macro mOrient(P1,P2)
#local yV1=vnormalize(P2-P1);
#local xV1=vnormalize(vcross(yV1,z));
#local zV1=vcross(xV1,yV1);
                matrix <xV1.x,xV1.y,xV1.z,yV1.x,yV1.y,yV1.z,zV1.x,zV1.y,zV1.z,P1.x,P1.y,P1.z>
#end

// The things in this picture
union {
  object { Mirror( <0,0.1,0.1> ) rotate y*87 translate <-10,0,0> }
  object { standard_edges Matrix()
  }
  // object { that_cube }
  object { cubelet
    rotate y*-30 translate <-4,0,-4>
  }
  translate <3,0,1> rotate <131,122,133> }

