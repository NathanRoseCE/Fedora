// Copyright 2017 Proyectos y Sistemas de Mantenimiento SL (eProsima).
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "Vector.h"
#include "Magnitude.h"

#include <uxr/client/client.h>
#include <ucdr/microcdr.h>
#include <uxr/client/core/type/xrce_types.h>

#include <stdio.h> //printf
#include <string.h> //strcmp
#include <stdlib.h> //atoi
#include <unistd.h>

#define STREAM_HISTORY  8
#define BUFFER_SIZE     UXR_CONFIG_UDP_TRANSPORT_MTU* STREAM_HISTORY

magnitude topic;

char activeAgentIp[16];
uint16_t activeAgentPort;
bool found = false;

bool on_agent_found(
        const TransportLocator* locator,
        void* args)
{
    (void) args;
    switch (locator->format) {
        case ADDRESS_FORMAT_MEDIUM: {
            uxrIpProtocol ip_protocol;
            uxr_locator_to_ip(locator, activeAgentIp, sizeof(activeAgentIp), &activeAgentPort, &ip_protocol);
            printf("Agent found => ip: %s, port: %d\n", activeAgentIp, activeAgentPort);
	    printf("Using this agent");
	    found=true;
            break;
        }
        case ADDRESS_FORMAT_LARGE: {
            char ip[46];
            uint16_t port;
            uxrIpProtocol ip_protocol;
            uxr_locator_to_ip(locator, ip, sizeof(ip), &port, &ip_protocol);
            printf("Agent found => ip: %s, port: %d\n", ip, port);
	    printf("Cannot handle this format");
            break;
        }
        default:
            break;
    }
    return false;
}

bool find_agent() {
  uxr_discovery_agents_default(10, 1000, on_agent_found, NULL);
  if(found){
    printf("FOUND AN AGENT!!!!\n");
    printf("IP Address: %s", activeAgentIp);
  }
  else {
    printf("Did not find an agent :(\n");
  }
  return found;
}
void on_topic(uxrSession* session,
	      uxrObjectId object_id,
	      uint16_t request_id,
	      uxrStreamId stream_id,
	      struct ucdrBuffer* ub,
	      uint16_t length,
	      void* args) {
  (void) session; (void) object_id; (void) request_id; (void) stream_id; (void) length;

  magnitude_deserialize_topic(ub, &topic);

  printf("Magnitude: %f", topic.val);
}

int main(int args,
	 char** argv) {
  printf("booting up client\n");
  
  if( !find_agent() ) {
    printf("Unable to find an agent, exiting\n");
    return 1;
  }
  char ip[20];
  char port[10];
  uint32_t max_topics = 10000;

  sprintf(port, "%d", activeAgentPort);
  // Transport
  uxrUDPTransport transport;
  if (!uxr_init_udp_transport(&transport, UXR_IPv4, activeAgentIp, port)) {
    printf("Error at create transport.\n");
    return 1;
  }

  // Session
  uxrSession session;
  uxr_init_session(&session, &transport.comm, 0xAAAABBBB);
  uxr_set_topic_callback(&session, on_topic, NULL);
  if (!uxr_create_session(&session)) {
    printf("Error at create session.\n");
    return 1;
  }

  // Streams
  uint8_t output_reliable_stream_buffer[BUFFER_SIZE];
  uxrStreamId reliable_out = uxr_create_output_reliable_stream(&session, output_reliable_stream_buffer,
							       BUFFER_SIZE, STREAM_HISTORY);

  uint8_t input_reliable_stream_buffer[BUFFER_SIZE];
  uxrStreamId reliable_in = uxr_create_input_reliable_stream(&session, input_reliable_stream_buffer,
							     BUFFER_SIZE, STREAM_HISTORY);

  // Create entities
  uxrObjectId participant_id = uxr_object_id(0x01, UXR_PARTICIPANT_ID);
  const char* participant_xml = "<dds>"
    "<participant>"
    "<rtps>"
    "<name>default_xrce_participant</name>"
    "</rtps>"
    "</participant>"
    "</dds>";
  uint16_t participant_req = uxr_buffer_create_participant_xml(&session, reliable_out, participant_id, 0,
							       participant_xml, UXR_REPLACE);

  uxrObjectId topic_id = uxr_object_id(0x01, UXR_TOPIC_ID);
  const char* topic_xml = "<dds>"
    "<topic>"
    "<name>vectorTopic</name>"
    "<dataType>Vector</dataType>"
    "</topic>"
    "</dds>";
  uint16_t topic_req = uxr_buffer_create_topic_xml(&session, reliable_out, topic_id, participant_id, topic_xml,
						   UXR_REPLACE);

  uxrObjectId publisher_id = uxr_object_id(0x01, UXR_PUBLISHER_ID);
  const char* publisher_xml = "";
  uint16_t publisher_req = uxr_buffer_create_publisher_xml(&session, reliable_out, publisher_id, participant_id,
							   publisher_xml, UXR_REPLACE);

  uxrObjectId datawriter_id = uxr_object_id(0x01, UXR_DATAWRITER_ID);
  const char* datawriter_xml = "<dds>"
    "<data_writer>"
    "<topic>"
    "<kind>NO_KEY</kind>"
    "<name>vectorTopic</name>"
    "<dataType>Vector</dataType>"
    "</topic>"
    "</data_writer>"
    "</dds>";
  uint16_t datawriter_req = uxr_buffer_create_datawriter_xml(&session, reliable_out, datawriter_id, publisher_id, datawriter_xml, UXR_REPLACE);

  //create subscriber to magnitude
  uxrObjectId subTopic_id = uxr_object_id(0x01, UXR_TOPIC_ID);
  const char* subTopic_xml = "<dds>"
    "<topic>"
    "<name>vectorMagnitude</name>"
    "<dataType>magnitude</dataType>"
    "</topic>"
    "</dds>";
  uint16_t subTopic_req = uxr_buffer_create_topic_xml(&session, reliable_out, subTopic_id, participant_id, subTopic_xml, UXR_REPLACE);

  uxrObjectId subscriber_id = uxr_object_id(0x01, UXR_SUBSCRIBER_ID);
  const char* subscriber_xml = "";
  uint16_t subscriber_req = uxr_buffer_create_subscriber_xml(&session, reliable_out, subscriber_id, participant_id, subscriber_xml, UXR_REPLACE);

  uxrObjectId sub_dataReader_id = uxr_object_id(0x01, UXR_DATAREADER_ID);
  const char* sub_datareader_xml = "<dds>"
    "<data_writer>"
    "<topic>"
    "<kind>NO_KEY</kind>"
    "<name>vectorMagnitude</name>"
    "<dataType>magnitude</dataType>"
    "</topic>"
    "</data_writer>"
    "</dds>";
  uint16_t sub_datareader_req = uxr_buffer_create_datareader_xml(&session, reliable_out, sub_dataReader_id, subscriber_id, sub_datareader_xml, UXR_REPLACE);

  uxrDeliveryControl delivery_control = {0};
  delivery_control.max_samples = UXR_MAX_SAMPLES_UNLIMITED;

  uint16_t read_data_req = uxr_buffer_request_data(&session, reliable_out, sub_dataReader_id, reliable_in, &delivery_control);
  
  // Send create entities message and wait its status
  uint8_t status[4];
  uint16_t requests[4] = {
    participant_req, topic_req, publisher_req, datawriter_req
  };
  if (!uxr_run_session_until_all_status(&session, 1000, requests, status, 4))
    {
      printf("Error at create entities: participant: %i topic: %i publisher: %i datawriter: %i\n", status[0],
	     status[1], status[2], status[3]);
      return 1;
    }

  // Write topics
  bool connected = true;
  uint32_t count = 0;
  printf("Init complete\n");
  float i = 0;
  while (connected && count < max_topics) {
    Vector topic = {
      3, {i,i+1,i+2}
    };

    ucdrBuffer ub;
    uint32_t topic_size = Vector_size_of_topic(&topic, 0);
    uxr_prepare_output_stream(&session, reliable_out, datawriter_id, &ub, topic_size);
    Vector_serialize_topic(&ub, &topic);

    printf("Sent topic: <%f, %f, %f>\n", topic.value[0], topic.value[1], topic.value[2]);
    connected = uxr_run_session_time(&session, 1000);
    if( !connected ) {
      printf("Agent Died\n");
      printf("Consider Making a new agent\n");
    }
    sleep(1);
    i++;
  }
  printf("Shutting down\n");
  // Delete resources
  uxr_delete_session(&session);
  uxr_close_udp_transport(&transport);

  return 0;
}
