/**
 * @file        reachgraph_sequential.h
 * @author      Rodrigo T. Saad
 * @email       rsaad@laas.fr
 * @company:    LAAS-CNRS
 * @created     on July 17, 2009, 5:20 PM
 *
 * @section LICENSE
 *
 * CeCILL FREE SOFTWARE LICENSE AGREEMENT
 *
 * Copyright LAAS-CNRS
 *
 * This software is governed by the CeCILL  license under French law and
 * abiding by the rules of distribution of free software.  You can  use,
 * modify and/ or redistribute the software under the terms of the CeCILL
 * license as circulated by CEA, CNRS and INRIA at the following URL
 * "http://www.cecill.info".
 *
 * As a counterpart to the access to the source code and  rights to copy,
 * modify and redistribute granted by the license, users are provided only
 * with a limited warranty  and the software's author,  the holder of the
 * economic rights,  and the successive licensors  have only  limited
 * liability.
 *
 * In this respect, the user's attention is drawn to the risks associated
 * with loading,  using,  modifying and/or developing or reproducing the
 * software by the user in light of its specific status of free software,
 * that may mean  that it is complicated to manipulate,  and  that  also
 * therefore means  that it is reserved for developers  and  experienced
 * professionals having in-depth computer knowledge. Users are therefore
 * encouraged to load and test the software's suitability as regards their
 * requirements in conditions enabling the security of their systems and/or
 * data to be ensured and,  more generally, to use and operate it in the
 * same conditions as regards security.
 *
 * The fact that you are presently reading this means that you have had
 * knowledge of the CeCILL license and that you accept its terms.
 *
 * @section DESCRIPTION
 *
 * This file defines the sequential graph exploration engine.
 *
 */


#ifndef REACHGRAPH_SEQUENTIAL_H
#define	REACHGRAPH_SEQUENTIAL_H

#define _MULTI_THREADED


#include "reset_define_includes.h"
#define ASSERTLIB
#define ERRORLIB
#define MATHLIB
#define STDIOLIB
#define MATHLIB
#include "standard_includes.h"

#include "list.h"
#include "stack.h"
#include "vector.h"
#include "petri_net.h"
#include "flags.h"
#include "bloom.h"

//Initiates parallel exploration
extern void reachgraph_sequential_start(const Net *net, const int dfirst, const ModeEnum mode);

#endif	/* _REACHGRAPH_H */

