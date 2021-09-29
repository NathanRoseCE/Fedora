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

#include <uxr/agent/AgentInstance.hpp>
#include <future>
#include "Broker.hpp"
extern "C" {
#include "Vector.h"
#include "Magnitude.h"

#include <uxr/client/client.h>
#define STREAM_HISTORY  8
#define BUFFER_SIZE     UXR_CONFIG_UDP_TRANSPORT_MTU* STREAM_HISTORY

  magnitude topic;

  char activeAgentIp[16];
  uint16_t activeAgentPort;
  bool found = false;
};

/*
void on_topic(struct ucdrBuffer* ub) {
  (void) session; (void) object_id; (void) request_id; (void) stream_id; (void) length;
  magnitude_deserialize_topic(ub, &topic);
  printf("Magnitude: %f", topic.val);
}
*/


int main(int args, char** argv) {
  sleep(5);
  /*
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
  */
  const char* participant_xml = "<dds>"
    "<participant>"
    "<rtps>"
    "<name>default_xrce_participant</name>"
    "</rtps>"
    "</participant>"
    "</dds>";
  uint8_t outBuffer[BUFFER_SIZE];
  uint8_t inBuffer[BUFFER_SIZE];
  Broker broker(false, outBuffer, BUFFER_SIZE, inBuffer, BUFFER_SIZE, participant_xml);
  broker.initialize();

  const char* topic_xml = "<dds>"
    "<topic>"
    "<name>vectorTopic</name>"
    "<dataType>Vector</dataType>"
    "</topic>"
    "</dds>";
  const char* publisher_xml = "";
  const char* datawriter_xml = "<dds>"
    "<data_writer>"
    "<topic>"
    "<kind>NO_KEY</kind>"
    "<name>vectorTopic</name>"
    "<dataType>Vector</dataType>"
    "</topic>"
    "</data_writer>"
    "</dds>";
  uint16_t id = broker.initPublisher(topic_xml, publisher_xml, datawriter_xml);

  // Write topics
  uint32_t count = 0;
  float i = 0;
  int max_topics =0;
  while (count < max_topics++) {
    Vector topic = {
      3, {i,i+1,i+2}
    };

    ucdrBuffer ub;
    uint32_t topic_size = Vector_size_of_topic(&topic, 0);
    Vector_serialize_topic(&ub, &topic);
    
    broker.prepPublish(id, &ub, topic_size);
    broker.runSession(1000);
    sleep(1);
    
    i++;
  }
  std::cout << "Shutting down" << std::endl;
  broker.close();
  return 0;
}

