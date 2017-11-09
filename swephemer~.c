#include "m_pd.h"
#include "m_imp.h"
#include "swephexp.h"   /* this includes  "sweodef.h" */
#include <regex.h>

#define UNUSED(x) (void)(x)
typedef enum { false, true } bool;

static t_class *swephemer_tilde_class;

typedef struct _swephemer_tilde {
        t_object x_obj;
        t_sample x_sample_rate;
        double x_jul_date;
        t_float x_step;
        long x_body;
        bool x_dsp_flag;
        bool x_heliocentric;
        long x_iflag;
        t_outlet* x_longitude_out, *x_latitude_out, *x_distance_out, *x_date_out;
} t_swephemer_tilde;

void swephemer_tilde_bang(t_swephemer_tilde *x)
{
        double x2[6];
        char serr[256];
        long iflgret = swe_calc(x->x_jul_date, x->x_body, x->x_iflag, x2, serr);
        if (iflgret < 0) error("%s", serr);
        else
        {
                outlet_float (x->x_longitude_out, x2[0]);
                outlet_float (x->x_latitude_out, x2[1]);
                outlet_float (x->x_distance_out, x2[2]);
                outlet_float (x->x_date_out, x->x_jul_date);
        }
}

void swephemer_tilde_float(t_swephemer_tilde *x, t_float f)
{
        x->x_jul_date += f;
        //post ("Received adjustment value %+g. New date is %10.4f", f, x->x_jul_date);
        swephemer_tilde_bang(x);
}

void swephemer_tilde_step(t_swephemer_tilde *x, t_float f)
{
        x->x_step = f;
        post ("Set step to %g", x->x_step);
}

void swephemer_tilde_audioflag(t_swephemer_tilde *x, t_float f)
{
        int v = (int)f;
        x->x_dsp_flag = (v == 0) ? false : true;
}

void swephemer_tilde_bj(t_swephemer_tilde *x, t_symbol *s, short argc, t_atom *argv)
{
        UNUSED(s);
        UNUSED(argc);
        unsigned short len;
        char *subbuf;
        double strval;
        switch (argv[0].a_type)
        {
        case A_FLOAT:
                x->x_jul_date = argv[0].a_w.w_float;
                //post ("Set date to %10.4f", x->x_jul_date);
                outlet_float (x->x_date_out, x->x_jul_date);
                break;
        case A_SYMBOL:
                len = strlen(argv[0].a_w.w_symbol->s_name);
                subbuf = malloc(len-2);
                sprintf(subbuf, "%.*s", len-2, argv[0].a_w.w_symbol->s_name+1);
                strval = atof(subbuf);
                x->x_jul_date = strval;
                //post ("Set date to %10.4f", x->x_jul_date);
                outlet_float (x->x_date_out, x->x_jul_date);
                free (subbuf);
                break;
        default:
                error ("Unexpected input");
        }
}

void swephemer_tilde_b(t_swephemer_tilde *x, t_symbol *s, short argc, t_atom *argv)
{
        UNUSED(s);
        long y = (argc > 0 && argv[0].a_type == A_FLOAT) ? argv[0].a_w.w_float : 0;
        long m = (argc > 1 && argv[1].a_type == A_FLOAT) ? argv[1].a_w.w_float : 1;
        long d = (argc > 2 && argv[2].a_type == A_FLOAT) ? argv[2].a_w.w_float : 1;
        double h = (argc > 3 && argv[3].a_type == A_FLOAT) ? argv[3].a_w.w_float : 0;
        x->x_jul_date = swe_julday(y,m,d,h,SE_GREG_CAL);
        //post ("Set %d.%d.%d.%f to Julian date %10.4f", y, m, d, h, x->x_jul_date);
        outlet_float (x->x_date_out, x->x_jul_date);
}

void swephemer_tilde_path(t_swephemer_tilde *x, t_symbol *s)
{
        UNUSED(x);
        swe_set_ephe_path(s->s_name); // tell Pd to look for ephemera files in same dir as this external
        post("Swephemer will look for ephemera files at %s", s->s_name);
}


void swephemer_tilde_body(t_swephemer_tilde *x, t_float f)
{
        x->x_body = (long)f;
        char snam[40];
        swe_get_planet_name(x->x_body, snam);
        post ("Celestial body: %s", snam);
}

void swephemer_tilde_helflag(t_swephemer_tilde *x)
{
        if (x->x_heliocentric == false) x->x_heliocentric = true;
        else x->x_heliocentric = false;
        x->x_iflag = (x->x_heliocentric == true) ? SEFLG_HELCTR : 0;
        post ("Setting heliocentric to %s", (x->x_heliocentric == true) ? "true" : "false");
}

void *swephemer_tilde_new(t_symbol *s, int argc, t_atom *argv)
{
        UNUSED(s);
        t_swephemer_tilde *x = (t_swephemer_tilde *)pd_new(swephemer_tilde_class);
        swe_set_ephe_path(NULL); // tell Pd to look for ephemera files in same dir as this external
        char *svers = malloc(1024);
        swe_version(svers);
        post ("Swiss Ephemeris version %s", svers);
        unsigned short i;
        char snam[40];
        for (i = 0; i < argc; i++)
        {
                switch (argv[i].a_type)
                {
                case A_FLOAT:
                        x->x_body = (long)argv[i].a_w.w_float;
                        swe_get_planet_name(x->x_body, snam);
                        post ("Celestial body: %s", snam);
                        break;
                case A_SYMBOL:
                        error("Ignoring %s", argv[i].a_w.w_symbol->s_name);
                        break;
                default:
                        error("Got something unexpected...");
                }
        }
        x->x_dsp_flag = false;
        x->x_sample_rate = sys_getsr();
        x->x_longitude_out = outlet_new(&x->x_obj, &s_float);
        x->x_latitude_out = outlet_new(&x->x_obj, &s_float);
        x->x_distance_out = outlet_new(&x->x_obj, &s_float);
        x->x_date_out = outlet_new(&x->x_obj, &s_float);
        outlet_new(&x->x_obj, gensym("signal"));
        x->x_body = 0;
        x->x_iflag = 0;
        x->x_step = 0;
        x->x_heliocentric = false;
        return (void *)x;
}

void swephemer_tilde_free(t_swephemer_tilde *x)
{
        UNUSED(x);
        swe_close();
}

static t_int *swephemer_tilde_perform(t_int *w)
{
        t_swephemer_tilde *x = (t_swephemer_tilde *)(w[1]);
        if (x->x_dsp_flag == true)
        {

        }
        return (w+5);
}

static void swephemer_tilde_dsp(t_swephemer_tilde *x, t_signal **sp)
{
        t_int dsp_add_args [3];
        t_int vector_size = sp[0]->s_n;

        //dsp_add(swephemer_tilde_perform, 4, x, sp[0]->s_vec, sp[1]->s_vec, sp[0]->s_n);
}

void swephemer_tilde_setup(void) {
        swephemer_tilde_class = class_new(gensym("swephemer~"),
                                          (t_newmethod)swephemer_tilde_new,
                                          (t_method)swephemer_tilde_free,
                                          sizeof(t_swephemer_tilde), 0, A_GIMME, 0);
        class_addbang(swephemer_tilde_class, swephemer_tilde_bang);
        class_addfloat(swephemer_tilde_class, swephemer_tilde_float);
        class_addmethod(swephemer_tilde_class, (t_method)swephemer_tilde_bj, gensym("-bj"), A_GIMME, 0);
        class_addmethod(swephemer_tilde_class, (t_method)swephemer_tilde_b, gensym("-b"), A_GIMME, 0);
        class_addmethod(swephemer_tilde_class, (t_method)swephemer_tilde_body, gensym("-p"), A_FLOAT, 0);
        class_addmethod(swephemer_tilde_class, (t_method)swephemer_tilde_step, gensym("-step"), A_FLOAT, 0);
        class_addmethod(swephemer_tilde_class, (t_method)swephemer_tilde_path, gensym("-path"), A_SYMBOL, 0);
        class_addmethod(swephemer_tilde_class, (t_method)swephemer_tilde_helflag, gensym("-hel"), 0);
        class_addmethod(swephemer_tilde_class, (t_method)swephemer_tilde_audioflag, gensym("-audio"), A_FLOAT, 0);
        class_addmethod(swephemer_tilde_class, (t_method)swephemer_tilde_dsp, gensym("dsp"), 0);
}
