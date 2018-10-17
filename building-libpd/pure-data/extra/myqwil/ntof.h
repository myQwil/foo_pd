#include "m_pd.h"
#include <math.h>

static t_float ntof(t_float f, double root, double semi) {
	return (root * exp(semi*f));
}