/*   
  multi-measure-rest.cc --  implement Multi_measure_rest
  
  source file of the GNU LilyPond music typesetter
  
  (c) 1998--2000 Jan Nieuwenhuizen <janneke@gnu.org>
  
 */

#include "multi-measure-rest.hh"
#include "debug.hh"
#include "paper-def.hh"
#include "paper-column.hh" // urg
#include "bar.hh"
#include "lookup.hh"
#include "rest.hh"
#include "molecule.hh"
#include "misc.hh"
#include "group-interface.hh"
#include "stem.hh"
#include "staff-symbol-referencer.hh"
void
Multi_measure_rest::set_interface (Score_element*me)
{
  me->set_elt_pointer ("columns", SCM_EOL);
}

Multi_measure_rest::Multi_measure_rest (SCM s)
  : Spanner(s)
{}

/*
   [TODO]                                      17
 * variable-sized multi-measure rest symbol: |====| ??
*/
MAKE_SCHEME_SCORE_ELEMENT_CALLBACK(Multi_measure_rest,brew_molecule);
SCM
Multi_measure_rest::brew_molecule (SCM smob) 
{
  Score_element *me = unsmob_element (smob);
  Spanner * sp = dynamic_cast<Spanner*> (me);
  Real staff_space
    = Staff_symbol_referencer_interface (me).staff_space ();

  Interval sp_iv;
  Direction d = LEFT;
  do
    {
      Item * col = sp->get_bound (d)->column_l ();

      Interval coldim = col->extent (X_AXIS)
	+ col->relative_coordinate (0, X_AXIS);

      sp_iv[d] = coldim[-d]  ;
    }
  while ((flip (&d)) != LEFT);
  Molecule mol;
  Real x_off = 0.0;

  Real rx  = sp->get_bound (LEFT)->relative_coordinate (0, X_AXIS);
  /*
    we gotta stay clear of sp_iv, so move a bit to the right if
    needed.
   */
  x_off += (sp_iv[LEFT] -  rx) >? 0;

  /*
    center between stuff.
   */
  x_off += sp_iv.length ()/ 2;

  
  Molecule s;

  int measures = 1;
  SCM m (me->get_elt_property ("measure-count"));
  if (gh_number_p (m))
    {
      measures = gh_scm2int (m);
    }
  

  if (measures <= me->paper_l() ->get_var ("multi_measure_rest_expand_limit"))
    {
      /*
	Build a rest from smaller parts. Distances inbetween are
	really variable, see Wanske pp. 125 */

      int l = measures;
      while (l)
	{
	  int k;
	  if (l >= 4)
	    {
	      l-=4;
	      k = -2;
	    }
	  else if (l>= 2)
	    {
	      l -= 2;
	      k = -1;
	    }
	  else
	    {
	      k = 0;
	      l --;
	    }

	  Real pad = s.empty_b ()
	    ? 0.0 : me->paper_l ()->get_var ("multi_measure_rest_padding");
      
	  Molecule r (me->lookup_l ()->afm_find ("rests-" + to_str (k)));
	  if (k == 0)
	    r.translate_axis (staff_space, Y_AXIS);
	  
	  s.add_at_edge (X_AXIS, RIGHT, r, pad);
	}


      s.align_to (X_AXIS, CENTER);
    }
  else 
    {
      String idx =  ("rests-") + to_str (-4);
      s = me->lookup_l ()->afm_find (idx);
    }
  
  mol.add_molecule (s);

  if (measures > 1)
    {
      Molecule s (me->lookup_l ()->text ("number", to_str (measures), me->paper_l ()));
      s.align_to (X_AXIS, CENTER);
      s.translate_axis (3.0 * staff_space, Y_AXIS);
      mol.add_molecule (s);
    }
  mol.translate_axis (x_off, X_AXIS);
  return mol.create_scheme();
}

/*
  UGH. JUNKME elt prop "columns" isn't really needed. 
 */
void
Multi_measure_rest::add_column (Score_element*me,Item* c)
{
  Pointer_group_interface gi (me, "columns");
  gi.add_element (c);

  add_bound_item (dynamic_cast<Spanner*> (me),c);
}


Array<Rod>
Multi_measure_rest::get_rods () const
{
  Array<Rod> a;

  if (!(get_bound (LEFT) && get_bound (RIGHT)))
    {
      programming_error ("Multi_measure_rest::get_rods (): I am not spanned!");
      return a;
    }

  Item * l = get_bound (LEFT)->column_l ();
  Item * r = get_bound (RIGHT)->column_l ();
  Item * lb = l->find_prebroken_piece (RIGHT);
  Item * rb = r->find_prebroken_piece (LEFT);      
  
  Item* combinations[4][2]={{l,r}, {lb,r}, {l,rb},{lb,rb}};
  for (int i=0; i < 4; i++)
    {
      Item * l =  combinations[i][0];
      Item *r = combinations[i][1];

      if (!l || !r)
	continue;

      Rod rod;
      rod.item_l_drul_[LEFT] = l;
      rod.item_l_drul_[RIGHT] = r;

	/*
	  should do something more advanced.
	 */
      rod.distance_f_ = l->extent (X_AXIS)[BIGGER] - r->extent (X_AXIS)[SMALLER]
	+ paper_l ()->get_var ("multi_measure_rest_x_minimum");
  
      a.push (rod);
    }
  
  return a;
}

