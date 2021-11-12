#include "Fedora/Broker.hpp"
#include "Fedora/BrokerImpl.hpp"
#include "rapidxml_utils.hpp"
#include "rapidxml_print.hpp"
#include <sstream>
#include <string>
#include <exception>
#include <stdexcept>

Fedora::Broker* Fedora::Broker::createBroker(uint32_t id, bool clientOnly, uint8_t *outputBuffer, uint32_t outBufferSize, uint8_t *inputBuffer, uint32_t inBufferSize, std::string participant_xml) {
  return new BrokerImpl(id, clientOnly, outputBuffer, outBufferSize, inputBuffer, inBufferSize, participant_xml);
}

std::string node_str(rapidxml::xml_node<> *xml_node) {
  if(xml_node->first_node()) {
    std::stringstream ss;
    ss << *(xml_node->first_node());
    return ss.str();
  }
  return "";
}
Fedora::Broker* Fedora::Broker::createBroker(rapidxml::xml_node<> *xml_config, uint8_t *output_buffer, uint32_t out_buffer_size, uint8_t *input_buffer, uint32_t in_buffer_size) {
  std::string version = xml_config->first_attribute("version")->value();
  if(version != "1.0") {
    std::stringstream what;
    what << "Version " << version << " not supported";
    throw std::invalid_argument(what.str());
  }
  rapidxml::xml_node<> *broker_config = xml_config->first_node("Broker");
  uint32_t broker_id = (uint32_t)(std::stoul(broker_config->first_node("id")->value(), nullptr, 16));
  bool client_only = (broker_config->first_node("clientOnly")->value()=="false") ? false : true;
  //  std::cout << "client only: " << client_only << std::endl;
  //  std::cout << "client only string: " << broker_config->first_node("clientOnly")->value() << std::endl;
  std::stringstream ss;
  ss << *(broker_config->first_node("participantConfig")->first_node("dds"));
  std::string temp = ss.str();
  char participant_xml[100];
  strcpy(participant_xml, temp.c_str());
  //  std::cout << "participant config:" << participant_xml << std::endl;
  Fedora::Broker* broker = new BrokerImpl(broker_id, client_only, output_buffer, out_buffer_size, input_buffer, in_buffer_size, participant_xml);

  rapidxml::xml_node<> *publishers = xml_config->first_node("Publishers");
  for (rapidxml::xml_node<> *publisher = publishers->first_node(); publisher; publisher = publisher->next_sibling()) {
    std::string publisher_xml = node_str(publisher->first_node("PublisherConfig"));
    std::string datawriter_xml = node_str(publisher->first_node("DataWriterConfig"));
    std::string topic_xml = node_str(publisher->first_node("TopicConfig"));
    
    uint16_t id = std::stoi(publisher->first_node("id")->value());
    broker->initPublisher(topic_xml, publisher_xml, datawriter_xml, false, id);
  }

  
  rapidxml::xml_node<> *subscribers = xml_config->first_node("Subscribers");
  std::cout << "Got to subscribers" << std::endl;
  std::cout << "ptr: " << subscribers << std::endl;
  for (rapidxml::xml_node<> *subscriber = subscribers->first_node(); subscriber; subscriber = subscriber->next_sibling()) {
    std::string subscriber_xml = node_str(subscriber->first_node("SubscriberConfig"));
    std::string datareader_xml = node_str(subscriber->first_node("DataReaderConfig"));
    std::string topic_xml = node_str(subscriber->first_node("TopicConfig"));
    
    uint16_t id = std::stoi(subscriber->first_node("id")->value());
    broker->initSubscriber(topic_xml, subscriber_xml, datareader_xml, false, nullptr, id);
  }
  std::cout << "finished " << std::endl;
  return broker;

  
}
