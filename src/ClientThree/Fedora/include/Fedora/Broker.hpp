#ifndef FEDORA_MAIN
#define FEDORA_MAIN

#include <cinttypes>
#include <string>
#include "rapidxml.hpp"

#define BROKER_PARTICIPANT_BUFFER_SIZE 100

extern "C" {
#include <uxr/client/client.h>
}

namespace Fedora {
typedef struct SubscriberDetails {
  uint16_t id;
  void (*callback)(struct ucdrBuffer* ub);
  std::string topicXml;
  std::string subscriberXml;
  std::string dataReaderXml;
} SubcriberDetails_t;

typedef struct PublisherDetails {
  uint16_t id;
  std::string topicXml;
  std::string publisherXml;
  std::string dataWriterXml;
} PublisherDetails_t;

class Broker {
 public:
  /*
   * This is the method that is used to create a broker, the output buffer is passed to the Client_dds, once set
   * this should not be changed or messed with!
   * This does connect to an agent, this is meant to simplify testing, and allow you to schedule when you will
   * make your network connection
   */
  static Broker* createBroker(uint32_t id, bool clientOnly, uint8_t *outputBuffer, uint32_t outBufferSize, uint8_t *inputBuffer, uint32_t inBufferSize, std::string participant_xml);

  /*
   * This is a creation method that reads a rapid xml configuration(and buffers)
   */
  static Broker* createBroker(rapidxml::xml_node<> *xml_config, uint8_t *output_buffer, uint32_t out_buffer_size, uint8_t *input_buffer, uint32_t in_buffer_size);
  /*
   * This returns the participantXml that is used every time a client connects to agent
   */
  virtual std::string participantXml() const = 0;
  virtual const uint32_t participantId() const = 0;
  /*
   * This is what connects to an agent, or creates a new one if it is not found, calling this twice is currently
   * undefined behavior
   */
  virtual void initialize() = 0;

  /*
   * This will create a publisher that will also re-create if the current agent ever dies
   */
  virtual uint16_t initPublisher(std::string topic_xml, std::string publisherXml, std::string dataWriter_xml, bool sync) = 0;
  virtual uint16_t initPublisher(std::string topic_xml, std::string publisherXml, std::string dataWriter_xml, bool sync, uint16_t id) = 0;
  /*
   * Gets a publisher with a specified id, will
   */
  virtual PublisherDetails_t getPublisher(uint16_t id) const = 0;
  virtual void removePublisher(uint16_t id) = 0;
  virtual void prepPublish(uint16_t id, ucdrBuffer *serializedBuffer, uint32_t topicSize) = 0;

  virtual uint16_t initSubscriber(std::string topic_xml, std::string subscriberXml, std::string dataReader_xml, bool sync, void (*callback)(struct ucdrBuffer* ub)) = 0;
  virtual uint16_t initSubscriber(std::string topic_xml, std::string subscriberXml, std::string dataReader_xml, bool sync, void (*callback)(struct ucdrBuffer* ub), uint16_t id) = 0;
  virtual SubcriberDetails_t getSubscriber(uint16_t id) const = 0;
  virtual void removeSubscriber(uint16_t id) = 0;
  
  virtual void runSession(int ms) = 0;
  virtual void close() = 0;
};
};
#endif
