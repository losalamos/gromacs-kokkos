/*
 * This file is part of the GROMACS molecular simulation package.
 *
 * Copyright (c) 1991-2000, University of Groningen, The Netherlands.
 * Copyright (c) 2001-2012, The GROMACS development team.
 * Copyright (c) 2012,2013,2014,2015, by the GROMACS development team, led by
 * Mark Abraham, David van der Spoel, Berk Hess, and Erik Lindahl,
 * and including many others, as listed in the AUTHORS file in the
 * top-level source directory and at http://www.gromacs.org.
 *
 * GROMACS is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation; either version 2.1
 * of the License, or (at your option) any later version.
 *
 * GROMACS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with GROMACS; if not, see
 * http://www.gnu.org/licenses, or write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA.
 *
 * If you want to redistribute modifications to GROMACS, please
 * consider that scientific software is very special. Version
 * control is crucial - bugs must be traceable. We will be happy to
 * consider code for inclusion in the official distribution, but
 * derived work must not be called official GROMACS. Details are found
 * in the README & COPYING files - if they are missing, get the
 * official version at http://www.gromacs.org.
 *
 * To help us fund GROMACS development, we humbly ask that you cite
 * the research papers on the package. Check out http://www.gromacs.org.
 */

/*! \internal \file
 *  \brief
 *  Nonbonded Kokkos kernel functions.
 *
 *  \author Sikandar Y. Mashayak <symashayak@gmail.com>
 *  \ingroup module_mdlib
 */

#ifndef NBNXN_KOKKOS_KERNEL_H
#define NBNXN_KOKKOS_KERNEL_H

#include "gromacs/mdlib/nbnxn_kokkos.h"

#include "nbnxn_kokkos_types.h"

// transfer data from host-to-device if the data on the host is modified
void kokkos_sync_h2d (kokkos_atomdata_t*  kk_nbat,const kokkos_pairlist_t* kk_plist)
{

    kk_nbat->k_x.sync<GMXDeviceType>();

    // kk_plist->k_ci.sync<GMXDeviceType>();
    // kk_plist->k_sci.sync<GMXDeviceType>();
    // kk_plist->k_cj.sync<GMXDeviceType>();
    // kk_plist->k_cj4.sync<GMXDeviceType>();
    // kk_plist->k_excl.sync<GMXDeviceType>();


}

// transfer data from device-to-host if the data on the device is modified
void kokkos_sync_d2h (kokkos_atomdata_t*  kk_nbat,const kokkos_pairlist_t* kk_plist)
{

    //    kk_nbat->k_f.sync<GMXHosType>();

}

struct nbnxn_kokkos_kernel_functor
{
    typedef GMXDeviceType device_type;
    //typedef Kokkos::Vectorization<GMXDeviceType, NeighClusterSize> vectorization;

    // list of structures needed for non-bonded interactions
    const kokkos_atomdata_t *_adat;
    const kokkos_pairlist_t *_plist;

    nbnxn_kokkos_kernel_functor (const kokkos_atomdata_t*  adat_,const kokkos_pairlist_t* plist_):
        _adat(adat_),_plist(plist_)
    {

    };

    ~nbnxn_kokkos_kernel_functor ( )
    {

    };

    KOKKOS_FUNCTION
    real compute_item(const typename Kokkos::TeamPolicy<device_type>::member_type& dev) const
    {

    }

    KOKKOS_INLINE_FUNCTION
    void operator()(const  typename Kokkos::TeamPolicy<device_type>::member_type& dev) const
    {
        //   compute_item(dev);
        // index for i cluster assiged to this team
        int my_ci = dev.league_rank();

        printf("my i cluster %d\n", my_ci);
        printf("my team size %d\n",dev.team_size());
        // load M coord+params for ci
        
        // for each cj cluster
        //   load N coords+params for cj
        //   for j = 0 to M
        //     for i = 0 to N
        //       calculate interaction of ci*M+i  with cj*N+j
        //   store N cj-forces
        // store M ci-forces
        
    }

};

real nbnxn_kokkos_launch_kernel (nbnxn_pairlist_t     *nbl,
                                 nbnxn_atomdata_t     *nbat)
{

    typedef nbnxn_kokkos_kernel_functor f_type;
    f_type nb_f(nbat->kk_nbat, nbl->kk_plist);

    // transfer data from host to device
    // transfer happens only if the data on the host is modified
    kokkos_sync_h2d(nbat->kk_nbat, nbl->kk_plist);

    // following loop structure based on clusters explained in fig. 6
    // of ref. S. Pall and B. Hess, Comp. Phys. Comm., 184, 2013

    // using teams of threads
    // each team conists of M threads (here M==N)
    // each team works on one ci cluster, hence there are as many teams as ci clusters

    //typedef Kokkos::TeamPolicy<device_type>::member_type member_type;
    const int nteams = nbl->nci;
    const int teamsize = nbl->na_ci;
    Kokkos::TeamPolicy<typename f_type::device_type> config(nteams,teamsize);


    printf("\n number of i clusters %d\n", nteams);
    printf("\n number of atoms in a cluster %d\n", teamsize);

    // launch kernel
    Kokkos::parallel_for(config,nb_f);

    // transfer data from device to device
    // transfer happens only if the data on the device is modified, mainly forces
    kokkos_sync_d2h(nbat->kk_nbat, nbl->kk_plist);
}

#endif
