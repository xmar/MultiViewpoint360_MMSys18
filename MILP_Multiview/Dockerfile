# To work, this compilation require CPLEX version 1271. You have to "mount --bind" the ibm installation folder of your cplex installation into a folder named mount_bind_opt_ibm_here inside the current repository to allow the docker container to use CPLEX
FROM debian:buster as builder

RUN apt-get update &&\
    apt-get install -y libboost-all-dev cmake libgomp1
RUN apt-get install -y build-essential

WORKDIR /home/optimal
COPY . /home/optimal/
RUN mv mount_bind_opt_ibm_here /opt/ibm && mkdir build && cd build && cmake -DCMAKE_MODULE_PATH=. ..    
RUN cd build && make -j

FROM debian:buster

RUN apt-get update &&\                                                           
    apt-get install -y libboost-program-options-dev libboost-serialization-dev libboost-system-dev libboost-filesystem-dev libboost-iostreams-dev libgomp1
COPY --from=builder /home/optimal/build/optimal /bin
WORKDIR /home/optimal
RUN useradd -r -u 1000 -g users xmar
USER xmar
ENTRYPOINT ["optimal"]
