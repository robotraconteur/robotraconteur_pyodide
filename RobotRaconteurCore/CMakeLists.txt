
if(CMAKE_COMPILER_IS_GNUCXX)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -fPIC" )
endif()

IF (MSVC)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4251 /wd4275")
ENDIF()

set(RobotRaconteurCore_src
	src/DataTypes.cpp 
	src/Endpoint.cpp 
	src/Error.cpp
	src/IOUtils.cpp
	src/Message.cpp
	src/NodeID.cpp
	src/PipeMember.cpp
	src/PipeMember_private.h
	src/RobotRaconteurNode.cpp
	src/RobotRaconteurNode_connector.cpp
	src/RobotRaconteurNode_connector_private.h
	src/RobotRaconteurServiceIndex_stubskel.cpp
	src/Client.cpp
	src/ServiceStructure.cpp	
	src/ServiceDefinition.cpp
	src/ServiceFactory.cpp	
	src/Transport.cpp
	src/WireMember.cpp
	src/WireMember_private.h
	src/ASIOStreamBaseTransport.h
	src/ASIOStreamBaseTransport.cpp
	src/Timer.cpp
	src/AsyncUtils.cpp	
	src/AsyncMessageReader.cpp
	src/AsyncMessageReader.h
	src/AsyncMessageWriter.cpp
	src/AsyncMessageWriter.h
	src/AsyncMessageIO.cpp
	src/Discovery.cpp
	src/Discovery_private.h
	src/Subscription.cpp
	src/Subscription_private.h
	src/Generator.cpp
	src/NodeSetup.cpp
	src/BrowserWebSocketTransport.cpp
	src/BrowserWebSocketTransport_private.h 
)

set(RobotRaconteurCore_header
	include/RobotRaconteur/RobotRaconteurConfig.h 
	include/RobotRaconteur/CallbackMember.h 
	include/RobotRaconteur/DataTypes.h 
	include/RobotRaconteur/Endpoint.h 
	include/RobotRaconteur/Error.h
	include/RobotRaconteur/ErrorUtil.h
	include/RobotRaconteur/IOUtils.h
	include/RobotRaconteur/Message.h
	include/RobotRaconteur/NodeID.h
	include/RobotRaconteur/PipeMember.h
	include/RobotRaconteur/RobotRaconteurNode.h
	include/RobotRaconteur/RobotRaconteurServiceIndex.h
	include/RobotRaconteur/RobotRaconteurServiceIndex_stubskel.h
	include/RobotRaconteur/Client.h	
	include/RobotRaconteur/ServiceStructure.h	
	include/RobotRaconteur/ServiceDefinition.h
	include/RobotRaconteur/ServiceFactory.h	
	include/RobotRaconteur/Transport.h
	include/RobotRaconteur/WireMember.h
	include/RobotRaconteur.h
	include/RobotRaconteur/RobotRaconteurConstants.h
	include/RobotRaconteur/AsyncUtils.h
	include/RobotRaconteur/Timer.h	
	include/RobotRaconteur/AsyncMessageIO.h
	include/RobotRaconteur/Discovery.h 
	include/RobotRaconteur/Subscription.h
	include/RobotRaconteur/Generator.h
	include/RobotRaconteur/NodeSetup.h
	include/RobotRaconteur/BrowserWebSocketTransport.h
)

link_directories("${Boost_LIBRARY_DIRS}")
include_directories("${CMAKE_SOURCE_DIR}/RobotRaconteurCore/include" ${Boost_INCLUDE_DIRS})


add_library(RobotRaconteurCore STATIC ${RobotRaconteurCore_src} ${RobotRaconteurCore_header} )
SET_TARGET_PROPERTIES(RobotRaconteurCore PROPERTIES PREFIX lib)

RRSetTargetDirs(RobotRaconteurCore "bin" "lib")

SET_TARGET_PROPERTIES(RobotRaconteurCore PROPERTIES LINKER_LANGUAGE CXX)
