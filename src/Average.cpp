///////////////////////////////////////////////////////////////////////////////
// INetGet - Lightweight command-line front-end to WinINet API
// Copyright (C) 2015-2018 LoRd_MuldeR <MuldeR2@GMX.de>
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

#include "Average.h"
#include "Compat.h"

//CRT
#include <stdint.h>
#include <limits>
#include <algorithm>

//Const
static const uint32_t MIN_LEN = 3;
static const double PI = 3.1415926535897932384626433832795028841971693993751058209749445923078164062862089986280348253421170679;
static const double ALPHA = 0.25;

Average::Average(const uint32_t &queue_len)
:
	m_queue_len(queue_len),
	m_current(std::numeric_limits<double>::quiet_NaN())
{
}

Average::~Average()
{
}

double Average::update(const double &value)
{
	m_values.push_back(value);
	while(m_values.size() > m_queue_len)
	{
		m_values.pop_front();
	}

	if(m_values.size() >= MIN_LEN)
	{
		std::vector<double> sorted(m_values.cbegin(), m_values.cend());
		std::sort(sorted.begin(), sorted.end());

		const size_t skip_count = sorted.size() / 10U;
		const size_t limit = sorted.size() - skip_count;
		
		double mean_value = 0.0;
		for(size_t i = skip_count; i < limit; i++)
		{
			mean_value += (sorted[i] / double(sorted.size()));
		}

		if(ISNAN(m_current))
		{
			m_current = mean_value;
		}

		return m_current = (mean_value * ALPHA) + (m_current * (1.0 - ALPHA));
	}

	return std::numeric_limits<double>::quiet_NaN();
}
