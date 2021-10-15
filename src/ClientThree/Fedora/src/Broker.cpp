#include "Fedora/Broker.hpp"
#include "BrokerImpl.hpp"

Broker* Broker::createBroker(uint32_t id, bool clientOnly, uint8_t *outputBuffer, uint32_t outBufferSize, uint8_t *inputBuffer, uint32_t inBufferSize, const char* participant_xml) {
  return new BrokerImpl(id, clientOnly, outputBuffer, outBufferSize, inputBuffer, inBufferSize, participant_xml);
}
