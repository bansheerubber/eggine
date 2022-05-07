# build stage, target 2018 ubuntu
FROM --platform=linux/amd64 ubuntu:bionic as builder

## install build dependencies
RUN apt-get update && \
		DEBIAN_FRONTEND=noninteractive apt-get -y install software-properties-common ca-certificates wget

RUN wget -O - https://apt.kitware.com/keys/kitware-archive-latest.asc 2>/dev/null | apt-key add -

RUN apt-add-repository ppa:ubuntu-toolchain-r/test && \
		apt update && \
		apt install -y gcc-9 g++-9

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

RUN apt-get update && \
    DEBIAN_FRONTEND=noninteractive apt-get install -y gcc build-essential python git autoconf libtool libreadline7 libreadline-dev libglfw3-dev vulkan-utils libpng-dev libxinerama-dev libxcursor-dev libxi-dev libssl-dev libfreetype6-dev libopenal-dev libvorbis-dev libogg-dev libglew-dev libglm-dev libfmt-dev

# move all the stuff over
RUN mkdir /eggine
RUN mkdir /eggine/dist
RUN mkdir /eggine/lib
ADD include /eggine/include

ADD cross-compile/setup.sh /eggine/setup.sh
# setup eggine dependencies
WORKDIR /eggine
RUN chmod +x ./setup.sh
RUN ./setup.sh

ADD resources /eggine/resources
ADD glad /eggine/glad
ADD imgui /eggine/imgui
ADD tools /eggine/tools
ADD Makefile /eggine
ADD src /eggine/src
