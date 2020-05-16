#include "RobotRaconteur/NodeSetup.h"

namespace RobotRaconteur
{
	RobotRaconteurNodeSetup::RobotRaconteurNodeSetup(RR_SHARED_PTR<RobotRaconteurNode> node, const std::vector<RR_SHARED_PTR<ServiceFactory> > service_types, 
		boost::string_ref node_name, uint16_t tcp_port, uint32_t flags)
	{

		this->node = node;

		node->SetLogLevelFromEnvVariable();

		ROBOTRACONTEUR_LOG_INFO_COMPONENT(node, NodeSetup, -1, "Setting up RobotRaconteurNode version " << node->GetRobotRaconteurVersion() 
			<< " with NodeName: \"" << node_name << "\" TCP port: " << tcp_port << " flags: 0x" << std::hex << flags);

		BOOST_FOREACH(RR_SHARED_PTR<ServiceFactory> f, service_types)
		{
			node->RegisterServiceType(f);
		}

		if (flags & RobotRaconteurNodeSetupFlags_ENABLE_TCP_TRANSPORT)
		{
			browser_websocket_transport = RR_MAKE_SHARED<BrowserWebSocketTransport>(node);
			node->RegisterTransport(browser_websocket_transport);
		}
		
		if (flags & RobotRaconteurNodeSetupFlags_DISABLE_TIMEOUTS)
		{
			node->SetRequestTimeout(std::numeric_limits<uint32_t>::max());
			node->SetTransportInactivityTimeout(std::numeric_limits<uint32_t>::max());
			node->SetEndpointInactivityTimeout(std::numeric_limits<uint32_t>::max());			
		}

		ROBOTRACONTEUR_LOG_TRACE_COMPONENT(node, NodeSetup, -1, "Node setup complete");

	}

	RR_SHARED_PTR<BrowserWebSocketTransport> RobotRaconteurNodeSetup::GetBrowserWebSocketTransport()
	{
		return browser_websocket_transport;
	}
	
	RobotRaconteurNodeSetup::~RobotRaconteurNodeSetup()
	{
		if (node)
		{
			
				node->Shutdown();
			
		}
	}

	ClientNodeSetup::ClientNodeSetup(RR_SHARED_PTR<RobotRaconteurNode> node, const std::vector<RR_SHARED_PTR<ServiceFactory> > service_types, 
		boost::string_ref node_name, uint32_t flags)
		: RobotRaconteurNodeSetup(node, service_types, node_name, 0, flags)
	{

	}

	ClientNodeSetup::ClientNodeSetup(const std::vector<RR_SHARED_PTR<ServiceFactory> > service_types, boost::string_ref node_name, uint32_t flags)
		: RobotRaconteurNodeSetup(RobotRaconteurNode::sp(), service_types, node_name, 0, flags)
	{

	}
}