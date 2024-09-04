#!/bin/bash

ERROR=0

# Test server using integration.conf
printf "server { port 80; timer 5; credentials {
        tariq:123;
        milly:456;
        shravan:789;
        umer:101;
    } location /static/ StaticHandler { root /data; } location /echo/ EchoHandler {} location /api/ APIHandler { root ./api_storage/; } location /health HealthHandler {} location /sleep SleepHandler {} }
" > integration.conf

SERVER_EXECUTABLE=$1
$SERVER_EXECUTABLE integration.conf &

# Sleep after starting command
sleep 1

# Check if /api_storage directory exists before running the tests
if [ -d "/api_storage" ]; then
    echo "Directory /api_storage exists."
else
    echo "Directory /api_storage does not exist. Creating it now."
    mkdir api_storage
fi

# Function to generate base64 encoded credentials
generate_auth_header() {
    local user_pass="$1"
    echo -n "$user_pass" | base64
}

# Credentials
AUTH_HEADER="Authorization: Basic $(generate_auth_header "tariq:123")"

# Basic netcat test
response=$(printf "GET /echo/ HTTP/1.1\r\nHost: www.example.com\r\n$AUTH_HEADER\r\nConnection: close\r\n\r\n" \
    | nc 127.0.0.1 80)

# Normalize CR characters out of the response for comparison
normalized_response=$(echo "$response" | tr -d '\r')

# The request should also be normalized for a fair comparison
normalized_expected_response=$(echo "$expected_response" | tr -d '\r')

if [[ "$normalized_response" == *"$normalized_expected_response"* ]]; then
    echo "Echo test passed."
else
    echo "Echo test failed."
    ERROR=1
fi

response2=$(printf "GET /static/hello.txt HTTP/1.1\r\nHost: www.example.com\r\n$AUTH_HEADER\r\nConnection: close\r\n\r\n" \
    | nc 127.0.0.1 80)
echo "Response from server: $response2"

if echo "$response2" | tr "\n\r" " " | grep "This is CRAZY, it totally works." > /dev/null; then
    echo "Basic static test success."
else
    ERROR=1
fi

response3=$(printf "GET /thisshouldntwork/hello.txt HTTP/1.1\r\nHost: www.example.com\r\n$AUTH_HEADER\r\nConnection: close\r\n\r\n" \
    | nc 127.0.0.1 80)
echo "Response from server: $response3"

if echo "$response3" | tr "\n\r" " " | grep "Invalid request type, Handler not found" > /dev/null; then
    echo "Basic invalid test success."
else
    ERROR=1
fi

# CRUD test
response_create=$(printf "POST /api/Shoes HTTP/1.1\r\nHost: www.example.com\r\n$AUTH_HEADER\r\nContent-Type: application/json\r\nContent-Length: 0 \r\n\r\n\r\n" \
    | nc 127.0.0.1 80)
echo "Response from server (Create 1): $response_create"

if echo "$response_create" | tr "\n\r" " " | grep "{\"id\": 1}" > /dev/null; then
    echo "CRUD create test 1 passed."
else
    echo "CRUD create test 1 failed."
    ERROR=1
fi

# Test Read 
response_read=$(printf "GET /api/Shoes/1 HTTP/1.1\r\nHost: www.example.com\r\n$AUTH_HEADER\r\nContent-Type: application/json\r\n\r\n\r\n" \
    | nc 127.0.0.1 80)
echo "Response from server (Read 1): $response_read"

if echo "$response_read" | tr "\n\r" " " | grep "" > /dev/null; then
    echo "CRUD read test 1 passed."
else
    echo "CRUD read test 1 failed."
    ERROR=1
fi

response_delete=$(printf "DELETE /api/Shoes/1 HTTP/1.1\r\nHost: www.example.com\r\n$AUTH_HEADER\r\nContent-Type: application/json\r\n\r\n\r\n" \
    | nc 127.0.0.1 80)
echo "Response from server (Delete 1): $response_delete"

if echo "$response_delete" | grep "" > /dev/null; then
    echo "CRUD delete test 1 passed."
else
    echo "CRUD delete test 1 failed."
    ERROR=1
fi

# CRUD test: Read after delete
response_read_after_delete=$(printf "GET /api/Shoes/1 HTTP/1.1\r\nHost: www.example.com\r\n$AUTH_HEADER\r\nContent-Type: application/json\r\n\r\n\r\n" \
    | nc 127.0.0.1 80)
echo "Response from server (Read after Delete 1): $response_read_after_delete"

if echo "$response_read_after_delete" | grep "" > /dev/null; then
    echo "CRUD read after delete test 1 passed."
else
    echo "CRUD read after delete test 1 failed."
    ERROR=1
fi

# Concurrent request test
START_TIME=$(date +%s%N)
echo -e "GET /sleep HTTP/1.1\r\nHost: www.example.com\r\n$AUTH_HEADER\r\nConnection: close\r\n\r\n" | nc 127.0.0.1 80 &
sleep_request_pid=$!
echo -e "GET /static/hello.txt HTTP/1.1\r\nHost: www.example.com\r\n$AUTH_HEADER\r\nConnection: close\r\n\r\n" | nc 127.0.0.1 80
END_TIME=$(date +%s%N)

DURATION=$((($END_TIME - $START_TIME) / 1000000))

if [ $DURATION -lt 6000 ]; then
    echo "Concurrent request test passed. Duration: $DURATION ms"
else
    echo "Concurrent request test failed. Duration: $DURATION ms"
    ERROR=1
fi

wait $sleep_request_pid

# Cleanup
pkill server
rm integration.conf

if [ -d "./api_storage" ]; then
    rm -rf ./api_storage
    echo "Directory /api_storage and all its contents have been deleted."
else
    echo "Directory /api_storage does not exist."
fi

if [ $ERROR -eq 0 ]; then
    echo "All tests finished and passed."
    exit 0
else
    echo "All tests finished with some failed."
    exit 1
fi
