/*   
  note-spacing.cc -- implement Note_spacing

  source file of the GNU LilyPond music typesetter

  (c) 2001--2002  Han-Wen Nienhuys <hanwen@cs.uu.nl>
*/


#include "paper-column.hh"
#include "item.hh"
#include "moment.hh"
#include "note-spacing.hh"
#include "grob.hh"
#include "note-column.hh"
#include "warn.hh"
#include "stem.hh"
#include "separation-item.hh"
#include "staff-spacing.hh"

bool
Note_spacing::has_interface (Grob* g)
{
  return g && g->has_interface (ly_symbol2scm ("note-spacing-interface"));
}

Real
Note_spacing::get_spacing (Grob *me)
{
  Drul_array<SCM> props(me->get_grob_property ("left-items"),
			me->get_grob_property ("right-items"));
  Direction d = LEFT;
  Drul_array<Interval> extents;
  do
    {
      for (SCM  s = props[d]; gh_pair_p (s); s = gh_cdr (s))
	{
	  Item * it= dynamic_cast<Item*> (unsmob_grob (gh_car(s)));
	  extents[d].unite (it->extent (it->column_l (), X_AXIS));

	  if (d == RIGHT)
	    {
	      Grob * accs = Note_column::accidentals (it);
	      if (!accs)
		accs = Note_column::accidentals (it->get_parent (X_AXIS));
	      
	      if (accs)
		extents[d].unite (accs->extent (it->column_l (), X_AXIS));
	    }
	}

      if (extents[d].empty_b ())
	extents[d] = Interval (0,0);
    }
  while (flip (&d) != LEFT);

  /*
    
    What's sticking out at the left of the right side has less
    influence.

  */
  Real dx= extents[LEFT][RIGHT] - 0.5 * extents[RIGHT][LEFT];
  return dx;
}

Item *
Note_spacing::left_column (Grob *me)
{
  if (me->immutable_property_alist_ == SCM_EOL)
    return 0;
  
  return dynamic_cast<Item*> (me)->column_l ();
}

/*
  Compute the column of the right-items.  This is a big function,
since RIGHT-ITEMS may span more columns (eg. if a clef if inserted,
this will add a new columns to RIGHT-ITEMS. Here we look at the
columns, and return the left-most. If there are multiple columns, we
prune RIGHT-ITEMS.
   
 */
Item *
Note_spacing::right_column (Grob*me)
{
  /*
    ugh. should have generic is_live() method?
   */
  if (me->immutable_property_alist_ == SCM_EOL)
    return 0;
  
  SCM right = me->get_grob_property ("right-items");
  Item *mincol = 0;
  int min_rank = INT_MAX;
  bool prune = false;
  for (SCM s = right ; gh_pair_p (s) ; s = gh_cdr (s))
    {
      Item * ri = unsmob_item (gh_car (s));

      Item * col = ri->column_l ();
      int rank = Paper_column::rank_i (col);

      if (rank < min_rank)
	{
	  min_rank = rank;
	  if (mincol)
	    prune = true;

	  mincol = col;
	}
    }
  
  if (prune)
    {
      // I'm a lazy bum. We could do this in-place.
      SCM newright  = SCM_EOL;
      for (SCM s = right ; gh_pair_p (s) ; s =gh_cdr (s))
	{
	  if (unsmob_item (gh_car (s))->column_l () == mincol)
	    newright = gh_cons (gh_car (s), newright);
	}

      me->set_grob_property ("right-items", newright);
    }
  
  if (!mincol)
    {
      /*
      int r = Paper_column::rank_i (dynamic_cast<Item*>(me)->column_l ());
      programming_error (_f("Spacing wish column %d has no right item.", r));
      */

      return 0;
    }

  return mincol;
}

/**
   Correct for optical illusions. See [Wanske] p. 138. The combination
   up-stem + down-stem should get extra space, the combination
   down-stem + up-stem less.

   TODO: have to check wether the stems are in the same staff.

   TODO: also correct for bar lines in RIGHT-ITEMS.  Should check if
   the barline is the leftmost object of the break alignment.

*/
Real
Note_spacing::stem_dir_correction (Grob*me) 
{
  Drul_array<Direction> stem_dirs(CENTER,CENTER);
  Drul_array<Interval> stem_posns;
  Drul_array<Interval> head_posns;  
  Drul_array<SCM> props(me->get_grob_property ("left-items"),
			me->get_grob_property ("right-items"));

  Real correction = 0.0;
  
  stem_dirs[LEFT] = stem_dirs[RIGHT] = CENTER;
  Interval intersect;
  Interval bar_xextent;
  Interval bar_yextent;  
  
  bool correct = true;
  Direction d = LEFT;
  bool acc_right = false;
  
  do
    {
      for (SCM  s = props[d]; gh_pair_p (s); s = gh_cdr (s))
	{
	  Item * it= dynamic_cast<Item*> (unsmob_grob (gh_car(s)));

	  if (d == RIGHT)
	    acc_right = acc_right || Note_column::accidentals (it);
	  
	  Grob *stem = Note_column::stem_l (it);

	  if (!stem)
	    {
	      if (d == RIGHT && Separation_item::has_interface (it))
		{
		  Grob *last = Staff_spacing::extremal_break_aligned_grob (it, LEFT, &bar_xextent);

		  if (last)
		    bar_yextent = Staff_spacing::bar_y_positions (last);

		  break;
		}

	      goto exit_func; 
	    }
	  if(Stem::invisible_b (stem))
	    {
	      correct = false;
	      goto exit_func ;
	    }

	  Direction sd = Stem::get_direction (stem);
	  if (stem_dirs[d] && stem_dirs[d] != sd)
	    {
	      correct = false;
	      goto exit_func;
	    }
	  stem_dirs[d] = sd;

	  Interval hp  = Stem::head_positions (stem);
	  Real chord_start = hp[sd];	  
	  Real stem_end = Stem::stem_end_position (stem);
	  
	  stem_posns[d] = Interval(chord_start<?stem_end, chord_start>? stem_end);
	  head_posns[d].unite (hp);
	}
    }
  while (flip (&d) != LEFT);
  

  /*
    don't correct if accidentals are sticking out of the right side.

  */
  if (acc_right)
    return 0.0;

  if (!bar_yextent.empty_b())
    {
      stem_dirs[RIGHT] = - stem_dirs[LEFT];
      stem_posns[RIGHT] = bar_yextent;
    }
  
  if (correct &&stem_dirs[LEFT] *stem_dirs[RIGHT] == -1)
    {
      
      intersect = stem_posns[LEFT];  
      intersect.intersect(stem_posns[RIGHT]);
      correct = correct && !intersect.empty_b ();

      if (!correct)
	return 0.0;
      /*
	Ugh. 7 is hardcoded.
      */
      correction = abs (intersect.length ());
      correction = (correction/7) <? 1.0;
      correction *= stem_dirs[LEFT] ;
      correction *= gh_scm2double (me->get_grob_property ("stem-spacing-correction"));

      if (!bar_yextent.empty_b())
	{
	  correction *= 0.5;
	}
    }
  else if (correct)
    {
      /*
	Correct for the following situation:

	 X      X
	|      | 
	|      |
 	|   X  |
	|  |   |
	========

           ^ move the center one to the left.
	

	this effect seems to be much more subtle than the
	stem-direction stuff (why?), and also does not scale with the
	difference in stem length.
	
       */

      
      Interval hp = head_posns[LEFT];
      hp.intersect  (head_posns[RIGHT]);
      if (!hp.empty_b())
	return 0.0;

      Direction lowest =
	(head_posns[LEFT][DOWN] > head_posns[RIGHT][UP]) ? RIGHT : LEFT;

      Real delta = head_posns[-lowest][DOWN] - head_posns[lowest][UP] ;
      Real corr = gh_scm2double (me->get_grob_property ("stem-spacing-correction"));
      corr =  (delta <= 1) ? 0.0 : 0.25;
      
      correction=  -lowest * corr ;
    }

  if (!bar_xextent.empty_b())
    correction += - bar_xextent[LEFT];
  
 exit_func:
  return correction;
}
 
