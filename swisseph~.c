#include "m_pd.h"  
#include "swephexp.h" 	/* this includes  "sweodef.h" */

static t_class *swisseph_tilde_class;  
 
typedef struct _swisseph_tilde {  
  t_object  x_obj;  
} t_swisseph_tilde;
 
void swisseph_tilde_bang(t_swisseph_tilde *x)  
{  
  post("Hello world!");
}  
 
void *swisseph_tilde_new(void)
{  
  t_swisseph_tilde *x = (t_swisseph_tilde *)pd_new(swisseph_tilde_class);  
  //swe_set_ephe_path(NULL);
  return (void *)x;  
}

void *swisseph_tilde_free(void)
{
  //swe_close();
}  

static t_int *swisseph_tilde_perform(t_int *w)
{
   return (w+5);
}

static void swisseph_tilde_dsp(t_swisseph_tilde *x, t_signal **sp)
{
    dsp_add(swisseph_tilde_perform, 4, x, sp[0]->s_vec, sp[1]->s_vec, sp[0]->s_n);
}
 
void swisseph_tilde_setup(void) {
   swisseph_tilde_class = class_new(gensym("swisseph~"), (t_newmethod)swisseph_tilde_new, swisseph_tilde_free,
    	sizeof(t_swisseph_tilde), 0, A_DEFFLOAT, 0);
 class_addbang(swisseph_tilde_class, swisseph_tilde_bang);  
}
