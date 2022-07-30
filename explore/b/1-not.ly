\version "2.20.0"
\book {
  \score {
    \new Staff = "Voice" <<
      \new Voice <<
        \relative <<
          \clef treble
	  \time 4/4
	  \tempo "ein bischen schnell jedoch nicht eilend" 4 = 92

	c'2 d4 e  f g a2  d2 d4 f,  b d2.
   
	g,1 a b4 e2.

        a,16 b a b  a e g f  b d b e  a,4

        a'2.

        \time 3/4
        \tempo "ruhig aber gleiche Viertel" 4 = 92

        d,2 a4 b2. f e >> % music, relative
      >> % Voice
    >> % Staff
    \new Staff = "Ensemble" <<
      \new Voice <<
        \relative <<
	  \clef bass
	  \time 4/4
	  c,1 c,,1 >>
      >>
    >>
    \header { }
    \layout { }
    \midi { }
  } % score
} % book