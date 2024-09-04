### Build/test container ###
# Define builder stage
FROM swifties:base as builder

# Share work directory
COPY . /usr/src/project
WORKDIR /usr/src/project/build

# Build and test
RUN cmake -DCMAKE_BUILD_TYPE=Coverage ..
RUN make coverage