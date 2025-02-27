
/* link-gev.h
 * 
 * Copyright (C) 2024-2024 Havard Rue
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
 */
#ifndef __INLA_LINK_GEV_H__
#define __INLA_LINK_GEV_H__

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

/* 
 *
 */
double inla_log_pgev(double y, double xi, double *l_xi);
double inla_log_pcgev(double y, double xi, double *l_xi);
double inla_pgev(double y, double xi, double *l_xi);
double inla_pcgev(double y, double xi, double *l_xi);
double inla_inv_pgev(double p, double xi, double *l_xi);
double inla_inv_pcgev(double p, double xi, double *l_xi);
double link_gev(int thread_id, double arg, map_arg_tp typ, void *param, double *cov);
double link_cgev(int thread_id, double arg, map_arg_tp typ, void *param, double *cov);
double link_gev_core(int thread_id, double arg, map_arg_tp typ, void *param, int type);

__END_DECLS
#endif
