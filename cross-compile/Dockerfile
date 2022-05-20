# build stage, target 2018 ubuntu
FROM --platform=linux/amd64 ubuntu:bionic as builder

## install build dependencies
RUN apt-get update && \
	DEBIAN_FRONTEND=noninteractive apt-get -y install software-properties-common ca-certificates wget

RUN wget -O - https://apt.kitware.com/keys/kitware-archive-latest.asc 2>/dev/null | apt-key add -

RUN apt-add-repository ppa:ubuntu-toolchain-r/test && \
	apt-get update && \
	apt-get install -y gcc-9 g++-9

RUN apt-add-repository 'deb https://apt.kitware.com/ubuntu/ bionic main' && \
	apt-get update && \
	DEBIAN_FRONTEND=noninteractive apt-get -y install cmake

RUN apt-add-repository ppa:deadsnakes/ppa && \
	apt-get update && \
	apt-get install -y python3.10

RUN apt-get update && \
	wget -qO - http://packages.lunarg.com/lunarg-signing-key-pub.asc | apt-key add - && \
	wget -qO /etc/apt/sources.list.d/lunarg-vulkan-bionic.list http://packages.lunarg.com/vulkan/lunarg-vulkan-bionic.list && \
	apt-get -y update && \
	DEBIAN_FRONTEND=noninteractive apt-get -y install vulkan-sdk

RUN wget https://bootstrap.pypa.io/get-pip.py && \
	DEBIAN_FRONTEND=noninteractive apt-get -y install python3.10-distutils && \
	python3.10 get-pip.py && \
	pip3.10 install -U setuptools && \
	pip3.10 install pyinotify

RUN apt-get update && \
	DEBIAN_FRONTEND=noninteractive apt-get install -y gcc build-essential python git autoconf libtool libreadline7 libreadline-dev libglfw3-dev vulkan-utils libpng-dev 	libxinerama-dev libxcursor-dev libxi-dev libssl-dev libfreetype6-dev libopenal-dev libvorbis-dev libogg-dev libglew-dev libglm-dev libfmt-dev screen

# move all the stuff over
RUN mkdir /eggine
ADD include /eggine/include
RUN mkdir /eggine/lib

ADD cross-compile/setup.sh /eggine/setup.sh
# setup eggine dependencies
WORKDIR /eggine
RUN chmod +x ./setup.sh
RUN ./setup.sh

RUN mkdir /eggine/dist

ADD .git /eggine/.git
ADD resources /eggine/resources
ADD glad /eggine/glad
ADD imgui /eggine/imgui
ADD tools /eggine/tools
ADD Makefile /eggine
ADD src /eggine/src
ADD cross-compile/auto.py /eggine/auto.py
ADD version.sh /eggine/version.sh

RUN rm imgui/*.o
RUN rm glad/*.o

ENTRYPOINT ["/usr/bin/python3.10", "./auto.py"]
ENTRYPOINT ["/usr/bin/screen", "-S", "compiler", "python3.10", "./auto.py"]
