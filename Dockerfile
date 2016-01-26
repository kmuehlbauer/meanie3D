FROM ubuntu:latest
MAINTAINER Jürgen Simon (juergen.simon@uni-bonn.de)

RUN sudo apt-get -y update 
RUN sudo apt-get -y upgrade
RUN sudo apt-get -y dist-upgrade
RUN sudo apt-get -y install software-properties-common
RUN sudo add-apt-repository -y ppa:george-edison55/cmake-3.x
RUN sudo apt-get -y update 
RUN sudo apt-get -y install git
RUN sudo apt-get -y install gcc
RUN sudo apt-get -y install g++
RUN sudo apt-get -y install libflann1.8  libflann-dev
RUN sudo apt-get -y install blitz++
RUN sudo apt-get -y install libhdf5-7 libhdf5-dev
RUN sudo apt-get -y install netcdf-bin libnetcdf-dev libnetcdfc++4  
RUN sudo apt-get -y install python python-pip
RUN sudo apt-get -y install python-netcdf
RUN pip install setuptools

RUN sudo apt-get -y install cmake
RUN sudo apt-get -y install zlib1g zlib1g-dev
RUN sudo apt-get -y install libboost1.55-all-dev

# Build NetCDF-CXX (always an extra bloody sausage with this package...)
RUN sudo apt-get -y install wget
RUN wget https://github.com/Unidata/netcdf-cxx4/archive/v4.2.1.tar.gz
RUN tar xvzf v4.2.1.tar.gz
RUN cd netcdf-cxx4-4.2.1 && ./configure && make install && cd ..
RUN rm -rf netcdf-cxx4-4.2.1

# Shapelib
RUN sudo apt-get -y install cvs
RUN cvs -d :pserver:cvsanon@cvs.maptools.org:/cvs/maptools/cvsroot login
RUN cvs -d :pserver:cvsanon@cvs.maptools.org:/cvs/maptools/cvsroot co shapelib 
RUN cd shapelib && make install && cd ..
RUN rm -rf shapelib

# Visualisation
RUN sudo apt-get -y install gnuplot
RUN sudo apt-get -y install --fix-missing vtk6 libvtk6-dev
RUN wget http://portal.nersc.gov/project/visit/releases/2.10.0/visit2_10_0.linux-x86_64-rhel6-wmesa.tar.gz
RUN wget http://portal.nersc.gov/project/visit/releases/2.10.0/visit-install2_10_0
RUN chmod a+x visit-install2_10_0
RUN echo "1" | ./visit-install2_10_0 2.10.0 linux-x86_64-rhel6-wmesa /usr/local/visit
RUN rm -rf visit*

# Meanie3D
RUN git clone http://git.meteo.uni-bonn.de/git/meanie3d
RUN cd meanie3d && git submodule init && git submodule update radolan && cd ..
RUN cd meanie3d && cmake -DWITH_OPENMP=1 -DWITH_VTK=1 -DCMAKE_BUILD_TYPE=Release . && make install && cd ..
RUN rm -rf meanie3d

ENTRYPOINT ["/usr/local/bin/meanie3D"]