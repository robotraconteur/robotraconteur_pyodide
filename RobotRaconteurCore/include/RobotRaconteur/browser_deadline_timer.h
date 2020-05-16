#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/system/error_code.hpp>
#include <boost/date_time.hpp>

#pragma once

class browser_deadline_timer : public boost::enable_shared_from_this<browser_deadline_timer>
{
    boost::function<void(const boost::system::error_code&)> handler;
    boost::posix_time::ptime expiration_time;
    
    std::map<void*,boost::weak_ptr<browser_deadline_timer> > timers;

    friend void browser_deadline_timer_handler(void* userData)
    {

    }

public:

    void expires_from_now(const boost::posix_time::time_duration& dur)
    {
        expiration_time = boost::posix_time::microsec_clock::universal_time() + dur; 
    }

    void async_wait(boost::function<void(const boost::system::error_code&)> handler)
    {
        this->handler = handler;
        timers.insert(std::make_pair(this,shared_from_this()));
        //TODO: Start timeout
    }

    void cancel()
    {
        //TODO: implement this
    }

};