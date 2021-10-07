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

#include <future>
#include "Broker.hpp"
#define STREAM_HISTORY  8
#define BUFFER_SIZE     UXR_CONFIG_UDP_TRANSPORT_MTU* STREAM_HISTORY

extern "C" {
#include "Vector.h"
#include "Magnitude.h"
};


void on_topic(struct ucdrBuffer* ub) {
  magnitude topic;
  magnitude_deserialize_topic(ub, &topic);
  std:: cout << "Magnitude: " <<  topic.val << std::endl;
}



int main(int args, char** argv) {
  const char* participant_xml = "<dds>"
    "<participant>"
    "<rtps>"
    "<name>default_xrce_participant</name>"
    "</rtps>"
    "</participant>"
    "</dds>";
  uint8_t outBuffer[BUFFER_SIZE];
  uint8_t inBuffer[BUFFER_SIZE];
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
  const char* magn_topic_xml = "<dds>"
    "<topic>"
    "<name>vectorMagnitude</name>"
    "<dataType>magnitude</dataType>"
    "</topic>"
    "</dds>";
  const char* subscriber_xml = "";
  const char* datareader_xml =  "<dds>"
    "<data_reader>"
    "<topic>"
    "<kind>NO_KEY</kind>"
    "<name>vectorMagnitude</name>"
    "<dataType>magnitude</dataType>"
    "</topic>"
    "</data_reader>"
    "</dds>";
  
  Broker broker(false, outBuffer, BUFFER_SIZE, inBuffer, BUFFER_SIZE, participant_xml);
  broker.initialize();
  uint16_t id = broker.initPublisher(topic_xml, publisher_xml, datawriter_xml);
  uint16_t subId = broker.initSubscriber(magn_topic_xml, subscriber_xml, datareader_xml, &on_topic);

  // Write topics
  uint32_t count = 0;
  float i = 0;
  int max_topics =100;
  while (count++ < max_topics) {
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
    std::cout << "beep" << std::endl;
  }
  std::cout << "Shutting down" << std::endl;
  broker.close();
  return 0;
}

