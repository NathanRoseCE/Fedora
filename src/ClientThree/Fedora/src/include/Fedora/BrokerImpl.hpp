#ifndef FEDORA_BROKER_IMPL
#define FEDORA_BROKER_IMPL

#include <vector> // TODO move to a stack solution for better determinability
#include <thread>

#include <uxr/agent/AgentInstance.hpp>

extern "C" {//libraries that dont play well with Cpp
#include <uxr/client/client.h>
#include <ucdr/microcdr.h>
#include <uxr/client/core/type/xrce_types.h>
#include <unistd.h>
};

#include "Fedora/Broker.hpp"


namespace Fedora {
class BrokerImpl : public Fedora::Broker {
public:
  BrokerImpl(uint32_t id, bool clientOnly, uint8_t *outputBuffer, uint32_t outBufferSize, uint8_t *inputBuffer, uint32_t inBufferSize, const char* participant_xml);
  const char* participantXml() const;

  void initialize();
  uint16_t initPublisher(const char* topic_xml, const char* publisherXml, const char* dataWriter_xml, bool sync);
  void registerPublisher(PublisherDetails details);
  uint16_t initSubscriber(const char* topic_xml, const char* subscriberXml, const char* dataReader_xml, bool sync, void (*callback)(struct ucdrBuffer* ub));
  void registerSubscriber(SubscriberDetails details);
  void prepPublish(uint16_t id, ucdrBuffer *serializedBuffer, uint32_t topicSize);
  
  Fedora::PublisherDetails_t getPublisher(uint16_t id) const;
  Fedora::SubcriberDetails_t getSubscriber(uint16_t id) const;
  void removePublisher(uint16_t id);
  void removeSubscriber(uint16_t id);

  void runSession(int ms);
  void close();
  
private:
  bool findAgent();
  void connectToAgent(); // will create an agent if not clientOnly
  void createAgent();
  void subcribeCallback();
  
  //callback functions
  static void createAgentHelper();
  static void subscribeCallback(uxrSession* session, uxrObjectId object_id, uint16_t request_id,
				uxrStreamId stream_id, struct ucdrBuffer* ub, uint16_t length, void* args);
  static bool onAgentFound(const TransportLocator* locator, void* args);
  
private: //state, TODO split this into multiple classes
  bool client_only_; 
  TransportLocator agent_;
  std::thread agent_thread_;
  
  uxrObjectId participant_id_; 
  const char *participant_xml_;

  std::vector<SubscriberDetails> subscribers_;
  std::vector<PublisherDetails> publishers_;
  
  uxrUDPTransport transport_;
  uxrSession session_;
  uxrStreamId reliable_out_;
  uxrStreamId reliable_in_;
  uint8_t *output_buffer_, *input_buffer_;
  uint32_t output_buffer_size_, input_buffer_size_;
  uint16_t id_incrament_;
  char agent_ip_[16];
  char port_[10];
  uint32_t id_;
};
};
#endif //FEDORA_BROKER_IMPL
