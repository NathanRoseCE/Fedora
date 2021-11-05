#include <gtest/gtest.h>
#include "Fedora/Broker.hpp"
#include <stdexcept>
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
  uint8_t outBuffer[BUFFER_SIZE];
  uint8_t inBuffer[BUFFER_SIZE];
  std::unique_ptr<Fedora::Broker> broker(Fedora::Broker::createBroker(0xabcdef12, false, outBuffer, BUFFER_SIZE, inBuffer, BUFFER_SIZE, participant_xml));

  EXPECT_EQ(broker->participantXml(), participant_xml);
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
