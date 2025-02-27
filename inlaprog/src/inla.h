
/* inla.h
 * 
 * Copyright (C) 2007-2024 Havard Rue
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * The author's contact information:
 *
 *        Haavard Rue
 *        CEMSE Division
 *        King Abdullah University of Science and Technology
 *        Thuwal 23955-6900, Saudi Arabia
 *        Email: haavard.rue@kaust.edu.sa
 *        Office: +966 (0)12 808 0640
 *
 *
 */
#ifndef __INLA_H__
#define __INLA_H__

#undef __BEGIN_DECLS
#undef __END_DECLS
#ifdef __cplusplus
#define __BEGIN_DECLS extern "C" {
#define __END_DECLS }
#else
#define __BEGIN_DECLS					       /* empty */
#define __END_DECLS					       /* empty */
#endif

__BEGIN_DECLS
#include "iniparser.h"
#include "dictionary.h"
#include "strlib.h"
#include "ar.h"
#include "fgn.h"
#include "stochvol.h"
#include "quantile-regression.h"
#include "cgeneric.h"
#define ONE_mexp(_x) (-expm1(_x))			       /* 1-exp(_x) */
#define LOG_1mp(_x) log1p(-(_x))			       /* log(1-(_x)) */
#define LOG_p(_x) log1p((_x) - 1.0)
#define LOG_NORMC_GAUSSIAN (-0.91893853320467274178032973640560)	/* -1/2 * log(2*pi) */
#define INLA_FAIL  1
#define INLA_OK    0
// just to have a big and small number to use
#define INLA_REAL_BIG   GSL_SQRT_FLT_MAX
#define INLA_REAL_SMALL FLT_EPSILON
#define INLA_SIGN(_x) ((_x) >= 0.0 ? 1.0 : -1.0)
#define INLA_SPECIAL_NUMBER (1048576.0)			       // 2^20
#define INLA_IS_SPECIAL(_x) ISZERO( (_x) - INLA_SPECIAL_NUMBER)

/*
 * The scaling of the critical 'alpha' parameter. If this value change, it must also be changed in models.R
 *
 * YES, CHANGE IT MANUALLY!
 */
#define INLA_WEIBULL_ALPHA_SCALE 0.1
#define INLA_GOMPERTZ_ALPHA_SCALE 0.1

/*
 * The scaling of the critical 'precision' parameter. If this value change, it must also be changed in models.R 
 *
 * YES, CHANGE IT MANUALLY!
 */
#define INLA_QKUMAR_PREC_SCALE 0.10
#define INLA_WISHARTK_KMAX (20)
#define INLA_WISHARTK_KMIN  (2)
#define INLA_WISHARTK_NTHETA(k_) (((k_)*((k_) + 1))/2L)
#define INLA_WISHARTK_NPARAM(k_) (INLA_WISHARTK_NTHETA(k_) + 1L)
#define INLA_LP_SCALE_MAX 100

/* 
 *
 */
    typedef enum {
	/*
	 * Failure time
	 */
	SURV_EVENT_FAILURE = 1,				       /* never change!!! */

	/*
	 * Right cencored
	 */
	SURV_EVENT_RIGHT = 0,				       /* never change!!! */

	/*
	 * Left cencored
	 */
	SURV_EVENT_LEFT = 2,

	/*
	 * Interval cencored
	 */
	SURV_EVENT_INTERVAL = 3,

	/*
	 * In an interval
	 */
	SURV_EVENT_ININTERVAL = 4
} inla_surv_event_tp;

typedef enum {
	INLA_MODE_DEFAULT = 0,
	INLA_MODE_HYPER,
	INLA_MODE_QINV,
	INLA_MODE_QSOLVE,
	INLA_MODE_QREORDERING,
	INLA_MODE_QSAMPLE,
	INLA_MODE_FINN,
	INLA_MODE_GRAPH,
	INLA_MODE_R,
	INLA_MODE_FGN,
	INLA_MODE_PARDISO,
	INLA_MODE_OPENMP,
	INLA_MODE_DRYRUN,
	INLA_MODE_TESTIT = 999
} inla_mode_tp;

typedef enum {
	/*
	 * map from internal representation to the user representation and link-functions. EX: precision = exp(log_prec), or the
	 * inverse-link-function
	 */
	MAP_FORWARD = GMRFLib_TRANSFORM_FORWARD,	       /* = 1 */
	INVLINK = GMRFLib_TRANSFORM_FORWARD,
	/*
	 * the inverse of the _FORWARD map, EX: log_prec = log(precision), or the link-function
	 */
	MAP_BACKWARD = GMRFLib_TRANSFORM_BACKWARD,	       /* = 2 */
	LINK = GMRFLib_TRANSFORM_BACKWARD,
	/*
	 * the derivative of the forward_map, EX: exp(log_prec). or the derivative of the inverse link-function.
	 */
	MAP_DFORWARD = GMRFLib_TRANSFORM_DFORWARD,	       /* = 3 */
	DINVLINK = GMRFLib_TRANSFORM_DFORWARD,
	/*
	 * return 1 is monotone increasing and 0 otherwise
	 */
	MAP_INCREASING = GMRFLib_TRANSFORM_INCREASING,	       /* = 4 */
	LINKINCREASING = GMRFLib_TRANSFORM_INCREASING
} map_arg_tp;

/* 
   this is needed so we can identify each component in the model
 */
typedef enum {
	INVALID_COMPONENT = 0,
	L_GAUSSIAN = 1,					       /* likelihood-models */
	L_LOGISTIC,
	L_SKEWNORMAL,
	L_GEV,
	L_T,
	L_TSTRATA,
	L_POISSON,
	L_GPOISSON,
	L_BINOMIAL,
	L_CBINOMIAL,					       /* clumped binomial */
	L_ZEROINFLATEDBINOMIAL0,
	L_ZEROINFLATEDBINOMIAL1,
	L_ZEROINFLATEDBINOMIAL2,
	L_ZEROINFLATEDBETABINOMIAL0,
	L_ZEROINFLATEDBETABINOMIAL1,
	L_ZEROINFLATEDBETABINOMIAL2,
	L_GAMMA,
	L_BETA,
	L_BETABINOMIAL,
	L_NBINOMIAL,
	L_ZEROINFLATEDNBINOMIAL0,
	L_ZEROINFLATEDNBINOMIAL1,
	L_ZEROINFLATEDNBINOMIAL2,
	L_ZEROINFLATEDNBINOMIAL1STRATA2,
	L_ZEROINFLATEDNBINOMIAL1STRATA3,
	L_STOCHVOL,
	L_STOCHVOL_T,
	L_STOCHVOL_NIG,
	L_LOGPERIODOGRAM,
	L_EXPONENTIAL,
	L_EXPONENTIALSURV,
	L_WEIBULL,
	L_WEIBULLSURV,
	L_LOGNORMAL,
	L_LOGNORMALSURV,
	L_ZEROINFLATEDPOISSON0,
	L_ZEROINFLATEDPOISSON1,
	L_ZEROINFLATEDPOISSON2,
	L_ZERO_N_INFLATEDBINOMIAL2,
	L_ZERO_N_INFLATEDBINOMIAL3,
	L_LOGGAMMA_FRAILTY,
	L_IID_GAMMA,
	L_IID_LOGITBETA,
	L_CIRCULAR_NORMAL,
	L_WRAPPED_CAUCHY,
	REMOVED___L_TEST_BINOMIAL_1,
	L_SIMPLEX,
	L_GAMMACOUNT,
	L_SKEWNORMAL2____NO_LONGER_IN_USE,
	L_QKUMAR,
	L_QCONTPOISSON,					       /* DISABLED in models.R */
	L_CENPOISSON,					       /* cencored poisson */
	L_NMIX,
	L_NMIXNB,
	L_GP,
	L_CONTPOISSON,					       /* DISABLED in models.R */
	L_LOGLOGISTIC,
	L_LOGLOGISTICSURV,
	L_QLOGLOGISTIC,
	L_QLOGLOGISTICSURV,
	L_POM,
	L_BGEV,
	L_NBINOMIAL2,
	L_GAMMASURV,
	L_BETABINOMIALNA,
	L_XPOISSON,
	L_DGP,
	L_XBINOMIAL,
	L_ZEROINFLATEDCENPOISSON0,
	L_ZEROINFLATEDCENPOISSON1,
	L_POISSON_SPECIAL1,
	L_GAMMAJW,
	L_GAMMAJWSURV,
	L_TWEEDIE,
	L_FMRI,
	L_FMRISURV,
	L_AGAUSSIAN,					       /* likelihood-models */
	L_GOMPERTZ,
	L_GOMPERTZSURV,
	L_STOCHVOL_SN,
	L_CENPOISSON2,					       /* cencored poisson (version 2) */
	L_CENNBINOMIAL2,				       /* cencored nbinomial (similar to cenpoisson2) */
	L_GAUSSIANJW,
	L_0POISSON,
	L_0POISSONS,
	L_0BINOMIAL,
	L_0BINOMIALS,
	L_BELL,
	L_NZPOISSON,
	L_STDGAUSSIAN,
	L_GGAUSSIAN,
	L_GGAUSSIANS,
	L_FL,
	L_RCPOISSON,
	L_TPOISSON,
	F_RW2D = 1000,					       /* f-models */
	F_BESAG,
	F_BESAG2,					       /* the [a*x, x/a] model */
	F_BESAGPROPER,
	F_BESAGPROPER2,					       /* The alternative parameterisation from Leroux et al. */
	F_SEASONAL,
	F_IID,
	F_2DIID,
	F_IID1D,
	F_IID2D,
	F_IID3D,
	F_IID4D,
	F_IID5D,
	F_RW1,
	F_RW2,
	F_CRW2,
	F_AR1,
	F_AR,
	F_OU,
	F_Z,
	F_BYM,
	F_BYM2,
	F_GENERIC0,
	F_GENERIC1,
	F_GENERIC2,
	F_MATERN2D,
	F_SPDE,
	F_SPDE2,
	F_COPY,
	F_MEC,
	F_MEB,
	F_R_GENERIC________DISABLED,
	F_SLM,
	F_CLINEAR,					       /* constrained fixed effect */
	F_SIGM,
	F_REVSIGM,
	F_RW2DIID,
	F_SPDE3,
	F_GENERIC3,
	F_LOG1EXP,
	F_LOGDIST,
	F_R_GENERIC,
	F_FGN,
	F_FGN2,
	F_AR1C,
	F_DMATERN,
	F_INTSLOPE,
	F_IIDKD,
	F_C_GENERIC,
	F_SCOPY,
	P_FIRST_ENTRY_FOR_PRIORS____NOT_FOR_USE = 2000,	       /* priors */
	P_BETACORRELATION,
	P_DIRICHLET,
	P_EXPRESSION,
	P_FLAT,
	P_GAMMA,
	P_GAUSSIAN,
	P_INVALID,
	P_JEFFREYS_T_DF,
	P_LOGFLAT,
	P_LOGGAMMA,
	P_LOGIFLAT,
	P_LOGITBETA,
	P_MINUSLOGSQRTRUNCGAUSSIAN,
	P_MVGAUSSIAN,
	P_MVNORM,
	P_NONE,
	P_PC_ALPHAW,
	P_PC_AR,
	P_PC_COR0,
	P_PC_COR1,
	P_PC_DOF,
	P_PC_FGN_H,
	P_PC_GAMMA,
	P_PC_GAMMACOUNT,
	P_PC_MATERN,
	P_PC_MGAMMA,
	P_PC_PREC,
	P_PC_RANGE,
	P_PC_SPDE_GA,					       /* Experimental prior from GA when dim(theta)=2 */
	P_PC_GEVTAIL,
	P_REF_AR,					       /* Reference prior for AR(p) for p=1,2,3 */
	P_TABLE,
	P_WISHART1D,
	P_WISHART2D,
	P_WISHART3D,
	P_WISHART4D,
	P_WISHART5D,
	P_PC_SN,
	P_SN_INTERCEPT,
	P_WISHARTK_2D,
	P_WISHARTK_3D,
	P_WISHARTK_4D,
	P_WISHARTK_5D,
	P_WISHARTK_6D,
	P_WISHARTK_7D,
	P_WISHARTK_8D,
	P_WISHARTK_9D,
	P_WISHARTK_10D,
	P_WISHARTK_11D,
	P_WISHARTK_12D,
	P_WISHARTK_13D,
	P_WISHARTK_14D,
	P_WISHARTK_15D,
	P_WISHARTK_16D,
	P_WISHARTK_17D,
	P_WISHARTK_18D,
	P_WISHARTK_19D,
	P_WISHARTK_20D,
	P_LAPLACE,
	P_RPRIOR,
	G_EXCHANGEABLE = 3000,				       /* group models */
	G_EXCHANGEABLE_POS,
	G_AR1,
	G_RW1,
	G_RW2,
	G_AR,
	G_BESAG,
	G_IID,
	MIX_GAUSSIAN = 4000,				       /* mix-models */
	MIX_LOGGAMMA,
	MIX_MLOGGAMMA,
	LINK_IDENTITY = 5000,				       /* link-models */
	LINK_LOG,
	LINK_NEGLOG,
	LINK_PROBIT,
	LINK_CLOGLOG,
	LINK_LOGIT,
	LINK_TAN,
	LINK_TEST1,
	LINK_SPECIAL1,
	LINK_SPECIAL2,					       /* exp(eta)*((1-x) + x*exp(beta)) for Poisson (JW) */
	LINK_LOGOFFSET,
	LINK_SSLOGIT,
	LINK_LOGLOG,
	LINK_CAUCHIT,
	LINK_LOGITOFFSET,
	LINK_INVERSE,
	LINK_QPOISSON,
	LINK_QBINOMIAL,
	LINK_QWEIBULL,
	LINK_QGAMMA,
	LINK_ROBIT,
	LINK_SN,
	LINK_LOGa,
	LINK_POWER_LOGIT,
	LINK_CCLOGLOG,
	LINK_GEV,
	LINK_CGEV
} inla_component_tp;

typedef enum {
	POM_CDF_DEFAULT = 1,
	POM_CDF_LOGIT = 1,
	POM_CDF_PROBIT = 2
} inla_pom_cdf_tp;

typedef double map_func_tp(double arg, map_arg_tp typ, void *param);
typedef double link_func_tp(int thread_id, double arg, map_arg_tp typ, void *param, double *cov);

typedef struct {
	const char *name;
	map_func_tp *func;
} map_table_tp;

typedef struct {
	size_t len;
	char *contents;
} inla_file_contents_tp;

/* 
   priors are defined using this template. return log(pi(precision, parameters....))
 */
typedef double inla_priorfunc_tp(double *param, double *parameters);

typedef struct {
	inla_component_tp id;				       /* prior Id */
	char *hyperid;					       /* hyperpar Id */
	char *name;					       /* name of prior */
	char *rprior;					       /* name of rprior */
	double *parameters;				       /* the parameters */
	char *to_theta;					       /* R-code */
	char *from_theta;				       /* R-code */
	inla_priorfunc_tp *priorfunc;			       /* Either a priorfunction, or */
	char *expression;				       /* an alternative expression/table */
} Prior_tp;

typedef struct {
	double *d;					       /* the d-array */
	int ndata;					       /* length of data (from file) */
	double *y;					       /* general responce */
	double quantile;				       /* value of the quantile for quantile parameterised likelihoods */

	double *attr;					       /* for inla.mdata() data */
	int n_attr;

	/*
	 * y ~ Poisson(E*exp(x)) 
	 */
	double *E;
	double *cen_low;				       /* cenpoisson2 */
	double *cen_high;				       /* cenpoisson2 */

	/*
	 * y ~ Binomial(nb, p(x))
	 */
	double *nb;
	double *p_scale;

	/*
	 * nbinomial
	 */
	double *S;

	/*
	 * y ~ BinomialRE()
	 */
	double **log_prec_binomialre;

	/*
	 * y ~ CBinomial(k n, p(x))
	 */
	double *cbinomial_k;
	double *cbinomial_n;

	/*
	 * y ~ Neg.Binomial(n, p(x)), n=size is a hyperparameter (overdispersion)
	 */
	double **log_size;

	/*
	 * y ~ Neg.Binomial(n, p(x)) strata2
	 */
	double *strata;					       /* type int */
	double ***log_sizes;

	/*
	 * y ~ POM
	 */
	double ***pom_theta;
	int pom_nclasses;
	inla_pom_cdf_tp pom_cdf;
	int pom_fast_probit;

	/*
	 * y ~ Normal(x, 1/(weight*prec)), also used for the log-normal
	 */
	double **log_prec_gaussian;
	double **log_prec_gaussian_offset;
	double *weight_gaussian;			       /* weights for the gaussian: Variance = 1/(weight*prec) */

	/*
	 *  Beta
	 */
	double *beta_weight;
	double beta_censor_value;

	/*
	 * y ~ Simplex(....,1/(weight*prec))
	 */
	double **log_prec_simplex;
	double *weight_simplex;				       /* weights: Variance = 1/(weight*prec) */

	/*
	 * y ~ Logistic, Variance = 1/ (weight * prec)
	 */
	double **log_prec_logistic;
	double *weight_logistic;

	/*
	 * y ~ T_dof(x, 1/(weight*prec)), where T_dof has Variance=1
	 */
	double **log_prec_t;
	double **dof_intern_t;
	double *weight_t;				       /* weights for the t: Variance = 1/(weight*prec) */

	/*
	 * y ~ Tstrata 
	 */
	double ***log_prec_tstrata;			       /* array of log_prec_t */
	double **dof_intern_tstrata;			       /* common dof */
	double *weight_tstrata;				       /* weight_t */
	double *strata_tstrata;				       /* strata_t (type int) */

	/*
	 * stocvol
	 */
	double **log_offset_prec;

	/*
	 * stochvol_t: y ~ Student-t_dof (x) with Variance=1. dof_intern_svt = log(dof - 2).
	 */
	double **dof_intern_svt;
	/*
	 * stochvol_nig: y ~ normal.inverse.gaussian(x) with Variance=1.
	 *
	 *  shew is the beta-parameter, which determine the skewness
	 *  shape is psi = 1+exp(shape), which determine the shape. 
	 */
	double **skew_intern_svnig;
	double **shape_intern_svnig;
	/*
	 * survival: event is 1 is y is the failure time or 0 if its right sensored 
	 */
	double **alpha_intern;				       /* For the Weibull, PS, loglogistic and gompertz */
	double *truncation;				       /* for survival */
	double *event;					       /* one of inla_surv_event_tp */
	double *lower;					       /* for survival */
	double *upper;					       /* for survival */
	// double *time == y
	double **p_intern;				       /* For the L_WEIBULL_CURE */

	int cure_ncov;
	double *cure_cov;
	double ***cure_beta;

	/*
	 * zero-inflated Poission/Binomial/NegativeBinomial/BetaBinomial version 0/1/2...
	 */
	double **prob_intern;
	double **zeroinflated_alpha_intern;		       /* alpha = exp(alpha_intern) */
	double **zeroinflated_delta_intern;		       /* delta = exp(delta_intern) */
	double **zeroinflated_rho_intern;		       /* rho = exp(rho_intern)/(1+exp(rho_intern)) */

	double ***probN_intern;				       /* the new for strata2 */

	/*
	 * the zero-n-inflated binomial 2 
	 */
	double **zero_n_inflated_alpha1_intern;
	double **zero_n_inflated_alpha2_intern;

	/*
	 * the zero-n-inflated binomial 3
	 */
	double **zero_n_inflated_alpha0_intern;
	double **zero_n_inflated_alphaN_intern;

	/*
	 * the overdispersion parameter for the betabinomial and betabinomialna, \rho = 1/(a+b+1).
	 */
	double **betabinomial_overdispersion_intern;
	double *betabinomialnb_scale;

	/*
	 * the precision parameter for the beta, \phi = exp(theta)
	 */
	double **beta_precision_intern;

	/*
	 * Skew-Normal
	 */
	double **sn_skew;
	double **sn_lprec;
	double *sn_scale;				       /* Variance = 1/(scale*prec) */

	/*
	 * GEV 
	 */
	double *weight_gev;				       /* weights for the gev: Variance propto 1/(weight*prec) */
	double **log_prec_gev;				       /* log prec for gev */
	double **xi_gev;				       /* the shape-parameter */
	double gev_scale_xi;				       /* scaling of the shape-parameter */

	/*
	 * bGEV
	 */
	double bgev_qlocation;
	double bgev_qspread;
	double bgev_beta_ab;
	double *bgev_tail_interval;
	double *bgev_qmix;
	double *bgev_scale;
	double **bgev_x;				       /* matrix of covariates */
	double ***bgev_betas;				       /* vector of betas */
	double **bgev_log_spread;
	double **bgev_intern_tail;
	int bgev_nbetas[2];
	int *bgev_ncols;

	/*
	 * Log gamma frailty
	 */
	double **log_prec_loggamma_frailty;

	/*
	 * iid gamma 
	 */
	double *iid_gamma_scale;
	double **iid_gamma_log_shape;
	double **iid_gamma_log_rate;

	/*
	 * iid logitbeta 
	 */
	double **iid_logitbeta_log_a;
	double **iid_logitbeta_log_b;

	/*
	 * Sinh-asinh
	 */
	double *sas_weight;
	double **sas_log_prec;
	double **sas_skew;
	double **sas_kurt;

	/*
	 * y ~ Circular Normal, with precision parameter: weight*prec
	 */
	double **log_prec_circular_normal;
	double *weight_circular_normal;

	/*
	 * y ~ Circular Cauchy, with precision parameter: weight*prec, related to the rho-parameter; see doc. 0<weight<=1
	 */
	double **log_prec_wrapped_cauchy;
	double *weight_wrapped_cauchy;

	/*
	 * generalised poisson
	 */
	double **gpoisson_overdispersion;
	double **gpoisson_p;

	/*
	 * cencored poisson. values in [interval[0], interval[1]] are cencored
	 */
	double *cenpoisson_interval;

	/*
	 * Gamma 
	 */
	double **gamma_log_prec;
	double *gamma_scale;				       /* the scalings 's' */

	/*
	 * MIX ~ Normal(x, 1/prec)
	 */
	double **mix_log_prec_gaussian;

	/*
	 * MIX ~ LogGamma(a,a)
	 */
	double **mix_log_prec_loggamma;

	/*
	 * Gammacount; parameter alpha
	 */
	double **gammacount_log_alpha;

	/*
	 * The qKumar likelihood
	 */
	double **qkumar_log_prec;

	/*
	 * qcontpoisson. Hold the solution to ``exp(lquantile)=pcontpois(exp(eta),alpha)'' for all (lquantile,eta),
	 * one for each thread
	 */
	GMRFLib_spline_tp **qcontpoisson_func;

	/*
	 * Aggregated Gaussian  
	 */
	double **agaussian;

	/*
	 * For both modelx nmix and nmixnb
	 */
	int nmix_m;
	double **nmix_y;				       /* vector of data */
	double **nmix_x;				       /* matrix of covariates */
	double ***nmix_beta;				       /* vector of betas */
	double **nmix_log_overdispersion;		       /* only for model nmixnb */

	/*
	 * generalized Pareto
	 */
	double **gp_intern_tail;			       /* log(xi) parameter */
	double *gp_tail_interval;

	/*
	 * Tweedie 
	 */
	double **tweedie_p_intern;
	double **tweedie_phi_intern;
	double *tweedie_w;

	/*
	 * fmri 
	 */
	double **fmri_lprec;
	double **fmri_ldof;
	double *fmri_scale;

	/*
	 * gaussianjw  
	 */
	double ***gjw_beta;
	double *gjw_n;
	double *gjw_df;
	double *gjw_var;

	link_func_tp *link_simple_invlinkfunc;		       /* the link-function for the simple part of the 0inflated models */
	char *link_simple_name;

	int poisson0_nbeta;
	double ***poisson0_beta;
	double **poisson0_x;
	double *poisson0_E;

	int binomial0_nbeta;
	double ***binomial0_beta;
	double **binomial0_x;
	double *binomial0_Ntrials;

	/*
	 * this cover both ggassuian and ggassuianS 
	 */
	int ggaussian_nbeta;
	double ***ggaussian_beta;
	double **ggaussian_x;
	double *ggaussian_scale;			       // only ggaussian
	double *ggaussian_offset;			       // only ggaussianS

	// 
	double **fl_c;


	// rcpoisson
	double *rcp_E;
	double *rcp_event;
	double *rcp_offset;
	double **rcp_x;
	double ***rcp_beta;
	int rcp_nbeta;

	// tpoisson
	double *tp_E;
	double *tp_event;
	double *tp_offset;
	double **tp_x;
	double ***tp_beta;
	int tp_nbeta;
} Data_tp;

typedef struct {
	int idx;
	int order;					       /* a copy of ds->link_order */
	int variant;					       /* a copy of ds->link_variant */
	int Ntrial;
	double quantile;
	double a;					       /* a copy of ds->link_a */
	double **alpha_intern;
	double **log_prec;
	double **beta;					       /* one covariate */
	double **beta_intern;				       /* one covariate */
	double ***betas;				       /* with variable number of covariates */
	double **sensitivity_intern;
	double **specificity_intern;
	double **prob_intern;
	double **dof_intern;
	double **sn_skew;
	double **sn_intercept;
	double *scale;
	double **power_intern;
	double **intercept_intern;
	double **bgev_tail;
	double **bgev_intercept;
	double *bgev_tail_interval;
} Link_param_tp;

typedef struct {
	GMRFLib_spline_tp *cdf, *icdf;
	double alpha, xmin, xmax, pmin, pmax;
} inla_sn_table_tp;

typedef struct {
	GMRFLib_spline_tp *cdf, *icdf;
	double a, eta_min, eta_max, p_intern_min, p_intern_max;
} inla_loga_table_tp;

/* 
   This is the macro to evaluate the prior. One and only one of `priorfunc' and `expression' is non-NULL, so we use that one
 */
#define PRIOR_EVAL(p_, arg_) (evaluate_hyper_prior ?		       \
			      ((p_).priorfunc ?			       \
			       (p_).priorfunc(arg_, (p_).parameters) :	\
			       inla_eval((p_).expression, arg_, theta, ntheta)) \
			       : 0.0)

typedef struct {
	GMRFLib_tabulate_Qfunc_tp *tab;
	double **beta;
	double **log_prec;

	int n;
	double *eigenvalues;
	double max_eigenvalue;
	double min_eigenvalue;
} inla_generic1_tp;

typedef struct {
	GMRFLib_tabulate_Qfunc_tp *tab;
	double **log_prec;
	double **h2_intern;
	int n;						       /* size of graph */
	int N;						       /* total size: N=2*n */
} inla_generic2_tp;

typedef struct {
	char *name;
	char *type;
} File_tp;

typedef struct {
	int gcpo;					       /* output GCPO */
	int cpo;					       /* output CPO */
	int po;						       /* output PO */
	int dic;					       /* output DIC */
	int summary;					       /* output marginal summaries (mean, stdev, etc) */
	int return_marginals;				       /* output detailed marginal density (even though they are computed) */
	int return_marginals_predictor;			       /* output detailed marginal density (even though they are computed) */
	int hyperparameters;				       /* compute also the marginal for the hyperparameters */
	int kld;					       /* output the (symmetric) kld between marginals */
	int mlik;					       /* compute the marginal likelihood? */
	int q;						       /* output image of the Q-matrix */
	int graph;					       /* output the graph */
	int config;					       /* output the configurations */
	int config_lite;				       /* output-lite the configurations */
	int likelihood_info;				       /* output likelihood_info (requires config=TRUE) */
	int internal_opt;				       /* do internal optimisation? default TRUE */
	int save_memory;				       /* save memory? default FALSE */
	int nquantiles;					       /* compute cdfs and/or quantiles; max 10 */
	int ncdf;
	int mode;
	double *quantiles;
	double *cdf;
} Output_tp;

typedef struct inla_tp_struct inla_tp;			       /* need it like this as they point to each other */

typedef enum {
	MIX_INT_DEFAULT = 0,
	MIX_INT_QUADRATURE = 1,
	MIX_INT_SIMPSON = 2
} inla_mix_integrator_tp;

#define MIX_INT_EPS  (1.0E-6)				       /* defines the cut-off for the integration weights */

typedef struct {
	char *data_likelihood;
	int variant;

	inla_component_tp data_id;
	File_tp data_file;
	File_tp weight_file;
	File_tp attr_file;
	File_tp lp_scale_file;
	Prior_tp data_prior;
	Prior_tp data_prior0;
	Prior_tp data_prior1;
	Prior_tp data_prior2;
	Prior_tp *data_nprior;
	Data_tp data_observations;
	int data_fixed;
	int data_fixed0;
	int data_fixed1;
	int data_fixed2;
	int *data_nfixed;
	int data_ntheta;
	GMRFLib_logl_tp *loglikelihood;
	double *offset;
	inla_tp *mb;					       /* to get the off_.... */

	double *lp_scale;				       /* index vector */
	double ***lp_scale_beta;
	int *lp_scale_nfixed;
	int *lp_scale_in_use;
	Prior_tp *lp_scale_nprior;

	/*
	 * the link model
	 */
	char *link_model;
	int *link_fixed;
	int link_ntheta;
	int link_order;
	int link_variant;
	double link_a;
	double *link_initial;
	GMRFLib_matrix_tp *link_covariates;
	Link_param_tp *link_parameters;
	Prior_tp *link_prior;
	inla_component_tp link_id;
	link_func_tp *predictor_invlinkfunc;
	void **predictor_invlinkfunc_arg;

	/*
	 * the re-extention
	 */
	int mix_use;
	int mix_npoints;
	inla_component_tp mix_id;
	inla_mix_integrator_tp mix_integrator;
	GMRFLib_logl_tp *mix_loglikelihood;
	Prior_tp mix_prior;
	int mix_fixed;
	int mix_ntheta;
} Data_section_tp;

typedef struct {
	int n;
	int dim;
	GMRFLib_matrix_tp *locations;
	gsl_matrix *dist;

	double **log_range;
	double **log_prec;
	double **log_nu;

	gsl_matrix **Q;
	double **param;
} dmatern_arg_tp;

typedef struct {
	int n;
	int p;

	double **log_prec;
	double ***pacf_intern;

	/*
	 * these are the stored values, all for prec = 1
	 */
	double **hold_pacf_intern;			       /* [i][id][0] */
	double **hold_Q;				       /* dim = 2*p + 1 */
	double **hold_Qmarg;				       /* dim = p */
} ar_def_tp;

typedef struct {
	double precision;
	double **beta;

	GMRFLib_Qfunc_tp *Qfunc;
	void *Qfunc_arg;

	map_func_tp *map_beta;
	void *map_beta_arg;
} inla_copy_arg_tp;

typedef struct {
	double *betas;
	double *betas_tmp;
	GMRFLib_spline_tp *splinefun;
} inla_scopy_cache_tp;

typedef struct {
	int nbeta;
	int rankdef;
	double *loc_beta;
	double *cov_beta;
	double ***betas;
	double precision;
	double loc_len;
	double loc_mid;

	GMRFLib_rwdef_tp *rwdef;
	GMRFLib_graph_tp *graph_prior;
	GMRFLib_Qfunc_tp *Qfunc_prior;
	void *Qfunc_arg_prior;

	GMRFLib_graph_tp *graph;
	GMRFLib_Qfunc_tp *Qfunc;
	void *Qfunc_arg;
	GMRFLib_matrix_tp *W;

	inla_scopy_cache_tp **cache00;
	inla_scopy_cache_tp **cache01;
} inla_scopy_arg_tp;

typedef struct {
	int ntheta;
	double *theta_mode;
	double *stdev_corr_pos;
	double *stdev_corr_neg;
	gsl_vector *sqrt_eigen_values;
	gsl_matrix *eigen_vectors;
} inla_update_tp;

typedef struct {
	char *file;
	char *model;					       /* function to be called: fun(theta) */
} inla_jp_tp;

struct inla_tp_struct {
	/*
	 * General stuff 
	 */
	int verbose;
	GMRFLib_openmp_strategy_tp strategy;
	char *smtp;

	GMRFLib_preopt_tp *preopt;

	/*
	 * parameters for global_nodes
	 */
	GMRFLib_global_node_tp gn;

	/*
	 * reuse the mode-stuff 
	 */
	int ntheta_file;
	int theta_counter_file;
	int reuse_mode;
	int reuse_mode_but_restart;
	int fixed_mode;					       /* if TRUE, then treat all thetas as known and fixed, otherwise, do as usual... */
	double *theta_file;
	double *x_file;
	int nx_file;

	/*
	 * libR options
	 */
	char *libR_R_HOME;

	/*
	 * Expert options 
	 */
	int expert_cpo_manual;
	int *expert_cpo_idx;				       /* list of indices */
	int expert_n_cpo_idx;				       /* the length */
	double expert_diagonal_emergencey;
	int expert_disable_gaussian_check;		       /* do not check for faster computations */

	/*
	 * section Problem
	 */
	char *name;					       /* Id of the problem */
	char *dir;					       /* Where to store the files */
	Output_tp *output;				       /* Global output-options */

	/*
	 * type Predictor
	 */
	int predictor_n;				       /* dimension of \eta */
	int predictor_m;				       /* dimension of \tilde{\eta}, the extended predictor */
	int predictor_ndata;				       /* dimension of the data, ie _n if _m is 0, otherwise _m */
	double *family_idx;				       /* which family {1,2,..} each data-point belongs to */
	int len_family_idx;				       /* need to pass this as well */

	char *predictor_tag;				       /* the tag */
	char *predictor_dir;				       /* the directory */
	double **predictor_log_prec;
	Prior_tp predictor_prior;
	int *predictor_cross_sumzero;
	int predictor_compute;
	int predictor_fixed;
	int predictor_user_scale;
	int predictor_vb_correct;
	link_func_tp **predictor_invlinkfunc;		       /* these are rebuilt */
	void **predictor_invlinkfunc_arg;		       /* these are rebuilt */
	GMRFLib_matrix_tp **predictor_invlinkfunc_covariates;
	Output_tp *predictor_output;
	double *predictor_family;
	double *offset;					       /* the offset y ~ f(eta + offset) */
	double *link_fitted_values;			       /* the index for the link function for missing observations */

	char *predictor_A_fnm;				       /* filename for the Amatrix */
	char *predictor_Aext_fnm;			       /* extension: filename for the extended Amatrix */
	double predictor_Aext_precision;		       /* extension: precision for the extended Amatrix */
	char *Apredictor_tag;				       /* the tag */

	GMRFLib_transform_array_func_tp **transform_funcs;     /* for the fitted values */

	/*
	 * type Data 
	 */
	int ds;						       /* current data-section (when reading) */
	int nds;					       /* number of data-sections in total */
	int data_ntheta_all;
	int *fl;					       /* to flag 'fl' likelihood */
	double *d;
	void **loglikelihood_arg;
	Data_section_tp *data_sections;
	GMRFLib_logl_tp **loglikelihood;

	/*
	 * type Ffield 
	 */
	int nf;
	char **f_tag;
	char **f_dir;
	char **f_modelname;
	int **f_c;
	int *f_n;					       /* basic size of f-model */
	int *f_N;					       /* size of f-model after internal augmentation */
	int *f_Ntotal;					       /* Grand total of f-model, including augmentation and replication */
	int *f_nrow;					       /* if 2d */
	int *f_ncol;					       /* if 2d */
	int *f_nrep;					       /* number of replicates */
	int *f_ngroup;
	int *f_group_model;
	int *f_group_cyclic;
	int *f_group_order;
	GMRFLib_graph_tp **f_group_graph;
	int *f_order;
	double **f_locations;
	double **f_weights;
	double **f_scale;
	GMRFLib_graph_tp **f_graph;
	GMRFLib_graph_tp **f_graph_orig;
	GMRFLib_Qfunc_tp **f_Qfunc;
	GMRFLib_Qfunc_tp **f_Qfunc_orig;
	void **f_Qfunc_arg;
	void **f_Qfunc_arg_orig;
	GMRFLib_bfunc2_tp **f_bfunc2;
	Prior_tp **f_prior;
	char *f_sumzero;
	GMRFLib_constr_tp **f_constr;
	GMRFLib_constr_tp **f_constr_orig;
	double *f_diag;
	double *f_rankdef;
	double ****f_theta;
	map_func_tp ***f_theta_map;
	void ***f_theta_map_arg;
	int *f_compute;
	int **f_fixed;
	double **f_initial;
	inla_component_tp *f_id;
	int *f_ntheta;
	int *f_cyclic;
	int *f_nu;
	int *f_Torder;
	char **f_Tmodel;
	int *f_Korder;
	char **f_Kmodel;
	void **f_model;
	char **f_of;
	char **f_same_as;
	double *f_precision;
	Output_tp **f_output;
	inla_file_contents_tp **f_id_names;
	int *f_correct;					       /* correct or not? */
	GMRFLib_idx_tp **f_vb_correct;			       /* correct or not? */

	GMRFLib_Qfunc_tp ***ff_Qfunc;			       /* interactions */
	void ***ff_Qfunc_arg;

	GMRFLib_matrix_tp **f_Alocal;

	/*
	 * type Linear
	 */
	int nlinear;
	char **linear_tag;
	char **linear_dir;
	double **linear_covariate;
	double *linear_precision;
	double *linear_mean;
	int *linear_compute;
	Output_tp **linear_output;

	/*
	 * linear combinations 
	 */
	int nlc;					       /* number of linear combinations */
	int lc_derived_correlation_matrix;		       /* compute correlations ? */
	char **lc_tag;					       /* the tags */
	char **lc_dir;
	double *lc_order;
	Output_tp **lc_output;
	GMRFLib_lc_tp **lc_lc;
	double *lc_derived_c;				       /* optional: correlation for the lincombs (derived) */

	/*
	 * global constraint (before its merged with the model-constr) ...[0] is A, ...[1] is e, both vectors 'n x 1' 
	 */
	GMRFLib_matrix_tp **global_constr;

	/*
	 * The final model 
	 */
	GMRFLib_hgmrfm_tp *hgmrfm;
	int ntheta;
	double ***theta;
	char **theta_hyperid;
	char **theta_tag;
	char **theta_tag_userscale;
	char **theta_dir;
	char **theta_from;
	char **theta_to;
	map_func_tp **theta_map;
	void **theta_map_arg;
	int *off_compute;
	char **off_modelname;
	int *off_id;

	/*
	 * INLA 
	 */
	GMRFLib_ai_param_tp *ai_par;

	/*
	 * Expermental stuff: joint prior in R.
	 */
	inla_jp_tp *jp;

	/*
	 * results 
	 */
	GMRFLib_density_tp **density;
	GMRFLib_density_tp **density_transform;
	GMRFLib_density_tp **density_hyper;
	GMRFLib_density_tp **density_lin;
	GMRFLib_gcpo_tp *gcpo;
	GMRFLib_gcpo_param_tp *gcpo_param;
	GMRFLib_ai_cpo_tp *cpo;
	GMRFLib_ai_po_tp *po;
	GMRFLib_ai_dic_tp *dic;
	GMRFLib_ai_marginal_likelihood_tp mlik;

	/*
	 * index-table 
	 */
	int idx_ntot;					       /* total length of x before lincomb */
	int idx_tot;					       /* total number of terms (ffield/linear) */
	char **idx_tag;					       /* the tags */
	int *idx_start;					       /* the starting index */
	int *idx_n;					       /* the length */
	map_stri idx_hash;				       /* the hash-table */

	/*
	 * misc output from INLA() 
	 */
	GMRFLib_ai_misc_output_tp *misc_output;

	/*
	 * if at use
	 */
	inla_update_tp *update;

	// experimental mode only
	int compute_initial_values;
};

typedef struct {
	GMRFLib_graph_tp *graph;
	double **log_prec;
	double *prec_scale;
} inla_besag_Qfunc_arg_tp;

typedef struct {
	inla_besag_Qfunc_arg_tp *besag_arg;
	double **log_a;					       /* the parameter a */
	double precision;				       /* the copy precision */
	double diag;					       /* as we need to this this on part1 only */
} inla_besag2_Qfunc_arg_tp;

typedef struct {
	int n;
	int N;
	double **log_prec_iid;
	inla_besag_Qfunc_arg_tp *besag_arg;
} inla_bym_Qfunc_arg_tp;

typedef struct {
	int n;
	int N;
	double **log_prec;
	double **logit_phi;
	inla_besag_Qfunc_arg_tp *besag_arg;
} inla_bym2_Qfunc_arg_tp;

typedef struct {
	int n;
	int N;
	double **log_prec;
	double **logit_phi;
	GMRFLib_rw2ddef_tp *rw2ddef;
} inla_rw2diid_Qfunc_arg_tp;

typedef struct {
	GMRFLib_graph_tp *graph;
	double **log_prec;				       /* precision */
	double **log_diag;				       /* the value added to the diagonal. diag = 0 is the besag model */
} inla_besag_proper_Qfunc_arg_tp;

typedef struct {
	GMRFLib_graph_tp *graph;
	double **log_prec;				       /* precision */
	double **logit_lambda;				       /* the lambda parameter */
} inla_besag_proper2_Qfunc_arg_tp;

typedef struct {
	/*
	 * the AR(1) model: X_t = phi * X_t-1 + Z_t. The arguments are Var(Z_t) = 1/exp(log_precision), and phi_intern =
	 * logit((phi+1)/2). experimental is the mean theta[2].
	 */
	int n;
	int cyclic;
	double **log_prec;				       /* theta[0] */
	double **phi_intern;				       /* theta[1] */
	double **mean;					       /* theta[2] */
} inla_ar1_arg_tp;

typedef struct {
	/*
	 * the OU model: dX_t = -phi * X_t-1 + sigma * dW_t. The arguments are phi > 0 and 2*phi/sigma^2 = precision
	 */
	int n;
	double *locations;
	double **log_prec;				       /* theta[0] */
	double **phi_intern;				       /* theta[1] */
} inla_ou_arg_tp;

typedef struct {
	/*
	 * the AR(1) model with covariates: X_t = phi * X_t-1 + beta'* Z_{t-1} + eps_t
	 * Prec(eps_t) = exp(log_precision)/(1-phi^2), and phi_intern =
	 * logit((phi+1)/2). 
	 *
	 * total internal length, is N = n+m = AR1 + beta's
	 */
	int N;
	int n;						       /* length of AR1 */
	int m;						       /* number of covariates Z */
	double **log_prec;				       /* theta[0] (marginal precision) */
	double **phi_intern;				       /* theta[1] */
	GMRFLib_matrix_tp *Z;
	GMRFLib_matrix_tp *ZZ;
	GMRFLib_matrix_tp *Qbeta;
	double logdet_Qbeta;
} inla_ar1c_arg_tp;

typedef struct {
	/*
	 * 2D iid random effects. The coding is (x0,y0,x1,y1,...,xn-1,yn-1), so the total length is N=2*n. For the 2DIIDWISHART the coding is (x,y).
	 */
	int n;
	double **log_prec0;				       /* Precision for x */
	double **log_prec1;				       /* Precision for y */
	double **rho_intern;				       /* Corr(x,y) */
} inla_2diid_arg_tp;

typedef struct {
	/*
	 * 3D iid random effects. The coding is (x,y,z), so the total length is N=3*n.
	 */
	int n;
	double **log_prec0;				       /* Precision for x */
	double **log_prec1;				       /* Precision for y */
	double **log_prec2;				       /* Precision for z */
	double **rho_intern01;				       /* Corr(x,y) */
	double **rho_intern02;				       /* Corr(x,z) */
	double **rho_intern12;				       /* Corr(y,z) */
} inla_3diid_arg_tp;

typedef struct {
	/*
	 * iid2d random effects. The coding is (x0,x1,..., y0,y1...)
	 */
	int n;
	int N;
	double **log_prec0;				       /* Precision for x */
	double **log_prec1;				       /* Precision for y */
	double **rho_intern;				       /* Corr(x,y) */
} inla_iid2d_arg_tp;

typedef struct {
	/*
	 * iid3d random effects. The coding is (x0,x1,..., y0,y1,....,z0,z1,...), so the total length is N=3*n.
	 */
	int n;
	int N;
	double **log_prec0;				       /* Precision for x */
	double **log_prec1;				       /* Precision for y */
	double **log_prec2;				       /* Precision for z */
	double **rho_intern01;				       /* Corr(x,y) */
	double **rho_intern02;				       /* Corr(x,z) */
	double **rho_intern12;				       /* Corr(y,z) */
} inla_iid3d_arg_tp;

typedef struct {
	double *vec;
	gsl_matrix *Q;
} inla_wishart_hold_tp;

typedef struct {
	/*
	 * iid random effects with a Wishart prior The coding is (x0,x1,..., y0,y1,....,z0,z1,...), so the total length is N=dim*n.
	 */
	int n;
	int N;
	int dim;
	double ***log_prec;				       /* 0, 1, 2,.... */
	double ***rho_intern;				       /* 01,02,... 12, 13, ... , 23, 24... */

	inla_wishart_hold_tp **hold;
} inla_iid_wishart_arg_tp;

typedef struct {
	double *vec;
	gsl_matrix *L;
	gsl_matrix *Q;
} inla_wishartk_hold_tp;

typedef struct {
	int n;						       /* n=k*m */
	int N;
	int dim;					       /* dim = order */
	int ntheta;					       /* k*(k+1)/2 */
	double ***theta;
	double **vec;

	inla_wishartk_hold_tp **hold;
} inla_iid_wishartk_arg_tp;

typedef struct {
	int n;
	int m;

	double rho_min;
	double rho_max;
	double **log_prec;
	double **logit_rho;

	GMRFLib_graph_tp *graph_A1;
	GMRFLib_graph_tp *graph_A2;
	GMRFLib_graph_tp *graph_B;
	GMRFLib_graph_tp *graph_C;
	GMRFLib_graph_tp *graph_slm;
	GMRFLib_tabulate_Qfunc_tp *Qfunc_A1;
	GMRFLib_tabulate_Qfunc_tp *Qfunc_A2;
	GMRFLib_tabulate_Qfunc_tp *Qfunc_B;
	GMRFLib_tabulate_Qfunc_tp *Qfunc_C;
} inla_slm_arg_tp;

typedef struct {
	double **log_prec;
	int n;						       // Z is n x m
	int m;
	GMRFLib_graph_tp *graph_A;
	GMRFLib_graph_tp *graph_B;
	GMRFLib_graph_tp *graph_AB;
	GMRFLib_tabulate_Qfunc_tp *Qfunc_A;
	GMRFLib_tabulate_Qfunc_tp *Qfunc_B;
} inla_z_arg_tp;

typedef struct {
	int *array;
	int n;
} inla_iarray_tp;

typedef struct {
	GMRFLib_Qfunc_tp *Qfunc;
	void *Qfunc_arg;
	int n;						       /* size of the original problem */
} inla_replicate_tp;

/* 
   constrained fixed effect
 */
typedef struct {
	double **beta;
	void *beta_arg;
	double *x;
	double precision;
} inla_clinear_tp;

/* 
   sigmodial and reverse sigmodial and log1exp
 */
typedef struct {
	double **beta;
	double **log_halflife;
	double **log_shape;
	double *x;
	double precision;
} inla_sigm_tp;

typedef struct {
	double **beta;
	double **alpha;
	double **gamma;
	double *x;
	double precision;
} inla_log1exp_tp;

typedef struct {
	double **beta;
	double **alpha1;
	double **alpha2;
	double *x;
	double precision;
} inla_logdist_tp;

/* 
   classic me model
 */
typedef struct {
	double **beta;
	double **log_prec_obs;
	double **mean_x;
	double **log_prec_x;
	double *x_obs;
	double *scale;
	void *map_beta_arg;
} inla_mec_tp;

/* 
   berkson me model
 */
typedef struct {
	double **beta;
	double **log_prec;
	double *x;
	double *scale;
	void *map_beta_arg;
} inla_meb_tp;

typedef struct {
	int Id;
	char *filename;					       /* file to load containing the model definition */
	char *model;					       /* the variable name that contains the model definition */
	int ntheta;
	int n;
	int mu_zero;					       /* often mu is zero, allow for fast return */
	int reset_cache;
	int *ilist;
	int *jlist;
	int len_list;
	double ***theta;
	double **param;
	GMRFLib_graph_tp *graph;
	GMRFLib_tabulate_Qfunc_tp **Q;
	double **mu;
	double **mu_param;
} inla_rgeneric_tp;

typedef struct {
	int Id;
	int debug;
	char *shlib;					       /* file to load containing the model definition */
	char *model;					       /* the name of the model definition */
	char *secname;
	int ntheta;
	int n;
	int mu_zero;					       /* often mu is zero, allow for fast return */
	int reset_cache;
	int *ilist;
	int *jlist;
	int len_list;
	inla_cgeneric_data_tp *data;
	double ***theta;
	double **param;
	GMRFLib_graph_tp *graph;
	GMRFLib_tabulate_Qfunc_tp **Q;
	double **mu;
	double **mu_param;
	inla_cgeneric_func_tp *model_func;
} inla_cgeneric_tp;

typedef struct {
	int n;						       /* size of graph */
	int m;						       /* number of terms in the sum */
	GMRFLib_graph_tp *graph;			       /* total graph */
	GMRFLib_graph_tp **g;				       /* individual graphs for each of the terms */
	GMRFLib_tabulate_Qfunc_tp **tab;		       /* Qfunc for each of the terms */
	double ***log_prec;				       /* log_prec for each term in the sum */
} inla_generic3_tp;

typedef struct {
	int N;
	int ngroup;
	int type;
	int cyclic;
	GMRFLib_graph_tp *graph;
	GMRFLib_Qfunc_tp *Qfunc;
	void *Qfunc_arg;
	double **group_rho_intern;
	double **group_prec_intern;
	GMRFLib_rwdef_tp *rwdef;
	GMRFLib_crwdef_tp *crwdef;
	ar_def_tp *ardef;
	inla_besag_Qfunc_arg_tp *besagdef;
} inla_group_def_tp;

typedef struct {
	int n;						       // length of covariates/subject/strata 
	int N;						       // size of matrix = n + warg->dim*m, m=#subjects, dim=2 
	int nsubject;
	int nstrata;
	double precision;				       // fixed high precision 
	double ***theta_gamma;
	GMRFLib_matrix_tp *def;
	GMRFLib_idx_tp **subject_idx;
	inla_iid_wishart_arg_tp *warg;
} inla_intslope_arg_tp;

typedef enum {
	INTSLOPE_SUBJECT = 0,
	INTSLOPE_STRATA = 1,
	INTSLOPE_Z = 2
} inla_intslope_column_tp;

typedef struct {
	double xi;
	double omega;
	double intercept;
	double alpha;
} inla_sn_arg_tp;

#define R_GENERIC_Q "Q"
#define R_GENERIC_GRAPH "graph"
#define R_GENERIC_MU "mu"
#define R_GENERIC_INITIAL "initial"
#define R_GENERIC_LOG_NORM_CONST "log.norm.const"
#define R_GENERIC_LOG_PRIOR "log.prior"
#define R_GENERIC_QUIT "quit"

#define R_GENERIC_MODEL ".inla.rgeneric.model"
#define R_GENERIC_WRAPPER "inla.rgeneric.wrapper"
#define R_JP_MODEL ".inla.jp.model"

#define INLA_LITTLE_ENDIAN 1
#define INLA_BIG_ENDIAN    2

/* 
   binary write macros. Faster to cache and write in bulk. See example number 63

   First variant is not thread safe, second one is
 */
#define Dinit_core(n_, filename_) size_t _d_store_len = n_; \
	double *_d_store = Calloc(_d_store_len + 32L, double); size_t _d_n = 0; \
	FILE * _fp = fopen(filename_ , "wb"); if (!_fp) inla_error_open_file(filename_)

#define Dinit(filename_)   Dinit_core(8388608L, filename_)
#define Dinit_s(filename_) Dinit_core(8192L, filename_)
#define Dwrite()							\
	if (_d_n >= _d_store_len) {					\
		fwrite((void*)_d_store, sizeof(double), _d_n, _fp);	\
		_d_n = 0;						\
	}
#define Dclose()							\
	if (1) {							\
		if (_fp) {						\
			if (_d_n) {					\
				fwrite((void*)_d_store, sizeof(double), _d_n, _fp); \
			}						\
			fclose(_fp);					\
		}							\
		_fp = NULL;						\
		_d_n = 0;						\
		Free(_d_store);						\
	}
#define D1W(a_)                 _d_store[_d_n++] = a_; Dwrite()
#define D2W(a_, b_)             _d_store[_d_n++] = a_; _d_store[_d_n++]= b_; Dwrite()
#define D3W(a_, b_, c_)         _d_store[_d_n++] = a_; _d_store[_d_n++]= b_; _d_store[_d_n++]= c_; Dwrite()
#define D4W(a_, b_, c_, d_)     _d_store[_d_n++] = a_; _d_store[_d_n++]= b_; _d_store[_d_n++]= c_; _d_store[_d_n++]= d_; Dwrite()
#define D5W(a_, b_, c_, d_, e_) _d_store[_d_n++] = a_; _d_store[_d_n++]= b_; _d_store[_d_n++]= c_; _d_store[_d_n++]= d_; _d_store[_d_n++]= e_; Dwrite()

// thread-safe: with these, one have to KNOW... no dynamic writing, just filling in the array
#define Dinit_r(n_, g_, filename_) size_t _d_store_len = n_; size_t _d_g = g_; double *_d_store = Calloc(_d_store_len * _d_g + 0L, double); \
	FILE * _fp = fopen(filename_, "wb"); if (!_fp) inla_error_open_file(filename_)

#define D1W_r(idx_, off_, a_)				\
	if (1) {					\
		size_t iidx_ = (idx_) * _d_g + off_;	\
		_d_store[iidx_] = a_;			\
	}
#define D2W_r(idx_, off_, a_, b_)			\
	if (1) {					\
		size_t iidx_ = (idx_) * _d_g + off_;	\
		_d_store[iidx_ + 0] = a_;		\
		_d_store[iidx_ + 1]= b_;		\
	}
#define D3W_r(idx_, off_, a_, b_, c_)			\
	if (1) {					\
		size_t iidx_ = (idx_) * _d_g + off_;	\
		_d_store[iidx_ + 0] = a_;		\
		_d_store[iidx_ + 1]= b_;		\
		_d_store[iidx_ + 2]= c_;		\
	}
#define D4W_r(idx_, off_, a_, b_, c_, d_)		\
	if (1) {					\
		size_t iidx_ = (idx_) * _d_g + off_;	\
		_d_store[iidx_ + 0] = a_;		\
		_d_store[iidx_ + 1]= b_;		\
		_d_store[iidx_ + 2]= c_;		\
		_d_store[iidx_ + 3]= d_;		\
	}
#define D5W_r(idx_, off_, a_, b_, c_, d_, e_)		\
	if (1) {					\
		size_t iidx_ = (idx_) * _d_g + off_;	\
		_d_store[iidx_ + 0] = a_;		\
		_d_store[iidx_ + 1]= b_;		\
		_d_store[iidx_ + 2]= c_;		\
		_d_store[iidx_ + 3]= d_;		\
		_d_store[iidx_ + 4]= e_;		\
	}
#define Dclose_r()							\
	if (_fp) {							\
		fwrite((void*)_d_store, sizeof(double), _d_store_len * _d_g, _fp); \
		fclose(_fp);						\
		_fp = NULL;						\
		_d_store_len = _d_g = 0;				\
		Free(_d_store);						\
	}

GMRFLib_constr_tp *inla_make_constraint(int n, int sumzero, GMRFLib_constr_tp * constr);
GMRFLib_constr_tp *inla_make_constraint2(int n, int replicate, int sumzero, GMRFLib_constr_tp * constr);
GMRFLib_constr_tp *inla_read_constraint(const char *filename, int n);
char *inla_create_hyperid(int id, const char *secname);
char *inla_make_tag(const char *string, int ds);
char *inla_make_tag2(const char *string, int ds, const char *estr);
const char *inla_string_join(const char *a, const char *b);
double Qfunc_2diid(int thread_id, int i, int j, double *values, void *arg);
double Qfunc_2diid_wishart(int thread_id, int i, int j, double *values, void *arg);
double Qfunc_ar(int thread_id, int i, int j, double *values, void *arg);
double Qfunc_ar1(int thread_id, int i, int j, double *values, void *arg);
double Qfunc_ar1c(int thread_id, int i, int j, double *values, void *arg);
double Qfunc_besag(int thread_id, int i, int j, double *values, void *arg);
double Qfunc_besag2(int thread_id, int i, int j, double *values, void *arg);
double Qfunc_besagproper(int thread_id, int i, int j, double *values, void *arg);
double Qfunc_besagproper2(int thread_id, int i, int j, double *values, void *arg);
double Qfunc_bym(int thread_id, int i, int j, double *values, void *arg);
double Qfunc_bym2(int thread_id, int i, int j, double *values, void *arg);
double Qfunc_cgeneric(int thread_id, int i, int j, double *values, void *arg);
double Qfunc_clinear(int thread_id, int i, int j, double *values, void *arg);
double Qfunc_copy_part00(int thread_id, int i, int j, double *values, void *arg);
double Qfunc_copy_part01(int thread_id, int i, int j, double *values, void *arg);
double Qfunc_copy_part11(int thread_id, int i, int j, double *values, void *arg);
double Qfunc_dmatern(int thread_id, int i, int j, double *values, void *arg);
double Qfunc_generic1(int thread_id, int i, int j, double *values, void *arg);
double Qfunc_generic2(int thread_id, int i, int j, double *values, void *arg);
double Qfunc_generic3(int thread_id, int i, int j, double *values, void *arg);
double Qfunc_group(int thread_id, int i, int j, double *values, void *arg);
double Qfunc_iid2d(int thread_id, int i, int j, double *values, void *arg);
double Qfunc_iid_wishart(int thread_id, int node, int nnode, double *values, void *arg);
double Qfunc_log1exp(int thread_id, int i, int j, double *values, void *arg);
double Qfunc_logdist(int thread_id, int i, int j, double *values, void *arg);
double Qfunc_mec(int thread_id, int i, int j, double *values, void *arg);
double Qfunc_ou(int thread_id, int i, int j, double *values, void *arg);
double Qfunc_replicate(int thread_id, int i, int j, double *values, void *arg);
double Qfunc_rgeneric(int thread_id, int i, int j, double *values, void *arg);
double Qfunc_rw2diid(int thread_id, int i, int j, double *values, void *arg);
double Qfunc_scopy_part00(int thread_id, int i, int j, double *values, void *arg);
double Qfunc_scopy_part01(int thread_id, int i, int j, double *values, void *arg);
double Qfunc_scopy_part11(int thread_id, int i, int j, double *values, void *arg);
double Qfunc_sigm(int thread_id, int i, int j, double *values, void *arg);
double Qfunc_slm(int thread_id, int i, int j, double *values, void *arg);
double Qfunc_z(int thread_id, int i, int j, double *values, void *arg);
double ar_map_pacf(double arg, map_arg_tp typ, void *param);
double ddexp_taylor(double x, double x0, int order);
double dexp_taylor(double x, double x0, int order);
double eval_log_contpoisson(double y, double lambda);
double eval_logsum_safe(double lA, double lB);
double exp_taylor(double x, double x0, int order);
double extra(int thread_id, double *theta, int ntheta, void *argument);
double iid_mfunc(int idx, void *arg);
double inla_Phi(double x);
double inla_Phi_fast(double x);
double inla_ar1_cyclic_logdet(int N_orig, double phi);
double inla_compute_initial_value(int idx, GMRFLib_logl_tp * logl, double *x_vec, void *arg);
double inla_compute_saturated_loglik(int thread_id, int idx, GMRFLib_logl_tp * loglfunc, double *x_vec, void *arg);
double inla_compute_saturated_loglik_core(int thread_id, int idx, GMRFLib_logl_tp * loglfunc, double *x_vec, void *arg);
double inla_dmatern_cf(double dist, double range, double nu);
double inla_dnchisq(double x, double df, double ncp);
double inla_get_sn_param(inla_sn_arg_tp * output, double **param);
double inla_interpolate_mode(double *x, double *y);
double inla_lgamma_fast(double x);
double inla_log_Phi(double x);
double inla_log_Phi_fast(double x);
double inla_logit_Phi(double x);
double inla_negative_binomial_interval(double size, double mu, int y_from, int y_to);
double inla_poisson_interval(double lambda, int y_from, int y_to);
double inla_sn_intercept(double intern_quantile, double skew);
double inla_update_density(double *theta, inla_update_tp * arg);
double link_bgev(int thread_id, double x, map_arg_tp typ, void *param, double *cov);
double link_cauchit(int thread_id, double x, map_arg_tp typ, void *param, double *cov);
double link_ccloglog(int thread_id, double x, map_arg_tp typ, void *param, double *cov);
double link_cloglog(int thread_id, double x, map_arg_tp typ, void *param, double *cov);
double link_identity(int thread_id, double x, map_arg_tp typ, void *param, double *cov);
double link_inverse(int thread_id, double x, map_arg_tp typ, void *param, double *cov);
double link_log(int thread_id, double x, map_arg_tp typ, void *param, double *cov);
double link_log_1m_invccloglog(double x);
double link_log_1m_invcloglog(double x);
double link_log_invccloglog(double x);
double link_log_invcloglog(double x);
double link_loga(int thread_id, double x, map_arg_tp typ, void *param, double *cov);
double link_logit(int thread_id, double x, map_arg_tp typ, void *param, double *cov);
double link_logitoffset(int thread_id, double x, map_arg_tp typ, void *param, double *cov);
double link_loglog(int thread_id, double x, map_arg_tp typ, void *param, double *cov);
double link_logoffset(int thread_id, double x, map_arg_tp typ, void *param, double *cov);
double link_neglog(int thread_id, double x, map_arg_tp typ, void *param, double *cov);
double link_power_logit(int thread_id, double x, map_arg_tp typ, void *param, double *cov);
double link_pqbinomial(int thread_id, double x, map_arg_tp typ, void *param, double *cov);
double link_probit(int thread_id, double x, map_arg_tp typ, void *param, double *cov);
double link_qbinomial(int thread_id, double x, map_arg_tp typ, void *param, double *cov);
double link_qpoisson(int thread_id, double x, map_arg_tp typ, void *param, double *cov);
double link_qweibull(int thread_id, double x, map_arg_tp typ, void *param, double *cov);
double link_robit(int thread_id, double x, map_arg_tp typ, void *param, double *cov);
double link_sn(int thread_id, double x, map_arg_tp typ, void *param, double *cov);
double link_special1(int thread_id, double x, map_arg_tp typ, void *param, double *cov);
double link_special2(int thread_id, double x, map_arg_tp typ, void *param, double *cov);
double link_sslogit(int thread_id, double x, map_arg_tp typ, void *param, double *cov);
double link_tan(int thread_id, double x, map_arg_tp typ, void *param, double *cov);
double link_test1(int thread_id, double x, map_arg_tp typ, void *param, double *cov);
double link_this_should_not_happen(int thread_id, double x, map_arg_tp typ, void *param, double *cov);
double map_invsn_core(double arg, map_arg_tp typ, void *param, inla_sn_arg_tp * output);

// define macros for some of the forward calls, but it seems like the compiler figure this out itself
#define map_dof_forward(a1_, a2_, a3_) (2.0 + exp(a1_))
#define map_exp_forward(a1_, a2_, a3_) exp(a1_)
#define map_identity_forward(a1_, a2_, a3_) (a1_)
#define map_inverse_forward(a1_, a2_, a3_) (1.0/(a1_))
#define map_negexp_forward(a1_, a2_, a3_) exp(-(a1_))
#define map_precision_forward(a1_, a2_, a3_) exp(a1_)
#define map_probability_forward(a1_, a2_, a3_) (1.0 / (1.0 + exp(-(a1_))))
#define map_rho_forward(a1_, a2_, a3_) (2.0 / (1.0 + exp(-(a1_))) - 1.0)

double inla_ipow(double x, int k);
double map_1exp(double arg, map_arg_tp typ, void *param);
double map_H(double x, map_arg_tp typ, void *param);
double map_alpha_gompertz(double arg, map_arg_tp typ, void *param);
double map_alpha_weibull(double arg, map_arg_tp typ, void *param);
double map_beta(double arg, map_arg_tp typ, void *param);
double map_dof(double arg, map_arg_tp typ, void *param);
double map_exp(double arg, map_arg_tp typ, void *param);
double map_exp_scale2(double arg, map_arg_tp typ, void *param);
double map_group_rho(double x, map_arg_tp typ, void *param);
double map_identity(double arg, map_arg_tp typ, void *param);
double map_identity_scale(double arg, map_arg_tp typ, void *param);
double map_interval(double x, map_arg_tp typ, void *param);
double map_invcauchit(double arg, map_arg_tp typ, void *param);
double map_invccloglog(double arg, map_arg_tp typ, void *param);
double map_invcloglog(double arg, map_arg_tp typ, void *param);
double map_inverse(double arg, map_arg_tp typ, void *param);
double map_invlogit(double x, map_arg_tp typ, void *param);
double map_invloglog(double arg, map_arg_tp typ, void *param);
double map_invprobit(double arg, map_arg_tp typ, void *param);
double map_invrobit(double arg, map_arg_tp typ, void *param);
double map_invsn(double arg, map_arg_tp typ, void *param);
double map_invtan(double arg, map_arg_tp typ, void *param);
double map_negexp(double arg, map_arg_tp typ, void *param);
double map_phi(double arg, map_arg_tp typ, void *param);
double map_prec_qkumar(double arg, map_arg_tp typ, void *param);
double map_precision(double arg, map_arg_tp typ, void *param);
double map_probability(double x, map_arg_tp typ, void *param);
double map_range(double arg, map_arg_tp typ, void *param);
double map_rho(double arg, map_arg_tp typ, void *param);
double map_shape_svnig(double arg, map_arg_tp typ, void *param);
double map_sqrt1exp(double arg, map_arg_tp typ, void *param);

double mfunc_ar1(int thread_id, int i, void *arg);
double mfunc_cgeneric(int thread_id, int i, void *arg);
double mfunc_clinear(int thread_id, int i, void *arg);
double mfunc_log1exp(int thread_id, int i, void *arg);
double mfunc_logdist(int thread_id, int i, void *arg);
double mfunc_mec(int thread_id, int i, void *arg);
double mfunc_revsigm(int thread_id, int i, void *arg);
double mfunc_rgeneric(int thread_id, int i, void *arg);
double mfunc_sigm(int thread_id, int i, void *arg);

double priorfunc_beta(double *x, double *parameters);
double priorfunc_betacorrelation(double *x, double *parameters);
double priorfunc_bymjoint(double *logprec_besag, double *p_besag, double *logprec_iid, double *p_iid);
double priorfunc_dirichlet(double *x, double *parameters);
double priorfunc_flat(double *x, double *parameters);
double priorfunc_gamma(double *precision, double *parameters);
double priorfunc_gaussian(double *x, double *parameters);
double priorfunc_invalid(double *x, double *parameters);
double priorfunc_jeffreys_df_student_t(double *x, double *parameters);
double priorfunc_laplace(double *x, double *parameters);
double priorfunc_linksnintercept(double *x, double *parameters);
double priorfunc_logflat(double *x, double *parameters);
double priorfunc_loggamma(double *x, double *parameters);
double priorfunc_logiflat(double *x, double *parameters);
double priorfunc_logitbeta(double *x, double *parameters);
double priorfunc_minuslogsqrtruncnormal(double *x, double *parameters);
double priorfunc_mvnorm(double *x, double *parameters);
double priorfunc_normal(double *x, double *parameters);
double priorfunc_pc_alphaw(double *x, double *parameters);
double priorfunc_pc_ar(double *x, double *parameters);
double priorfunc_pc_cor0(double *x, double *parameters);
double priorfunc_pc_cor1(double *x, double *parameters);
double priorfunc_pc_dof(double *x, double *parameters);
double priorfunc_pc_gamma(double *x, double *parameters);
double priorfunc_pc_gammacount(double *x, double *parameters);
double priorfunc_pc_gevtail(double *x, double *parameters);
double priorfunc_pc_matern(double *x, double *parameters);
double priorfunc_pc_mgamma(double *x, double *parameters);
double priorfunc_pc_prec(double *x, double *parameters);
double priorfunc_pc_range(double *x, double *parameters);
double priorfunc_pc_sn(double *x, double *parameters);
double priorfunc_pc_spde_ga(double *x, double *parameters);
double priorfunc_ref_ar(double *x, double *parameters);
double priorfunc_wishart(int dim, double *x, double *parameters);
double priorfunc_wishart1d(double *x, double *parameters);
double priorfunc_wishart1d(double *x, double *parameters);
double priorfunc_wishart2d(double *x, double *parameters);
double priorfunc_wishart2d(double *x, double *parameters);
double priorfunc_wishart3d(double *x, double *parameters);
double priorfunc_wishart3d(double *x, double *parameters);
double priorfunc_wishart4d(double *x, double *parameters);
double priorfunc_wishart5d(double *x, double *parameters);
double priorfunc_wishart_generic(int idim, double *x, double *parameters);
double priorfunc_wishartk_10d(double *x, double *parameters);
double priorfunc_wishartk_11d(double *x, double *parameters);
double priorfunc_wishartk_12d(double *x, double *parameters);
double priorfunc_wishartk_13d(double *x, double *parameters);
double priorfunc_wishartk_14d(double *x, double *parameters);
double priorfunc_wishartk_15d(double *x, double *parameters);
double priorfunc_wishartk_16d(double *x, double *parameters);
double priorfunc_wishartk_17d(double *x, double *parameters);
double priorfunc_wishartk_18d(double *x, double *parameters);
double priorfunc_wishartk_19d(double *x, double *parameters);
double priorfunc_wishartk_20d(double *x, double *parameters);
double priorfunc_wishartk_2d(double *x, double *parameters);
double priorfunc_wishartk_3d(double *x, double *parameters);
double priorfunc_wishartk_4d(double *x, double *parameters);
double priorfunc_wishartk_5d(double *x, double *parameters);
double priorfunc_wishartk_6d(double *x, double *parameters);
double priorfunc_wishartk_7d(double *x, double *parameters);
double priorfunc_wishartk_8d(double *x, double *parameters);
double priorfunc_wishartk_9d(double *x, double *parameters);
double priorfunc_wishartk_generic(int idim, double *x, double *parameters);

inla_file_contents_tp *inla_read_file_contents(const char *filename);
inla_iarray_tp *find_all_f(inla_tp * mb, inla_component_tp id);
inla_tp *inla_build(const char *dict_filename, int verbose, int make_dir);

int ar_marginal_distribution(int p, double *pacf, double *prec, double *Q);
int ar_pacf2phi(int p, double *pacf, double *phi);
int ar_phi2pacf(int p, double *phi, double *pacf);
int ar_test1();
int count_f(inla_tp * mb, inla_component_tp id);
int find_f(inla_tp * mb, inla_component_tp id);
int find_tag(inla_tp * mb, const char *name);
int inla_INLA(inla_tp * mb);
int inla_INLA_preopt_experimental(inla_tp * mb);
int inla_INLA_preopt_stage1(inla_tp * mb, GMRFLib_preopt_res_tp * rpreopt);
int inla_INLA_preopt_stage2(inla_tp * mb, GMRFLib_preopt_res_tp * rpreopt);
int inla_R(char **argv);
int inla_add_copyof(inla_tp * mb);
int inla_add_scopyof(inla_tp * mb);
int inla_besag_scale(int thread_id, inla_besag_Qfunc_arg_tp * arg, int adj, int verbose);
int inla_cgeneric_debug(FILE * fp, char *secname, inla_cgeneric_cmd_tp cmd, double *out);
int inla_check_pardiso(void);
int inla_computed(GMRFLib_density_tp ** d, int n);
int inla_divisible(int n, int by);
int inla_endian(void);

int inla_error_field_is_void(const char *funcname, const char *secname, const char *field, const char *value);
int inla_error_file_error(const char *funcname, const char *filename, int n, int element_number, double val);
int inla_error_file_error2(const char *funcname, const char *filename, int n, int element_number, double val, int element_number2, double val2);
int inla_error_file_error_sorted(const char *funcname, const char *filename, int n, int element_number, double val);
int inla_error_file_numelm(const char *funcname, const char *filename, int n, int div);
int inla_error_file_totnumelm(const char *funcname, const char *filename, int n, int total);
int inla_error_general(const char *msg);
int inla_error_general2(const char *msg, const char *msg2);
int inla_error_missing_required_field(const char *funcname, const char *secname, const char *field);
int inla_error_open_file(const char *msg);

int inla_fgn(char *H_arg, char *outfile);
int inla_finn(const char *filename);
int inla_iid3d_adjust(double *rho);
int inla_iid_wishart_adjust(int dim, double *rho);
int inla_iid_wishart_nparam(int dim);
int inla_initial_setup(inla_tp * mb);
int inla_integrate_func(double *d_mean, double *d_stdev, double *d_mode, GMRFLib_density_tp * density, map_func_tp * func, void *func_arg,
			GMRFLib_transform_array_func_tp * tfunc);
int inla_is_NAs(int nx, const char *string);
int inla_layout_x(double **x_vec, int *len_x, GMRFLib_density_tp * density);
int inla_layout_x_ORIG(double **x, int *n, double xmin, double xmax, double mean);
int inla_make_2diid_graph(GMRFLib_graph_tp ** graph, inla_2diid_arg_tp * arg);
int inla_make_2diid_wishart_graph(GMRFLib_graph_tp ** graph, inla_2diid_arg_tp * arg);
int inla_make_3diid_graph(GMRFLib_graph_tp ** graph, inla_3diid_arg_tp * arg);
int inla_make_3diid_wishart_graph(GMRFLib_graph_tp ** graph, inla_3diid_arg_tp * arg);
int inla_make_ar1_graph(GMRFLib_graph_tp ** graph, inla_ar1_arg_tp * arg);
int inla_make_ar1c_graph(GMRFLib_graph_tp ** graph, inla_ar1c_arg_tp * arg);
int inla_make_besag2_graph(GMRFLib_graph_tp ** graph_out, GMRFLib_graph_tp * graph);
int inla_make_bym_graph(GMRFLib_graph_tp ** new_graph, GMRFLib_graph_tp * graph);
int inla_make_group_graph(GMRFLib_graph_tp ** new_graph, GMRFLib_graph_tp * graph, int ngroup, int type, int cyclic, int order,
			  GMRFLib_graph_tp * group_graph);
int inla_make_iid2d_graph(GMRFLib_graph_tp ** graph, inla_iid2d_arg_tp * arg);
int inla_make_iid3d_graph(GMRFLib_graph_tp ** graph, inla_iid3d_arg_tp * arg);
int inla_make_iid_wishart_graph(GMRFLib_graph_tp ** graph, inla_iid_wishart_arg_tp * arg);
int inla_make_iid_wishartk_graph(GMRFLib_graph_tp ** graph, inla_iid_wishartk_arg_tp * arg);
int inla_make_intslope_graph(GMRFLib_graph_tp ** graph, inla_intslope_arg_tp * arg);
int inla_make_ou_graph(GMRFLib_graph_tp ** graph, inla_ou_arg_tp * arg);
int inla_make_rw2diid_graph(GMRFLib_graph_tp ** graph, GMRFLib_rw2ddef_tp * def);
int inla_mix_int_quadrature_gaussian(int thread_id, double **x, double **w, int *n, void *arg);
int inla_mix_int_quadrature_loggamma(int thread_id, double **x, double **w, int *n, void *arg);
int inla_mix_int_quadrature_mloggamma(int thread_id, double **x, double **w, int *n, void *arg);
int inla_mix_int_simpson_gaussian(int thread_id, double **x, double **w, int *n, void *arg);
int inla_mix_int_simpson_loggamma(int thread_id, double **x, double **w, int *n, void *arg);
int inla_mix_int_simpson_mloggamma(int thread_id, double **x, double **w, int *n, void *arg);
int inla_mkdir(const char *dirname);
int inla_ncpu(void);

int inla_output(inla_tp * mb);
int inla_output_Q(inla_tp * mb, const char *dir, GMRFLib_graph_tp * graph);
int inla_output_detail(const char *dir, GMRFLib_density_tp ** density, double *locations, int n, int nrep, Output_tp * output, const char *sdir,
		       int return_marginals, map_func_tp * func, void *func_arg, GMRFLib_transform_array_func_tp ** tfunc, const char *tag,
		       const char *modelname, int verbose);
int inla_output_detail_cpo(const char *dir, GMRFLib_ai_cpo_tp * cpo, int predictor_n, int verbose);
int inla_output_detail_dic(const char *dir, GMRFLib_ai_dic_tp * dic, double *family_idx, int len_family_idx, int verbose);
int inla_output_detail_gcpo(const char *dir, GMRFLib_gcpo_tp * gcpo, int verbose);
int inla_output_detail_mlik(const char *dir, GMRFLib_ai_marginal_likelihood_tp * mlik, int verbose);
int inla_output_detail_po(const char *dir, GMRFLib_ai_po_tp * cpo, int predictor_n, int verbose);
int inla_output_detail_theta(const char *dir, double ***theta, int n_theta);
int inla_output_detail_x(const char *dir, double *x, int n_x);
int inla_output_gitid(const char *dir);
int inla_output_graph(inla_tp * mb, const char *dir, GMRFLib_graph_tp * graph);
int inla_output_hyperid(const char *dir, const char *sdir, char *hyperid);
int inla_output_id_names(const char *dir, const char *sdir, inla_file_contents_tp * fc);
int inla_output_linkfunctions(const char *dir, inla_tp * mb);
int inla_output_matrix(const char *dir, const char *sdir, const char *filename, int n, double *matrix, int *order);
int inla_output_misc(const char *dir, GMRFLib_ai_misc_output_tp * mo, int ntheta, char **theta_tag, char **from_theta, char **to_theta,
		     double *lc_order, int verbose, inla_tp * mb);
int inla_output_names(const char *dir, const char *sdir, int n, const char **names, const char *suffix);
int inla_output_ok(const char *dir);
int inla_output_size(const char *dir, const char *sdir, int n, int N, int Ntotal, int ngroup, int nrep);

int inla_parse_INLA(inla_tp * mb, dictionary * ini, int sec, int make_dir);
int inla_parse_data(inla_tp * mb, dictionary * ini, int sec);
int inla_parse_expert(inla_tp * mb, dictionary * ini, int sec);
int inla_parse_ffield(inla_tp * mb, dictionary * ini, int sec);
int inla_parse_libR(inla_tp * mb, dictionary * ini, int sec);
int inla_parse_lincomb(inla_tp * mb, dictionary * ini, int sec);
int inla_parse_linear(inla_tp * mb, dictionary * ini, int sec);
int inla_parse_lp_scale(inla_tp * mb, dictionary * ini, int sec, int UNUSED(make_dir));
int inla_parse_mode(inla_tp * mb, dictionary * ini, int sec);
int inla_parse_output(inla_tp * mb, dictionary * ini, int sec, Output_tp ** out);
int inla_parse_pardiso(inla_tp * mb, dictionary * ini, int sec, int make_dir);
int inla_parse_predictor(inla_tp * mb, dictionary * ini, int sec);
int inla_parse_problem(inla_tp * mb, dictionary * ini, int sec, int mkdir);
int inla_parse_update(inla_tp * mb, dictionary * ini, int sec, int make_dir);

int inla_qinv(const char *filename, const char *outfile, const char *constrfile);
int inla_qreordering(const char *filename);
int inla_qsample(const char *filename, const char *outfile, const char *nsamples, const char *rngfile, const char *samplefile, const char *bfile,
		 const char *mufile, const char *constr_file, const char *meanfile, const char *selectionfile, int verbose);
int inla_qsolve(const char *Qfilename, const char *Afilename, const char *Bfilename, const char *method);

int inla_read_data_all(double **x, int *n, const char *filename, int *ncol_data_all);
int inla_read_data_general(double **xx, int **ix, int *nndata, const char *filename, int n, int column, int n_columns, int verbose,
			   double default_value);
int inla_read_data_likelihood(inla_tp * mb, dictionary * ini, int sec);
int inla_read_fileinfo(inla_tp * mb, dictionary * ini, int sec, File_tp * file, const char *FILENAME);
int inla_read_graph(const char *filename);
int inla_read_prior(inla_tp * mb, dictionary * ini, int sec, Prior_tp * prior, const char *default_prior, void *args);
int inla_read_prior0(inla_tp * mb, dictionary * ini, int sec, Prior_tp * prior, const char *default_prior, void *args);
int inla_read_prior1(inla_tp * mb, dictionary * ini, int sec, Prior_tp * prior, const char *default_prior, void *args);
int inla_read_prior10(inla_tp * mb, dictionary * ini, int sec, Prior_tp * prior, const char *default_prior, void *args);
int inla_read_prior2(inla_tp * mb, dictionary * ini, int sec, Prior_tp * prior, const char *default_prior, void *args);
int inla_read_prior3(inla_tp * mb, dictionary * ini, int sec, Prior_tp * prior, const char *default_prior, void *args);
int inla_read_prior4(inla_tp * mb, dictionary * ini, int sec, Prior_tp * prior, const char *default_prior, void *args);
int inla_read_prior5(inla_tp * mb, dictionary * ini, int sec, Prior_tp * prior, const char *default_prior, void *args);
int inla_read_prior6(inla_tp * mb, dictionary * ini, int sec, Prior_tp * prior, const char *default_prior, void *args);
int inla_read_prior7(inla_tp * mb, dictionary * ini, int sec, Prior_tp * prior, const char *default_prior, void *args);
int inla_read_prior8(inla_tp * mb, dictionary * ini, int sec, Prior_tp * prior, const char *default_prior, void *args);
int inla_read_prior9(inla_tp * mb, dictionary * ini, int sec, Prior_tp * prior, const char *default_prior, void *args);
int inla_read_priorN(inla_tp * mb, dictionary * ini, int sec, Prior_tp * prior, const char *default_prior, int N, void *args);
int inla_read_priorN_scopy(inla_tp * mb, dictionary * ini, int sec, Prior_tp * prior, const char *default_prior, int N, void *args);
int inla_read_prior_generic(inla_tp * mb, dictionary * ini, int sec, Prior_tp * prior, const char *prior_tag, const char *param_tag,
			    const char *from_theta, const char *to_theta, const char *hyperid, const char *default_prior, void *args);
int inla_read_prior_group(inla_tp * mb, dictionary * ini, int sec, Prior_tp * prior, const char *default_prior, void *args);
int inla_read_prior_group0(inla_tp * mb, dictionary * ini, int sec, Prior_tp * prior, const char *default_prior, void *args);
int inla_read_prior_group1(inla_tp * mb, dictionary * ini, int sec, Prior_tp * prior, const char *default_prior, void *args);
int inla_read_prior_group10(inla_tp * mb, dictionary * ini, int sec, Prior_tp * prior, const char *default_prior, void *args);
int inla_read_prior_group2(inla_tp * mb, dictionary * ini, int sec, Prior_tp * prior, const char *default_prior, void *args);
int inla_read_prior_group3(inla_tp * mb, dictionary * ini, int sec, Prior_tp * prior, const char *default_prior, void *args);
int inla_read_prior_group4(inla_tp * mb, dictionary * ini, int sec, Prior_tp * prior, const char *default_prior, void *args);
int inla_read_prior_group5(inla_tp * mb, dictionary * ini, int sec, Prior_tp * prior, const char *default_prior, void *args);
int inla_read_prior_group6(inla_tp * mb, dictionary * ini, int sec, Prior_tp * prior, const char *default_prior, void *args);
int inla_read_prior_group7(inla_tp * mb, dictionary * ini, int sec, Prior_tp * prior, const char *default_prior, void *args);
int inla_read_prior_group8(inla_tp * mb, dictionary * ini, int sec, Prior_tp * prior, const char *default_prior, void *args);
int inla_read_prior_group9(inla_tp * mb, dictionary * ini, int sec, Prior_tp * prior, const char *default_prior, void *args);
int inla_read_prior_link(inla_tp * mb, dictionary * ini, int sec, Prior_tp * prior, const char *default_prior, void *args);
int inla_read_prior_link0(inla_tp * mb, dictionary * ini, int sec, Prior_tp * prior, const char *default_prior, void *args);
int inla_read_prior_link1(inla_tp * mb, dictionary * ini, int sec, Prior_tp * prior, const char *default_prior, void *args);
int inla_read_prior_link2(inla_tp * mb, dictionary * ini, int sec, Prior_tp * prior, const char *default_prior, void *args);
int inla_read_prior_mix(inla_tp * mb, dictionary * ini, int sec, Prior_tp * prior, const char *default_prior, void *args);
int inla_read_weightsinfo(inla_tp * mb, dictionary * ini, int sec, File_tp * file);

int inla_replicate_graph(GMRFLib_graph_tp ** g, int replicate);
int inla_setup_ai_par_default(inla_tp * mb);
int inla_sread(void *x, int nx, const char *str, int code);
int inla_sread_colon_ints(int *i, int *j, const char *str);
int inla_sread_doubles(double *x, int nx, const char *str);
int inla_sread_doubles_q(double **x, int *nx, const char *str);
int inla_sread_ints(int *x, int nx, const char *str);
int inla_sread_ints_q(int **x, int *nx, const char *str);
int inla_sread_q(void **x, int *nx, const char *str, int code);
int inla_sread_str_int(char **tag, int *i, const char *str);
int inla_sread_str_str(char **tag, int nmax, char *str);
int inla_tolower(char *string);
int inla_trim_family(char *family);
int inla_wishart3d_adjust(double *rho);
int inla_wishartk_build_Q(int dim, double *theta, gsl_matrix * Q, gsl_matrix * L);
int inla_write_file_contents(const char *filename, inla_file_contents_tp * fc);

int loglikelihood_0binomial(int thread_id, double *__restrict logll, double *__restrict x, int m, int idx, double *x_vec, double *y_cdf, void *arg,
			    char **arg_str);
int loglikelihood_0binomialS(int thread_id, double *__restrict logll, double *__restrict x, int m, int idx, double *x_vec, double *y_cdf, void *arg,
			     char **arg_str);
int loglikelihood_0poisson(int thread_id, double *__restrict logll, double *__restrict x, int m, int idx, double *x_vec, double *y_cdf, void *arg,
			   char **arg_str);
int loglikelihood_0poissonS(int thread_id, double *__restrict logll, double *__restrict x, int m, int idx, double *x_vec, double *y_cdf, void *arg,
			    char **arg_str);
int loglikelihood_agaussian(int thread_id, double *__restrict logll, double *__restrict x, int m, int idx, double *x_vec, double *y_cdf, void *arg,
			    char **arg_str);
int loglikelihood_agaussian(int thread_id, double *__restrict logll, double *__restrict x, int m, int idx, double *x_vec, double *y_cdf, void *arg,
			    char **arg_str);
int loglikelihood_bell(int thread_id, double *__restrict logll, double *__restrict x, int m, int idx, double *x_vec, double *y_cdf, void *arg,
		       char **arg_str);
int loglikelihood_bell(int thread_id, double *__restrict logll, double *__restrict x, int m, int idx, double *x_vec, double *y_cdf, void *arg,
		       char **arg_str);
int loglikelihood_beta(int thread_id, double *__restrict logll, double *__restrict x, int m, int idx, double *x_vec, double *y_cdf, void *arg,
		       char **arg_str);
int loglikelihood_betabinomial(int thread_id, double *__restrict logll, double *__restrict x, int m, int idx, double *x_vec, double *y_cdf,
			       void *arg, char **arg_str);
int loglikelihood_betabinomialna(int thread_id, double *__restrict logll, double *__restrict x, int m, int idx, double *x_vec, double *y_cdf,
				 void *arg, char **arg_str);
int loglikelihood_bgev(int thread_id, double *__restrict logll, double *__restrict x, int m, int idx, double *x_vec, double *y_cdf, void *arg,
		       char **arg_str);
int loglikelihood_binomial(int thread_id, double *__restrict logll, double *__restrict x, int m, int idx, double *x_vec, double *y_cdf, void *arg,
			   char **arg_str);
int loglikelihood_cbinomial(int thread_id, double *__restrict logll, double *__restrict x, int m, int idx, double *x_vec, double *y_cdf, void *arg,
			    char **arg_str);
int loglikelihood_cenpoisson(int thread_id, double *__restrict logll, double *__restrict x, int m, int idx, double *x_vec, double *y_cdf, void *arg,
			     char **arg_str);
int loglikelihood_cenpoisson2(int thread_id, double *__restrict logll, double *__restrict x, int m, int idx, double *x_vec, double *y_cdf,
			      void *arg, char **arg_str);
int loglikelihood_circular_normal(int thread_id, double *__restrict logll, double *__restrict x, int m, int idx, double *x_vec, double *y_cdf,
				  void *arg, char **arg_str);
int loglikelihood_contpoisson(int thread_id, double *__restrict logll, double *__restrict x, int m, int idx, double *x_vec, double *y_cdf,
			      void *arg, char **arg_str);
int loglikelihood_dgp(int thread_id, double *__restrict logll, double *__restrict x, int m, int idx, double *x_vec, double *y_cdf, void *arg,
		      char **arg_str);
int loglikelihood_exp(int thread_id, double *__restrict logll, double *__restrict x, int m, int idx, double *x_vec, double *y_cdf, void *arg,
		      char **arg_str);
int loglikelihood_expsurv(int thread_id, double *__restrict logll, double *__restrict x, int m, int idx, double *x_vec, double *y_cdf, void *arg,
			  char **arg_str);
int loglikelihood_fmri(int thread_id, double *__restrict logll, double *__restrict x, int m, int idx, double *x_vec, double *y_cdf, void *arg,
		       char **arg_str);
int loglikelihood_fmrisurv(int thread_id, double *__restrict logll, double *__restrict x, int m, int idx, double *x_vec, double *y_cdf, void *arg,
			   char **arg_str);
int loglikelihood_gamma(int thread_id, double *__restrict logll, double *__restrict x, int m, int idx, double *x_vec, double *y_cdf, void *arg,
			char **arg_str);
int loglikelihood_gammacount(int thread_id, double *__restrict logll, double *__restrict x, int m, int idx, double *x_vec, double *y_cdf, void *arg,
			     char **arg_str);
int loglikelihood_gammajw(int thread_id, double *__restrict logll, double *__restrict x, int m, int idx, double *x_vec, double *y_cdf, void *arg,
			  char **arg_str);
int loglikelihood_gammasurv(int thread_id, double *__restrict logll, double *__restrict x, int m, int idx, double *x_vec, double *y_cdf, void *arg,
			    char **arg_str);
int loglikelihood_gammasurvjw(int thread_id, double *__restrict logll, double *__restrict x, int m, int idx, double *x_vec, double *y_cdf,
			      void *arg, char **arg_str);
int loglikelihood_gaussian(int thread_id, double *__restrict logll, double *__restrict x, int m, int idx, double *x_vec, double *y_cdf, void *arg,
			   char **arg_str);
int loglikelihood_gaussianjw(int thread_id, double *__restrict logll, double *__restrict x, int m, int idx, double *UNUSED(x_vec), double *y_cdf,
			     void *arg, char **arg_str);
int loglikelihood_generic_surv(int thread_id, double *__restrict logll, double *__restrict x, int m, int idx, double *x_vec, double *y_cdf,
			       void *arg, GMRFLib_logl_tp * loglfun, char **arg_str);
int loglikelihood_generic_surv_NEW(int thread_id, double *__restrict logll, double *__restrict x, int m, int idx, double *x_vec, double *y_cdf,
				   void *arg, GMRFLib_logl_tp * loglfun, char **arg_str);
int loglikelihood_gev(int thread_id, double *__restrict logll, double *__restrict x, int m, int idx, double *x_vec, double *y_cdf, void *arg,
		      char **arg_str);
int loglikelihood_ggaussian(int thread_id, double *__restrict logll, double *__restrict x, int m, int idx, double *x_vec, double *y_cdf, void *arg,
			    char **arg_str);
int loglikelihood_ggaussianS(int thread_id, double *__restrict logll, double *__restrict x, int m, int idx, double *x_vec, double *y_cdf, void *arg,
			     char **arg_str);
int loglikelihood_gompertz(int thread_id, double *__restrict logll, double *__restrict x, int m, int idx, double *x_vec, double *y_cdf, void *arg,
			   char **arg_str);
int loglikelihood_gompertzsurv(int thread_id, double *__restrict logll, double *__restrict x, int m, int idx, double *x_vec, double *y_cdf,
			       void *arg, char **arg_str);
int loglikelihood_gp(int thread_id, double *__restrict logll, double *__restrict x, int m, int idx, double *x_vec, double *y_cdf, void *arg,
		     char **arg_str);
int loglikelihood_gpoisson(int thread_id, double *__restrict logll, double *__restrict x, int m, int idx, double *x_vec, double *y_cdf, void *arg,
			   char **arg_str);
int loglikelihood_iid_gamma(int thread_id, double *__restrict logll, double *__restrict x, int m, int idx, double *x_vec, double *y_cdf, void *arg,
			    char **arg_str);
int loglikelihood_iid_logitbeta(int thread_id, double *__restrict logll, double *__restrict x, int m, int idx, double *x_vec, double *y_cdf,
				void *arg, char **arg_str);
int loglikelihood_inla(int thread_id, double *__restrict logll, double *__restrict x, int m, int idx, double *x_vec, double *y_cdf, void *arg,
		       char **arg_str);
int loglikelihood_loggamma_frailty(int thread_id, double *__restrict logll, double *__restrict x, int m, int idx, double *x_vec, double *y_cdf,
				   void *arg, char **arg_str);
int loglikelihood_logistic(int thread_id, double *__restrict logll, double *__restrict x, int m, int idx, double *x_vec, double *y_cdf, void *arg,
			   char **arg_str);
int loglikelihood_loglogistic(int thread_id, double *__restrict logll, double *__restrict x, int m, int idx, double *x_vec, double *y_cdf,
			      void *arg, char **arg_str);
int loglikelihood_loglogisticsurv(int thread_id, double *__restrict logll, double *__restrict x, int m, int idx, double *x_vec, double *y_cdf,
				  void *arg, char **arg_str);
int loglikelihood_lognormal(int thread_id, double *__restrict logll, double *__restrict x, int m, int idx, double *x_vec, double *y_cdf, void *arg,
			    char **arg_str);
int loglikelihood_lognormalsurv(int thread_id, double *__restrict logll, double *__restrict x, int m, int idx, double *x_vec, double *y_cdf,
				void *arg, char **arg_str);
int loglikelihood_logperiodogram(int thread_id, double *__restrict logll, double *__restrict x, int m, int idx, double *x_vec, double *y_cdf,
				 void *arg, char **arg_str);
int loglikelihood_mix_core(int thread_id, double *__restrict logll, double *__restrict x, int m, int idx, double *x_vec, double *y_cdf, void *arg,
			   int (*quadrature)(int, double **, double **, int *, void *), int(*simpson)(int, double **, double **, int *, void *),
			   char **arg_str);
int loglikelihood_mix_loggamma(int thread_id, double *__restrict logll, double *__restrict x, int m, int idx, double *x_vec, double *y_cdf,
			       void *arg, char **arg_str);
int loglikelihood_mix_mloggamma(int thread_id, double *__restrict logll, double *__restrict x, int m, int idx, double *x_vec, double *y_cdf,
				void *arg, char **arg_str);
int loglikelihood_nbinomial2(int thread_id, double *__restrict logll, double *__restrict x, int m, int idx, double *x_vec, double *y_cdf, void *arg,
			     char **arg_str);
int loglikelihood_negative_binomial(int thread_id, double *__restrict logll, double *__restrict x, int m, int idx, double *x_vec, double *y_cdf,
				    void *arg, char **arg_str);
int loglikelihood_nmix(int thread_id, double *__restrict logll, double *__restrict x, int m, int idx, double *x_vec, double *y_cdf, void *arg,
		       char **arg_str);
int loglikelihood_nmixnb(int thread_id, double *__restrict logll, double *__restrict x, int m, int idx, double *x_vec, double *y_cdf, void *arg,
			 char **arg_str);
int loglikelihood_nzpoisson(int thread_id, double *__restrict logll, double *__restrict x, int m, int idx, double *x_vec, double *y_cdf, void *arg,
			    char **arg_str);
int loglikelihood_poisson(int thread_id, double *__restrict logll, double *__restrict x, int m, int idx, double *x_vec, double *y_cdf, void *arg,
			  char **arg_str);
int loglikelihood_poisson_special1(int thread_id, double *__restrict logll, double *__restrict x, int m, int idx, double *x_vec, double *y_cdf,
				   void *arg, char **arg_str);
int loglikelihood_pom(int thread_id, double *__restrict logll, double *__restrict x, int m, int idx, double *x_vec, double *y_cdf, void *arg,
		      char **arg_str);
int loglikelihood_qcontpoisson(int thread_id, double *__restrict logll, double *__restrict x, int m, int idx, double *x_vec, double *y_cdf,
			       void *arg, char **arg_str);
int loglikelihood_qkumar(int thread_id, double *__restrict logll, double *__restrict x, int m, int idx, double *x_vec, double *y_cdf, void *arg,
			 char **arg_str);
int loglikelihood_qloglogistic(int thread_id, double *__restrict logll, double *__restrict x, int m, int idx, double *x_vec, double *y_cdf,
			       void *arg, char **arg_str);
int loglikelihood_qloglogisticsurv(int thread_id, double *__restrict logll, double *__restrict x, int m, int idx, double *x_vec, double *y_cdf,
				   void *arg, char **arg_str);
int loglikelihood_rcpoisson(int thread_id, double *__restrict logll, double *__restrict x, int m, int idx, double *UNUSED(x_vec), double *y_cdf,
			    void *arg, char **arg_str);
int loglikelihood_simplex(int thread_id, double *__restrict logll, double *__restrict x, int m, int idx, double *x_vec, double *y_cdf, void *arg,
			  char **arg_str);
int loglikelihood_skew_normal(int thread_id, double *__restrict logll, double *__restrict x, int m, int idx, double *x_vec, double *y_cdf,
			      void *arg, char **arg_str);
int loglikelihood_stdgaussian(int thread_id, double *__restrict logll, double *__restrict x, int m, int idx, double *UNUSED(x_vec), double *y_cdf,
			      void *arg, char **arg_str);
int loglikelihood_stochvol(int thread_id, double *__restrict logll, double *__restrict x, int m, int idx, double *x_vec, double *y_cdf, void *arg,
			   char **arg_str);
int loglikelihood_stochvol_nig(int thread_id, double *__restrict logll, double *__restrict x, int m, int idx, double *x_vec, double *y_cdf,
			       void *arg, char **arg_str);
int loglikelihood_stochvol_sn(int thread_id, double *__restrict logll, double *__restrict x, int m, int idx, double *x_vec, double *y_cdf,
			      void *arg, char **arg_str);
int loglikelihood_stochvol_t(int thread_id, double *__restrict logll, double *__restrict x, int m, int idx, double *x_vec, double *y_cdf, void *arg,
			     char **arg_str);
int loglikelihood_t(int thread_id, double *__restrict logll, double *__restrict x, int m, int idx, double *x_vec, double *y_cdf, void *arg,
		    char **arg_str);
int loglikelihood_testit(int thread_id, double *__restrict logll, double *__restrict x, int m, int idx, double *x_vec, double *y_cdf, void *arg,
			 char **arg_str);
int loglikelihood_testit1(int thread_id, double *__restrict logll, double *__restrict x, int m, int idx, double *x_vec, double *y_cdf, void *arg,
			  char **arg_str);
int loglikelihood_tpoisson(int thread_id, double *__restrict logll, double *__restrict x, int m, int idx, double *UNUSED(x_vec), double *y_cdf,
			   void *arg, char **arg_str);
int loglikelihood_tstrata(int thread_id, double *__restrict logll, double *__restrict x, int m, int idx, double *x_vec, double *y_cdf, void *arg,
			  char **arg_str);
int loglikelihood_tweedie(int thread_id, double *__restrict logll, double *__restrict x, int m, int idx, double *x_vec, double *y_cdf, void *arg,
			  char **arg_str);
int loglikelihood_weibull(int thread_id, double *__restrict logll, double *__restrict x, int m, int idx, double *x_vec, double *y_cdf, void *arg,
			  char **arg_str);
int loglikelihood_weibullsurv(int thread_id, double *__restrict logll, double *__restrict x, int m, int idx, double *x_vec, double *y_cdf,
			      void *arg, char **arg_str);
int loglikelihood_wrapped_cauchy(int thread_id, double *__restrict logll, double *__restrict x, int m, int idx, double *x_vec, double *y_cdf,
				 void *arg, char **arg_str);
int loglikelihood_xbinomial(int thread_id, double *__restrict logll, double *__restrict x, int m, int idx, double *UNUSED(x_vec), double *y_cdf,
			    void *arg, char **arg_str);
int loglikelihood_zero_n_inflated_binomial2(int thread_id, double *__restrict logll, double *__restrict x, int m, int idx, double *x_vec,
					    double *y_cdf, void *arg, char **arg_str);
int loglikelihood_zero_n_inflated_binomial3(int thread_id, double *__restrict logll, double *__restrict x, int m, int idx, double *x_vec,
					    double *y_cdf, void *arg, char **arg_str);
int loglikelihood_zeroinflated_betabinomial0(int thread_id, double *__restrict logll, double *__restrict x, int m, int idx, double *x_vec,
					     double *y_cdf, void *arg, char **arg_str);
int loglikelihood_zeroinflated_betabinomial1(int thread_id, double *__restrict logll, double *__restrict x, int m, int idx, double *x_vec,
					     double *y_cdf, void *arg, char **arg_str);
int loglikelihood_zeroinflated_betabinomial2(int thread_id, double *__restrict logll, double *__restrict x, int m, int idx, double *x_vec,
					     double *y_cdf, void *arg, char **arg_str);
int loglikelihood_zeroinflated_binomial0(int thread_id, double *__restrict logll, double *__restrict x, int m, int idx, double *x_vec,
					 double *y_cdf, void *arg, char **arg_str);
int loglikelihood_zeroinflated_binomial1(int thread_id, double *__restrict logll, double *__restrict x, int m, int idx, double *x_vec,
					 double *y_cdf, void *arg, char **arg_str);
int loglikelihood_zeroinflated_binomial2(int thread_id, double *__restrict logll, double *__restrict x, int m, int idx, double *x_vec,
					 double *y_cdf, void *arg, char **arg_str);
int loglikelihood_zeroinflated_cenpoisson0(int thread_id, double *__restrict logll, double *__restrict x, int m, int idx, double *x_vec,
					   double *y_cdf, void *arg, char **arg_str);
int loglikelihood_zeroinflated_cenpoisson1(int thread_id, double *__restrict logll, double *__restrict x, int m, int idx, double *x_vec,
					   double *y_cdf, void *arg, char **arg_str);
int loglikelihood_zeroinflated_negative_binomial0(int thread_id, double *__restrict logll, double *__restrict x, int m, int idx, double *x_vec,
						  double *y_cdf, void *arg, char **arg_str);
int loglikelihood_zeroinflated_negative_binomial1(int thread_id, double *__restrict logll, double *__restrict x, int m, int idx, double *x_vec,
						  double *y_cdf, void *arg, char **arg_str);
int loglikelihood_zeroinflated_negative_binomial1_strata2(int thread_id, double *__restrict logll, double *__restrict x, int m, int idx,
							  double *x_vec, double *y_cdf, void *arg, char **arg_str);
int loglikelihood_zeroinflated_negative_binomial1_strata3(int thread_id, double *__restrict logll, double *__restrict x, int m, int idx,
							  double *x_vec, double *y_cdf, void *arg, char **arg_str);
int loglikelihood_zeroinflated_negative_binomial2(int thread_id, double *__restrict logll, double *__restrict x, int m, int idx, double *x_vec,
						  double *y_cdf, void *arg, char **arg_str);
int loglikelihood_zeroinflated_poisson0(int thread_id, double *__restrict logll, double *__restrict x, int m, int idx, double *x_vec, double *y_cdf,
					void *arg, char **arg_str);
int loglikelihood_zeroinflated_poisson1(int thread_id, double *__restrict logll, double *__restrict x, int m, int idx, double *x_vec, double *y_cdf,
					void *arg, char **arg_str);
int loglikelihood_zeroinflated_poisson2(int thread_id, double *__restrict logll, double *__restrict x, int m, int idx, double *x_vec, double *y_cdf,
					void *arg, char **arg_str);

int inla_reset(void);
int inla_testit_timer(void);
int my_dir_exists(const char *dirname);
int my_file_exists(const char *filename);
int my_setenv(char *str, int prefix);
int testit(int argc, char **argv);
map_table_tp *mapfunc_find(const char *name);
unsigned long long getTotalSystemMemory();
void inla_signal(int sig);

double testit_Qfunc(int thread_id, int i, int j, double *values, void *arg);

// defined in cores.c
int UTIL_countPhysicalCores(void);
int UTIL_countCores(int);
int UTIL_countLogicalCores(void);

int gsl_bfgs4_test1(size_t);
int bfgs4_robust_minimize(double *xmin, double *ymin, int nn, double *x, double *y, int mm, double *xd, double *yd, int order);

/* 
***
*/

typedef struct {
	int fast_mode;					       /* avoid detailed calculations but use ok approximations */
	inla_mode_tp mode;				       /* which mode to run in */
	double log_prec_initial;			       /* inititial value for log-precisions */
	double mcmc_scale;				       /* scaling */
	int mcmc_thinning;				       /* thinning */
	int mcmc_niter;					       /* number of iterations: 0 is infinite */
	GMRFLib_reorder_tp reorder;			       /* reorder strategy: -1 for optimize */
	int mcmc_fifo;					       /* use fifo to communicate in mcmc mode */
	int mcmc_fifo_pass_data;			       /* use fifo to communicate in mcmc mode, pass also all data */
} G_tp;

#define HYPER_NEW2(name_, initial_, n_)  \
	if (1) {							\
		name_ = Calloc(n_, double **);				\
		for(int j_=0; j_ < n_; j_++){				\
			name_[j_] = Calloc(GMRFLib_MAX_THREADS(), double *); \
			for(int i_ = 0; i_ < GMRFLib_MAX_THREADS(); i_++) { \
				name_[j_][i_] = Calloc(1, double);	\
				name_[j_][i_][0] = initial_;		\
			}						\
		}							\
	}								\

#define HYPER_NEW(name_, initial_)					\
	if (1) {							\
		name_ = Calloc(GMRFLib_MAX_THREADS(), double *);	\
		for(int i_ = 0; i_ < GMRFLib_MAX_THREADS(); i_++) {	\
			name_[i_] = Calloc(1, double);			\
			name_[i_][0] = initial_;			\
		}							\
	}

#define HYPER_FREE(name_)						\
	if (1) {							\
		for(int i_ = 0; i_ < GMRFLib_MAX_THREADS(); i_++) {	\
			Free(name_[i_]);				\
		}							\
		Free(name_);						\
	}

#define HYPER_INIT(name_, initial_)		\
	if (1) {				\
		for(int i_ = 0; i_ < GMRFLib_MAX_THREADS(); i_++) {	\
			name_[i_][0] = initial_;			\
		}							\
	}

#define WISHART_DIM(idx) (mb->f_id[idx] == F_IID1D ? 1 :		\
			  (mb->f_id[idx] == F_IID2D ? 2 :		\
			   (mb->f_id[idx] == F_IID3D ? 3 :		\
			    (mb->f_id[idx] == F_IID4D ? 4 :		\
			     (mb->f_id[idx] == F_IID5D ? 5 : -1)))))

__END_DECLS
#endif
