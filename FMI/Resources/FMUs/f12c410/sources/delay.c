/*
 * Copyright (C) 1995-2001 Dynasim AB.
 * All rights reserved.
 *
 */
 /* DLL-status: Lot of static data used/defined in file => turned into TLS */

#if !defined(NO_FILE)
#include <stdio.h>
#endif
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "dymutil.h"
#include "delay.h"
#include "dlldata.h"
#include "dymdstypes.h"
DYMOLA_STATIC int allocDelayBuffers(delayStruct* del, const size_t nbrDel, const int buffer) {
	/*returns 0 for success 1 for allocation error;*/
	char mess[500];
	int i = 0;
	for (i = 0; i < (int)nbrDel; ++i) {
		del[i].x = (double *)calloc(buffer * 2, sizeof(double));
		if (!del[i].x) {
			sprintf(mess, "Failed to allocate memory for delay.\n You might decrease the number of delays\n "
				" or the 'double Buffersize=%d;' in dymola/source/dsblock5.c\n", buffer);
			DymosimErr(mess);
			goto allocError;
		}
		del[i].y = del[i].x + buffer;
		del[i].nx = buffer;
	}
	return 0;
allocError:
	--i;
	for (; i >= 0; --i) {
		free(del[i].x);
		del[i].x = del[i].y = NULL;
		del[i].nx = 0;
	}
	return 1;
}


DYMOLA_STATIC int delayIni2(delayStruct * del, int delayID, double delaytime, int buffer, double startTime, double u)
{
	/*returns 1 for sucess 0 for failure*/
	del[delayID].timedelay = delaytime;
	del[delayID].x[0] = startTime - fabs(delaytime) - 1.0e6;
	/* the first time value of the buffer x[0] is
	the starTime minus the absolute value
	of the delaytime. To make sure that
	x[0] is smaller than x[1] even for delaytimes
	of zero, an additional an additional 1.0 is
	substracted from x[0]. */
	del[delayID].x[1] = startTime;

	del[delayID].y[0] = u;              /* <u>,here the inital u               */
	del[delayID].y[1] = u;              /* Extrapolation backwards will give constant u. */
	del[delayID].firstBoundaryHit = 1e33;
	del[delayID].minVal = del[delayID].maxVal = u;
	del[delayID].first = 0;
	del[delayID].add = 1;               /* at this position the last element
										was placed */
	del[delayID].old = 1;               /* set initial interpolation position;
										at this value <delayget> will start
										finding the upper and lower bounds  */

	return 1;
}
DYMOLA_STATIC int transportIni2(delayStruct * del, int delayID, int buffer, double startX, double u) {
	int i;
	int initSize = buffer / 10;
	int ret = (delayIni2(del, delayID, 1, buffer, startX + 1, u));
	del[delayID].x[0] = startX;
	del[delayID].x[1] = startX + 1e-9;

	for (i = 0; i<initSize; ++i)
		del[delayID].x[2 + i] = startX + (i + 0.5) / (initSize);
	del[delayID].x[initSize + 2] = startX + 1 - 1e-9;
	del[delayID].x[initSize + 3] = startX + 1;
	for (i = 0; i<initSize + 4; ++i) del[delayID].y[i] = u;
	del[delayID].add = initSize + 3;
	return ret;
}
static double getXTransport2(delayStruct * del, int id, int num) {
#if 0
	if (del[id].x[num] >= 1e100) {
		char str[100];
		sprintf(str, "Get %d => %g", num, del[id].x[num]);
		DymosimErr(str);
	}
#endif
	if (!del) return 1e100; /* Error */
	return del[id].x[num];
}
DYMOLA_STATIC void transportGet3(delayStruct* del, double T1, double T2, double x, int vs, int id, double* Tf1, double* Tf2, int ev) {
	double idd = id;

#if 0
	static const double neguno = -0.99999999999999;
	static const double uno = 0.999999999999999;
	double xp1 = x + 1;
	if (!vs) {
		*Tf1 = delayRead(&idd, &uno, &xp1);
		*Tf2 = T2;
	} else {
		*Tf1 = T1;
		*Tf2 = delayRead(&idd, &neguno, &x);
	}
#else
	if (del[id].x == 0) {
		*Tf1 = T1;
		*Tf2 = T2;
		return;
	}
	if (!vs) {
		int old = del[id].old;
		while (getXTransport2(del, id, old) <= x && old != del[id].add) {
			int oldnext = old + 1;
			if (oldnext >= del[id].nx) oldnext = 0;
#if 0
			{
				char str[200];
				sprintf(str, "Up %g %g %g", x, del[id].x[old], del[id].x[oldnext]);
				DymosimMessage(str);
			}
#endif
			old = oldnext;
		}

		for (;;) {
			int oldpre = old - 1;
			if (oldpre < 0) oldpre = del[id].nx - 1;
			if (oldpre == del[id].first) break;
			if (del[id].x[oldpre] < x || del[id].x[old] == x) break;
#if 0
			{
				char str[200];
				sprintf(str, "Down %g %g %g", x, del[id].x[oldpre], del[id].x[old]);
				DymosimMessage(str);
			}
#endif
			old = oldpre;
		}
		/* Post: del[id].x[oldpre]<=x<del[id].x[old], or at bounds */
		del[id].old = old;
		*Tf2 = T2;
		{
			int oldpre = (old == 0) ? (del[id].nx - 1) : (old - 1);
			double x0 = getXTransport2(del, id, oldpre);
			double x1 = getXTransport2(del, id, old);
			double y0 = del[id].y[oldpre];
			double y1 = del[id].y[old];
#if 0
			if (!(x0 <= x && x < x1)) {
				char str[200];
				sprintf(str, "Ooops2: %d %g %g %g %g %g %d %d %d %d", id, x, x0, x1, y0, y1, oldpre, old, del[id].first, del[id].add);
				DymosimMessage(str);
			}
#endif
			/* Note if x0=x=x1 we will take y1 or y0 depending on event and _direction_, compare other */
			if (x <= x0 && !(ev && x == x1))
				*Tf1 = y0;
			else if (x >= x1)
				*Tf1 = y1;
			else
				*Tf1 = y0 + (y1 - y0) * ((x)-x0) / (x1 - x0);
		}
	} else {
		int old = del[id].old;
		while (getXTransport2(del, id, old) <= x + 1 && old != del[id].add) {
			int oldnext = old + 1;
			if (oldnext >= del[id].nx) oldnext = 0;
			old = oldnext;
		}

		for (;;) {
			int oldpre = old - 1;
			if (oldpre < 0) oldpre = del[id].nx - 1;
			if (oldpre == del[id].first) break;
			if (del[id].x[oldpre] < x + 1 || del[id].x[old] == x + 1) break;
			old = oldpre;
		}
		/* Post: del[id].x[oldpre]<x<=del[id].x[old], or at bounds */
		*Tf1 = T1;
		del[id].old = old;
		{
			int oldpre = (old == 0) ? (del[id].nx - 1) : (old - 1);
			double x0 = getXTransport2(del, id, oldpre);
			double x1 = getXTransport2(del, id, old);
			double y0 = del[id].y[oldpre];
			double y1 = del[id].y[old];
#if 0
			if (!(x0 <= x + 1 && x + 1 < x1)) {
				char str[200];
				sprintf(str, "Ooops: %d %.17g %.17g %.17g %g %g", id, x, x0, x1, y0, y1);
				DymosimMessage(str);
			}
#endif
			if (x + 1 <= x0 && !(!ev && x + 1 == x1))
				*Tf2 = y0;
			else if (x + 1 >= x1)
				*Tf2 = y1;
			else
				*Tf2 = y0 + (y1 - y0) * ((x + 1) - x0) / (x1 - x0);
		}
	}
#endif
}

static void transportPut2A(delayStruct* del, double T1, double T2, double x, int vs, int id, int ev) {

	if (del[id].x == 0) return;
	if (!vs) {
		if (id == 1) {
			char str[200];
			sprintf(str, "Adding %lg %lg %d ", x, T2, ev);
			DymosimMessage(str);
		}
		/* Add T2 at x+1 as last value */
		int add = del[id].add;
		int preadd = add - 1;
		if (preadd < 0) preadd = del[id].nx - 1;

		if (ev && getXTransport2(del, id, add) == x + 1 && getXTransport2(del, id, preadd) == x + 1) {
			; /* Just replace 2nd item */
		} else if (getXTransport2(del, id, add) >= x + 1) {
			/* Remove future points*/
			if (getXTransport2(del, id, add) > x + 1) {
				while (add != del[id].first && getXTransport2(del, id, preadd) > x + 1) {
					int prepreadd = preadd;
					prepreadd = prepreadd - 1;
					if (prepreadd < 0) prepreadd = del[id].nx - 1;
#if 0
					{
						char str[200];
						sprintf(str, "Removing value2 %g %g %g", x, del[id].x[add], del[id].x[preadd]);
						DymosimMessage(str);
						del[id].x[add] = 1e300;
					}
#endif
					if (prepreadd == del[id].first) {
						/* Smooth handling of reversing flow*/
						del[id].x[preadd] = x + 0.999999999;
						break;
					}
					if (del[id].old == add)
						del[id].old = preadd;
					add = preadd;
					preadd = prepreadd;
		}
	}
			if (del[id].x[add] == x + 1 && del[id].x[preadd] != x + 1 && fabs(del[id].y[add] - T2) > 0.1) {
				/* Significant change in y during event */
				if (ev) {
					int postadd = add + 1;
					if (postadd >= del[id].nx)
						postadd = 0;
					if (postadd != del[id].first) {
						add = postadd;
					}
				} else if (del[id].y[preadd] == del[id].y[add]) {
					/* discontinuous change in y without event */
					del[id].x[preadd] = x + 1;
				}
			}
} else if (fabs(getXTransport2(del, id, preadd) - (x + 1)) >= 1.4 / del[id].nx && !(del[id].y[add] == T2 && del[id].y[preadd] == T2)) {
	/* Significant change in x, some change in y */
	int postadd = add + 1;
	if (postadd >= del[id].nx)
		postadd = 0;
	if (postadd == del[id].first && fabs(T2 - del[id].y[add]) + fabs(T2 - del[id].y[preadd]) <
		0.7 * fabs(T2 - del[id].y[postadd]) / del[id].nx) {
		/* overwrite existing: skip since no major change. We need two points that are unchanged to avoid extra ramps */
	} else {
		add = postadd;
		if (add == del[id].first) {
			int newfirst = del[id].first;
			newfirst++;
			if (newfirst >= del[id].nx) newfirst = 0;
			if (del[id].old == del[id].first) del[id].old = newfirst;
			del[id].first = newfirst;
		}
	}
} else {
	/* Skip it */
}
del[id].add = add;
del[id].x[add] = x + 1;
del[id].y[add] = T2;
		} else {
		/* Add T1 at x as first value */
		int first = del[id].first;
		int postfirst = first + 1;
		if (postfirst >= del[id].nx) postfirst = 0;
		if (ev && getXTransport2(del, id, first) == x && getXTransport2(del, id, postfirst) == x) {
			; /* Just replace it*/
		} else if (getXTransport2(del, id, first) <= x) {
			/* Remove future points*/
			if (getXTransport2(del, id, first) < x) while (first != del[id].add && getXTransport2(del, id, postfirst) < x) {
				int postpostfirst = postfirst + 1;
				if (postpostfirst >= del[id].nx) postpostfirst = 0;
#if 0
				{
					char str[200];
					sprintf(str, "Removing value %g %g %g", x, del[id].x[first], del[id].x[postfirst]);
					DymosimMessage(str);
					del[id].x[first] = 1e300;
				}
#endif
				if (postpostfirst == del[id].add) {
					/* Smooth handling of reversing flow*/
					del[id].x[postfirst] = x + 0.00000001;
					break;
				}
				if (del[id].old == first)
					del[id].old = postfirst;
				first = postfirst;
				postfirst = postpostfirst;
			}
			if (del[id].x[first] == x && del[id].x[postfirst] != x && fabs(del[id].y[first] - T1) > 0.1) {
				if (ev) {
					/* Significant change in y during event */
					int prefirst = first - 1;
					if (prefirst < 0)
						prefirst = del[id].nx - 1;
					if (prefirst != del[id].add) {
						first = prefirst;
					}
				} else if (del[id].y[postfirst] == del[id].y[first]) {
					/* discontinuous change in y without event */
					del[id].x[postfirst] = x;
				}
			}
		} else if (fabs(getXTransport2(del, id, postfirst) - x) >= 1.4 / del[id].nx && !(del[id].y[first] == T1 && del[id].y[postfirst] == T1)) {
			/* Significant change in x, some change in y */
			int prefirst = first - 1;
			if (prefirst < 0)
				prefirst = del[id].nx - 1;
#if 0
			if (prefirst == del[id].add) {
				char str[200];
				sprintf(str, "Possible overwrite: %g(%g) %g(%g) %g(%g)", T1, x, del[id].y[first], del[id].x[first],
					del[id].y[prefirst], del[id].x[prefirst]);
				DymosimMessage(str);
			}
#endif
			if (prefirst == del[id].add && fabs(T1 - del[id].y[first]) + fabs(T1 - del[id].y[postfirst]) <
				0.7 * fabs(T1 - del[id].y[prefirst]) / del[id].nx) {
				/* overwrite existing: skip since no major change. We need two points that are unchanged to avoid extra ramps */
		} else {
				first = prefirst;
				if (first == del[id].add) {
					int newadd = del[id].add;
					newadd--;
					if (newadd < 0) newadd = del[id].nx - 1;
					if (del[id].old == del[id].add) del[id].old = newadd;
					del[id].add = newadd;
				}
			}
		} else {
			/* Skip it */
		}
		del[id].first = first;
		del[id].x[first] = x;
		del[id].y[first] = T1;
	}
}
DYMOLA_STATIC void transportPut2(delayStruct* del, double T1, double T2, double x, int vs, int id) {
	transportPut2A(del, T1, T2, x, vs, id, 0);
			}
DYMOLA_STATIC void transportFunction3(delayStruct* del, double T1, double T2, double x, int vs, int id, double* Tf1, double* Tf2, int doStore) {
	if (doStore) transportPut2(del, T1, T2, x, vs, id);
	transportGet2(del, T1, T2, x, vs, id, Tf1, Tf2);
}
DYMOLA_STATIC void transportPut3(delayStruct* del, double T1pre, double T1, double T2pre, double T2, double x, int vs, int id, int ev) {
	if (ev == 1 && (T1pre != T1 || T2pre != T2)) {
		/* Note ev can be 2 as well */
		transportPut2A(del, T1pre, T2pre, x, vs, id, 0);
		transportPut2A(del, T1, T2, x, vs, id, ev);
	} else {
		transportPut2A(del, T1, T2, x, vs, id, ev);
	}
}
DYMOLA_STATIC void transportGet2(delayStruct* del, double T1, double T2, double x, int vs, int id, double* Tf1, double* Tf2) {
	transportGet3(del, T1, T2, x, vs, id, Tf1, Tf2, 0);
}
DYMOLA_STATIC double delayFunction2(delayStruct * del, int idd, double delaytime, double maxDelayTime, double t, double u, const char*exprStr)
{
	double dummy, y;
	if (delaytime>maxDelayTime*1.00001) {
		char str[DYNDiagnosticBuf];
		sprintf(str, "Delay time %g exceeded maximum delay time %g in \"" DYNDiagnosticStr "\"\n", delaytime, maxDelayTime, exprStr ? exprStr : "");
		DymosimErr(str);
	}
	dummy = delayWrite2(del, idd, t, u);     /* call delayWrite before ...           */
	y = delayRead2(del, idd, delaytime, t);  /*                    ... delayRead      */
	return (y);
}
DYMOLA_STATIC double delayFunction3(delayStruct* del, int idd, double delaytime, double maxDelayTime, double t, double u, const char* exprStr,
	int Event, double* rel, double* qp, double* qn, int* qen, double* qz2, int* AnyEvent, int* AnyDEvent /* may be null */)
{
	double dummy, y;
	if (delaytime > maxDelayTime * 1.00001) {
		char str[DYNDiagnosticBuf];
		sprintf(str, "Delay time %g exceeded maximum delay time %g in \"" DYNDiagnosticStr "\"\n", delaytime, maxDelayTime, exprStr ? exprStr : "");
		DymosimErr(str);
	}
	dummy = delayWrite2(del, idd, t, u);     /* call delayWrite before ...           */
	if (Event) {
		if (u > del[idd].maxVal) del[idd].maxVal = u;
		if (u < del[idd].minVal) del[idd].minVal = u;
	}
	y = delayRead2(del, idd, delaytime, t);  /*                    ... delayRead      */
	if (Event) {
		int ix;
		double y0;
		double span = del[idd].maxVal - del[idd].minVal;
		ix = del[idd].old;
		y0 = del[idd].y[ix];
		if (t - delaytime - del[idd].firstBoundaryHit >= 0 && (fabs(y0 - del[idd].minVal) < span * 1e-6 || fabs(y0 - del[idd].maxVal) < span * 1e-6)) {
			if (AnyEvent) *AnyEvent = 1;
			if (AnyDEvent) *AnyDEvent = 1;
		}
		del[idd].firstBoundaryHit = 1e33;
		for (; ;) {
			if (ix == del[idd].add) break;
			ix++;
			if (ix == del[idd].nx) ix = 0;
			{
				if ((fabs(del[idd].y[ix] - del[idd].minVal) < span * 1e-6 || fabs(del[idd].y[ix] - del[idd].maxVal) < span * 1e-6) && fabs(y - del[idd].y[ix]) > span * 1e-6) {
					del[idd].firstBoundaryHit = del[idd].x[ix];
					break;
				}
			}
		}
	}
	*rel = t - delaytime - del[idd].firstBoundaryHit;
	qz2[0] = *rel;
	qz2[1] = 1;
	return (y);
}
static void delayDWrite2(delayStruct * del, int id, double expr, double Time) {
	while (Time <= del[id].x[del[id].add]) {
		del[id].add = del[id].add - 1;
		if (del[id].add < 0)
			del[id].add = del[id].nx - 1;
	}
	if (del[id].y[del[id].add] == expr)
		return;

	del[id].add++;
	if (del[id].add > del[id].nx - 1)
		del[id].add = 0;

	/* Update first position if we are removing the oldest value. */
	if (del[id].add == del[id].first) {
		del[id].first++;
		if (del[id].first>del[id].nx - 1)
			del[id].first = 0;
	}
	del[id].x[del[id].add] = Time;
	del[id].y[del[id].add] = expr;
}
static int delayDnext2(delayStruct * del, int id, double x, double*timeNext) {

	if (x >= del[id].x[del[id].add]) {
		/* Extrapolation to future. */
		/* sprintf(mess,"Delay No. %d: "
		"Extrapolation to future occured at time %g \n", id, *t);
		DymosimMessage(mess); */
		/* Set second point for interpolation to the last point in buffer. */
		del[id].old = del[id].add;
		return 0;
	}
	else if (x < del[id].x[del[id].first]) {
		/* Extrapolation to past. */
		char mess[500];
		double buffersize = del[id].nx;
		sprintf(mess, "Extrapolation to past occured in Delay-Block No. %d: \n"
			"Buffersize(=%g) too small or too many changes for discrete variable\n"
			"for chosen simulation settings\n"
			"You might increase the double Buffersize=... in dymola/source/dsblock5.c", id, buffersize);
		/*  DymosimMessage(mess); */
		DymosimErr(mess);
	}
	if (x >= del[id].x[del[id].old]) {
		/* Search forward from previous time. */
		for (;;) {
			int newOld = del[id].old + 1;
			if (newOld>del[id].nx - 1) newOld = 0;
			*timeNext = del[id].x[newOld];
			if (x<*timeNext) {
				return 1;
			}
			del[id].old = newOld;
		}
	}
	else {
		/* Search backward from previous time. */
		for (;;) {
			volatile double d;
			int newOld = del[id].old - 1;
			if (newOld<0) newOld = del[id].nx - 1;
			d = del[id].x[newOld];
			del[id].old = newOld;
			if (x >= d) {
				*timeNext = del[id].x[del[id].old];
				return 1;
			}
		}
	}
}
DYMOLA_STATIC void registerTimeEvent(const double atTime);
DYMOLA_STATIC double delayDDFunction2(delayStruct * del, double expr, double dt, int *isAllocated, int delayId,
	int BufferSize, double Time, int Event, int*timed) {
	struct DymosimSimulator*dymosimData;
	double timeNext;
	dymosimData = dymosimGetThreadData();

	if (*isAllocated == 0) {
		*isAllocated = delayIni2(del, delayId, dt, BufferSize, Time, expr);
		if (!isAllocated) return 0;
		del[delayId].add = del[delayId].old = 0;
	}
	if (Event) delayDWrite2(del, delayId, expr, Time);
	if (Event && delayDnext2(del, delayId, Time - dt, &timeNext)) {
		if (Time<timeNext - 4e-14*(fabs(Time) + dymosimData->modelData.basicD->mOrigTimeError)) {
			registerTimeEvent(timeNext);
			*timed = 0;
		}
		else {
			*timed = 1;
		}
	}
	else {
		*timed = 0;
	}
	return del[delayId].y[del[delayId].old];
}
DYMOLA_STATIC double delayDCFunction3(delayStruct * del, double expr, double dt, int *isAllocated, int delayId, int BufferSize, double Time, int Event,
	double*rel, double*qp, double*qn, int*qen, double*qz2, int*AnyEvent, int *AnyDEvent /* may be null */) {
	double timeNext;
	if (*isAllocated == 0) {
		*isAllocated = delayIni2(del, delayId, dt, BufferSize, Time, expr);
		if (!*isAllocated) return 0;
		del[delayId].add = del[delayId].old = 0;
	}
	if (Event) delayDWrite2(del, delayId, expr, Time);
	if (Event) {
		int oldD = del[delayId].old;
		delayDnext2(del, delayId, Time - dt, &timeNext);
		if (oldD != del[delayId].old) {
			*AnyEvent = 1;
			if (AnyDEvent) *AnyDEvent = 1;
		}
	}
	*rel = Time - dt - del[delayId].x[del[delayId].old];
	qz2[0] = -*rel;
	if (del[delayId].old == del[delayId].add) {
		qz2[1] = 1;
	}
	else {
		int oldNext = del[delayId].old + 1;
		if (oldNext>del[delayId].nx - 1) oldNext = 0;
		qz2[1] = -(Time - dt - del[delayId].x[oldNext]) + 1e-16;
	}
	return del[delayId].y[del[delayId].old];
}

DYMOLA_STATIC double delayDCFunction2(delayStruct* del, double expr, double dt, int* isAllocated, int delayId, int BufferSize, double Time, int Event,
	double* rel, double* qp, double* qn, int* qen, double* qz2, int* AnyEvent) {
	return delayDCFunction3(del, expr, dt, isAllocated, delayId, BufferSize, Time, Event, rel, qp, qn, qen, qz2, AnyEvent, 0);
}

DYMOLA_STATIC double delayWrite2(delayStruct * del, int id, double t, double u)
{
	/* Remove "future" values if any. */
	while (t <= del[id].x[del[id].add]) {
		del[id].add = del[id].add - 1;
		if (del[id].add < 0)
			del[id].add = del[id].nx - 1;
	}

	/* Update last position. */
	del[id].add++;
	if (del[id].add > del[id].nx - 1)
		del[id].add = 0;

	/* Update first position if we are removing the oldest value. */
	if (del[id].add == del[id].first) {
#if 1
		/* Throw away 1-keepnom/keepdenom of first 0.7 of values */
		/* Better than simply stopping */
		int id2 = (del[id].first + (int)(del[id].nx*0.7)) % del[id].nx;
		int iold = id2, inew = id2;
		const int keepnom = 3, keepdenom = 4;
		int keepcounter = 0;
		for (;;) {
			del[id].x[inew] = del[id].x[iold];
			del[id].y[inew] = del[id].y[iold];
			if (iold == del[id].first) break;
			iold--;
			if (iold<0) iold = del[id].nx - 1;
			keepcounter++;
			if (keepcounter >= keepdenom) keepcounter = 0;
			if (keepcounter<keepnom) {
				inew--;
				if (inew<0) inew = del[id].nx - 1;
			}
		}
		del[id].first = inew;
#else
		del[id].first++;
		if (del[id].first > del[id].nx - 1)
			del[id].first = 0;
#endif
	}

	/* Store values. */
	del[id].x[del[id].add] = t;        /* store new t */
	del[id].y[del[id].add] = u;        /* store new u */

	return (u);
}
DYMOLA_STATIC double delayRead2(delayStruct * del, int id, double delaytime, double t)
{
	/* convert id to integer. */
	double x, xup, xlow;
	double y, yup, ylow;
	int oldold;
	char mess[500];
	double buffersize;

	oldold = del[id].old;
	del[id].timedelay = delaytime;
	x = t - delaytime;                /* calculate interpolation x */

	if (getBasicIDymosimStruct()->mInJacobian == 2) {
		if (!getenv("DYN_ENABLE_DELAY_LIN"))
			DymosimErr("A model using delay is linearized; the results would not be correct.\n"
				"If you really want to get some results you can set the environment variable, DYN_ENABLE_DELAY_LIN, please contact support for more information.");
		getBasicIDymosimStruct()->mInJacobian = 1; /* Avoid repeated warnings/errors*/
	}
	if (x > del[id].x[del[id].add]) {
		/* Extrapolation to future. */
		/* sprintf(mess,"Delay No. %d: "
		"Extrapolation to future occured at time %g \n", id, *t);
		DymosimMessage(mess); */
		/* Set second point for interpolation to the last point in buffer. */
		del[id].old = del[id].add;
	}
	else if (x < del[id].x[del[id].first]) {
		/* Extrapolation to past. */
		buffersize = del[id].nx;
		sprintf(mess, "Extrapolation to past occured in Delay-Block No. %d: \n"
			"Buffersize(=%g) too small or DelayTime(=%g sec) too big\n"
			"for chosen simulation settings\n"
			"You might increase the double Buffersize=... in dymola/source/dsblock5.c", id, buffersize, delaytime);
		/*  DymosimMessage(mess); */
		DymosimErr(mess);
		/* Set second point for interpolation to the first+1 point in buffer. */
		del[id].old = del[id].first + 1;
		if (del[id].old > del[id].nx - 1)
			del[id].old = 0;
	}
	else if (x > del[id].x[del[id].old]) {
		/* Search forward from previous time. */
		while (x > del[id].x[del[id].old]) {
			del[id].old++;
			if (del[id].old > del[id].nx - 1)
				del[id].old = 0;
		}
	}
	else {
		/* Search backward from previous time. */
		while (x <= del[id].x[del[id].old] && del[id].old != del[id].first) {
			del[id].old--;
			if (del[id].old < 0)
				del[id].old = del[id].nx - 1;
		}

		/* old should be second point for interpolation. */
		del[id].old++;
		if (del[id].old > del[id].nx - 1)
			del[id].old = 0;
	}

	xup = del[id].x[del[id].old];       /* calculate upper bound for interpolation */
	if (del[id].old > 0)                /* calculate xlow               */
		xlow = del[id].x[del[id].old - 1];
	else
		xlow = del[id].x[del[id].nx - 1];

	yup = del[id].y[del[id].old];       /* calculate final yup */
	if (del[id].old > 0)                /* calculate final ylow */
		ylow = del[id].y[del[id].old - 1];
	else
		ylow = del[id].y[del[id].nx - 1];

	/* The interpolation/extrapolation formula is given by: */
	y = ylow + (x - xlow) * (yup - ylow) / (xup - xlow);

	return (y);
}

DYMOLA_STATIC int copyDelay(delayStruct tgt[], delayStruct src[], const int nbrDel) {

	int i = 0;
	for (i = 0; i < nbrDel; ++i) {
		double * tmp = 0;
		size_t sizeDataBlock = (size_t)2 * src[i].nx * sizeof(double);
		if (tgt[i].nx != src[i].nx) {
			free(tgt[i].x);
			tgt[i].x = (double*)malloc(sizeDataBlock);
			if (!tgt[i].x)return 1;
		}
		memcpy(tgt[i].x, src[i].x, sizeDataBlock);
		tmp = tgt[i].x;
		memcpy(&tgt[i], &src[i], sizeof(tgt[i]));
		tgt[i].x = tmp;
		tgt[i].y = tgt[i].x + tgt[i].nx;
	}
	return 0;
}

DYMOLA_STATIC void freeDelay(delayStruct* del, const size_t nbrDel) {
	size_t i = 0;
	for (i = 0; i < nbrDel; ++i) {
		free(del[i].x);
		memset(&del[i], 0, sizeof(delayStruct));
	}
}
