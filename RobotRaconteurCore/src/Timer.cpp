// Copyright 2011-2019 Wason Technology, LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "RobotRaconteur/Timer.h"
#include "RobotRaconteur/RobotRaconteurNode.h"

#include <boost/asio/placeholders.hpp>

namespace RobotRaconteur
{	
	void WallTimer::timer_handler(const boost::system::error_code& ec)
	{
		TimerEvent ev;

		RR_SHARED_PTR<RobotRaconteurNode> n=node.lock();
		if (!n) throw InvalidOperationException("Node released");

		boost::function<void (const TimerEvent&)> h;

		{
		
			if (ec)
			{
				running=false;
			}
			ev.stopped=!running;
			ev.last_expected=last_time;
			ev.last_real=actual_last_time;
			ev.current_expected=last_time+period;
			ev.current_real=n->NowUTC();
			h=handler;

			if (oneshot)
			{
				handler.clear();
			} 

			if (oneshot)
			{
				running=false;
			}
		}

		

		try
		{
			if (h) h(ev);
		}
		catch (std::exception& exp)
		{
			n->HandleException(&exp);
		}
		if (!oneshot)
		{
			if (running)
			{
				last_time=ev.current_expected;
				actual_last_time=ev.current_real;
				
				while (last_time + period < actual_last_time)
				{
					last_time += period;
				}
				
				//timer->expires_at(last_time + period);				
				RobotRaconteurNode::SetTimeout(period.total_milliseconds(), boost::bind(&WallTimer::timer_handler,shared_from_this(),boost::asio::placeholders::error));
			}

		}
		else
		{
			running=false;
			RobotRaconteurNode::ClearTimeout(timer);
			timer = 0;
			
		}

	}

	WallTimer::WallTimer(const boost::posix_time::time_duration& period, boost::function<void (const TimerEvent&)> handler, bool oneshot, RR_SHARED_PTR<RobotRaconteurNode> node) 
	{
		this->period=period;
		this->oneshot=oneshot;
		this->handler=handler;
		running=false;
		if (!node) node=RobotRaconteurNode::sp();
		this->node=node;
		this->timer = 0;
	}

	void WallTimer::Start()
	{
		if (running) throw InvalidOperationException("Already running");

		if (!handler) throw InvalidOperationException("Timer has expired");

		
		RR_SHARED_PTR<RobotRaconteurNode> n=node.lock();
		if (!n) throw InvalidOperationException("Node released");

		start_time=n->NowUTC();
		last_time=start_time;
		actual_last_time=last_time;

		timer = 0;
		
		
		timer = RobotRaconteurNode::SetTimeout(period.total_milliseconds(), boost::bind(&WallTimer::timer_handler, shared_from_this(), boost::asio::placeholders::error));
		
		running=true;
	}

	void WallTimer::Stop()
	{
		if (!running) throw InvalidOperationException("Not running");

		try
		{
			if (timer != 0)
			{
				RobotRaconteurNode::ClearTimeout(timer);
			}
		}
		catch (std::exception&) {}		
		running=false;

		if (oneshot) handler.clear();
		
	}

	boost::posix_time::time_duration WallTimer::GetPeriod()
	{
		return this->period;
	}

	void WallTimer::SetPeriod(const boost::posix_time::time_duration& period)
	{
		this->period=period;
	}

	bool WallTimer::IsRunning()
	{
		return running;
	}

	void WallTimer::Clear()
	{
		handler.clear();
	}
}
