#Ubuntu Base image
FROM ubuntu:latest

#Ensure bash shell is used
SHELL ["/bin/bash", "-c"]

# Source Code goes here
RUN mkdir /usr/src/Fedora/
WORKDIR /usr/src/Fedora/

# oUpdate images
ENV LANG en_US.utf8

# needed to fix TZData hanging
ENV TZ=America/New_York
RUN ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone

# install needed applications
RUN apt-get update && apt-get  -y -qq install cmake \
    	     	    	    g++ \
			    python3-pip \
			    wget \
			    git \
			    libasio-dev \
			    libtinyxml2-dev \
			    libssl-dev \
			    openjdk-8-jdk \
			    curl \
			    unzip \
			    zip \
			    sed

# Install DDS
RUN pip3 install -U colcon-common-extensions vcstool
RUN mkdir /usr/src/dds/
RUN wget https://raw.githubusercontent.com/eProsima/Fast-DDS/master/fastrtps.repos -P /usr/src/dds/
RUN mkdir /usr/src/dds/src/
RUN vcs import /usr/src/dds/src < /usr/src/dds/fastrtps.repos
RUN colcon build --base-paths /usr/src/dds/. --install-base /usr/src/dds/install/ --build-base /usr/src/dds/build
RUN chmod +x /usr/src/dds/install/setup.bash
RUN /usr/src/dds/install/setup.bash
RUN echo 'source ~/Fast-DDS/install/setup.bash' >> ~/.bashrc

# Install DDS-GEN
RUN mkdir /usr/src/ddsGen/
RUN git clone --recursive https://github.com/eProsima/Fast-DDS-Gen.git /usr/src/ddsGen/
RUN /bin/bash -c "cd /usr/src/ddsGen/ && ./gradlew assemble"


#copy over source files
COPY . /usr/src/Fedora/

# Create the messages
RUN mkdir /usr/src/Fedora/messages/
RUN /usr/src/ddsGen/scripts/fastddsgen idl/*.idl -d ./messages/


#Build the app
RUN /bin/bash -c "source /usr/src/dds/install/setup.bash && cmake -B build ."
RUN /bin/bash -c "cd build && make"
#RUN Command
CMD ["./run.sh"]
