#include "m_pd.h"
#include "hotbinop.h"

/* -------------------------- hot >= -------------------------- */

static t_class *hge_class;
static t_class *hge_proxy_class;

static void *hge_new(t_floatarg f) {
	return (hotbinop_new(hge_class, hge_proxy_class, f));
}

static void hge_bang(t_hotbinop *x) {
	outlet_float(x->x_obj.ob_outlet, x->x_f1 >= x->x_f2);
}

static void hge_float(t_hotbinop *x, t_float f) {
	outlet_float(x->x_obj.ob_outlet, (x->x_f1=f) >= x->x_f2);
}

static void hge_proxy_bang(t_hotbinop_proxy *x) {
	t_hotbinop *m = x->p_master;
	outlet_float(m->x_obj.ob_outlet, m->x_f1 >= m->x_f2);
}

static void hge_proxy_float(t_hotbinop_proxy *x, t_float f) {
	t_hotbinop *m = x->p_master;
	outlet_float(m->x_obj.ob_outlet, m->x_f1 >= (m->x_f2=f));
}

void setup_0x230x3e0x3d(void) {
	hge_class = class_new(gensym("#>="),
		(t_newmethod)hge_new, (t_method)hotbinop_free,
		sizeof(t_hotbinop), 0,
		A_DEFFLOAT, 0);
	class_addbang(hge_class, hge_bang);
	class_addfloat(hge_class, hge_float);
	
	hge_proxy_class = class_new(gensym("_#>=_proxy"), 0, 0,
		sizeof(t_hotbinop_proxy),
		CLASS_PD | CLASS_NOINLET, 0);
	class_addbang(hge_proxy_class, hge_proxy_bang);
	class_addfloat(hge_proxy_class, hge_proxy_float);
	
	class_sethelpsymbol(hge_class, gensym("hotbinops2"));
}
