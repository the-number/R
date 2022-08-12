// Matrix is an object transformation that does rotation about the Y axis,
// shear along the Y axis, and translation along the Y axis
// https://www.povray.org/documentation/3.7.0/r3_3.html#r3_3_1_12_4
// transformation matrix
matrix
<
  0.886, 0.5, 0.5,       // the first 3 lines form a rotation matrix
  0,     1,   0,         // since it is not orthogonal, shearing occurs
  0.5,   0,  -0.886,
  0,     1.5, 0          // the last 3 values contain the translation
>
