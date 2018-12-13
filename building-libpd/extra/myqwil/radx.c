#include "m_pd.h"
#include <math.h>
#include <float.h>
#include <limits.h>

typedef union {
	float f;
	struct { unsigned mnt:23,exp:8,sgn:1; } u;
} ufloat;
#define mnt u.mnt
#define exp u.exp
#define sgn u.sgn

#define MAX(a,b) ((a)>(b) ? (a):(b))
#define MIN(a,b) ((a)<(b) ? (a):(b))

#ifdef _WIN32
#define LEAD 3
#else
#define LEAD 2
#endif

/* -------------------------- radx -------------------------- */

static t_class *radx_class;

typedef struct _radx {
	t_object x_obj;
	t_float x_b;
	int x_p, x_e;
} t_radx;

static void out(char num[], int *i, const char *s, int l) {
	for (; l--; s++) num[(*i)++] = *s;
}

static const char dgt[64] = {
	"0123456789abcdef"
	"ghijkmnopqrstuvw"
	"xyzACDEFGHJKLMNP"
	"QRTUVWXYZ?!@#%^&"
};

static char *fmt_u(uintmax_t u, char *s, int radx) {
	unsigned v;
	for (   ; u>UINT_MAX; u/=radx) *--s = dgt[u%radx];
	for (v=u;          v; v/=radx) *--s = dgt[v%radx];
	return s;
}

static void fmt_fp(t_radx *x, long double y) {
	int radx = x->x_b;
	if (radx<2) radx=2; else if (radx>64) radx=64;
	
	uint64_t b2=4294967296, pwr=radx*radx;
	int rp=1, up=32;
	while (pwr<b2) { rp++; pwr*=radx; }
	pwr/=radx;
	while (b2>pwr) { b2/=2; up--; }
	
	int pr=rp-1, pu=up-1;
	int neg=0, p=x->x_p, t='g';
	
	/* based on: musl-libc /src/stdio/vfprintf.c */
	unsigned big[(LDBL_MANT_DIG+pu)/up + 1			// mantissa expansion
		+ (LDBL_MAX_EXP+LDBL_MANT_DIG+pu+pr)/rp];	// exponent expansion 
	unsigned *a, *d, *r, *z;
	int e2=0, e, i, j, l;
	char buf[rp+LDBL_MANT_DIG/4];
	char ebuf0[3*sizeof(int)], *ebuf=&ebuf0[3*sizeof(int)], *estr;
	
	if (signbit(y)) y=-y, neg=1;
	y = frexpl(y, &e2) * 2;
	if (y) y *= (1<<pu), e2-=up;
	if (p<0) p=6;
	
	if (e2<0) a=r=z=big;
	else a=r=z=big+sizeof(big)/sizeof(*big) - LDBL_MANT_DIG - 1;

	do { *z=y; y=pwr*(y-*z++); } while (y);

	while (e2>0)
	{	unsigned carry=0;
		int sh=MIN(up,e2);
		for (d=z-1; d>=a; d--)
		{	uint64_t u = ((uint64_t)*d<<sh)+carry;
			*d = u % pwr;
			carry = u / pwr;   }
		if (carry) *--a = carry;
		while (z>a && !z[-1]) z--;
		e2-=sh;   }
	
	while (e2<0)
	{	unsigned carry=0, *b;
		int sh=MIN(rp,-e2), need=1+(p+LDBL_MANT_DIG/3U+pr)/rp;
		for (d=a; d<z; d++)
		{	unsigned rm = *d & ((1<<sh)-1);
			*d = (*d>>sh) + carry;
			carry = (pwr>>sh) * rm;   }
		if (!*a) a++;
		if (carry) *z++ = carry;
		/* Avoid (slow!) computation past requested precision */
		b = a;
		if (z-b > need) z = b+need;
		e2+=sh;   }

	if (a<z)
		for (i=radx, e=rp*(r-a); *a>=i; i*=radx, e++);
	else e=0;

	/* Perform rounding: j is precision after the radix (possibly neg) */
	j = p-e-1;
	if (j < rp*(z-r-1))
	{	unsigned u;
		/* We avoid C's broken division of negative numbers */
		d = r + 1 + ((j+rp*LDBL_MAX_EXP)/rp - LDBL_MAX_EXP);
		j += rp*LDBL_MAX_EXP;
		j %= rp;
		for (i=radx, j++; j<rp; i*=radx, j++);
		u = *d % i;
		/* Are there any significant digits past j? */
		if (u || d+1!=z)
		{	long double round = 2/LDBL_EPSILON;
			long double small;
			if ((*d/i & 1) || (i==pwr && d>a && (d[-1]&1)))
				round += 2;
			if (u<i/2) small=0x0.8p0;
			else if (u==i/2 && d+1==z) small=0x1.0p0;
			else small=0x1.8p0;
			if (neg) round*=-1, small*=-1;
			*d -= u;
			/* Decide whether to round by probing round+small */
			if (round+small != round)
			{	*d = *d + i;
				while (*d > (pwr-1))
				{	*d--=0;
					if (d<a) *--a=0;
					(*d)++;   }
				for (i=radx, e=rp*(r-a); *a>=i; i*=radx, e++);   }   }
		if (z>d+1) z=d+1;   }
	
	for (; z>a && !z[-1]; z--);
	
	if (!p) p++;
	if (p>e && e>=-4)
	{	t--; p-=e+1;   }
	else
	{	t-=2; p--;   }
	
	/* Count trailing zeros in last place */
	if (z>a && z[-1])
		for (i=radx, j=0; z[-1]%i==0; i*=radx, j++);
	else j=rp;
	
	if ((t|32)=='f')
		p = MIN(p,MAX(0,rp*(z-r-1)-j));
	else
		p = MIN(p,MAX(0,rp*(z-r-1)+e-j));
	
	l = 1 + p + (p>0);
	if ((t|32)=='f')
	{	if (e>0) l+=e;   }
	else
	{	int erad = x->x_e;
		if (erad<2) erad=2; else if (erad>64) erad=64;
		estr=fmt_u(e<0 ? -e : e, ebuf, erad);
		while(ebuf-estr<LEAD) *--estr='0';
		*--estr = (e<0 ? '-' : '+');
		*--estr = t;
		l += ebuf-estr;   }

	char num[p+13]; // -1.23456e-10010101\0
	int ni=0;
	out(num, &ni, "-", neg);
	
	if ((t|32)=='f')
	{	if (a>r) a=r;
		for (d=a; d<=r; d++)
		{	char *s = fmt_u(*d, buf+rp, radx);
			if (d!=a) while (s>buf) *--s='0';
			else if (s==buf+rp) *--s='0';
			out(num, &ni, s, buf+rp-s);   }
			
		if (p) out(num, &ni, ".", 1);
		for (; d<z && p>0; d++, p-=rp)
		{	char *s = fmt_u(*d, buf+rp, radx);
			while (s>buf) *--s='0';
			out(num, &ni, s, MIN(rp,p));   }   }
	else
	{	if (z<=a) z=a+1;
		for (d=a; d<z && p>=0; d++)
		{	char *s = fmt_u(*d, buf+rp, radx);
			if (s==buf+rp) *--s='0';
			if (d!=a) while (s>buf) *--s='0';
			else
			{	out(num, &ni, s++, 1);
				if (p>0) out(num, &ni, ".", 1);   }
			out(num, &ni, s, MIN(buf+rp-s, p));
			p -= buf+rp-s;   }
		out(num, &ni, estr, ebuf-estr);   }
	
	num[ni] = '\0';
	outlet_symbol(x->x_obj.ob_outlet, gensym(num));
}

static void radx_precision(t_radx *x, t_floatarg f) {
	x->x_p = f;
}

static void radx_base(t_radx *x, t_floatarg f) {
	x->x_b = f;
}

static void radx_ebase(t_radx *x, t_floatarg f) {
	x->x_e = f;
}

static void radx_be(t_radx *x, t_floatarg f) {
	x->x_e = x->x_b = f;
}

static void radx_float(t_radx *x, t_float f) {
	ufloat uf = {.f=f};
	if (!isfinite(f))
	{	const char *s;
		int neg = uf.sgn;
		#ifdef _WIN32
			int mt = uf.mnt;
			if (f!=f)
			{	if (mt==0x400000 && neg) s="-1.#IND";
				else s=(neg?"-1.#QNAN":"1.#QNAN");   }
			else s=(neg?"-1.#INF":"1.#INF");
		#else
			if (f!=f) s=(neg?"-nan":"nan");
			else s=(neg?"-inf":"inf");
		#endif
		outlet_symbol(x->x_obj.ob_outlet, gensym(s));
		return;   }
	
	fmt_fp(x, f);
}

static void *radx_new(t_float f) {
	t_radx *x = (t_radx *)pd_new(radx_class);
	outlet_new(&x->x_obj, &s_symbol);
	floatinlet_new(&x->x_obj, &x->x_b);
	x->x_e = x->x_b = f?f:16;
	x->x_p = 6;
	return (x);
}

void radx_setup(void) {
	radx_class = class_new(gensym("radx"),
		(t_newmethod)radx_new, 0,
		sizeof(t_radx), 0,
		A_DEFFLOAT, 0);
	class_addfloat(radx_class, radx_float);
	class_addmethod(radx_class, (t_method)radx_precision,
		gensym("p"), A_FLOAT, 0);
	class_addmethod(radx_class, (t_method)radx_base,
		gensym("b"), A_FLOAT, 0);
	class_addmethod(radx_class, (t_method)radx_ebase,
		gensym("e"), A_FLOAT, 0);
	class_addmethod(radx_class, (t_method)radx_be,
		gensym("be"), A_FLOAT, 0);
}
