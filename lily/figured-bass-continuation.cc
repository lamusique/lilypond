/*
  figured-bass-continuation.cc -- implement Figured_bass_continuation

  source file of the GNU LilyPond music typesetter

  (c) 2005 Han-Wen Nienhuys <hanwen@xs4all.nl>

*/

#include "line-interface.hh"
#include "lily-guile.hh"
#include "spanner.hh"
#include "output-def.hh"
#include "item.hh"
#include "stencil.hh"
#include "pointer-group-interface.hh"
#include "axis-group-interface.hh"


#include "horizontal-bracket.hh"

struct Figured_bass_bracket
{
  static bool has_interface (Grob*);
  
public:
  DECLARE_SCHEME_CALLBACK(print, (SCM));
};


ADD_INTERFACE(Figured_bass_bracket,
	      "figured-bass-bracket-interface",
	      "Brackets alongside bass figures.",
	      
	      /* props */

	      /* ugh: should make bracket interface. */
	      "bracket-flare "
	      "shorten-pair "
	      "edge-height "
	      "padding "
	      "thickness "
	      "elements "
	      );

MAKE_SCHEME_CALLBACK (Figured_bass_bracket, print, 1);
SCM
Figured_bass_bracket::print (SCM grob)
{
  Grob *me = unsmob_grob (grob);
  extract_grob_set (me, "elements", elements);
  if (elements.is_empty ())
    {
      me->suicide ();
      return SCM_EOL;
    }

  Grob *common_x = common_refpoint_of_array (elements, me, X_AXIS); 
  Interval xext = Axis_group_interface::relative_group_extent (elements, common_x, X_AXIS);
  
  Stencil left_br = Horizontal_bracket::make_bracket (me, me, elements,
						      Y_AXIS, LEFT);
  Stencil right_br = Horizontal_bracket::make_bracket (me, me, elements,
						       Y_AXIS, RIGHT);

  xext.widen (robust_scm2double (me->get_property ("padding"), 0.25));
  left_br.translate_axis (xext[LEFT], X_AXIS);
  right_br.translate_axis (xext[RIGHT], X_AXIS);

  left_br.add_stencil (right_br);
  left_br.translate_axis (-me->relative_coordinate (common_x, X_AXIS), X_AXIS);
  return left_br.smobbed_copy ();
}


struct Figured_bass_continuation
{
  static bool has_interface (Grob*);
  
public:
  DECLARE_SCHEME_CALLBACK(print, (SCM));
  DECLARE_SCHEME_CALLBACK(center_on_figures, (SCM, SCM));
};

MAKE_SCHEME_CALLBACK (Figured_bass_continuation, center_on_figures, 2);
SCM
Figured_bass_continuation::center_on_figures (SCM grob, SCM axis)
{
  Spanner *me = dynamic_cast<Spanner*> (unsmob_grob (grob));
  (void) axis;
  
  extract_grob_set (me, "figures", figures);
  Grob *common = common_refpoint_of_array (figures, me, Y_AXIS);

  Interval ext = Axis_group_interface::relative_group_extent (figures, common, Y_AXIS);
  
  return scm_from_double (ext.center () - me->relative_coordinate (common, Y_AXIS));
}

MAKE_SCHEME_CALLBACK (Figured_bass_continuation, print, 1);
SCM
Figured_bass_continuation::print (SCM grob)
{
  Spanner *me = dynamic_cast<Spanner*> (unsmob_grob (grob));

  Real thick =
    me->get_layout ()->get_dimension (ly_symbol2scm ("linethickness"))
    * robust_scm2double (me->get_property ("thickness"), 1);
  
  Interval spanned;
  Direction d = LEFT;
  Grob *common = me->get_bound (LEFT)->common_refpoint (me->get_bound (RIGHT),
							X_AXIS);
  do
    {
      spanned[d] 
	= robust_relative_extent (me->get_bound (d), common, X_AXIS)[RIGHT]
	- me->relative_coordinate (common, X_AXIS);
    }
  while (flip (&d) !=  LEFT);
  spanned.widen (- robust_scm2double (me->get_property ("padding"), 0.2));
  
  Stencil extender;
  if (!spanned.is_empty ())
    extender = Line_interface::make_line (thick,
					  Offset (spanned[LEFT], 0),
					  Offset (spanned[RIGHT], 0));
  
  return extender.smobbed_copy ();
}

ADD_INTERFACE(Figured_bass_continuation,
	      "figured-bass-continuation-interface",
	      "Simple extender line between bounds.",
	      
	      /* props */
	      "thickness "
	      "padding "
	      "figures "
	      );
	      
