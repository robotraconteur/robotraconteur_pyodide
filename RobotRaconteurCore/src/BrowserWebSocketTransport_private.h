#include "RobotRaconteur/BrowserWebSocketTransport.h"
#include "RobotRaconteur/ASIOStreamBaseTransport.h"
#include <list>

#include <emscripten/websocket.h>

#pragma once

namespace RobotRaconteur
{
    class BrowserWebSocketTransportConnection : public detail::ASIOStreamBaseTransport
	{
        friend EM_BOOL websocket_open_callback_func(int eventType, const EmscriptenWebSocketOpenEvent *websocketEvent, void *userData);

        friend EM_BOOL websocket_message_callback_func(int eventType, const EmscriptenWebSocketMessageEvent *websocketEvent, void *userData);

        friend EM_BOOL websocket_error_callback_func(int eventType, const EmscriptenWebSocketErrorEvent *websocketEvent, void *userData);

        friend EM_BOOL websocket_close_callback_func(int eventType, const EmscriptenWebSocketCloseEvent *websocketEvent, void *userData);

	public:

        friend class BrowserWebSocketTransport;
        
    protected:
        static std::map<void*,RR_SHARED_PTR<BrowserWebSocketTransportConnection> > active_transports;

        EMSCRIPTEN_WEBSOCKET_T socket;

        std::string url;

		RR_WEAK_PTR<BrowserWebSocketTransport> parent;

		uint32_t m_RemoteEndpoint;
		uint32_t m_LocalEndpoint;

        bool closing;

        struct recv_buf_entry
        {
            boost::asio::const_buffer buffer;
            boost::shared_array<uint8_t> data_storage;            
        };

        std::list<recv_buf_entry> recv_buf;

        boost::function<void (const boost::system::error_code& error, size_t bytes_transferred)> active_read_handler;
        mutable_buffers active_read_buffers;
        
        boost::function<void (RR_SHARED_PTR<RobotRaconteurException>)> active_connect_handler;

        void websocket_message_received(const void* data, size_t len);

    public:

        BrowserWebSocketTransportConnection(RR_SHARED_PTR<BrowserWebSocketTransport> parent, const std::string& url, uint32_t local_endpoint);

        virtual void AsyncConnect(boost::function<void (RR_SHARED_PTR<RobotRaconteurException>)> err);

        virtual void AsyncConnect1();        

        virtual void Close();

        virtual  uint32_t GetLocalEndpoint() ;

		virtual  uint32_t GetRemoteEndpoint() ;

		virtual void CheckConnection(uint32_t endpoint);

        virtual void async_write_some(const_buffers& b, boost::function<void (const boost::system::error_code& error, size_t bytes_transferred)>& handler);

		virtual void async_read_some(mutable_buffers& b, boost::function<void (const boost::system::error_code& error, size_t bytes_transferred)>& handler) ;
		
		virtual size_t available();

		virtual bool IsLargeTransferAuthorized();

        virtual void MessageReceived(RR_INTRUSIVE_PTR<Message> m);

        virtual RR_SHARED_PTR<Transport> GetTransport();

    };
}