#pragma once

#include <uxr/agent/AgentInstance.hpp>
#include <future>
#include <vector> // TODO move to a stack solution for better determinability
#include <thread>

extern "C" {//libraries that dont play well with Cpp
#include <uxr/client/client.h>
#include <ucdr/microcdr.h>
#include <uxr/client/core/type/xrce_types.h>
#include <unistd.h>
};

#define SUBSCRIBER_CALLBACK void (*callback)(struct ucdrBuffer* ub)

typedef struct SubscriberDetails {
  uint16_t id;
  SUBSCRIBER_CALLBACK;
  const char* topixcXml;
  const char* publisherXml;
  const char* dataReaderXml;
} SubcriberDetails_t;

typedef struct PublisherDetails {
  uint16_t id;
  const char* topicXml;
  const char* publisherXml;
  const char* dataWriterXml;
} PublisherDetails_t;

class Broker {
public:
  Broker(bool clientOnly, uint8_t *outputBuffer, uint32_t outBufferSize, uint8_t *inputBuffer, uint32_t inBufferSize, const char* participant_xml);

  void initialize();
  uint16_t initPublisher(const char* topic_xml, const char* publisherXml, const char* dataWriter_xml);
  void registerPublisher(PublisherDetails details);
  void initSubscriber(const char* topic_xml, const char* dataReader_xml, SUBSCRIBER_CALLBACK);
  void registerSubscriber(SubscriberDetails details);
  void prepPublish(uint16_t id, ucdrBuffer *serializedBuffer, uint32_t topicSize);

  void runSession(int ms);
  void close();
  
private:
  bool findAgent();
  void connectToAgent(); // will create an agent if not clientOnly
  void createAgent();
  void overallSubscriberCallback();
  void subcribeCallback();
  
  //callback functions
  static void createAgentHelper();
  static bool onAgentFound(const TransportLocator* locator, void* args);
  static void subscribeCallback(uxrSession* session, uxrObjectId object_id, uint16_t request_id,
				uxrStreamId stream_id, struct ucdrBuffer* ub, uint16_t length, void* args);
  
private: //state, TODO split this into multiple classes
  bool clientOnly; 
  TransportLocator agent;
  std::thread agentThread;
  
  uxrObjectId participant_id; 
  const char *participant_xml;

  std::vector<SubscriberDetails> subscribers;
  std::vector<PublisherDetails> publishers;
  
  uxrUDPTransport transport;
  uxrSession session;
  uxrStreamId reliable_out;
  uxrStreamId reliable_in;
  uint8_t *outputBuffer, *inputBuffer;
  uint32_t outputBufferSize, inputBufferSize;
  uint16_t idIncramenter;
};
