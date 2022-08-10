#version 3.7;
 global_settings { assumed_gamma 1.0 }

#debug "Hello World\n"

#switch (clock*360)
  #range (0,180)
   #debug "Clock in 0 to 180 range\n"
  #break
  #range (180,360)
   #debug "Clock in 180 to 360 range\n"
  #break
  #else
   #warning "Clock outside expected range\n"
   #warning concat("Value is:",str(clock*360,5,0),"\n")
 #end