/* Generated by SimInf (v6.2.0) 2019-03-13 14:23 */

#include <R_ext/Visibility.h>
#include <R_ext/Rdynload.h>
#include "SimInf.h"

/* Offset in integer compartment state vector */
enum {S, E, I, R};

/* Offsets in node local data (ldata) to parameters in the model */
enum {PHI, END_T1, END_T2, END_T3, END_T4, NEIGHBOR};

/* Offsets in global data (gdata) to parameters in the model */
enum {BETA, UPSILON, EPSILON, GAMMA, BETA_T1, BETA_T2, BETA_T3, BETA_T4, COUPLING};

/**
 * Decay of environmental infectious pressure with a forward Euler step.
 *
 * The time dependent beta is divided into four intervals of the year
 * where 0 <= day < 365
 *
 * Case 1: END_1 < END_2 < END_3 < END_4
 * INTERVAL_1 INTERVAL_2     INTERVAL_3     INTERVAL_4     INTERVAL_1
 * [0, END_1) [END_1, END_2) [END_2, END_3) [END_3, END_4) [END_4, 365)
 *
 * Case 2: END_3 < END_4 < END_1 < END_2
 * INTERVAL_3 INTERVAL_4     INTERVAL_1     INTERVAL_2     INTERVAL_3
 * [0, END_3) [END_3, END_4) [END_4, END_1) [END_1, END_2) [END_2, 365)
 *
 * Case 3: END_4 < END_1 < END_2 < END_3
 * INTERVAL_4 INTERVAL_1     INTERVAL_2     INTERVAL_3     INTERVAL_4
 * [0, END_4) [END_4, END_1) [END_1, END_2) [END_2, END_3) [END_3, 365)
 *
 * @param phi The currrent value of the environmental infectious pressure.
 * @param day The day of the year 0 <= day < 365.
 * @param end_t1 The non-inclusive day that ends interval 1.
 * @param end_t2 The non-inclusive day that ends interval 2.
 * @param end_t3 The non-inclusive day that ends interval 3.
 * @param end_t4 The non-inclusive day that ends interval 4.
 * @param beta_t1 The value for beta in interval 1.
 * @param beta_t2 The value for beta in interval 2.
 * @param beta_t3 The value for beta in interval 3.
 * @param beta_t4 The value for beta in interval 4.
 * @return phi * (1.0 - beta) (where beta is the value for the interval)
 */
double SimInf_forward_euler_linear_decay(
    double phi, int day,
    int end_t1, int end_t2, int end_t3, int end_t4,
    double beta_t1, double beta_t2, double beta_t3, double beta_t4)
{
    if (day < end_t2) {
        if (day < end_t1) {
            if (end_t1 < end_t4)
                return phi * (1.0 - beta_t1);
            if (day < end_t4) {
                if (end_t4 < end_t3)
                    return phi * (1.0 - beta_t4);
                if (day < end_t3)
                    return phi * (1.0 - beta_t3);
                return phi * (1.0 - beta_t4);
            }
            return phi * (1.0 - beta_t1);
        }

        return phi * (1.0 - beta_t2);
    }

    if (end_t3 < end_t1 || day < end_t3)
        return phi * (1.0 - beta_t3);

    if (end_t4 < end_t1 || day < end_t4)
        return phi * (1.0 - beta_t4);

    return phi * (1.0 - beta_t1);
}

double trFun1( /* Transition from S to E */
    const int *u,
    const double *v,
    const double *ldata,
    const double *gdata,
    double t)
{
    return gdata[BETA]*u[S]*u[I]/(u[S]+u[E]+u[I]+u[R])+gdata[UPSILON]*ldata[PHI]*u[S];
}

double trFun2( /* Transition from E to I */
    const int *u,
    const double *v,
    const double *ldata,
    const double *gdata,
    double t)
{
    return gdata[EPSILON]*u[E];
}

double trFun3( /* Transition from I to R */
    const int *u,
    const double *v,
    const double *ldata,
    const double *gdata,
    double t)
{
    return gdata[GAMMA]*u[I];
}

int ptsFun( /* Post time-step function */
    double *v_new,
    const int *u,
    const double *v,
    const double *ldata,
    const double *gdata,
    int node,
    double t)
{
    const int day = (int)t % 365;
    const double E_i = u[E];
    const double I_i = u[I];
    const double N_i = u[S] + E_i + I_i + u[R];
    const double phi = v[PHI];
    const int Nc = 2;

    /* Determine the pointer to the continuous state vector in the
     * first node. Use this to find phi at neighbours to the current
     * node. */
    const double *phi_0 = &v[-node];

    /* Determine the pointer to the compartment state vector in the
     * first node. Use this to find the number of individuals at
     * neighbours to the current node. */
    const int *u_0 = &u[-Nc*node];

    /* Time dependent beta in each of the four intervals of the
     * year. Forward Euler step. */
    v_new[PHI] = SimInf_forward_euler_linear_decay(
        phi, day,
        ldata[END_T1], ldata[END_T2], ldata[END_T3], ldata[END_T4],
        gdata[BETA_T1], gdata[BETA_T2], gdata[BETA_T3], gdata[BETA_T4]);

    /* Local spread among proximal nodes. */
    if (N_i > 0.0) {
        v_new[PHI] += gdata[ALPHA] * I_i / N_i +
            SimInf_local_spread(&ldata[NEIGHBOR], phi_0, u_0,
                                N_i, phi, Nc, gdata[COUPLING]);
    }

    if (!isfinite(v_new[PHI]))
        return SIMINF_ERR_V_IS_NOT_FINITE;
    if (v_new[PHI] < 0.0)
        return SIMINF_ERR_V_IS_NEGATIVE;
    return phi != v_new[PHI]; /* 1 if needs update */
}

/* MODEL SOLVER STUFF */

SEXP SimInf_model_run(SEXP model, SEXP threads, SEXP solver)
{
    TRFun tr_fun[] = {&trFun1, &trFun2, &trFun3};
    DL_FUNC SimInf_run = R_GetCCallable("SimInf", "SimInf_run");
    return SimInf_run(model, threads, solver, tr_fun, &ptsFun);
}

static const R_CallMethodDef callMethods[] =
{
    {"SimInf_model_run", (DL_FUNC)&SimInf_model_run, 3},
    {NULL, NULL, 0}
};

void attribute_visible R_init_SEIRe_sp(DllInfo *info)
{
    R_registerRoutines(info, NULL, callMethods, NULL, NULL);
    R_useDynamicSymbols(info, FALSE);
    R_forceSymbols(info, TRUE);
}

