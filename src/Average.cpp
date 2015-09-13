///////////////////////////////////////////////////////////////////////////////
// INetGet - Lightweight command-line front-end to WinInet API
// Copyright (C) 2015 LoRd_MuldeR <MuldeR2@GMX.de>
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version, but always including the *additional*
// restrictions defined in the "License.txt" file.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along
// with this program; if not, write to the Free Software Foundation, Inc.,
// 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
//
// http://www.gnu.org/licenses/gpl-2.0.txt
///////////////////////////////////////////////////////////////////////////////

#include "Average.h"

//CRT
#include <stdint.h>

//Const
static const size_t MIN_LEN = 3;
static const double PI = 3.1415926535897932384626433832795028841971693993751058209749445923078164062862089986280348253421170679;

Average::Average(const size_t &queue_len)
:
	m_queue_len(queue_len)
{
	if((m_queue_len < MIN_LEN) || ((m_queue_len % 2) != 1))
	{
		throw std::runtime_error("Filter size must be a positive and odd value!");
	}
	
	for(size_t len = MIN_LEN; len <= queue_len; len += 2)
	{
		initialize_weights(len);
	}
}

Average::~Average()
{
}

void Average::initialize_weights(const size_t filter_len)
{
	std::vector<double> weights(filter_len, 0.0);
	double totalWeight = 0.0;

	const double sigma = (((double(filter_len) / 2.0) - 1.0) / 3.0) + (1.0 / 3.0);
	const uint32_t offset = filter_len / 2;
	const double c1 = 1.0 / (sigma * sqrt(2.0 * PI));
	const double c2 = 2.0 * pow(sigma, 2.0);

	for(uint32_t i = 0; i < filter_len; i++)
	{
		const int32_t x = int32_t(i) - int32_t(offset);
		weights[i] = c1 * exp(-(pow(x, 2.0) / c2));
		totalWeight += weights[i];
	}

	const double adjust = 1.0 / totalWeight;
	for(uint32_t i = 0; i < filter_len; i++)
	{
		weights[i] *= adjust;
	}

	m_weights.insert(std::make_pair(filter_len, weights));
}

double Average::update(const double &value)
{
	m_values.push_front(value);
	while(m_values.size() > m_queue_len)
	{
		m_values.pop_back();
	}

	if(m_values.size() >= MIN_LEN)
	{
		const size_t key = ((m_values.size() % 2) == 1) ? m_values.size() : (m_values.size() - 1);

		const std::unordered_map<size_t,std::vector<double>>::const_iterator weights = m_weights.find(key);
		if(weights == m_weights.end())
		{
			throw std::runtime_error("Weights for current size not found!");
		}

		std::deque<double>::const_iterator value_iter = m_values.cbegin();
		double accumulator = 0.0;
		for(std::vector<double>::const_iterator weights_iter = weights->second.cbegin(); weights_iter != weights->second.cend(); weights_iter++, value_iter++)
		{
			accumulator += (*value_iter) * (*weights_iter);
		}

		return accumulator;
	}

	return -1.0;
}
