MQTT library for multiple platforms including embedded targets.

## Goals for this library

1. Readable code, modern C
2. Simple and fool-proof API
3. High test coverage (aim is >85% of all lines tested)
4. Suitable for embedded or Unixy targets (no static allocation though)

## Current state

- MQTT connection as a client is working
- All packet types are implemented
- Supports MQTT 3.1.1 (aka. protocol level 4)
- Platform support for linux is working
- Test have a coverage of about 92% (lines) and 97% (functions), untested code is just bail-out error handling for fatal errors (usually programming errors or network failure)

## TODO

- [ ] QoS testing, server -> client does not
- [ ] Running in MQTT Broker mode
- [ ] Implement Protocol level 3 (low prio)
- [ ] Implement Draft Protocol level 5 (somewhat low prio as it's a draft spec)
- [ ] Support ESP8266 (RTOS)
- [ ] Support ESP32 (IDF)

And no, I will not do an Arduino port.

## How to use

### Building on Linux

Requirements:

- CMake
- GCC


1. Checkout the repo
2. Run cmake:
```bash
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX:PATH=/ ..
```
3. Build the library
```bash
cd build
make all
```
4. Install/Make a Distribution (Only works in `RELEASE` mode)
```bash
make DESTDIR=/path/to/put/it install
```
The dynamic and static libs will be placed in the `lib/` sub-dir of that folder and the include in `include/mqtt/mqtt.h`

### Testing on Linux

Requirements:

- Local MQTT broker, simplest is mosquitto (can be run by calling `mosquitto -v` in a terminal for debug output)
- `Gcov` and `Lcov` for coverage reports, be aware you'll need a git version of lcov for newer GCC versions (> 6.0.0)

#### Running the tests:

```bash
cd build
make check
```

#### Building a coverage report

**Attention:** Only works in `DEBUG` mode, switch like this:

```bash
cd build
cmake -DCMAKE_BUILD_TYPE=Debug ..
```

Building the report:

```bash
cd build
make coverage
```

Be aware to create the coverage report the tests must run, so you'll need the MQTT broker.

### API

#### Configuration

Create an instance of the config struct as following:

```c
MQTTConfig config = { 0 }; // IMPORTANT: zero out the struct before usage! (this is C99 style)

config.client_id = "my_client";
config.hostname = "localhost";
```

The snippet above is the absolute minimum you'll need, but there's more to configure if you want:

```
typedef struct {
    char *hostname;  /**< Hostname to connect to, will do DNS resolution */
    uint16_t port;   /**< Port the broker listens on, set to 0 for 1883 default */

    char *client_id;    /**< Client identification max 23 chars */
    bool clean_session; /**< Set to true to reset the session on reconnect */

    char *username;  /**< User name, set to NULL to connect anonymously */
    char *password;  /**< Password, set to NULL to connect without password */

    char *last_will_topic;   /**< last will topic that is automatically published on connection loss */
    char *last_will_message; /**< last will message */
    bool last_will_retain;   /**< tell server to retain last will message */
} MQTTConfig;
```

#### Connect to MQTT broker

```c
MQTTHandle *mqtt_connect(MQTTConfig *config, MQTTEventHandler callback, void *callback_context, MQTTErrorHandler error_callback);
```
- `config`: MQTT configuration
- `callback`: Callback function to call on successful connect
- `callback_context`: Just a void pointer that is stored and given to the callback on connection
- `error_callback`: Error handler callback
- Returns handle to mqtt connection or NULL on error

If the error handler is called with Host not found or Connection refused,
the handler is in charge of freeing the handle by returning true
or re-trying by changing settings and calling mqtt_reconnect() and returning false

The error handler should be defined like this:

```c
bool error_handler(MQTTHandle *handle, MQTTConfig *config, MQTTErrorCode code) {

    return true; // kill the handle, return false to keep it
}
```

The event handler looks like this:

```c
void event_handler(MQTTHandle *handle, void *context) {
    // The context pointer is the same as the time when you called `mqtt_connect`
}
```

#### Re-Connect to MQTT broker

```c
MQTTStatus mqtt_reconnect(MQTTHandle *handle, MQTTEventHandler callback, void *callback_context);
```

- `handle`: MQTT Handle from `mqtt_connect`
- `callback`: Callback to call when reconnect was successful
- `callback_context`: Pointer to give the callback
- Returns status code

Usually called in the MQTTErrorHandler callback, if called on a working
connection the connection will be disconnected before reconnecting.

If there were registered subscriptions they will be re-instated after
a successful reconnect.

#### Subscribe to a topic

```c
MQTTStatus mqtt_subscribe(MQTTHandle *handle, char *topic, MQTTQosLevel qos_level, MQTTPublishEventHandler callback);
```
- `handle`: MQTT Handle from `mqtt_connect`
- `topic`: Topic to subscribe
- `qos_level`: Maximum QoS level to subscribe for
- `callback`: Callback function to call when receiving something for that topic
- Returns status code

You may use the same callback function for multiple topics.
The callback function looks like this:

```c
void publish_handler(MQTTHandle *handle, char *topic, char *payload) {
    // the payload is zero terminated by the library, so it can be used as
    // a standard c-string immediately.
}
```

#### Un-Subscribe from a topic

```c
MQTTStatus mqtt_unsubscribe(MQTTHandle *handle, char *topic);
```

- `handle`: MQTT Handle from `mqtt_connect`
- `topic`: Topic to unsubscribe
- Returns status code

#### Publish something to the broker

```c
MQTTStatus mqtt_publish(MQTTHandle *handle, char *topic, char *payload, MQTTQosLevel qos_level, MQTTPublishEventCallback callback);
```

- `handle`: MQTT Handle from `mqtt_connect`
- `topic`: Topic to publish to
- `payload`: Message payload to publish
- `qos_level`: QoS Level for the publish (0 = Fire and forget, 1 = At least once, 2 = One time for sure)
- `callback`: Callback function that is called when publish cleared the QoS handlers
- Returns status code

This uses a c-string as the payload, theoretically the protocol would allow for binary payloads, but this is currently
not supported.

#### Disconnect from MQTT broker

```c
MQTTStatus mqtt_disconnect(MQTTHandle *handle, MQTTEventHandler callback, void *callback_context);
```

- `handle`: MQTT Handle from `mqtt_connect`
- Returns status code

Attention: do not use the handle after calling this function,
all resources will be freed, this handle is now invalid!
