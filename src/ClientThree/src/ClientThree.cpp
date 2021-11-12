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

#include <iostream>
#include <unistd.h>
#include <memory>

#include "rapidxml.hpp"
#include "rapidxml_utils.hpp"
#include "rapidxml_print.hpp"
extern "C" {
#include "Vector.h"
#include "Magnitude.h"
#include <uxr/client/client.h>
};

#include "Fedora/Broker.hpp"

#define STREAM_HISTORY  8
#define BUFFER_SIZE     100* STREAM_HISTORY

void on_topic(struct ucdrBuffer* ub) {
  magnitude topic;
  magnitude_deserialize_topic(ub, &topic);
  std::cout << "Magnitude-----------: " <<  (float)(topic.val) << std::endl;
}



int main(int args, char** argv) {
  
  rapidxml::file<> xmlFile("resources/config.xml"); // Default template is char
  rapidxml::xml_document<> doc;
  doc.parse<0>(xmlFile.data());

  uint8_t outBuffer[BUFFER_SIZE];
  uint8_t inBuffer[BUFFER_SIZE];
  int pub_id = 1;
  std::unique_ptr<Fedora::Broker> broker(Fedora::Broker::createBroker(doc.first_node("Fedora"), outBuffer, BUFFER_SIZE, inBuffer, BUFFER_SIZE));
  broker->registerCallback(1, &on_topic);
  broker->initialize();

  
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
    broker->prepPublish(pub_id, &ub, topic_size);
    Vector_serialize_topic(&ub, &topic);
    
    broker->runSession(1000);
    sleep(1);
    i++;
    std::cout << "beep" << std::endl;
  }
  std::cout << "Shutting down" << std::endl;
  broker->close();
  return 0;
}

