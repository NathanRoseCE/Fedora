#include "Vector.h"
#include "Magnitude.h"


#include <uxr/client/client.h>
#include <ucdr/microcdr.h>

#include <stdio.h> //printf
#include <string.h> //strcmp
#include <stdlib.h> //atoi
#include <unistd.h>
#include <pthread.h>

#define AGENT_IP_ADDR ""
#define AGENT_PORT ""


#define PUBLISHER_STREAM_HISTORY  8
#define PUBLISHER_BUFFER_SIZE     UXR_CONFIG_UDP_TRANSPORT_MTU* STREAM_HISTORY


pthread_t startupPublisher() {
  pthread_t id;
  pthread_create(&id, NULL, &publisher, NULL);
  return id;
		 
}
void *publisher(void *vargp) {
  uxrUDPTransport transport;
  if( !uxr_init_udp_transport(&transport, UXR_IPv4, AGENT_IP_ADDR, AGENT_PORT)) {
    printf("Error creating Publisher transport\n");
    return 1;
  }
  uxrSession session;
  uxr_init_session(&session, &transport.comm, 0xAAAABBBB);
  if( !uxr_create_session(&session)) {
    printf("Error creating Publisher session\n");
    return 1;
  }

  uint8_t output_reliable_stream_buffer[PUBLISHER_BUFFER_SIZE];
  uxrStreamId reliableOut = uxr_create_output_reliable_stream(&session, output_reliable_stream_buffer, PUBLISHER_BUFFER_SIZE, PUBLISHER_STREAM_HISTORY);

  uint8_t input_reliable_stream_buffer[PUBLISHER_BUFFER_SIZE];
  uxr_Create_input_reliable_stream(&session, input_reliable_stream_buffer, PUBLISHER_BUFFER_SIZE, PUBLISHER_STREAM_HISTORY);

  uxrObjectId participant_id = uxr_object_id(0x02, UXR_PARTICIPANT_ID);
  const char* participant_xml = "<dds>"
    "<participant>"
    "<rtps>"
    "<name>default_xrce_participant</name>"
    "</rtps>"
    "</participant>"
    "</dds>";
  uint16_t participant_req = uxr_buffer_create_participant_xml(&session, reliable_out, participant_id, 0, participant_xml, UXR_REPLACE);

  uxrObjectId topic_id = uxr_object_id(0x02, UXR_TOPIC_ID);
  const char* topic_xml = "<dds>"
    "<topic>"
    "<name>vectorMagnitude</name>"
    "<dataType>Magnitude</datatype>"
    "</topic>"
    "</dds>";
  uint16_t topic_req = uxr_buffer_create_topic_xml(&session, reliable_out, topic_id, participant_id, topic_xml, UXR_REPLACE);
  uxrObjectId publisher_id = uxr_object_id(0x02, UXR_PUBLISHER_ID);
  const char* publisher_xml = "";
  uint16_t publisher_req = uxr_buffer_create_publisher_xml(&session, reliable_out, publisher_id, participant_id, publisher_xml, UXR_REPLACE);

  //dataWriter
  uxrObjectId datawriter_id = uxr_object_id(0x01, UXR_DATAWRITER_ID);
  const char* datawriter_xml = "<dds>"
    "<data_writer>"
    "<topic>"
    "<kind>NO_KEY</kind>"
    "<name>vectorMagnitude</name>"
    "<dataType>Magnitude</dataType>"
    "</topic>"
    "</data_writer>"
    "</dds>";
  uint16_t datawriter_req = uxr_buffer_create_datawriter_xml(&session, reliable_out, datawriter_id, publisher_id,
							     datawriter_xml, UXR_REPLACE);

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
    sleep(1);
    i++;
  }
  printf("Shutting down\n");
  // Delete resources
  uxr_delete_session(&session);
  uxr_close_udp_transport(&transport);


  
    
      
  return NULL;
}


