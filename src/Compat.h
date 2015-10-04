///////////////////////////////////////////////////////////////////////////////
// INetGet - Lightweight command-line front-end to WinINet API
// Copyright (C) 2015 LoRd_MuldeR <MuldeR2@GMX.de>
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License along
// with this program; if not, write to the Free Software Foundation, Inc.,
// 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
//
// See https://www.gnu.org/licenses/gpl-2.0-standalone.html for details!
///////////////////////////////////////////////////////////////////////////////

#if _MSC_VER >= 1800
#define ISNAN(X) std::isnan((X))
#define ROUND(X) std::round((X))
#else
#define ISNAN(X) _isnan((X))
#include <cmath>
static inline double ROUND(const double &d)
{
	return (d >= 0.0) ? floor(d + 0.5) : ceil(d - 0.5);
}
#endif

#define DBL_TO_UINT32(X) (((X) < UINT32_MAX) ? uint32_t((X)) : UINT32_MAX)

#define DBL_VALID_GTR(X,Y) ((!ISNAN((X))) && ((X) > (Y)))
#define DBL_VALID_LSS(X,Y) ((!ISNAN((X))) && ((X) < (Y)))
#define DBL_VALID_GEQ(X,Y) ((!ISNAN((X))) && ((X) >= (Y)))
#define DBL_VALID_LEQ(X,Y) ((!ISNAN((X))) && ((X) <= (Y)))
