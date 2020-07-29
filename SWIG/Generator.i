// Copyright 2011-2020 Wason Technology, LLC
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

%shared_ptr(RobotRaconteur::WrappedGeneratorClient);

%feature("director") RobotRaconteur::AsyncGeneratorReturnRequestDirector;
%feature("director") RobotRaconteur::WrappedGeneratorServerDirector;
%feature("director") RobotRaconteur::AsyncGeneratorClientReturnDirector;

namespace RobotRaconteur
{
	%nodefaultctor WrappedGeneratorClient;
	class WrappedGeneratorClient
	{
	public:		

	
		
		virtual void AsyncNext(boost::intrusive_ptr<MessageElement> v, int32_t timeout, AsyncRequestDirector* handler, int32_t id);

		
		virtual void AsyncAbort(int32_t timeout, AsyncVoidReturnDirector* handler, int32_t id);

		
		virtual void AsyncClose(int32_t timeout, AsyncVoidReturnDirector* handler, int32_t id);

		
		
	
	};

	class AsyncGeneratorClientReturnDirector
	{
	public:
		virtual ~AsyncGeneratorClientReturnDirector() {}
		virtual void handler(boost::shared_ptr<WrappedGeneratorClient> ret, HandlerErrorInfo& error);
	};
}