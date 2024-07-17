FROM alpine

RUN apk update && \
    apk add --no-cache \
        build-base=0.5-r3 \
        cmake=3.24.3-r0


WORKDIR /server

COPY src/ ./src/
COPY include/ ./include/
COPY tests/ ./tests/
COPY libs/ ./tests/
COPY assets/ ./assets/

COPY CMakeLists.txt .
