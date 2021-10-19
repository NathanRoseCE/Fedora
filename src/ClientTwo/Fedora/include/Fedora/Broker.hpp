#ifndef FEDORA_MAIN
#define FEDORA_MAIN
#include <cinttypes>

namespace Fedora {
class Broker {
 public:
  static Broker* createBroker(uint32_t id, bool clientOnly, uint8_t *outputBuffer, uint32_t outBufferSize, uint8_t *inputBuffer, uint32_t inBufferSize, const char* participant_xml);
  
  virtual void initialize() = 0;
  virtual uint16_t initPublisher(const char* topic_xml, const char* publisherXml, const char* dataWriter_xml) = 0;
  virtual uint16_t initSubscriber(const char* topic_xml, const char* subscriberXml, const char* dataReader_xml, void (*callback)(struct ucdrBuffer* ub)) = 0;
  virtual void prepPublish(uint16_t id, ucdrBuffer *serializedBuffer, uint32_t topicSize) = 0;

  virtual void runSession(int ms) = 0;
  virtual void close() = 0;
};
};
#endif
