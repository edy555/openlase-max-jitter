/* 
	Copyright 2001-2005 - Cycling '74
	Joshua Kit Clayton jkc@cycling74.com	
*/

#include "jit.common.h"
#include "max.jit.mop.h"

typedef struct _max_jit_openlase_trace 
{
	t_object			ob;
	void				*obex;
	void 				*minout;
	void 				*meanout;
	void 				*maxout;	
	t_atom				*av;
} t_max_jit_openlase_trace;

t_jit_err jit_openlase_trace_init(void); 

void *max_jit_openlase_trace_new(t_symbol *s, long argc, t_atom *argv);
void max_jit_openlase_trace_free(t_max_jit_openlase_trace *x);
void max_jit_openlase_trace_assist(t_max_jit_openlase_trace *x, void *b, long m, long a, char *s);
void *max_jit_openlase_trace_class;

t_symbol *ps_getmin,*ps_getmean,*ps_getmax;
		 	
void main(void)
{	
	void *p,*q;
	
	jit_openlase_trace_init();	
	setup((t_messlist **)&max_jit_openlase_trace_class, (method)max_jit_openlase_trace_new, (method)max_jit_openlase_trace_free, (short)sizeof(t_max_jit_openlase_trace), 
		0L, A_GIMME, 0);

	p = max_jit_classex_setup(calcoffset(t_max_jit_openlase_trace,obex));
	q = jit_class_findbyname(gensym("jit_openlase_trace"));
    max_jit_classex_mop_wrap(p,q,0); 	
    max_jit_classex_standard_wrap(p,q,0); 	
 	addmess((method)max_jit_openlase_trace_assist,			"assist",			A_CANT,0);
}

void max_jit_openlase_trace_assist(t_max_jit_openlase_trace *x, void *b, long m, long a, char *s)
{
	if (m == 1) { //input
		max_jit_mop_assist(x,b,m,a,s);
	}
}

void max_jit_openlase_trace_free(t_max_jit_openlase_trace *x)
{
	max_jit_mop_free(x);
	jit_object_free(max_jit_obex_jitob_get(x));
	max_jit_obex_free(x);
}

void *max_jit_openlase_trace_new(t_symbol *s, long argc, t_atom *argv)
{
	t_max_jit_openlase_trace *x;
	void *o;

	if (x=(t_max_jit_openlase_trace *)max_jit_obex_new(max_jit_openlase_trace_class,gensym("jit_openlase_trace"))) {
		x->av = NULL;
		if (o=jit_object_new(gensym("jit_openlase_trace"))) {
			max_jit_mop_setup_simple(x,o,argc,argv);			
			max_jit_attr_args(x,argc,argv);
		} else {
			jit_object_error((t_object *)x,"jit.openlase.trace: could not allocate object");
			freeobject((t_object *)x);
			x=NULL;
		}
	}
	return (x);
}

