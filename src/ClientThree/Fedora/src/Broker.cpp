#include "Fedora/Broker.hpp"
#include "Fedora/BrokerImpl.hpp"
#include "rapidxml_utils.hpp"
#include "rapidxml_print.hpp"
#include <sstream>
#include <exception>
#include <stdexcept>

Fedora::Broker* Fedora::Broker::createBroker(uint32_t id, bool clientOnly, uint8_t *outputBuffer, uint32_t outBufferSize, uint8_t *inputBuffer, uint32_t inBufferSize, const char* participant_xml) {
  return new BrokerImpl(id, clientOnly, outputBuffer, outBufferSize, inputBuffer, inBufferSize, participant_xml);
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
  return new BrokerImpl(broker_id, client_only, output_buffer, out_buffer_size, input_buffer, in_buffer_size, participant_xml);
}
