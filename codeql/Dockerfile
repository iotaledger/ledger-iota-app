FROM ghcr.io/ledgerhq/ledger-app-builder/ledger-app-builder:latest

# switch to non-interactive
RUN echo 'debconf debconf/frontend select Noninteractive' | debconf-set-selections

RUN apt-get update && \
    apt-get install -y --no-install-recommends \
    git wget unzip sed

WORKDIR /opt

ARG CODEQL_CLI_VERSION="v2.9.2"

RUN wget https://github.com/github/codeql-cli-binaries/releases/download/${CODEQL_CLI_VERSION}/codeql-linux64.zip -O codeql.zip && \
    unzip codeql.zip && \
    rm codeql.zip

ENV PATH="/opt/codeql:${PATH}"

RUN codeql pack download codeql/cpp-queries

ENV BOLOS_SDK="/app/ledger-iota-app/dev/sdk/nanos-secure-sdk"
ENV TARGET_NAME="TARGET_NANOS"

COPY codeql-local.sh /app/codeql-local.sh
RUN chmod +x /app/codeql-local.sh

WORKDIR /app


