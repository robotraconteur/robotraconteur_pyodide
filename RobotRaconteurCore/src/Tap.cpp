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


// TODO: Implement taps in browser


#include "RobotRaconteur/Tap.h"

#include "RobotRaconteur/IOUtils.h"
#include "RobotRaconteur/RobotRaconteurNode.h"


namespace RobotRaconteur
{

    namespace detail
    {

        RR_INTRUSIVE_PTR<MessageEntry> RRLogRecordToMessageEntry(const RRLogRecord& record)
        {
            RR_INTRUSIVE_PTR<MessageEntry> ret = CreateMessageEntry(MessageEntryType_StreamOp, "logrecord");
            RR_SHARED_PTR<RobotRaconteurNode> node = record.Node.lock();
            if (!node)
            {
                ret->AddElement("node",stringToRRArray("unknown"));
            }
            else
            {
                NodeID id;
                if(node->TryGetNodeID(id))
                {
                    ret->AddElement("node",stringToRRArray(id.ToString()));
                }
                else
                {
                    ret->AddElement("node",stringToRRArray("unknown"));
                }
            }
            ret->AddElement("level",ScalarToRRArray<int32_t>(record.Level));
            ret->AddElement("component",ScalarToRRArray<int32_t>(record.Component));
            ret->AddElement("componentname",stringToRRArray(record.ComponentName));
            ret->AddElement("componentobjectid",stringToRRArray(record.ComponentObjectID));
            ret->AddElement("endpoint",ScalarToRRArray<int64_t>(record.Endpoint));
            ret->AddElement("servicepath",stringToRRArray(record.ServicePath));
            ret->AddElement("member",stringToRRArray(record.Member));
            ret->AddElement("message",stringToRRArray(record.Message));
            ret->AddElement("time",stringToRRArray(boost::posix_time::to_iso_extended_string(record.Time)));
            ret->AddElement("sourcefile",stringToRRArray(record.SourceFile));
            ret->AddElement("sourceline",ScalarToRRArray<uint32_t>(record.SourceLine));
            ret->AddElement("threadid",stringToRRArray(record.ThreadID));
            ret->AddElement("fiberid",stringToRRArray(record.FiberID));
            return ret;
        }

        RR_INTRUSIVE_PTR<Message> RRLogRecordToMessage(const RRLogRecord& record)
        {
            RR_INTRUSIVE_PTR<Message> ret = CreateMessage();
            ret->header = CreateMessageHeader();

            RR_SHARED_PTR<RobotRaconteurNode> node = record.Node.lock();
            if (node)
            {
                NodeID id;
                if(node->TryGetNodeID(id))
                {
                    ret->header->SenderNodeID = id;
                }

                std::string name;
                if (node->TryGetNodeName(name))
                {
                    ret->header->SenderNodeName = name;
                }
            }

            ret->entries.push_back(RRLogRecordToMessageEntry(record));
            return ret;
        }

    }

        
    void LocalMessageTap::Open()
    {
      
    }

    void LocalMessageTap::Close()
    {
       
    }

    void LocalMessageTap::RecordLogRecord(const RRLogRecord& log_record)
    {
        
    }

    void LocalMessageTap::RecordMessage(RR_INTRUSIVE_PTR<Message> message)
    {
        
    }
}
