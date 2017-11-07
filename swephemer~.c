#include "m_pd.h"  
#include "m_imp.h"
#include "swephexp.h" 	/* this includes  "sweodef.h" */
#include <regex.h>

#define UNUSED(x) (void)(x)
#define MAX_PLANETS 12

static t_class *swephemer_tilde_class;  
 
typedef struct _swephemer_tilde {  
  t_object  x_obj;
  t_sample x_sample_rate;
  double x_jul_date;
  long* x_planets;
  short x_num_planets;
} t_swephemer_tilde;
 
void swephemer_tilde_bang(t_swephemer_tilde *x)  
{  
  UNUSED(x);
  double x2[6];
  char serr[256];
  char snam[40];
  long iflag = 0;
  short i;
  for (i = 0; i < x->x_num_planets; i++)
  {
   long iflgret = swe_calc(x->x_jul_date, x->x_num_planets+i, iflag, x2, serr);
   if (iflgret < 0) error("%s", serr);
   swe_get_planet_name(x->x_num_planets+i, snam);
   post("%10s\t%11.7f\t%10.7f\t%10.7f\t%10.7f", snam, x2[0], x2[1], x2[2], x2[3]);
 }
}

void swephemer_tilde_float(t_swephemer_tilde *x, t_float jul)  
{  
  x->x_jul_date += jul;
  post ("Received adjustment value %g. New date is %g", jul, x->x_jul_date);
}

void swephemer_tilde_bj(t_swephemer_tilde *x, t_float bj)  
{  
  x->x_jul_date = bj;
  post ("Set date to %g", x->x_jul_date);
}

void swephemer_tilde_b(t_swephemer_tilde *x, t_float y, t_float m, t_float d, t_float h)  
{  
  x->x_jul_date = swe_julday((int)y,(int)m,(int)d,(double)h,SE_GREG_CAL);
  post ("Set %d.%d.%d to Julian date %g", (int)y, (int)m, (int)d, x->x_jul_date);
}



void swephemer_tilde_planets(t_swephemer_tilde *x, t_symbol *s, short argc, t_atom *argv)  
{
  post ("Arguments: %d", argc);
  x->x_num_planets = argc;
  short i;
  for (i = 0; i < argc; i++)
  {
    if (argv[i].a_type == A_FLOAT)
    {
          x->x_planets = (long)argv[i].a_w.w_float;
          post ("Value %d = %d", i, x->x_planets);
     }
  }
}


void *swephemer_tilde_new(void)
{  
  t_swephemer_tilde *x = (t_swephemer_tilde *)pd_new(swephemer_tilde_class);
  x->x_sample_rate = sys_getsr();
  outlet_new(&x->x_obj, gensym("signal"));

  char *externdir = swephemer_tilde_class->c_externdir->s_name;
  //post("external is located at: %s",externdir);
  swe_set_ephe_path(externdir); // tell Pd to look for ephemera files in same dir as this external
  char *svers = malloc(1024);
  swe_version(svers);
  post ("Swiss Ephemeris version %s", svers);
  x->x_planets = malloc(MAX_PLANETS * sizeof(long));
  short i;
  for (i = 0; i < MAX_PLANETS; i++) x->x_planets[i] = 0;
  x->x_num_planets = 1;
  return (void *)x;
}

void *swephemer_tilde_free(void)
{
  swe_close();
}

static t_int *swephemer_tilde_perform(t_int *w)
{
   return (w+5);
}

static void swephemer_tilde_dsp(t_swephemer_tilde *x, t_signal **sp)
{
    dsp_add(swephemer_tilde_perform, 4, x, sp[0]->s_vec, sp[1]->s_vec, sp[0]->s_n);
}
 
void swephemer_tilde_setup(void) {
   swephemer_tilde_class = class_new(gensym("swephemer~"), (t_newmethod)swephemer_tilde_new, swephemer_tilde_free,
    	sizeof(t_swephemer_tilde), 0, A_DEFFLOAT, 0);
 class_addbang(swephemer_tilde_class, swephemer_tilde_bang);
 class_addfloat(swephemer_tilde_class, swephemer_tilde_float);
  class_addmethod(swephemer_tilde_class, (t_method)swephemer_tilde_bj, gensym("-bj"), A_FLOAT, 0);
  class_addmethod(swephemer_tilde_class, (t_method)swephemer_tilde_b, gensym("-b"), A_FLOAT, A_FLOAT, A_FLOAT, A_FLOAT, 0);
  class_addmethod(swephemer_tilde_class, (t_method)swephemer_tilde_planets, gensym("-p"), A_GIMME, 0);
}
