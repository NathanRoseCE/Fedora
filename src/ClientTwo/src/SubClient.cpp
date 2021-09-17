
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

#include <stdio.h> //printf
#include <string.h> //strcmp
#include <stdlib.h> //atoi

#include <math.h>

#define STREAM_HISTORY  8
#define BUFFER_SIZE     UXR_CONFIG_UDP_TRANSPORT_MTU* STREAM_HISTORY

#define MAGN_STREAM_HISTORY  8
#define MAGN_BUFFER_SIZE     UXR_CONFIG_UDP_TRANSPORT_MTU* STREAM_HISTORY

Vector topic;

void on_topic(
	      uxrSession* session,
	      uxrObjectId object_id,
	      uint16_t request_id,
	      uxrStreamId stream_id,
	      struct ucdrBuffer* ub,
	      uint16_t length,
	      void* args) {
  (void) session; (void) object_id; (void) request_id; (void) stream_id; (void) length;

  Vector_deserialize_topic(ub, &topic);

  printf("Recievedtopic: <%f, %f, %f>\n", topic.value[0], topic.value[1], topic.value[2]);

  uint32_t* count_ptr = (uint32_t*) args;
  (*count_ptr)++;
}

int main(
	 int args,
	 char** argv)
{
  printf("Booting up\n");
  // CLI
  if (3 > args || 0 == atoi(argv[2]))
    {
      printf("usage: program [-h | --help] | ip port [<max_topics>]\n");
      return 0;
    }

  char* ip = argv[1];
  char* port =  argv[2];
  uint32_t max_topics = (args == 4) ? (uint32_t)atoi(argv[3]) : UINT32_MAX;

  // State
  uint32_t count = 0;

  // Transport
  uxrUDPTransport transport;
  if (!uxr_init_udp_transport(&transport, UXR_IPv4, ip, port))
    {
      printf("Error at create transport.\n");
      return 1;
    }

  // Session
  uxrSession session;
  uxr_init_session(&session, &transport.comm, 0xCCCCDDDD);
  uxr_set_topic_callback(&session, on_topic, &count);
  if (!uxr_create_session(&session))
    {
      printf("Error at create session.\n");
      return 1;
    }

  // Streams
  uint8_t sub_output_reliable_stream_buffer[BUFFER_SIZE];
  uxrStreamId sub_reliable_out = uxr_create_output_reliable_stream(&session, sub_output_reliable_stream_buffer, BUFFER_SIZE, STREAM_HISTORY);

  uint8_t sub_input_reliable_stream_buffer[BUFFER_SIZE];
  uxrStreamId sub_reliable_in = uxr_create_input_reliable_stream(&session, sub_input_reliable_stream_buffer, BUFFER_SIZE, STREAM_HISTORY);

  // Create entities
  uxrObjectId sub_participant_id = uxr_object_id(0x01, UXR_PARTICIPANT_ID);
  const char* sub_participant_xml = "<dds>"
    "<participant>"
    "<rtps>"
    "<name>default_xrce_participant</name>"
    "</rtps>"
    "</participant>"
    "</dds>";
  uint16_t sub_participant_req = uxr_buffer_create_participant_xml(&session, sub_reliable_out, sub_participant_id, 0, sub_participant_xml, UXR_REPLACE);

  uxrObjectId sub_topic_id = uxr_object_id(0x01, UXR_TOPIC_ID);
  const char* sub_topic_xml = "<dds>"
    "<topic>"
    "<name>vectorTopic</name>"
    "<dataType>Vector</dataType>"
    "</topic>"
    "</dds>";
  uint16_t sub_topic_req = uxr_buffer_create_topic_xml(&session, sub_reliable_out, sub_topic_id, sub_participant_id, sub_topic_xml, UXR_REPLACE);

  uxrObjectId subscriber_id = uxr_object_id(0x01, UXR_SUBSCRIBER_ID);
  const char* subscriber_xml = "";
  uint16_t subscriber_req = uxr_buffer_create_subscriber_xml(&session, sub_reliable_out, subscriber_id, sub_participant_id, subscriber_xml, UXR_REPLACE);

  uxrObjectId sub_datareader_id = uxr_object_id(0x01, UXR_DATAREADER_ID);
  const char* sub_datareader_xml = "<dds>"
    "<data_reader>"
    "<topic>"
    "<kind>NO_KEY</kind>"
    "<name>vectorTopic</name>"
    "<dataType>Vector</dataType>"
    "</topic>"
    "</data_reader>"
    "</dds>";
  uint16_t sub_datareader_req = uxr_buffer_create_datareader_xml(&session, sub_reliable_out, sub_datareader_id, subscriber_id, sub_datareader_xml, UXR_REPLACE);
    

  // Request topics
  uxrDeliveryControl delivery_control = {
    0
  };
  delivery_control.max_samples = UXR_MAX_SAMPLES_UNLIMITED;
  uint16_t read_data_req = uxr_buffer_request_data(&session, sub_reliable_out, sub_datareader_id, sub_reliable_in, &delivery_control);

  printf("Subscriber registered\n");
  printf("Attemption to register publisher\n");

  
  // set up the publisher
  
  // Streams
  /*
  uint8_t pub_output_reliable_stream_buffer[BUFFER_SIZE];
  uxrStreamId pub_reliable_out = uxr_create_output_best_effort_stream(&session, pub_output_reliable_stream_buffer, MAGN_BUFFER_SIZE);
  
  uint8_t pub_input_reliable_stream_buffer[BUFFER_SIZE];
  uxrStreamId pub_reliable_in = uxr_create_input_best_effort_stream(&session);
  // Create entities
  uxrObjectId pub_participant_id = uxr_object_id(0x01, UXR_PARTICIPANT_ID);
  const char* pub_participant_xml = "<dds>"
    "<participant>"
    "<rtps>"
    "<name>default_xrce_participant_two</name>"
    "</rtps>"
    "</participant>"
    "</dds>";
  uint16_t pub_participant_req = uxr_buffer_create_participant_xml(&session, pub_reliable_out, pub_participant_id, 0, pub_participant_xml, UXR_REPLACE);
  */
  
  uxrObjectId magn_topic_id = uxr_object_id(0x01, UXR_TOPIC_ID);
  const char* magn_topic_xml = "<dds>"
    "<topic>"
    "<name>vectorMagnitude</name>"
    "<dataType>magnitude</dataType>"
    "</topic>"
    "</dds>";
  uint16_t magn_topic_req = uxr_buffer_create_topic_xml(&session, sub_reliable_out, magn_topic_id, sub_participant_id, magn_topic_xml, UXR_REPLACE);
  uxrObjectId magn_publisher_id = uxr_object_id(0x01, UXR_PUBLISHER_ID);
  const char* magn_publisher_xml = "";
  uint16_t magn_publisher_req = uxr_buffer_create_publisher_xml(&session, sub_reliable_out, magn_publisher_id, sub_participant_id, magn_publisher_xml, UXR_REPLACE);
  
  uxrObjectId magn_datawriter_id = uxr_object_id(0x01, UXR_DATAWRITER_ID);
  const char* magn_datawriter_xml = "<dds>"
    "<data_writer>"
    "<topic>"
    "<kind>NO_KEY</kind>"
    "<name>vectorMagnitude</name>"
    "<dataType>magnitude</dataType>"
    "</topic>"
    "</data_writer>"
    "</dds>";
  uint16_t magn_datawriter_req = uxr_buffer_create_datawriter_xml(&session, sub_reliable_out, magn_datawriter_id, magn_publisher_id, magn_datawriter_xml, UXR_REPLACE);
  // Send create entities message and wait its status
  uint8_t status[7];
  uint16_t requests[7] = {
    sub_participant_req, sub_topic_req, subscriber_req, sub_datareader_req,
    magn_topic_req, magn_publisher_req, magn_datawriter_req
  };
  if (!uxr_run_session_until_all_status(&session, 1000, requests, status, 7)) {
    printf("Error at create entities: topic: %i, %i, %i, %i, %i, %i, %i", requests[0], requests[1], requests[2], requests[3], requests[4], requests[5], requests[6]);
    return 1;
  }
    
    
  // Iterate
  bool connected = true;
  while (connected && count < max_topics){
    printf("iteration\n");
    uint8_t read_data_status;
    connected = uxr_run_session_until_all_status(&session, UXR_TIMEOUT_INF, &read_data_req, &read_data_status, 1);
    magnitude pubTopic = {
      sqrtf( (topic.value[0] * topic.value[0]) +
    	     (topic.value[1] * topic.value[1]) +
	     (topic.value[2] * topic.value[2]) )
    };
    ucdrBuffer outBuf;
    uint32_t topic_size = magnitude_size_of_topic(&pubTopic, 0);
    uxr_prepare_output_stream(&session, sub_reliable_out, magn_datawriter_id, &outBuf, topic_size);
    magnitude_serialize_topic(&outBuf, &pubTopic);
    connected &= uxr_run_session_time(&session, 1000);
    printf("Sent: %f\n", pubTopic.val);
  }

  // Delete resources
  uxr_delete_session(&session);
  uxr_close_udp_transport(&transport);

  return 0;
}