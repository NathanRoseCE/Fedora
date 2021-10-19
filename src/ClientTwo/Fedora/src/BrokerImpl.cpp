#include "Fedora/BrokerImpl.hpp"

#define STREAM_HISTORY  8
struct FindAgentArgs{
  bool* found;
  Fedora::BrokerImpl* context;
};


Fedora::BrokerImpl::BrokerImpl(uint32_t id, bool clientOnly, uint8_t *outputBuffer, uint32_t outBufferSize, uint8_t *inputBuffer, uint32_t inBufferSize, const char* participant_xml) :
  id_(id),
  client_only_(clientOnly),
  output_buffer_(outputBuffer),
  output_buffer_size_(outBufferSize),
  input_buffer_(inputBuffer),
  input_buffer_size_(inBufferSize),
  participant_xml_(participant_xml),
  id_incrament_(1)
{
}
void Fedora::BrokerImpl::initialize() {
  connectToAgent();
}
  
uint16_t Fedora::BrokerImpl::initPublisher(const char* topic_xml, const char* publisher_xml, const char* data_writer_xml) {
  PublisherDetails publisher = {
    id_incrament_++,
    topic_xml,
    publisher_xml,
    data_writer_xml
  };
  registerPublisher(publisher);
  publishers_.push_back(publisher);
  std::cout << "Publisher created successfully: " << publisher.id << std::endl;
  return publisher.id;
}
void Fedora::BrokerImpl::registerPublisher(PublisherDetails details) {
  uint16_t topic_req = uxr_buffer_create_topic_xml(&session_, reliable_out_,
						   uxr_object_id(details.id, UXR_TOPIC_ID), participant_id_,
						   details.topicXml, UXR_REPLACE);
  uxrObjectId publisher_id = uxr_object_id(details.id, UXR_PUBLISHER_ID);
  uint16_t publisher_req = uxr_buffer_create_publisher_xml(&session_, reliable_out_, publisher_id,
							   participant_id_, details.publisherXml, UXR_REPLACE);
  uint16_t datawriter_req = uxr_buffer_create_datawriter_xml(&session_, reliable_out_, 
							     uxr_object_id(details.id, UXR_DATAWRITER_ID),
							     publisher_id, details.dataWriterXml, UXR_REPLACE);
  int num_requests = 3;
  uint16_t requests[num_requests] = {topic_req, publisher_req, datawriter_req};
  uint8_t status[num_requests];
  if (!uxr_run_session_until_all_status(&session_, 1000, requests, status, num_requests)) {
    std::cout << "Error when registering publishser" << std::endl;
    std::cout << "Topic: " << (int)(status[0]) << std::endl;
    std::cout << "Publisher: " << (int)(status[1]) << std::endl;
    std::cout << "DataWriter: " << (int)(status[2]) << std::endl;
    throw "Error Registering Publisher";
  }
}
void Fedora::BrokerImpl::prepPublish(uint16_t id, ucdrBuffer *serialized_buffer, uint32_t topic_size) {
  uxr_prepare_output_stream(&session_, reliable_out_, uxr_object_id(id, UXR_DATAWRITER_ID), serialized_buffer, topic_size);
}
uint16_t Fedora::BrokerImpl::initSubscriber(const char* topic_xml, const char* subscriber_xml, const char* dataReader_xml, void (*callback)(struct ucdrBuffer* ub)) {
  
  SubscriberDetails subscriber = {
    id_incrament_++,
    callback, 
    topic_xml,
    subscriber_xml,
    dataReader_xml
  };
  registerSubscriber(subscriber);
  subscribers_.push_back(subscriber);
  std::cout << "Subscriber created successfully: " << subscriber.id << std::endl;
  return subscriber.id;
}
void Fedora::BrokerImpl::registerSubscriber(SubscriberDetails details) {
  uint16_t topic_req = uxr_buffer_create_topic_xml(&session_, reliable_out_,
						   uxr_object_id(details.id, UXR_TOPIC_ID), participant_id_,
						   details.topicXml, UXR_REPLACE);
  uxrObjectId subscriber_id = uxr_object_id(details.id, UXR_SUBSCRIBER_ID);
  uint16_t subscriber_req = uxr_buffer_create_subscriber_xml(&session_, reliable_out_, subscriber_id, participant_id_, details.subscriberXml, UXR_REPLACE);
  
  uxrObjectId datareader_id = uxr_object_id(details.id, UXR_DATAREADER_ID);
  uint16_t datareader_req = uxr_buffer_create_datareader_xml(&session_, reliable_out_,
							     datareader_id,
							     subscriber_id, details.dataReaderXml, UXR_REPLACE);
  int num_requests = 3;
  uint16_t requests[num_requests] = {topic_req, subscriber_req, datareader_req};
  uint8_t status[num_requests];
  if (!uxr_run_session_until_all_status(&session_, 1000, requests, status, num_requests)) {
    std::cout << "Error when registering subscriber" << std::endl;
    std::cout << "topic: " << (int)(status[0]) << std::endl;
    std::cout << "subscriber: " << (int)(status[1]) << std::endl;
    std::cout << "datareader: " << (int)(status[2]) << std::endl;
    throw "Error Registering Subscriber";
  }
  
  uxrDeliveryControl delivery_control = {
    0
  };
  delivery_control.max_samples = UXR_MAX_SAMPLES_UNLIMITED; 
  uint16_t read_data_req = uxr_buffer_request_data(&session_, reliable_out_, datareader_id, reliable_in_, &delivery_control); // This is not put with the rest of the requests?
}

void Fedora::BrokerImpl::runSession(int ms) {
  bool connected = uxr_run_session_until_confirm_delivery(&session_, ms);
  if( !connected ) {
    std::cout << "Detected that Agent is down" << std::endl;
    close();
    connectToAgent();
    connected = uxr_run_session_until_confirm_delivery(&session_, ms);
    if( !connected) {
      throw "Unable to connect to agent"; //TODO multiple retries?
    }
  }
}
void Fedora::BrokerImpl::close() {
  std::cout << "Closing down BrokerImpl" << std::endl;
  uxr_delete_session(&session_);
  uxr_close_udp_transport(&transport_);
}

bool Fedora::BrokerImpl::findAgent() {
  bool found = false;
  struct FindAgentArgs args = {&found, this};
  uxr_discovery_agents_default(10, 1000, Fedora::BrokerImpl::onAgentFound, (void*)&args);
  if(!found) {
    sleep(1);//try again in a second TODO randomize
    struct FindAgentArgs args = {&found, this};
    uxr_discovery_agents_default(10, 1000, Fedora::BrokerImpl::onAgentFound, (void*)&args);
    if(!found) {
      std::cout << "self booting agent" << std::endl;
      createAgent();
      struct FindAgentArgs args = {&found, this};
      uxr_discovery_agents_default(10, 1000, Fedora::BrokerImpl::onAgentFound, (void*)&args);
    }
  }
  if(!found) {
    std::cout << "Unable to get an agent" <<std::endl;
    throw "unable to find an agent";
  }
  return found;
}
void Fedora::BrokerImpl::connectToAgent() {//TODO refactor, make actual exceptions
  findAgent();
  uxrIpProtocol ip_protocol;
  uint16_t agent_port;
  uxr_locator_to_ip(&agent_, agent_ip_, sizeof(agent_ip_), &agent_port, &ip_protocol);
  sprintf(port_, "%d", agent_port);

  //init transport
  if (!uxr_init_udp_transport(&transport_, UXR_IPv4, agent_ip_, port_)) {
    throw "Unable to create Transport";
  }

  //setup session
  uxr_init_session(&session_, &transport_.comm, id_);
  uxr_set_topic_callback(&session_, subscribeCallback, this);
  if (!uxr_create_session(&session_)) {
    throw "Unable to create Session";
  }
  // initialize buffers for the sesion
  reliable_out_ = uxr_create_output_reliable_stream(&session_, output_buffer_, output_buffer_size_, STREAM_HISTORY);
  reliable_in_ = uxr_create_input_reliable_stream(&session_, input_buffer_, input_buffer_size_, STREAM_HISTORY);

  //set up the participant
  participant_id_ = uxr_object_id(0x01, UXR_PARTICIPANT_ID);
  uint16_t participant_req = uxr_buffer_create_participant_xml(&session_, reliable_out_, participant_id_, 0,
							       participant_xml_, UXR_REPLACE);
  uint8_t status;
  if (!uxr_run_session_until_all_status(&session_, 1000, &participant_req, &status, 1)) {
    std::cout << "Unable to create participant. Error: " << status << std::endl;
    throw "Unable to create participant";
  }
  else {
    std::cout << "Successfully created participant" << std::endl;
    std::cout << &session_ << std::endl;
    
  }
  //reregister any previous items
  for(PublisherDetails pubDetails : publishers_) {
    registerPublisher(pubDetails);
  }
  for(SubscriberDetails subDetails : subscribers_) {
    registerSubscriber(subDetails);
  }
  std::cout << "Agent Connection Established and Ready" << std::endl;
  std::cout << "IP: " << agent_ip_ << ":" << port_ << std::endl;
}
void Fedora::BrokerImpl::createAgent() {
  std::cout << "Creating new agent" << std::endl;
  agent_thread_ = std::thread(createAgentHelper);
}
  
//callback functions
void Fedora::BrokerImpl::createAgentHelper() {
  const char *args[6] = {
    "./agent", "udp4",
    "-p", "2020",
    "-d", "7400"
  };
  eprosima::uxr::AgentInstance& agent_instance = agent_instance.getInstance();
  if (!agent_instance.create(6, const_cast<char**>(args))) {
    std::cout << "Failed to create agent" << std::endl;
    return;
  }
  std::cout << "Starting custom agent" << std::endl;
  agent_instance.run();
  return;
}

bool Fedora::BrokerImpl::onAgentFound(const TransportLocator* locator, void* args) {
  //any needed checking here
  struct FindAgentArgs trueArgs = *((struct FindAgentArgs*)args);
  *(trueArgs.found)=true;
  trueArgs.context->agent_ = *locator;
  return true;
}

void Fedora::BrokerImpl::subscribeCallback(uxrSession* session,
			       uxrObjectId object_id,
			       uint16_t request_id,
			       uxrStreamId stream_id,
			       struct ucdrBuffer* ub,
			       uint16_t length,
			       void* args) {
  for(SubscriberDetails sub : ((BrokerImpl*)args)->subscribers_) {
    if( object_id.id == sub.id ) {
      sub.callback(ub);
    }
  }
}
