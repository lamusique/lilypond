#include "lookupsyms.hh"
#include "debug.hh"
#include "symtable.hh"
#include "dimen.hh"
#include "tex.hh"
void
Lookup::parse(Text_db&t)
{
    symtables_->read(t) ;
}

Lookup::Lookup()
{
    symtables_ = new Symtables;
}

Lookup::~Lookup()
{
    delete symtables_;
}

Symbol
Lookup::ball(int j)
{
    if (j > 4)
	j = 4;

    Symtable * st = (*symtables_)("balls");
    return st->lookup(String(j));
}

Symbol
Lookup::rest(int j)
{
    return (*symtables_)("rests")->lookup(String(j));
}


Symbol
Lookup::bar(String s)
{
    return (*symtables_)("bars")->lookup(s);
}
 Symbol
Lookup::dots(int j)
{
    if (j>3)
	error("max 3 dots");
    return (*symtables_)("dots")->lookup(j);
}

/****************************************************************/
// bare bones.

struct Linestaf_symbol : Parametric_symbol {
    int lines;
    Linestaf_symbol(int n, Symtables*s): Parametric_symbol(s) { lines = n;}
    Symbol eval(svec<String>)const;
};


Symbol
Linestaf_symbol::eval(svec<String> w)const
{
    Real wid = w[0].fvalue();

    Symbol s;
    s.dim.x = Interval(0,wid);
    Real dy=lines*convert_dimen(5,"pt");
    s.dim.y = Interval(0,dy);
    svec<String> a;
    a.add(lines);
    a.add(w[0]);
    s.tex = (*symtables_)("param")->lookup("linestaf").tex;
    s.tex = substitute_args(s.tex, a);
    return s;
}

/****************************************************************/


struct Meter_sym:Parametric_symbol {

    Meter_sym(Symtables*s) : Parametric_symbol(s){  }
    Symbol eval(svec<String> a) const{
	Symbol s;
	s.dim.x = Interval( convert_dimen(-5,"pt"), convert_dimen(10,"pt"));
	s.dim.y = Interval(0, convert_dimen(10,"pt") );	// todo
	String src = (*symtables_)("param")->lookup("meter").tex;
	s.tex = substitute_args(src,a);
	return s;
    }
};
/****************************************************************/

Parametric_symbol *
Lookup::meter(String )
{
    return new Meter_sym(symtables_);
}

Parametric_symbol *
Lookup::linestaff(int n)
{
    return new Linestaf_symbol(n,symtables_);
}

