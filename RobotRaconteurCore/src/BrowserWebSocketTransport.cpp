#include "RobotRaconteur/BrowserWebSocketTransport.h"
#include "BrowserWebSocketTransport_private.h"

#include <boost/algorithm/string.hpp>
#include <boost/shared_array.hpp>
#include <set>

#include <boost/range/adaptors.hpp>
#include <boost/range/algorithm.hpp>
#include <boost/foreach.hpp>

namespace RobotRaconteur
{
BrowserWebSocketTransport::BrowserWebSocketTransport(RR_SHARED_PTR<RobotRaconteurNode> node)
	: Transport(node)
{
	if (!node) throw InvalidArgumentException("Node cannot be null");

	default_connect_timeout=5000;
	default_receive_timeout=15000;
	this->node=node;
	this->heartbeat_period=10000;

#ifndef ROBOTRACONTEUR_DISABLE_MESSAGE3
	disable_message3 = false;
#else
	disable_message3 = true;
#endif
#ifndef ROBOTRACONTEUR_DISABLE_STRINGTABLE
	disable_string_table = false;
#else
	disable_string_table = true;
#endif
	disable_async_message_io = false;
	closed = false;

}

BrowserWebSocketTransport::~BrowserWebSocketTransport()
{

}

void BrowserWebSocketTransport::Close()
{
	{		
		if (closed) return;
		closed = true;
	}

    std::vector<RR_SHARED_PTR<ITransportConnection> > t;

	{
		boost::copy(TransportConnections | boost::adaptors::map_values, std::back_inserter(t));		
	}

	BOOST_FOREACH (RR_SHARED_PTR<ITransportConnection>& e, t)
	{
		try
		{
			e->Close();
        }
        catch (std::exception&)
        {

        }
    }
}

bool BrowserWebSocketTransport::IsServer() const
{
	return false;
}

bool BrowserWebSocketTransport::IsClient() const
{
	return true;
}

int32_t BrowserWebSocketTransport::GetDefaultReceiveTimeout()
{
	return default_receive_timeout;
}
void BrowserWebSocketTransport::SetDefaultReceiveTimeout(int32_t milliseconds)
{
	if (!(milliseconds>0)) throw InvalidArgumentException("Timeout must be positive");
	default_receive_timeout=milliseconds;
}
int32_t BrowserWebSocketTransport::GetDefaultConnectTimeout()
{
	return default_connect_timeout;
}
void BrowserWebSocketTransport::SetDefaultConnectTimeout(int32_t milliseconds)
{
	if (!(milliseconds>0)) throw InvalidArgumentException("Timeout must be positive");
	default_connect_timeout=milliseconds;
}

std::string BrowserWebSocketTransport::GetUrlSchemeString() const
{
	return "rr+ws";
}

void BrowserWebSocketTransport::AsyncSendMessage(RR_INTRUSIVE_PTR<Message> m, boost::function<void (RR_SHARED_PTR<RobotRaconteurException> )>& handler)
{
    RR_SHARED_PTR<ITransportConnection> t;
	
	{		
		RR_UNORDERED_MAP<uint32_t, RR_SHARED_PTR<ITransportConnection> >::iterator e1 = TransportConnections.find(m->header->SenderEndpoint);
		if (e1 == TransportConnections.end()) throw ConnectionException("Transport connection to remote host not found");
		t = e1->second;
	}

	t->AsyncSendMessage(m,handler);
}

void BrowserWebSocketTransport::AsyncCreateTransportConnection(const std::string& url, RR_SHARED_PTR<Endpoint> e, boost::function<void (RR_SHARED_PTR<ITransportConnection>, RR_SHARED_PTR<RobotRaconteurException> ) >& handler)
{
    RR_SHARED_PTR<BrowserWebSocketTransportConnection> t=RR_MAKE_SHARED<BrowserWebSocketTransportConnection>(shared_from_this(), url, e->GetLocalEndpoint());
    t->AsyncConnect(boost::bind(handler,t,_1));
}

void BrowserWebSocketTransport::CloseTransportConnection(RR_SHARED_PTR<Endpoint> e)
{
    RR_SHARED_PTR<ITransportConnection> t;
	{
		RR_UNORDERED_MAP<uint32_t, RR_SHARED_PTR<ITransportConnection> >::iterator e1 = TransportConnections.find(e->GetLocalEndpoint());
		if (e1 == TransportConnections.end()) return;
		t = e1->second;
		TransportConnections.erase(e1);
	}

	if (t)
	{
		try
		{
			t->Close();
		}
		catch (std::exception&) {}
    }
}

bool BrowserWebSocketTransport::CanConnectService(const std::string& url)
{
		
	if (url.size() < 6) return false;

	if (boost::starts_with(url,"tcp://"))
		return true;

	if (boost::starts_with(url, "rr+tcp://"))
		return true;

	if (boost::starts_with(url, "rr+ws://"))
		return true;

	if (boost::starts_with(url, "rr+wss://"))
		return true;

	return false;
}

void BrowserWebSocketTransport::CheckConnection(uint32_t endpoint)
{
    RR_SHARED_PTR<ITransportConnection> t;
	{		
		RR_UNORDERED_MAP<uint32_t, RR_SHARED_PTR<ITransportConnection> >::iterator e1 = TransportConnections.find(endpoint);
		if (e1 == TransportConnections.end()) throw ConnectionException("Transport connection to remote host not found");
		t = e1->second;
	}
	t->CheckConnection(endpoint);
}

void BrowserWebSocketTransport::PeriodicCleanupTask()
{
    for (RR_UNORDERED_MAP<uint32_t, RR_SHARED_PTR<ITransportConnection> >::iterator e=TransportConnections.begin(); e!=TransportConnections.end(); )
	{
		try
		{
			RR_SHARED_PTR<BrowserWebSocketTransportConnection> e2=rr_cast<BrowserWebSocketTransportConnection>(e->second);
			if (!e2->IsConnected())
			{
				e=TransportConnections.erase(e);
			}
			else
			{
				++e;
			}
		}
		catch (std::exception&) {}
	}
}

uint32_t BrowserWebSocketTransport::TransportCapability(const std::string& name)
{
    return 0;
}

void BrowserWebSocketTransport::MessageReceived(RR_INTRUSIVE_PTR<Message> m)
{
    GetNode()->MessageReceived(m);
}

int32_t BrowserWebSocketTransport::GetDefaultHeartbeatPeriod()
{
    return heartbeat_period;
}

void BrowserWebSocketTransport::SetDefaultHeartbeatPeriod(int32_t milliseconds)
{
    if (!(milliseconds>0)) throw InvalidArgumentException("Heartbeat must be positive");
	heartbeat_period=milliseconds;
}

bool BrowserWebSocketTransport::GetDisableMessage3()
{
    return disable_message3;
}

void BrowserWebSocketTransport::SetDisableMessage3(bool d)
{
    disable_message3 = d;
}

bool BrowserWebSocketTransport::GetDisableStringTable()
{
    return disable_string_table;
}   

void BrowserWebSocketTransport::SetDisableStringTable(bool d)
{
    disable_string_table = d;
}

bool BrowserWebSocketTransport::GetDisableAsyncMessageIO()
{
    return disable_async_message_io;
}

void BrowserWebSocketTransport::SetDisableAsyncMessageIO(bool d)
{
    disable_async_message_io = d;
}

void BrowserWebSocketTransport::register_transport(RR_SHARED_PTR<ITransportConnection> connection)
{
	TransportConnections.insert(std::make_pair(connection->GetLocalEndpoint(),connection));
}

void BrowserWebSocketTransport::erase_transport(RR_SHARED_PTR<ITransportConnection> connection)
{
	try
	{		
		if (TransportConnections.count(connection->GetLocalEndpoint())!=0)
		{
			if (TransportConnections.at(connection->GetLocalEndpoint())==connection)
			{
			
				TransportConnections.erase(connection->GetLocalEndpoint());
			}
		}
	}
	catch (std::exception&) {}
}

std::map<void*,RR_SHARED_PTR<BrowserWebSocketTransportConnection> > BrowserWebSocketTransportConnection::active_transports;

BrowserWebSocketTransportConnection::BrowserWebSocketTransportConnection(RR_SHARED_PTR<BrowserWebSocketTransport> parent, const std::string& url, uint32_t local_endpoint)
    : ASIOStreamBaseTransport(parent->GetNode())
{
    this->parent=parent;
    this->url=url;
    this->m_LocalEndpoint=local_endpoint;
}

EM_BOOL websocket_open_callback_func(int eventType, const EmscriptenWebSocketOpenEvent *websocketEvent, void *userData)
{
    std::map<void*,RR_SHARED_PTR<BrowserWebSocketTransportConnection> >::iterator e = BrowserWebSocketTransportConnection::active_transports.find(userData);
    if (e == BrowserWebSocketTransportConnection::active_transports.end())
    {
        return EM_TRUE;
    }

    RR_SHARED_PTR<BrowserWebSocketTransportConnection> t=e->second;

    try
    {
        t->AsyncConnect1();
    }
    catch (std::exception& exp)
    {
        RobotRaconteurNode::TryHandleException(t->node,&exp);
    }

    return EM_TRUE;
}

EM_BOOL websocket_message_callback_func(int eventType, const EmscriptenWebSocketMessageEvent *websocketEvent, void *userData)
{
    std::cout << "Got a message " << websocketEvent->numBytes << " isText: " << websocketEvent->isText << std::endl;

    std::map<void*,RR_SHARED_PTR<BrowserWebSocketTransportConnection> >::iterator e = BrowserWebSocketTransportConnection::active_transports.find(userData);
    if (e == BrowserWebSocketTransportConnection::active_transports.end())
    {
        return EM_TRUE;
    }

    RR_SHARED_PTR<BrowserWebSocketTransportConnection> t=e->second;

    if (websocketEvent->isText)
    {
        BrowserWebSocketTransportConnection::active_transports.erase(e);
        t->Close();
        return EM_TRUE;
    }

    t->websocket_message_received(websocketEvent->data, websocketEvent->numBytes);

    return EM_TRUE;
}

EM_BOOL websocket_error_callback_func(int eventType, const EmscriptenWebSocketErrorEvent *websocketEvent, void *userData)
{
    std::map<void*,RR_SHARED_PTR<BrowserWebSocketTransportConnection> >::iterator e = BrowserWebSocketTransportConnection::active_transports.find(userData);
    if (e == BrowserWebSocketTransportConnection::active_transports.end())
    {
        return EM_TRUE;
    }

    RR_SHARED_PTR<BrowserWebSocketTransportConnection> t=e->second;

    BrowserWebSocketTransportConnection::active_transports.erase(e);

    t->Close();

    return EM_TRUE;
}

EM_BOOL websocket_close_callback_func(int eventType, const EmscriptenWebSocketCloseEvent *websocketEvent, void *userData)
{
    std::map<void*,RR_SHARED_PTR<BrowserWebSocketTransportConnection> >::iterator e = BrowserWebSocketTransportConnection::active_transports.find(userData);
    if (e == BrowserWebSocketTransportConnection::active_transports.end())
    {
        return EM_TRUE;
    }

    RR_SHARED_PTR<BrowserWebSocketTransportConnection> t=e->second;

    BrowserWebSocketTransportConnection::active_transports.erase(e);

    t->Close();

    return EM_TRUE;
}

void BrowserWebSocketTransportConnection::AsyncConnect(boost::function<void (RR_SHARED_PTR<RobotRaconteurException>)> handler)
{
    if (emscripten_websocket_is_supported() != EM_TRUE)
    {
        throw ConnectionException("Browser does not support WebSockets!");
    }

    std::string ws_url;
    if(boost::starts_with(url,"tcp://"))
    {
        ws_url = boost::replace_first_copy(url,"tcp://","ws://");
    }
    else if(boost::starts_with(url,"rr+tcp://"))
    {
        ws_url = boost::replace_first_copy(url,"rr+tcp://","ws://");
    }
    else if(boost::starts_with(url,"rr+ws://"))
    {
        ws_url = boost::replace_first_copy(url,"rr+ws://","ws://");
    }
    else if(boost::starts_with(url,"rr+wss://"))
    {
        ws_url = boost::replace_first_copy(url,"rr+wss://","wss://");
    }
    else
    {
        throw ConnectionException("Invalid connection URL");
    }

    EmscriptenWebSocketCreateAttributes create_attributes;
    emscripten_websocket_init_create_attributes(&create_attributes);

    create_attributes.createOnMainThread=EM_TRUE;
    create_attributes.url=ws_url.c_str();
    const char* protocols[2];
    protocols[0] = "robotraconteur.robotraconteur.com";
    protocols[1] = NULL;
    create_attributes.protocols = protocols;

    socket = emscripten_websocket_new(&create_attributes);
    if (socket <= 0)
    {
        throw ConnectionException("Could not create WebSocket connection");
    }

    if((emscripten_websocket_set_onopen_callback(socket, this, &websocket_open_callback_func) < 0)
        || (emscripten_websocket_set_onerror_callback(socket, this, &websocket_error_callback_func) < 0)
        || (emscripten_websocket_set_onclose_callback(socket, this, &websocket_close_callback_func) < 0)
        || (emscripten_websocket_set_onmessage_callback(socket, this, &websocket_message_callback_func) < 0)
    
    )
    {
        throw ConnectionException("Could not configure WebSocket callbacks");
    }

    active_transports.insert(std::make_pair((void*)this,RR_STATIC_POINTER_CAST<BrowserWebSocketTransportConnection>(shared_from_this())));    

    std::cout << "Websocket connection started" << socket << std::endl;

    active_connect_handler = handler;
}

void BrowserWebSocketTransportConnection::AsyncConnect1()
{

    RR_SHARED_PTR<BrowserWebSocketTransport> parent1 = parent.lock();
    if (!parent1)
    {
        Close();
        return;
    }

    std::cout << "Websocket connected! " << socket << std::endl;

    boost::function<void (RR_SHARED_PTR<RobotRaconteurException>)> active_connect_handler1 = active_connect_handler;
    active_connect_handler.clear();

    try
    {
        std::string noden;

        ParseConnectionURLResult url_res = ParseConnectionURL(url);
            
        target_nodeid = url_res.nodeid;
        
        target_nodename = url_res.nodename;
        if (!(url_res.nodeid.IsAnyNode() && url_res.nodename != ""))
        {
            noden = url_res.nodeid.ToString();
        }
        else
        {
            noden = url_res.nodename;
        }

        parent1->register_transport(shared_from_this());

        std::cout << "AsyncAttachStream" << socket << std::endl;

        ASIOStreamBaseTransport::AsyncAttachStream(server,target_nodeid, target_nodename, active_connect_handler1);
    }
    catch (std::exception& exp)
    {
        RobotRaconteurNode::TryHandleException(node, &exp);
    }
}

void BrowserWebSocketTransportConnection::Close()
{
    std::cout << "BrowserWebSocketTransportConnection::Close" << std::endl;
    if (active_connect_handler)
    {
        std::cout << "Websocket connection failed" << socket << std::endl;
        boost::function<void (RR_SHARED_PTR<RobotRaconteurException>)> active_connect_handler1 = active_connect_handler;
        active_connect_handler.clear();
        try
        {
            active_connect_handler1(RR_MAKE_SHARED<ConnectionException>("WebSocket connection failed"));
        }
        catch (std::exception& exp)
        {
            RobotRaconteurNode::TryHandleException(node, &exp);
        }
        return;
    }

    if(closing) return;
	closing=true;

    emscripten_websocket_close(socket, 0, "transport closed");

    try
	{
		RR_SHARED_PTR<BrowserWebSocketTransport> p=parent.lock();
		if (p) p->erase_transport(RR_STATIC_POINTER_CAST<BrowserWebSocketTransportConnection>(shared_from_this()));
	}
	catch (std::exception&) {}

    ASIOStreamBaseTransport::Close();

}

uint32_t BrowserWebSocketTransportConnection::GetLocalEndpoint()
{
    return m_LocalEndpoint;
}

uint32_t BrowserWebSocketTransportConnection::GetRemoteEndpoint()
{
    return m_RemoteEndpoint;
}

void BrowserWebSocketTransportConnection::CheckConnection(uint32_t endpoint)
{
    if (endpoint!=m_LocalEndpoint || !connected) throw ConnectionException("Connection lost");
}

void BrowserWebSocketTransportConnection::async_write_some(const_buffers& b, boost::function<void (const boost::system::error_code& error, size_t bytes_transferred)>& handler)
{
    if (b.empty())
    {
        boost::system::error_code ec;
        RobotRaconteurNode::TryPostToThreadPool(node, boost::bind(handler,ec,0));
        return;
    }

    const size_t max_message_size=65536;

    const_buffers::value_type& b1 = b.front();
    size_t l = b1.size();
    if (l > max_message_size)
    {
        l = max_message_size;
    }

    emscripten_websocket_send_binary(socket, (void*)b1.data(), l);
    
    boost::system::error_code ec;
    RobotRaconteurNode::TryPostToThreadPool(node, boost::bind(handler,ec,l));
}

void BrowserWebSocketTransportConnection::async_read_some(mutable_buffers& b, boost::function<void (const boost::system::error_code& error, size_t bytes_transferred)>& handler) 
{
    while(!recv_buf.empty() && recv_buf.front().buffer.size() == 0)
    {
        recv_buf.pop_front();
    }

    if (recv_buf.empty())
    {
        active_read_handler = handler;
        active_read_buffers = b;
        return;
    }
    
    size_t l = boost::asio::buffer_copy(b, recv_buf.front().buffer);
    recv_buf.front().buffer += l;
    if (recv_buf.front().buffer.size() == 0)
    {
        recv_buf.pop_front();
    }

    boost::system::error_code ec;
    RobotRaconteurNode::TryPostToThreadPool(node, boost::bind(handler,ec,l));
}

size_t BrowserWebSocketTransportConnection::available()
{
    size_t l = 0;
    BOOST_FOREACH(std::list<recv_buf_entry>::value_type& e, recv_buf)
    {
        l += e.buffer.size();
    }

    return l;
}

void BrowserWebSocketTransportConnection::websocket_message_received(const void* data, size_t len)
{
    boost::asio::const_buffer buffer(data,len);    
    if (active_read_handler)
    {
        boost::function<void (const boost::system::error_code& error, size_t bytes_transferred)> active_read_handler1 = active_read_handler;
        active_read_handler.clear();
        size_t l = boost::asio::buffer_copy(active_read_buffers, buffer);
        buffer += l;
        boost::system::error_code ec;
        RobotRaconteurNode::TryPostToThreadPool(node, boost::bind(active_read_handler1,ec,l));
        if (buffer.size() == 0)
        {
            return;
        }
    }

    recv_buf_entry entry;
    entry.data_storage.reset(new uint8_t[buffer.size()]);    
    boost::asio::mutable_buffer data_storage_buf(entry.data_storage.get(), buffer.size());
    boost::asio::buffer_copy(data_storage_buf, buffer);
    entry.buffer = data_storage_buf;

    recv_buf.push_back(entry);
}

bool BrowserWebSocketTransportConnection::IsLargeTransferAuthorized()
{
    return true;
}

void BrowserWebSocketTransportConnection::MessageReceived(RR_INTRUSIVE_PTR<Message> m)
{
    GetNode()->MessageReceived(m);
}

}