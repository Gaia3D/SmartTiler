#
# CentOS
#
ARG BUILD_DATE
ARG IMAGE_VERSION
ARG GIT_VERSION
ARG CMAKE_VERSION

FROM centos:${IMAGE_VERSION:-centos7}

LABEL maintainer="Gaia3D Inc. <mago3d@gaia3d.com>" \
		description="Development environment for CentOS" \
		org.label-schema.name="" \
		org.label-schema.build-date=$BUILD_DATE \
		org.label-schema.schema-version="1.0.0"

ENV GIT_VERSION ${GIT_VERSION:-2.30.0}
ENV CMAKE_VERSION ${CMAKE_VERSION:-3.19.2}

# Dependencies for development environment
RUN yum -y install wget centos-release-scl epel-release && \
	yum -y update --exclude=kernel* && \
	yum -y install \
		make gcc-c++ \
		devtoolset-8 \
		openssl-devel curl-devel zlib-devel gettext \
		&& \
	yum clean all && \
    rm -fr /var/cache/yum/*

# Build and install git from source.
WORKDIR /usr/src

RUN wget --no-check-certificate https://www.kernel.org/pub/software/scm/git/git-${GIT_VERSION}.tar.gz && \
	tar xvzf git-${GIT_VERSION}.tar.gz && \
	cd git-${GIT_VERSION} && \
	./configure --with-expat --with-openssl --prefix=/usr/local && \
	make -j$(grep -c processor /proc/cpuinfo) && \
	make install && \
	cd .. && rm -rf git-${GIT_VERSION}*

# Build and install CMake from source.
RUN VERSION="$(cut -d '.' -f 1 <<<"${CMAKE_VERSION}")"."$(cut -d '.' -f 2 <<<"${CMAKE_VERSION}")" && \
	wget --no-check-certificate https://cmake.org/files/v${VERSION}/cmake-${CMAKE_VERSION}.tar.gz && \
	tar xvzf cmake-${CMAKE_VERSION}.tar.gz && \
	cd cmake-${CMAKE_VERSION} && \
	./configure --prefix=/usr/local && \
	make -j$(grep -c processor /proc/cpuinfo) && \
	./bin/cmake \
		-DCMAKE_BUILD_TYPE:STRING=Release \
		-DCMAKE_USE_OPENSSL:BOOL=ON . && \
	make install && \
	cd .. && rm -rf cmake-${CMAKE_VERSION}*

RUN git clone --recursive https://github.com/Gaia3D/SmartTiler.git && \
	cd SmartTiler && \
	mkdir build && cd build && \
	source scl_source enable devtoolset-8 && \
	cmake .. -DCMAKE_INSTALL_PREFIX=/opt/gaia3d/smart-tiler && \
	make -j$(grep -c processor /proc/cpuinfo) && \
	make install && \
	cd ../.. && rm -rf SmartTiler

WORKDIR /opt/gaia3d

ENV PATH /opt/gaia3d/smart-tiler/bin:$PATH
ENV LD_LIBRARY_PATH /opt/gaia3d/smart-tiler/lib:/usr/local/lib:$LD_LIBRARY_PATH

# Set the locale
RUN localedef -i ko_KR -f UTF-8 ko_KR.utf8 && \
	ln -sf /usr/share/zoneinfo/Asia/Seoul /etc/localtime
ENV LANG ko_KR.utf8
ENV LC_ALL ko_KR.utf8

# Enable the SCL for all bash scripts.
# source scl_source enable devtoolset-8
COPY env/enabledevtoolset-8.sh /etc/profile.d/
COPY env/set-locale.sh /etc/profile.d/

RUN chmod 644 /etc/profile.d/set-locale.sh \
	/etc/profile.d/enabledevtoolset-8.sh

CMD ["/bin/bash"]

