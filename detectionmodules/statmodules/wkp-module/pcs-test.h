/**************************************************************************/
/*    Copyright (C) 2006 Romain Michalec                                  */
/*                                                                        */
/*    This library is free software; you can redistribute it and/or       */
/*    modify it under the terms of the GNU Lesser General Public          */
/*    License as published by the Free Software Foundation; either        */
/*    version 2.1 of the License, or (at your option) any later version.  */
/*                                                                        */
/*    This library is distributed in the hope that it will be useful,     */
/*    but WITHOUT ANY WARRANTY; without even the implied warranty of      */
/*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU   */
/*    Lesser General Public License for more details.                     */
/*                                                                        */
/*    You should have received a copy of the GNU Lesser General Public    */
/*    License along with this library; if not, write to the Free Software   */
/*    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,          */
/*    MA  02110-1301, USA                                                 */
/*                                                                        */
/**************************************************************************/

#ifndef _PCS_TEST_H_
#define _PCS_TEST_H_

#include <vector>
#include <list>
#include <gsl/gsl_cdf.h>

void pcs_categories ( std::list<int64_t>, std::list<int64_t>,
		      std::vector<unsigned> &, std::vector<unsigned> &,
		      std::ofstream & outfile );

double pcs_test ( const std::list<int64_t> &,
		  const std::list<int64_t> &,
		  std::ofstream & outfile );

#endif
