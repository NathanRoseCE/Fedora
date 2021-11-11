FROM ubuntu:latest AS builder

SHELL ["/bin/bash", "-c"]
RUN mkdir /usr/src/Fedora/
WORKDIR /usr/src/

# Update images
RUN apt-get update && apt-get upgrade -y
ENV LANG en_US.utf8

# needed to fix TZData hanging
ENV TZ=America/New_York
RUN ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone
RUN apt-get  -y -qq install cmake \
    	     	    	    g++ \
			    git \
			    openjdk-8-jdk \
			    gradle

# Install XRCE-DDS
RUN mkdir /usr/src/ddsXrce/
RUN git clone https://github.com/eProsima/Micro-XRCE-DDS.git /usr/src/ddsXrce/
RUN mkdir /usr/src/ddsXrce/build/ 
RUN cmake /usr/src/ddsXrce/ -B /usr/src/ddsXrce/build/ -DPROFILE_DISCOVERY=ON
RUN cd /usr/src/ddsXrce/build/ && make install
RUN ldconfig /usr/local/lib/

# Install XRCE-DDS-GEN
RUN mkdir /usr/src/ddsXrceGen/
RUN git clone --recursive https://github.com/eProsima/Micro-XRCE-DDS-Gen.git /usr/src/ddsXrceGen/
RUN cd /usr/src/ddsXrceGen/ && gradle build -Dbranch=v1.2.5
RUN chmod +x /usr/src/ddsXrceGen/scripts/microxrceddsgen

# Build messages
WORKDIR /usr/src/Fedora
RUN mkdir idl build messages src
COPY ./idl /usr/src/Fedora/idl
RUN /usr/src/ddsXrceGen/scripts/microxrceddsgen idl/*.idl -d ./messages/

#Build software
COPY ./src /usr/src/Fedora/src
COPY ./Fedora /usr/src/Fedora/Fedora
COPY ./CMakeLists.txt .
RUN cmake . -B build
RUN cd build && make
RUN cd build && ctest --output-on-failure


FROM ubuntu:latest AS production

RUN mkdir /usr/src/Fedora/build -p
WORKDIR /usr/src/Fedora
COPY --from=builder /usr/src/Fedora/build /usr/src/Fedora/build
COPY --from=builder /usr/local/lib/. /usr/local/lib/.
RUN ldconfig /usr/local/lib/
COPY ./run.sh .

#RUN Command
CMD ["./run.sh"]
