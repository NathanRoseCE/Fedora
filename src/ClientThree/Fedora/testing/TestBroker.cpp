#include <gtest/gtest.h>
#include "Fedora/Broker.hpp"
#include <stdexcept>
#include <fstream>
#include "rapidxml.hpp"
#include "rapidxml_utils.hpp"
#include "rapidxml_print.hpp"

#define BUFFER_SIZE 80

extern "C" {
#include <uxr/client/client.h>
}
void on_topic(struct ucdrBuffer* ub) {
}
TEST(BROKER, registerPublisher) {
  const char* participant_xml = "<dds>"
    "<participant>"
    "<rtps>"
    "<name>default_xrce_participant</name>"
    "</rtps>"
    "</participant>"
    "</dds>";
  uint8_t outBuffer[BUFFER_SIZE];
  uint8_t inBuffer[BUFFER_SIZE];
  std::unique_ptr<Fedora::Broker> broker(Fedora::Broker::createBroker(0xabcdef12, false, outBuffer, BUFFER_SIZE, inBuffer, BUFFER_SIZE, participant_xml));
  
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
  uint16_t id = broker->initPublisher(topic_xml, publisher_xml, datawriter_xml, false);
  Fedora::PublisherDetails_t publisher_details = broker->getPublisher(id);
  
  EXPECT_EQ(publisher_details.id, id);
  EXPECT_EQ(publisher_details.topicXml, topic_xml);
  EXPECT_EQ(publisher_details.publisherXml, publisher_xml);
  EXPECT_EQ(publisher_details.dataWriterXml, datawriter_xml);
};

TEST(BROKER, registerSubsciber) {
  const char* participant_xml = "<dds>"
    "<participant>"
    "<rtps>"
    "<name>default_xrce_participant</name>"
    "</rtps>"
    "</participant>"
    "</dds>";
  uint8_t outBuffer[BUFFER_SIZE];
  uint8_t inBuffer[BUFFER_SIZE];
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
  
  std::unique_ptr<Fedora::Broker> broker(Fedora::Broker::createBroker(0xabcdef12, false, outBuffer, BUFFER_SIZE, inBuffer, BUFFER_SIZE, participant_xml));
  uint16_t sub_id = broker->initSubscriber(magn_topic_xml, subscriber_xml, datareader_xml, false, &on_topic);
  Fedora::SubcriberDetails_t sub_details = broker->getSubscriber(sub_id);

  EXPECT_EQ(sub_details.id, sub_id);
  EXPECT_EQ(sub_details.topicXml, magn_topic_xml);
  EXPECT_EQ(sub_details.dataReaderXml, datareader_xml);
  EXPECT_EQ(sub_details.subscriberXml, subscriber_xml);  
}
TEST(BROKER, participantXml) {
  const char* participant_xml = "<dds>"
    "<participant>"
    "<rtps>"
    "<name>default_xrce_participant</name>"
    "</rtps>"
    "</participant>"
    "</dds>";
  std::string correct_xml(participant_xml);
  uint8_t outBuffer[BUFFER_SIZE];
  uint8_t inBuffer[BUFFER_SIZE];
  std::unique_ptr<Fedora::Broker> broker(Fedora::Broker::createBroker(0xabcdef12, false, outBuffer, BUFFER_SIZE, inBuffer, BUFFER_SIZE, participant_xml));

  std::string actual_xml(broker->participantXml());
  EXPECT_EQ(correct_xml, actual_xml);
}
TEST(BROKER, registerPublisherGivenId) {
  const char* participant_xml = "<dds>"
    "<participant>"
    "<rtps>"
    "<name>default_xrce_participant</name>"
    "</rtps>"
    "</participant>"
    "</dds>";
  uint8_t outBuffer[BUFFER_SIZE];
  uint8_t inBuffer[BUFFER_SIZE];
  std::unique_ptr<Fedora::Broker> broker(Fedora::Broker::createBroker(0xabcdef12, false, outBuffer, BUFFER_SIZE, inBuffer, BUFFER_SIZE, participant_xml));
  
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
  uint16_t id = 1234;
  broker->initPublisher(topic_xml, publisher_xml, datawriter_xml, false, id);
  Fedora::PublisherDetails_t publisher_details = broker->getPublisher(id);
  
  EXPECT_EQ(publisher_details.id, id);
  EXPECT_EQ(publisher_details.topicXml, topic_xml);
  EXPECT_EQ(publisher_details.publisherXml, publisher_xml);
  EXPECT_EQ(publisher_details.dataWriterXml, datawriter_xml);
};

TEST(BROKER, registerSubsciberWithId) {
  const char* participant_xml = "<dds>"
    "<participant>"
    "<rtps>"
    "<name>default_xrce_participant</name>"
    "</rtps>"
    "</participant>"
    "</dds>";
  uint8_t outBuffer[BUFFER_SIZE];
  uint8_t inBuffer[BUFFER_SIZE];
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
  
  std::unique_ptr<Fedora::Broker> broker(Fedora::Broker::createBroker(0xabcdef12, false, outBuffer, BUFFER_SIZE, inBuffer, BUFFER_SIZE, participant_xml));
  uint16_t sub_id = 132;
  broker->initSubscriber(magn_topic_xml, subscriber_xml, datareader_xml, false, &on_topic, sub_id);
  Fedora::SubcriberDetails_t sub_details = broker->getSubscriber(sub_id);

  EXPECT_EQ(sub_details.id, sub_id);
  EXPECT_EQ(sub_details.topicXml, magn_topic_xml);
  EXPECT_EQ(sub_details.dataReaderXml, datareader_xml);
  EXPECT_EQ(sub_details.subscriberXml, subscriber_xml);  
}

TEST(BROKER, removePublisher) {
  const char* participant_xml = "<dds>"
    "<participant>"
    "<rtps>"
    "<name>default_xrce_participant</name>"
    "</rtps>"
    "</participant>"
    "</dds>";
  uint8_t outBuffer[BUFFER_SIZE];
  uint8_t inBuffer[BUFFER_SIZE];
  std::unique_ptr<Fedora::Broker> broker(Fedora::Broker::createBroker(0xabcdef12, false, outBuffer, BUFFER_SIZE, inBuffer, BUFFER_SIZE, participant_xml));
  
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
  uint16_t id = broker->initPublisher(topic_xml, publisher_xml, datawriter_xml, false);
  Fedora::PublisherDetails_t publisher_details = broker->getPublisher(id);
  try {
    broker->removePublisher(id);
    EXPECT_TRUE(true);
  }
  catch(std::invalid_argument& e) {
    EXPECT_TRUE(false);
  }

  try {
    broker->getPublisher(id);
    EXPECT_TRUE(false);
  }
  catch(std::invalid_argument& e) {
    EXPECT_TRUE(true);
  }
}

TEST(BROKER, removeSubsciber) {
  const char* participant_xml = "<dds>"
    "<participant>"
    "<rtps>"
    "<name>default_xrce_participant</name>"
    "</rtps>"
    "</participant>"
    "</dds>";
  uint8_t outBuffer[BUFFER_SIZE];
  uint8_t inBuffer[BUFFER_SIZE];
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
  std::unique_ptr<Fedora::Broker> broker(Fedora::Broker::createBroker(0xabcdef12, false, outBuffer, BUFFER_SIZE, inBuffer, BUFFER_SIZE, participant_xml));
  uint16_t sub_id = broker->initSubscriber(magn_topic_xml, subscriber_xml, datareader_xml, false, &on_topic);
  Fedora::SubcriberDetails_t sub_details = broker->getSubscriber(sub_id);
  try {
    broker->removeSubscriber(sub_id);
    EXPECT_TRUE(true);
  }
  catch(std::invalid_argument& e) {
    EXPECT_TRUE(false);
  }
  try {
    broker->getSubscriber(sub_id);
    EXPECT_TRUE(false);
  }
  catch(std::invalid_argument& e) {
    EXPECT_TRUE(true);
  }
}

TEST(BROKER, createFromXML) {
  // Read from file version
  //  rapidxml::file<> xmlFile("resources/config.xml"); // Default template is char
  //  rapidxml::xml_document<> doc;
  //  doc.parse<0>(xmlFile.data());

  rapidxml::xml_document<> doc;
  rapidxml::xml_node<> *fedora = doc.allocate_node(rapidxml::node_element, "Fedora");
  fedora->append_attribute(doc.allocate_attribute("version", "1.0"));
  
  rapidxml::xml_node<> *broker = doc.allocate_node(rapidxml::node_element, "Broker");
  fedora->append_node(broker);
  
  rapidxml::xml_node<> *id = doc.allocate_node(rapidxml::node_element, "id", "0xAAAABBBB");
  broker->append_node(id);
  rapidxml::xml_node<> *client_only = doc.allocate_node(rapidxml::node_element, "clientOnly", "false");
  broker->append_node(client_only);

  rapidxml::xml_node<> *participant_xml = doc.allocate_node(rapidxml::node_element, "participantConfig");
  broker->append_node(participant_xml);
  rapidxml::xml_node<> *dds = doc.allocate_node(rapidxml::node_element, "dds");
  participant_xml->append_node(dds);
  rapidxml::xml_node<> *participant = doc.allocate_node(rapidxml::node_element, "participant");
  dds->append_node(participant);
  rapidxml::xml_node<> *rtps = doc.allocate_node(rapidxml::node_element, "rtps");
  participant->append_node(rtps);
  rapidxml::xml_node<> *name = doc.allocate_node(rapidxml::node_element, "name", "default_xrce_participant");
  rtps->append_node(name);
  
  
  uint8_t outBuffer[BUFFER_SIZE];
  uint8_t inBuffer[BUFFER_SIZE];
  
  std::unique_ptr<Fedora::Broker> broker_obj(Fedora::Broker::createBroker(fedora, outBuffer, BUFFER_SIZE, inBuffer, BUFFER_SIZE));
  rapidxml::xml_document<> participant_xml_doc;
  char test[BROKER_PARTICIPANT_BUFFER_SIZE];
  strcpy(test, broker_obj->participantXml());
  participant_xml_doc.parse<0>(test);
  std::cout << participant_xml_doc;
  EXPECT_EQ(broker_obj->participantId(), 0xAAAABBBB);
  
  std::string part_name_actual(participant_xml_doc.first_node("dds")->first_node("participant")->first_node("rtps")->first_node("name")->value());
  EXPECT_EQ(part_name_actual, "default_xrce_participant");
  EXPECT_TRUE(false);
}

