FROM debian:12 AS build
LABEL description="Build the Glimpse server"

ARG VCPKG_VERSION="2024.07.12"
ARG VCPKG_ROOT="/tmp/vcpkg"


RUN apt update && apt install -y \
    autoconf build-essential binutils cmake curl gcc g++ git make ninja-build tar unzip zip wget pkg-config

RUN export VCPKG_FORCE_SYSTEM_BINARIES=1

RUN cd /tmp \
    && git clone https://github.com/Microsoft/vcpkg.git -n \
    && cd vcpkg \
    && git checkout ${VCPKG_VERSION} \
    && VCPKG_FORCE_SYSTEM_BINARIES=1 ./bootstrap-vcpkg.sh

WORKDIR /server

COPY . .

RUN cmake --preset=default

RUN cmake --build build

FROM debian:12-slim AS runtime
LABEL description="Run the Glimpse server"

COPY --from=build /server/build/main /usr/local/bin/glimpse_server

CMD ["glimpse_server"]

EXPOSE 8080
