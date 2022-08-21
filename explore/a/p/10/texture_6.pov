// 20220821 (C) Gunter Liszewski -*- mode: pov; -*-
// from Checkered_plane.pov
#version 3.7;
global_settings{ assumed_gamma 1.0 }
#default{ finish{ ambient 0.1 diffuse 0.9 }} 
#include "colors.inc"
#include "textures.inc"
// #include "glass.inc"
// #include "metals.inc"
// #include "golds.inc"
// #include "stones.inc"
// #include "woods.inc"
#include "shapes.inc"
// #include "shapes2.inc"
// #include "functions.inc"
// #include "math.inc"
// #include "transforms.inc"
#declare Camera_0 = 
  camera {perspective angle 75            // 0: front view
    location  <0.0 , 1.0 ,-3.0>
    right     x*image_width/image_height
    look_at   <0.0 , 1.0 , 0.0>}
#declare Camera_1 = 
  camera {/*ultra_wide_angle*/ angle 90   // 1: diagonal view
    location  <2.0 , 2.5 ,-3.0>
    right     x*image_width/image_height
    look_at   <0.0 , 1.0 , 0.0>}
#declare Camera_2 =
  camera {/*ultra_wide_angle*/ angle 90   // 2: right side view
    location  <3.0 , 1.0 , 0.0>
    right     x*image_width/image_height
    look_at   <0.0 , 1.0 , 0.0>}
#declare Camera_3 =
  camera {/*ultra_wide_angle*/ angle 90   // 3: top view
    location  <0.0 , 3.0 ,-0.001>
    right     x*image_width/image_height
    look_at   <0.0 , 1.0 , 0.0>}
#macro a_view_from_the(a)
  the_eye((a="front"?0:(a="diagonal"?1:(a="right"?2:(a="top"?3:0)))))
#end
#macro the_eye(a)
  #switch (a)
    #case (0)
      #debug "front view\n"
      camera{Camera_0}
    #break
    #case (1)
      #debug "diagonal view\n"      
      camera{Camera_1}
    #break
    #case (2)
      #debug "right side view\n"      
      camera{Camera_2}
    #break
    #case (3)
      #debug "top view\n"      
      camera{Camera_3}
    #break
  #end
#end
#declare the_sun =
light_source{< 3000,3000,-3000> color White};

#macro the_sky()
sky_sphere {
  pigment {
    gradient <0,1,0>
    color_map { [0.00 rgb <0.6,0.7,1.0>]
      [0.35 rgb <0.1,0.0,0.8>]
      [0.65 rgb <0.1,0.0,0.8>]
      [1.00 rgb <0.6,0.7,1.0>] 
    } 
    scale 2         
  }
}
#end
#declare the_ground =
plane{ <0,1,0>, 0 
       texture{
	 pigment{ 
	   checker 
	   color rgb<1,1,1>*1.2 
	   color rgb<0.25,0.15,0.1>*0}
	 finish { phong 0.1}
       }
};
#declare the_thing = 
sphere { <0,0,0>, 1 
  texture { Polished_Chrome
  } 
  scale<1,1,1>  rotate<0,0,0>  translate<0,1.35,0>  
};

a_view_from_the("diagonal") // front, diagonal, right, top
     // the_eye(1)          // 0,     1,        2,     3
the_sun
the_sky()
the_ground
the_thing