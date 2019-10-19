#include "RobotRaconteur/NodeSetup.h"

namespace RobotRaconteur
{
	RobotRaconteurNodeSetup::RobotRaconteurNodeSetup(RR_SHARED_PTR<RobotRaconteurNode> node, const std::vector<RR_SHARED_PTR<ServiceFactory> > service_types, 
		const std::string& node_name, uint16_t tcp_port, uint32_t flags)
	{
		this->node = node;

		
		if (flags & RobotRaconteurNodeSetupFlags_ENABLE_TCP_TRANSPORT)
		{
			
		}
		
		BOOST_FOREACH(RR_SHARED_PTR<ServiceFactory> f, service_types)
		{
			node->RegisterServiceType(f);
		}

		if (flags & RobotRaconteurNodeSetupFlags_DISABLE_TIMEOUTS)
		{
			node->SetRequestTimeout(std::numeric_limits<uint32_t>::max());
			node->SetTransportInactivityTimeout(std::numeric_limits<uint32_t>::max());
			node->SetEndpointInactivityTimeout(std::numeric_limits<uint32_t>::max());			
		}

	}
	
	RobotRaconteurNodeSetup::~RobotRaconteurNodeSetup()
	{
		if (node)
		{
			node->Shutdown();
		}
	}

	ClientNodeSetup::ClientNodeSetup(RR_SHARED_PTR<RobotRaconteurNode> node, const std::vector<RR_SHARED_PTR<ServiceFactory> > service_types, 
		const std::string& node_name, uint32_t flags)
		: RobotRaconteurNodeSetup(node, service_types, node_name, 0, flags)
	{

	}

	ClientNodeSetup::ClientNodeSetup(const std::vector<RR_SHARED_PTR<ServiceFactory> > service_types, const std::string& node_name, uint32_t flags)
		: RobotRaconteurNodeSetup(RobotRaconteurNode::sp(), service_types, node_name, 0, flags)
	{

	}
}