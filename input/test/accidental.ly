
foo = \notes\relative c''   {   \key as \major; dis4 dis dis! dis? }

\score {

  < \foo 
   \context NoteNames \foo
  >
}
