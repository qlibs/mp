FROM ubuntu:24.04

RUN apt-get update && apt-get install -y software-properties-common
RUN add-apt-repository universe
RUN apt-get update
RUN apt-get install -y g++-14
RUN apt-get install -y python3-pip
RUN pip3 install numpy pandas --break-system-packages
