#include "Broker.hpp"

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
    0x01,
    topic_xml,
    publisherXml,
    dataWriter_xml
  };
  publishers.push_back(publisher);
  registerPublisher(publisher);
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
  uint8_t status[num_requests];
  uint16_t requests[num_requests] = {topic_req, publisher_req, datawriter_req};
  if (!uxr_run_session_until_all_status(&session, 1000, requests, status, num_requests)) {
    std::cout << "Session: " << &session << std::endl;
    std::cout << "Error when registering publishser" << std::endl;
    std::cout << "Topic: " << (int)(status[0]) << std::endl;
    std::cout << "Publisher: " << (int)(status[1]) << std::endl;
    std::cout << "DataWriter: " << (int)(status[2]) << std::endl;
    throw "Error Registering Publisher";
  }
}
void Broker::initSubscriber(const char* topic_xml, const char* dataReader_xml, SUBSCRIBER_CALLBACK) {
  throw "NOT IMPLIMENTED";
}
void Broker::prepPublish(uint16_t id, ucdrBuffer *serializedBuffer, uint32_t topicSize) {
  uxr_prepare_output_stream(&session, reliable_out, uxr_object_id(id, UXR_DATAWRITER_ID), serializedBuffer, topicSize);
}
void Broker::registerSubscriber(SubscriberDetails details) {
  throw "NOT IMPLIMENTED";
}

void Broker::runSession(int ms) {
  bool connected = uxr_run_session_time(&session, ms);
  if( !connected ) {
    std::cout << "Detected that Agent is down" << std::endl;
    connectToAgent();
    connected = uxr_run_session_time(&session, ms);
    if( !connected) {
      throw "Unable to connect to agent"; //TODO multiple retries?
    }
  }
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
  return found;
}
void Broker::connectToAgent() {//TODO refactor, make actual exceptions
  if(!findAgent()) {
    //wait random period of time
    //start agent thread
    //Find its ip
  }
  uxrIpProtocol ip_protocol;
  char agentIp[16];//TODO update to handle IPV6
  uint16_t agentPort;
  uxr_locator_to_ip(&agent, agentIp, sizeof(agentIp), &agentPort, &ip_protocol);
  char port[10];
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
  uxrObjectId participant_id = uxr_object_id(0x01, UXR_PARTICIPANT_ID);
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

void Broker::overallSubscriberCallback() {
  throw "Not implimented";
}
void Broker::subcribeCallback() {
  throw "Not implimented";
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

void Broker::subscribeCallback(uxrSession* session, uxrObjectId object_id, uint16_t request_id,
				      uxrStreamId stream_id, struct ucdrBuffer* ub, uint16_t length,
				      void* args) {
  for(SubscriberDetails sub : ((Broker*)args)->subscribers) {
    if( object_id.id == sub.id ) {
      sub.callback(ub);
    }
  }
}
