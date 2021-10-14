#include "Broker.hpp"
extern "C"{
  #include "Vector.h"
  #include "Magnitude.h"
}

#define STREAM_HISTORY  8
struct FindAgentArgs{
  bool* found;
  Broker* context;
};

  
Broker::Broker(bool clientOnly, uint8_t *outputBuffer, uint32_t outBufferSize, uint8_t *inputBuffer, uint32_t inBufferSize, const char* participant_xml) :
  clientOnly(clientOnly),
  outputBuffer(outputBuffer),
  outputBufferSize(outBufferSize),
  inputBuffer(inputBuffer),
  inputBufferSize(inputBufferSize),
  participant_xml(participant_xml),
  idIncramenter(1)
{
}
void Broker::initialize() {
  connectToAgent();
}
  
uint16_t Broker::initPublisher(const char* topic_xml, const char* publisherXml, const char* dataWriter_xml) {
  PublisherDetails publisher = {
    idIncramenter++,
    topic_xml,
    publisherXml,
    dataWriter_xml
  };
  registerPublisher(publisher);
  publishers.push_back(publisher);
  std::cout << "Publisher created successfully: " << publisher.id << std::endl;
  return publisher.id;
}
void Broker::registerPublisher(PublisherDetails details) {
  uint16_t topic_req = uxr_buffer_create_topic_xml(&session, reliable_out,
						   uxr_object_id(details.id, UXR_TOPIC_ID), participant_id,
						   details.topicXml, UXR_REPLACE);
  uxrObjectId publisherID = uxr_object_id(details.id, UXR_PUBLISHER_ID);
  uint16_t publisher_req = uxr_buffer_create_publisher_xml(&session, reliable_out, publisherID,
							   participant_id, details.publisherXml, UXR_REPLACE);
  uint16_t datawriter_req = uxr_buffer_create_datawriter_xml(&session, reliable_out, 
							     uxr_object_id(details.id, UXR_DATAWRITER_ID),
							     publisherID, details.dataWriterXml, UXR_REPLACE);
  int num_requests = 3;
  uint16_t requests[num_requests] = {topic_req, publisher_req, datawriter_req};
  uint8_t status[num_requests];
  if (!uxr_run_session_until_all_status(&session, 1000, requests, status, num_requests)) {
    std::cout << "Error when registering publishser" << std::endl;
    std::cout << "Topic: " << (int)(status[0]) << std::endl;
    std::cout << "Publisher: " << (int)(status[1]) << std::endl;
    std::cout << "DataWriter: " << (int)(status[2]) << std::endl;
    throw "Error Registering Publisher";
  }
}
void Broker::prepPublish(uint16_t id, ucdrBuffer *serializedBuffer, uint32_t topicSize) {
  uxr_prepare_output_stream(&session, reliable_out, uxr_object_id(id, UXR_DATAWRITER_ID), serializedBuffer, topicSize);
}
uint16_t Broker::initSubscriber(const char* topic_xml, const char* subscriber_xml, const char* dataReader_xml, SUBSCRIBER_CALLBACK) {
  
  SubscriberDetails subscriber = {
    idIncramenter++,
    callback, 
    topic_xml,
    subscriber_xml,
    dataReader_xml
  };
  registerSubscriber(subscriber);
  subscribers.push_back(subscriber);
  std::cout << "Subscriber created successfully: " << subscriber.id << std::endl;
  return subscriber.id;
}
void Broker::registerSubscriber(SubscriberDetails details) {
  uint16_t topic_req = uxr_buffer_create_topic_xml(&session, reliable_out,
						   uxr_object_id(details.id, UXR_TOPIC_ID), participant_id,
						   details.topicXml, UXR_REPLACE);
  uxrObjectId subscriber_id = uxr_object_id(details.id, UXR_SUBSCRIBER_ID);
  uint16_t subscriber_req = uxr_buffer_create_subscriber_xml(&session, reliable_out, subscriber_id, participant_id, details.subscriberXml, UXR_REPLACE);
  
  uxrObjectId datareader_id = uxr_object_id(details.id, UXR_DATAREADER_ID);
  uint16_t datareader_req = uxr_buffer_create_datareader_xml(&session, reliable_out,
							     datareader_id,
							     subscriber_id, details.dataReaderXml, UXR_REPLACE);
  uxrDeliveryControl delivery_control = {
    0
  };
  delivery_control.max_samples = UXR_MAX_SAMPLES_UNLIMITED; 
  uint16_t read_data_req = uxr_buffer_request_data(&session, reliable_out, datareader_id, reliable_in, &delivery_control);
  int num_requests = 3;
  uint16_t requests[num_requests] = {topic_req, subscriber_req, datareader_req};
  uint8_t status[num_requests];
  if (!uxr_run_session_until_all_status(&session, 1000, requests, status, num_requests)) {
    std::cout << "Error when registering subscriber" << std::endl;
    std::cout << "topic: " << (int)(status[0]) << std::endl;
    std::cout << "subscriber: " << (int)(status[1]) << std::endl;
    std::cout << "datareader: " << (int)(status[2]) << std::endl;
    throw "Error Registering Subscriber";
  }
  
}

void Broker::runSession(int ms) {
  std::cout << "new session run" << std::endl;
  bool connected = uxr_run_session_until_confirm_delivery(&session, ms);
  if( !connected ) {
    std::cout << "Detected that Agent is down" << std::endl;
    close();
    connectToAgent();
    connected = uxr_run_session_until_confirm_delivery(&session, ms);
    if( !connected) {
      throw "Unable to connect to agent"; //TODO multiple retries?
    }
    std::cout << "Fail!! --------------" << std::endl;
  }
  else {
    std::cout << "SUCCESS!! --------------" << std::endl;
  }
  std::cout << "session stop" << std::endl;
}
void Broker::close() {
  std::cout << "Closing down Broker" << std::endl;
  uxr_delete_session(&session);
  uxr_close_udp_transport(&transport);
}

bool Broker::findAgent() {
  bool found = false;
  struct FindAgentArgs args = {&found, this};
  uxr_discovery_agents_default(10, 1000, Broker::onAgentFound, (void*)&args);
  if(!found) {
    sleep(1);//try again in a second TODO randomize
    struct FindAgentArgs args = {&found, this};
    uxr_discovery_agents_default(10, 1000, Broker::onAgentFound, (void*)&args);
    if(!found) {
      std::cout << "self booting agent" << std::endl;
      createAgent();
      struct FindAgentArgs args = {&found, this};
      uxr_discovery_agents_default(10, 1000, Broker::onAgentFound, (void*)&args);
    }
  }
  if(!found) {
    std::cout << "Unable to get an agent" <<std::endl;
    throw "unable to find an agent";
  }
  return found;
}
void Broker::connectToAgent() {//TODO refactor, make actual exceptions
  findAgent();
  uxrIpProtocol ip_protocol;
  uint16_t agentPort;
  uxr_locator_to_ip(&agent, agentIp, sizeof(agentIp), &agentPort, &ip_protocol);
  sprintf(port, "%d", agentPort);

  //init transport
  if (!uxr_init_udp_transport(&transport, UXR_IPv4, agentIp, port)) {
    throw "Unable to create Transport";
  }

  //setup session
  uxr_init_session(&session, &transport.comm, 0xAAAABBBB);
  uxr_set_topic_callback(&session, subscribeCallback, this);
  if (!uxr_create_session(&session)) {
    throw "Unable to create Session";
  }
  // initialize buffers for the sesion
  reliable_out = uxr_create_output_reliable_stream(&session, outputBuffer, outputBufferSize, STREAM_HISTORY);
  reliable_in = uxr_create_input_reliable_stream(&session, inputBuffer, inputBufferSize, STREAM_HISTORY);

  //set up the participant
  participant_id = uxr_object_id(0x01, UXR_PARTICIPANT_ID);
  uint16_t participant_req = uxr_buffer_create_participant_xml(&session, reliable_out, participant_id, 0,
							       participant_xml, UXR_REPLACE);
  uint8_t status;
  if (!uxr_run_session_until_all_status(&session, 1000, &participant_req, &status, 1)) {
    std::cout << "Unable to create participant. Error: " << status << std::endl;
    throw "Unable to create participant";
  }
  else {
    std::cout << "Successfully created participant" << std::endl;
    std::cout << &session << std::endl;
    
  }
  //reregister any previous items
  for(PublisherDetails pubDetails : publishers) {
    registerPublisher(pubDetails);
  }
  for(SubscriberDetails subDetails : subscribers) {
    registerSubscriber(subDetails);
  }
  std::cout << "Agent Connection Established and Ready" << std::endl;
  std::cout << "IP: " << agentIp << ":" << (int)agentPort << std::endl;
}
void Broker::createAgent() {
  std::cout << "Creating new agent" << std::endl;
  agentThread = std::thread(createAgentHelper);
}
  
//callback functions
void Broker::createAgentHelper() {
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

bool Broker::onAgentFound(const TransportLocator* locator, void* args) {
  //any needed checking here
  struct FindAgentArgs trueArgs = *((struct FindAgentArgs*)args);
  *(trueArgs.found)=true;
  trueArgs.context->agent = *locator;
  return true;
}

void Broker::subscribeCallback(uxrSession* session,
			       uxrObjectId object_id,
			       uint16_t request_id,
			       uxrStreamId stream_id,
			       struct ucdrBuffer* ub,
			       uint16_t length,
			       void* args) {
  std::cout << "------------------ CALLBACK! ------------------" << std::endl;
  std::cout << "object id: " << object_id.id << std::endl;
  for(SubscriberDetails sub : ((Broker*)args)->subscribers) {
    std::cout << "sub id: " << sub.id << std::endl;
    if( object_id.id == sub.id ) {
      std::cout << "calling callback" << std::endl;
      sub.callback(ub);
    }
  }
  std::cout << "---------------- CALLBACK! END ----------------" << std::endl;
}
