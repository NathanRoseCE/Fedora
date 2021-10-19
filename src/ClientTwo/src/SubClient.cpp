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


#define STREAM_HISTORY  8
#define BUFFER_SIZE     100* STREAM_HISTORY

extern "C" {
#include "Vector.h"
#include "Magnitude.h"
};
#include <math.h>
#include <unistd.h>
#include <iostream>
#include <uxr/client/client.h>
#include <memory>
#include "Fedora/Broker.hpp"
Vector topic;

void on_topic(struct ucdrBuffer* ub) {
  
  Vector_deserialize_topic(ub, &topic);
  std::cout <<"Recievedtopic: <" << topic.value[0] <<","<< topic.value[1]  <<","<< topic.value[2] << ">" << std::endl;
}
int main(int args,
	 char** argv) {
  std::cout << "Booting up" << std::endl;
  uint8_t outBuffer[BUFFER_SIZE];
  uint8_t inBuffer[BUFFER_SIZE];
  const char* participant_xml = "<dds>"
    "<participant>"
    "<rtps>"
    "<name>default_xrce_participant</name>"
    "</rtps>"
    "</participant>"
    "</dds>";

  const char* sub_topic_xml = "<dds>"
    "<topic>"
    "<name>vectorTopic</name>"
    "<dataType>Vector</dataType>"
    "</topic>"
    "</dds>";
  const char* subscriber_xml = "";
  const char* sub_datareader_xml = "<dds>"
    "<data_reader>"
    "<topic>"
    "<kind>NO_KEY</kind>"
    "<name>vectorTopic</name>"
    "<dataType>Vector</dataType>"
    "</topic>"
    "</data_reader>"
    "</dds>";
  
  
  // set up the publisher
  const char* magn_topic_xml = "<dds>"
    "<topic>"
    "<name>vectorMagnitude</name>"
    "<dataType>magnitude</dataType>"
    "</topic>"
    "</dds>";
  const char* magn_publisher_xml = "";
  const char* magn_datawriter_xml = "<dds>"
    "<data_writer>"
    "<topic>"
    "<kind>NO_KEY</kind>"
    "<name>vectorMagnitude</name>"
    "<dataType>magnitude</dataType>"
    "</topic>"
    "</data_writer>"
    "</dds>";
    
  std::unique_ptr<Fedora::Broker> broker(Fedora::Broker::createBroker(0x12345678, false, outBuffer, BUFFER_SIZE, inBuffer, BUFFER_SIZE, participant_xml));
  broker->initialize();
  uint16_t subId = broker->initSubscriber(sub_topic_xml, subscriber_xml, sub_datareader_xml, &on_topic);
  uint16_t pubId = broker->initPublisher(magn_topic_xml, magn_publisher_xml, magn_datawriter_xml);

  std::cout << "Beep boop ready to go" << std::endl;
  // Iterate
  bool connected = true;
  int count = 0;
  int max_topics = 1000;
  while (count < max_topics) {
    uint8_t read_data_status;
    std::cout << "in loop" << std::endl;
    std::cout << "magn for: <" << topic.value[0] <<","<< topic.value[1]  <<","<< topic.value[2] << ">" << " is " <<
      sqrtf( (topic.value[0] * topic.value[0]) +
    	     (topic.value[1] * topic.value[1]) +
	     (topic.value[2] * topic.value[2]) ) << std::endl;
    magnitude pubTopic = {
      sqrtf( (topic.value[0] * topic.value[0]) +
    	     (topic.value[1] * topic.value[1]) +
	     (topic.value[2] * topic.value[2]) )
    };
    ucdrBuffer ub;
    magnitude_serialize_topic(&ub, &pubTopic);
    uint32_t topic_size = magnitude_size_of_topic(&pubTopic, 0);
    std::cout << "serialized" << std::endl;
    broker->prepPublish(pubId, &ub, topic_size);
    std::cout << "ready to run sesion" << std::endl;
    broker->runSession(1000);
    std::cout << "Sent: " <<  pubTopic.val << std::endl;
    sleep(1);
  }
  std::cout << "Shutting down" << std::endl;
  broker->close();

  return 0;
}
