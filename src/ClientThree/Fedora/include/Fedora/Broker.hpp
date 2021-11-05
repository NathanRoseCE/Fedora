#ifndef FEDORA_MAIN
#define FEDORA_MAIN
#include <cinttypes>
extern "C" {
#include <uxr/client/client.h>
}
namespace Fedora {
typedef struct SubscriberDetails {
  uint16_t id;
  void (*callback)(struct ucdrBuffer* ub);
  const char* topicXml;
  const char* subscriberXml;
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
  /*
   * This is the method that is used to create a broker, the output buffer is passed to the Client_dds, once set
   * this should not be changed or messed with!
   * This does connect to an agent, this is meant to simplify testing, and allow you to schedule when you will
   * make your network connection
   */
  static Broker* createBroker(uint32_t id, bool clientOnly, uint8_t *outputBuffer, uint32_t outBufferSize, uint8_t *inputBuffer, uint32_t inBufferSize, const char* participant_xml);
  /*
   * This returns the participantXml that is used every time a client connects to agent
   */
  virtual const char* participantXml() const = 0;
  /*
   * This is what connects to an agent, or creates a new one if it is not found, calling this twice is currently
   * undefined behavior
   */
  virtual void initialize() = 0;

  /*
   * This will create a publisher that will also re-create if the current agent ever dies
   */
  virtual uint16_t initPublisher(const char* topic_xml, const char* publisherXml, const char* dataWriter_xml, bool sync) = 0;
  /*
   * Gets a publisher with a specified id, will
   */
  virtual PublisherDetails_t getPublisher(uint16_t id) const = 0;
  virtual void removePublisher(uint16_t id) = 0;
  virtual void prepPublish(uint16_t id, ucdrBuffer *serializedBuffer, uint32_t topicSize) = 0;

  virtual uint16_t initSubscriber(const char* topic_xml, const char* subscriberXml, const char* dataReader_xml, bool sync, void (*callback)(struct ucdrBuffer* ub)) = 0;
  virtual SubcriberDetails_t getSubscriber(uint16_t id) const = 0;
  virtual void removeSubscriber(uint16_t id) = 0;
  
  virtual void runSession(int ms) = 0;
  virtual void close() = 0;
};
};
#endif
