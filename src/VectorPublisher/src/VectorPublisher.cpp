/**
 * @file Physics.cxx
 *
 */

#include "VectorPubSubTypes.h"

#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/publisher/DataWriterListener.hpp>
#include <vector>

using namespace eprosima::fastdds::dds;

class Physics {
private:
  Vector _vector;
  DomainParticipant* _participant;
  
  Publisher* _publisher;
  Topic* _topic;
  DataWriter* _writer;
  TypeSupport _type;
  class PubListener : public DataWriterListener
  {
  public:
    PubListener():matched_(0){}
    ~PubListener() override{}
    void on_publication_matched(DataWriter*,
				const PublicationMatchedStatus& info) override {
      if (info.current_count_change == 1) {
	matched_ = info.total_count;
	std::cout << "Publisher matched." << std::endl;
      }
      else if (info.current_count_change == -1) {
	matched_ = info.total_count;
	std::cout << "Publisher unmatched." << std::endl;
      }
      else {
	std::cout << info.current_count_change
		  << " is not a valid value for PublicationMatchedStatus current count change." << std::endl;
      }
    }
    std::atomic_int matched_;
  } _listener;

public:
  Physics() :
    _participant(nullptr),
    _publisher(nullptr),
    _topic(nullptr),
    _writer(nullptr),
    _type(new VectorPubSubType()) {
  }
  virtual ~Physics() {
    if (_writer != nullptr) {
      _publisher->delete_datawriter(_writer);
    }
    if (_publisher != nullptr) {
      _participant->delete_publisher(_publisher);
    }
    if(_topic != nullptr) {
      _participant->delete_topic(_topic);
    }
    DomainParticipantFactory::get_instance()->delete_participant(_participant);
  }
  bool init() {
    _vector.value(std::vector<float>{0,0,0});

    DomainParticipantQos participantQos;
    participantQos.name("Physics");
    _participant = DomainParticipantFactory::get_instance()->create_participant(0, participantQos);

    if (_participant == nullptr){
      return false;
    }
    // Register the Type
    _type.register_type(_participant);

    // Create the publications Topic
    _topic = _participant->create_topic("SomeVector", "Vector", TOPIC_QOS_DEFAULT);
    if (_topic == nullptr){
      return false;
    }

    // Create the Publisher
    _publisher = _participant->create_publisher(PUBLISHER_QOS_DEFAULT, nullptr);
    if (_publisher == nullptr){
      return false;
    }

    // Create the DataWriter
    _writer = _publisher->create_datawriter(_topic, DATAWRITER_QOS_DEFAULT, &_listener);
    if (_writer == nullptr){
      return false;
    }
    return true;
  }
  bool publish() {
    if(_listener.matched_ > 0) {
      _vector.value(std::vector<float>{_vector.value()[0]+1,0,0});
      _writer->write(&_vector);
      return true;
    }
    return false;
  }
  void run(uint32_t iterations) {
    uint32_t currentIteration = 0;
    while (currentIteration < iterations) {
      if(publish()) {
	currentIteration++;
	std::cout << "Vector: <";
	for(const auto& value: _vector.value()) {
	  std::cout << (int)(value) << ", ";
	}
	std::cout << "> sent" << std::endl;
      }
      std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
  }
};
int main(int argc, char** argv) {
  std::cout << "Booting up Publisher" << std::endl;
  int samples = 10;
  Physics* physics = new Physics();
  if(physics->init()) {
    physics->run(static_cast<uint32_t>(samples));
  }
  delete physics;
  return 0;
}
