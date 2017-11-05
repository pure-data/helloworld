#include "m_pd.h"  
 
static t_class *swisseph_class;  
 
typedef struct _swisseph {  
  t_object  x_obj;  
} t_swisseph;  
 
void swisseph_bang(t_swisseph *x)  
{  
  post("Hello world!");  
}  
 
void *swisseph_new(void)  
{  
  t_swisseph *x = (t_swisseph *)pd_new(swisseph_class);  
  
  return (void *)x;  
}  
 
void swisseph_setup(void) {  
  swisseph_class = class_new(gensym("swisseph"),  
        (t_newmethod)swisseph_new,  
        0, sizeof(t_swisseph),  
        CLASS_DEFAULT, 0);  
  class_addbang(swisseph_class, swisseph_bang);  
}
