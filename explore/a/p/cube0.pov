 #version 3.6;
// Includes a separate file defining a number of common colours
 #include "colors.inc"
 global_settings { assumed_gamma 1.0 }

// Sets a background colour for the image (dark grey)
 background   { color rgb <0.25, 0.25, 0.25> }

// Places a camera
// direction: Sets, among other things, the field of view of the camera
// right: Sets the aspect ratio of the image
// look_at: Tells the camera where to look
 camera       { location  <0.0, 0.5, -4.0>
                direction 1.5*z
                right     x*image_width/image_height
                look_at   <0.0, 0.0, 0.0> }

// Places a light source
// color: Sets the color of the light source (white)
// translate: Moves the light source to a desired location
 light_source { <0, 0, 0>
                color rgb <1, 1, 1>
                translate <-5, 5, -5> }
// Places another light source
// color: Sets the color of the light source (dark grey)
// translate: Moves the light source to a desired location
 light_source { <0, 0, 0>
                color rgb <0.25, 0.25, 0.25>
                translate <6, -6, -6> }

// Sets a box
// pigment: Sets a color for the box ("Red" as defined in "colors.inc")
// finish: Sets how the surface of the box reflects light
// normal: Sets a bumpiness for the box using the "agate" in-built model
// rotate: Rotates the box
 box          { <-0.5, -0.5, -0.5>,
                <0.5, 0.5, 0.5>
                texture { pigment { color Red }
                          finish  { specular 0.6 }
                          normal  { agate 0.25 scale 1/2 }
                        }
		
                rotate <45,46,47> }
 box          { <-0.5, -0.5, -0.5>,
                <0.5, 0.5, 0.5>
                texture { pigment { color Green }
                          finish  { specular 0.6 }
                          normal  { agate 0.25 scale 1/2 }
                        }
		translate z
                rotate <45,46,47> }
box          { <-0.5, -0.5, -0.5>,
                <0.5, 0.5, 0.5>
                texture { pigment { color Blue }
                          finish  { specular 0.6 }
                          normal  { agate 0.25 scale 1/2 }
                        }
		translate -y
                rotate <45,46,47> }
box          { <-0.5, -0.5, -0.5>,
                <0.5, 0.5, 0.5>
                texture { pigment { color Cyan }
                          finish  { specular 0.6 }
                          normal  { agate 0.25 scale 1/2 }
                        }
		translate -z
		translate -y
                rotate <45,46,47> }
box          { <-0.5, -0.5, -0.5>,
                <0.5, 0.5, 0.5>
                texture { pigment { color Yellow }
                          finish  { specular 0.6 }
                          normal  { agate 0.25 scale 1/2 }
                        }
		translate y
                rotate <45,46,47> }
box          { <-0.5, -0.5, -0.5>,
                <0.5, 0.5, 0.5>
                texture { pigment { color Magenta }
                          finish  { specular 0.6 }
                          normal  { agate 0.25 scale 1/2 }
                        }
		translate y
		translate -z
                rotate <45,46,47> }
