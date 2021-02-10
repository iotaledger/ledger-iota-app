FROM ubuntu:20.04

ENV DEVICE=nanos

WORKDIR /root

COPY repository.url .
COPY install_all.sh .

RUN chmod a+x install_all.sh

RUN ./install_all.sh DOCKER

# expose port
EXPOSE 9999



