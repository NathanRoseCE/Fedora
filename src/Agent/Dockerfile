#Ubuntu Base image
FROM ubuntu:latest

#Ensure bash shell is used
SHELL ["/bin/bash", "-c"]

# Source Code goes here
RUN mkdir /usr/src/Fedora/
WORKDIR /usr/src/Fedora/

# Update images
RUN apt-get update && apt-get upgrade
ENV LANG en_US.utf8

# needed to fix TZData hanging
ENV TZ=America/New_York
RUN ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone

# install needed applications
RUN apt-get  -y -qq install cmake \
    	     	    	    g++ \
			    git \
			    openjdk-8-jdk \
			    gradle

# Install XRCE-DDS-Agent
RUN mkdir /usr/src/ddsXrce/
RUN git clone https://github.com/eProsima/Micro-XRCE-DDS.git /usr/src/ddsXrce/
RUN mkdir /usr/src/ddsXrce/build/
RUN cmake /usr/src/ddsXrce/ -B /usr/src/ddsXrce/build/
RUN cd /usr/src/ddsXrce/build/ && make install
RUN ldconfig /usr/local/lib/

# Install XRCE-DDS-GEN
RUN mkdir /usr/src/ddsXrceGen/
RUN git clone --recursive https://github.com/eProsima/Micro-XRCE-DDS-Gen.git /usr/src/ddsXrceGen/
RUN cd /usr/src/ddsXrceGen/ && gradle build -Dbranch=v1.2.5
RUN chmod +x /usr/src/ddsXrceGen/scripts/microxrceddsgen

#copy over source files
COPY . /usr/src/Fedora/

# Create the messages
RUN mkdir /usr/src/Fedora/messages/
RUN /usr/src/ddsXrceGen/scripts/microxrceddsgen idl/*.idl -d ./messages/


#Build the app
RUN cmake -B build .
RUN cd build && make

#RUN Command
CMD ["./run.sh"]
