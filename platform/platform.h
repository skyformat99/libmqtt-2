#ifndef platform_h__included
#define platform_h__included

#include "mqtt_internal.h"

typedef void *(*PlatformTask)(MQTTHandle *handle);

/** maximum receiver buffer size, defined by platform */
extern const size_t max_receive_buffer_size;

typedef enum {
    PlatformStatusOk,    /**< Everything ok */
    PlatformStatusError, /**< Non-recoverable error */
    PlatformStatusRetry  /**< Recoverable error */
} PlatformStatusCode;


/**
 * Initialize platform specific data
 * 
 * @param handle: The handle to initialize
 * @return Platform status code
 */
PlatformStatusCode platform_init(MQTTHandle *handle);

/**
 * Platform specific function to release resources associated with a MQTTHandle
 *
 * @param handle: The handle to clean up
 * @return Platform status code
 */
PlatformStatusCode platform_release(MQTTHandle *handle);

/**
 * Platform specific function to start a reading thread
 * 
 * @param handle: The broker connection handle
 * @param task_handle: Task handle output
 * @param callback: callback to run in the thread
 * @return Platform status code
 */
PlatformStatusCode platform_run_task(MQTTHandle *handle, int *task_handle, PlatformTask callback);

/**
 * Platform specific function to clean up the reading thread
 *
 * @param handle: State handle
 * @param task_handle: Task handle to clean up
 * @return Platform status code
 */
PlatformStatusCode platform_cleanup_task(MQTTHandle *handle, int task_handle);

/**
 * Resolve host
 *
 * @param hostname: Hostname to resolve
 * @param ip_out: resulting IP address if no error occured
 * @return Platform status code
 */
PlatformStatusCode platform_resolve_host(char *hostname, char *ip_out);

/**
 * Connect to host from configuration
 *
 * @param handle: The configuration
 * @return Platform status code
 */
PlatformStatusCode platform_connect(MQTTHandle *handle);

/**
 * Read from the "socket" in the handle
 *
 * @param handle: State handle
 * @param buffer: Read target
 * @return Platform status code
 */
PlatformStatusCode platform_read(MQTTHandle *handle, Buffer *buffer);

/**
 * Write to the "socket" in the handle
 *
 * @param handle: State handle
 * @param buffer: Write source
 * @return Platform status code
 */
PlatformStatusCode platform_write(MQTTHandle *handle, Buffer *buffer);

/**
 * Disconnect the "socket" in the handle
 *
 * @param handle: State handle
 * @return Platform status code
 */
PlatformStatusCode platform_disconnect(MQTTHandle *handle);


#endif /* platform_h__included */
