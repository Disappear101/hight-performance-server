# Use Ubuntu 22.04 as the base image
FROM ubuntu:22.04

# Set environment variables
ENV DEBIAN_FRONTEND=noninteractive

# Update the package list and install essential packages
RUN apt-get update && \
    apt-get install -y build-essential \
                       libboost-all-dev \
                       ragel \
                       libssl-dev \
                       libjsoncpp-dev \
                       maven \
                       openjdk-11-jdk \
                       libmysqlclient-dev \
                       libtinyxml2-dev \
                       libjemalloc-dev \
                       libsqlite3-dev \
                       git \
                       cmake \
                       autoconf \
                       wget && \
    apt-get clean

# Install yaml-cpp
RUN git clone https://github.com/jbeder/yaml-cpp.git && \
    cd yaml-cpp && \
    cmake -S . -B build && \
    cd build && \
    make && \
    make install

# Install Zookeeper
RUN wget https://downloads.apache.org/zookeeper/zookeeper-3.9.2/apache-zookeeper-3.9.2-bin.tar.gz && \
    tar -xzf apache-zookeeper-3.9.2-bin.tar.gz && \
    cd apache-zookeeper-3.9.2/zookeeper-jute/ && \
    mvn compile && \
    cd ../zookeeper-client/zookeeper-client-c/ && \
    autoreconf -if && \
    ./configure && \
    make -j4

# Install hiredis-vip
RUN git clone https://github.com/vipshop/hiredis-vip.git && \
    cd hiredis-vip && \
    make && \
    make install

# Clean up to reduce image size
RUN rm -rf /var/lib/apt/lists/* \
           /yaml-cpp \
           /apache-zookeeper-3.9.2-bin.tar.gz \
           /apache-zookeeper-3.9.2 \
           /hiredis-vip

# Copy source code and build files into the container
COPY src /app/src
COPY build /app/build
COPY bin /app/bin
COPY CMakeLists.txt /app/

# Set working directory
WORKDIR /app

# Build the project using CMake and Make
RUN cmake -S . -B build && \
    cd build && \
    make

# Set default command to bash
CMD ["/bin/bash"]
