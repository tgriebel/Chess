/*
* MIT License
*
* Copyright( c ) 2022-2023 Thomas Griebel
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this softwareand associated documentation files( the "Software" ), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and /or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions :
*
* The above copyright noticeand this permission notice shall be included in all
* copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*/

#pragma once

#include <string>
#include <chrono>

enum class timerPrecision_t : uint32_t
{
	NANOSECOND,
	MICROSECOND,
	MILLISECOND,
	SECOND,
};


class Timer
{
protected:
	std::chrono::nanoseconds	m_startTime;
	std::chrono::nanoseconds	m_endTime;
	std::string					m_label;
	timerPrecision_t			m_precision;

	uint64_t Convert( const std::chrono::nanoseconds& start, const std::chrono::nanoseconds& end ) const;

public:

	Timer()
	{
		m_label = "";
		m_precision = timerPrecision_t::MILLISECOND;
		Start();
	}

	Timer( std::string label, const timerPrecision_t precision = timerPrecision_t::MILLISECOND )
	{
		m_label = label;
		m_precision = precision;
		Start();
	}

	void				Start();
	void				Stop();
	void				SetPrecision( const timerPrecision_t precision );
	
	[[nodiscard]]
	timerPrecision_t	GetPrecision() const;

	[[nodiscard]]
	std::string			GetLabel() const;

	[[nodiscard]]
	uint64_t			GetElapsed() const;

	[[nodiscard]]
	uint64_t			GetCurrentElapsed() const;
};


typedef void( *scopedTimerLogCallback_t )( const Timer* );

class ScopedLogTimer : public Timer
{
private:
	scopedTimerLogCallback_t m_callback;

public:
	ScopedLogTimer( std::string label, const timerPrecision_t precision = timerPrecision_t::MILLISECOND, scopedTimerLogCallback_t callback = nullptr )
	{
		m_label = label;
		m_callback = callback;
		m_precision = precision;
		Start();
	}

	~ScopedLogTimer()
	{
		if( m_callback != nullptr ) {
			(*m_callback )( this );
		}
	}
};

void TimerPrint( const Timer* timer );

#define SCOPED_TIMER_PRINT( label ) ScopedLogTimer scopedTimer_##label( #label, timerPrecision_t::MILLISECOND, &TimerPrint );