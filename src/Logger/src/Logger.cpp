#include "VectorPubSubTypes.h"
#include "MagnitudePubSubTypes.h"

#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>
#include <fastdds/dds/subscriber/Subscriber.hpp>
#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastdds/dds/subscriber/DataReaderListener.hpp>
#include <fastdds/dds/subscriber/qos/DataReaderQos.hpp>
#include <fastdds/dds/subscriber/SampleInfo.hpp>
#include <iostream>

using namespace eprosima::fastdds::dds;

class VectorLogger : public DataReaderListener {
public:
  VectorLogger(std::string name): valueName(name){}
  ~VectorLogger() override {}
  void on_subscription_matched(DataReader*, const SubscriptionMatchedStatus& info) override {
  }
  void on_data_available(DataReader* reader) override {
    SampleInfo info;
    Vector value;
    if (reader->take_next_sample(&value, &info) == ReturnCode_t::RETCODE_OK) {
      if(info.valid_data) {
	std::cout << valueName << ": <";
	for(const auto& value : value.value()) {
	  std::cout << value << ", ";
	}
	std::cout << ">" << std::endl;
      }
    }
  }
private:
  std::string valueName;
};

class MagnitudeLogger : public DataReaderListener {
public:
  MagnitudeLogger(std::string name): valueName(name){}
  ~MagnitudeLogger() override {}
  void on_subscription_matched(DataReader*, const SubscriptionMatchedStatus& info) override {
  }
  void on_data_available(DataReader* reader) override {
    SampleInfo info;
    magnitude value;
    if (reader->take_next_sample(&value, &info) == ReturnCode_t::RETCODE_OK) {
      if(info.valid_data) {
	std::cout << valueName << ": " << value.val() << std::endl;
      }
    }
  }
private:
  std::string valueName;
};
class Logger {
private:
  DomainParticipant* _participant;
  TypeSupport _vectorType;
  TypeSupport _magnType;
  
  Topic *_someVector;
  Subscriber *_someVectorSubscriber;
  DataReader *_someVectorReader;
  VectorLogger _someVectorLogger;

  Topic *_vectorMagn;
  Subscriber *_vectorMagnSubscriber;
  DataReader *_vectorMagnReader;
  MagnitudeLogger _magnitudeLogger;

public:
  Logger() :
    _participant(nullptr),
    _someVector(nullptr),
    _someVectorSubscriber(nullptr),
    _someVectorReader(nullptr),
    _someVectorLogger("vectorTopic"),
    _vectorMagn(nullptr),
    _vectorMagnSubscriber(nullptr),
    _vectorMagnReader(nullptr),
    _magnitudeLogger("vectorMagnitude"),
    _vectorType(new VectorPubSubType()),
    _magnType(new magnitudePubSubType())
  {}
  virtual ~Logger() {
    closeDataReader(_someVector, _someVectorSubscriber, _someVectorReader);
    closeDataReader(_vectorMagn, _vectorMagnSubscriber, _vectorMagnReader);
    DomainParticipantFactory::get_instance()->delete_participant(_participant);
  }
  void closeDataReader(Topic* topic, Subscriber* subscriber, DataReader* reader) {
    if (reader != nullptr) {
      subscriber->delete_datareader(reader);
    }
    if (topic != nullptr) {
      _participant->delete_topic(topic);
    }
    if( subscriber != nullptr ) {
      _participant->delete_subscriber(subscriber);
    }
  }
  
  bool init() {
    //create Participant
    std::cout << "Init start" << std::endl;
    DomainParticipantQos participantQos;
    participantQos.name("Participant_subscriber");
    _participant = DomainParticipantFactory::get_instance()->create_participant(0, participantQos);
    _vectorType.register_type(_participant);
    _magnType.register_type(_participant);
    
    if(_participant == nullptr) {
      std::cout << "Unable to create participant" << std::endl;
      return false;
    }

    //Some Vector Subscriber
    _someVector = _participant->create_topic("vectorTopic", "Vector", TOPIC_QOS_DEFAULT);
    if(_someVector == nullptr) {
      std::cout << "Failed to create topic" << std::endl;
    }
    _someVectorSubscriber = _participant->create_subscriber(SUBSCRIBER_QOS_DEFAULT, nullptr);
    if(_someVectorSubscriber == nullptr) {
      std::cout << "failed to create subscriber" << std::endl;
      return false;
    }
    _someVectorReader = _someVectorSubscriber->create_datareader(_someVector, DATAREADER_QOS_DEFAULT, &_someVectorLogger);
    if (_someVectorReader == nullptr) {
      std::cout << "Failed to create Data Reader" << std::endl;
      return false;
    }

    
    //VectorMagnitude Subsciber
    _vectorMagn = _participant->create_topic("vectorMagnitude", "magnitude", TOPIC_QOS_DEFAULT);
    if(_vectorMagn == nullptr) {
      std::cout << "Failed to create topic" << std::endl;
    }
    _vectorMagnSubscriber = _participant->create_subscriber(SUBSCRIBER_QOS_DEFAULT, nullptr);
    if(_vectorMagnSubscriber == nullptr) {
      std::cout << "failed to create subscriber" << std::endl;
      return false;
    }
    _vectorMagnReader = _someVectorSubscriber->create_datareader(_vectorMagn, DATAREADER_QOS_DEFAULT, &_magnitudeLogger);
    if (_vectorMagnReader == nullptr) {
      std::cout << "Failed to create Data Reader" << std::endl;
      return false;
    }

    std::cout << "Initialization complete" << std::endl;
    return true;
  }
    
    //true acceleration subscriber
  void run() {
    while(true) {
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
  }
  bool createSubscriber(Subscriber*& subscriber, Topic*& topic, std::string topicName, std::string type,
			DataReader*& reader, DataReaderListener listener) {
    subscriber = _participant->create_subscriber(SUBSCRIBER_QOS_DEFAULT, nullptr);
    if(subscriber == nullptr) {
      std::cout << "failed to create subscriber" << std::endl;
      return false;
    }
    topic = _participant->create_topic(topicName, type, TOPIC_QOS_DEFAULT);
    if(topic == nullptr) {
      std::cout << "Failed to create topic" << std::endl;
      return false;
    }
    reader = subscriber->create_datareader(topic, DATAREADER_QOS_DEFAULT, &listener);
    if (reader == nullptr) {
      std::cout << "Failed to create Data Reader" << std::endl;
      return false;
    }
    return true;
  }
};

int main(int argc, char** argv) {
  std::cout << "Starting Logging" << std::endl;
  Logger* log = new Logger();
  if(log->init()) {
    log->run();
  }
  delete log;
  return 0;
}
