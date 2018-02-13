#include <node_api.h>
#include <sys/select.h>
#include <string.h>
#include <errno.h>
#define CHECK_STATUS \
if (status != napi_ok) { \
  napi_throw_error(env, nullptr, "N-API call failed"); \
  return nullptr; \
}

napi_value Select(napi_env env, napi_callback_info info) {
  napi_status status;

  size_t argc = 5;
  napi_value args[5];
  status = napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
  CHECK_STATUS;

  if (argc < 5) {
    napi_throw_type_error(env, nullptr, "Wrong number of arguments");
    return nullptr;
  }

  napi_valuetype type;
  status = napi_typeof(env, args[0], &type);
  CHECK_STATUS;
  
  if (type != napi_number) {
    napi_throw_type_error(env, nullptr, "nfds should be an integer");
    return nullptr;
  } 
  
  int nfds;
  status = napi_get_value_int32(env, args[0], &nfds);
  CHECK_STATUS;
  
  bool isArray;
  
  status = napi_is_array(env, args[1], &isArray);
  CHECK_STATUS;
  if (!isArray) {
    napi_throw_type_error(env, nullptr, "readFds should be an array of file descriptors");
    return nullptr;
  }
  status = napi_is_array(env, args[2], &isArray);
  CHECK_STATUS;
  if (!isArray) {
    napi_throw_type_error(env, nullptr, "writeFds should be an array of file descriptors");
    return nullptr;
  }
  status = napi_is_array(env, args[3], &isArray);
  CHECK_STATUS;  
  if (!isArray) {
    napi_throw_type_error(env, nullptr, "exceptFds should be an array of file descriptors");
    return nullptr;
  }

  fd_set readfds, writefds, exceptfds;
  napi_value value;
  uint32_t i, args1l, args2l, args3l;
  int32_t fd;
  
  FD_ZERO(&readfds);
  status = napi_get_array_length(env, args[1], &args1l);
  CHECK_STATUS;
  
  for(i = 0; i < args1l; i++){
    status = napi_get_element(env, args[1], i, &value);
    CHECK_STATUS;
    status = napi_get_value_int32(env, value, &fd);
    CHECK_STATUS;
    FD_SET(fd, &readfds);
  }
  
  FD_ZERO(&writefds);
  status = napi_get_array_length(env, args[2], &args2l);
  CHECK_STATUS;
  
  for(i = 0; i < args2l; i++){
    status = napi_get_element(env, args[2], i, &value);
    CHECK_STATUS;
    status = napi_get_value_int32(env, value, &fd);
    CHECK_STATUS;
    FD_SET(fd, &writefds);
  }
  
  FD_ZERO(&exceptfds);
  status = napi_get_array_length(env, args[3], &args3l);
  CHECK_STATUS;
  
  for(i = 0; i < args3l; i++){
    status = napi_get_element(env, args[3], i, &value);
    CHECK_STATUS;
    status = napi_get_value_int32(env, value, &fd);
    CHECK_STATUS;
    FD_SET(fd, &exceptfds);
  }
  
  status = napi_typeof(env, args[4], &type);
  CHECK_STATUS;

  int retval;
  
  if (type == napi_null) {
    retval = select(nfds, &readfds, &writefds, &exceptfds, NULL);
  } else if(type == napi_number){
    struct timeval timeout;
    int ms;
    status = napi_get_value_int32(env, args[4], &ms);
    CHECK_STATUS;
    
    timeout.tv_sec = ms/1000;
    timeout.tv_usec = ms%1000*1000;
    retval = select(nfds, &readfds, &writefds, &exceptfds, &timeout);
  }else{
    napi_throw_type_error(env, nullptr, "timeout should be an integer ms or null");
    return nullptr;
  }
  
  napi_value result, readfdsArray, writefdsArray, exceptfdsArray;
  status = napi_create_array_with_length(env, 3, &result);
  CHECK_STATUS;
  status = napi_create_array(env, &readfdsArray);
  CHECK_STATUS;
  status = napi_create_array(env, &writefdsArray);
  CHECK_STATUS;
  status = napi_create_array(env, &exceptfdsArray);
  CHECK_STATUS;
  
  if (retval == -1){
    napi_throw_error(env, nullptr, strerror(errno));
    return nullptr;
  }
  
  if (retval){
    for(i = 0; i < args1l; i++){
      status = napi_get_element(env, args[1], i, &value);
      CHECK_STATUS;
      status = napi_get_value_int32(env, value, &fd);
      CHECK_STATUS;
      if (FD_ISSET(fd, &readfds)){
        napi_set_element(env, readfdsArray, i, value);
        CHECK_STATUS;
      }
    }
    
    for(i = 0; i < args2l; i++){
      status = napi_get_element(env, args[2], i, &value);
      CHECK_STATUS;
      status = napi_get_value_int32(env, value, &fd);
      CHECK_STATUS;
      if (FD_ISSET(fd, &readfds)){
        napi_set_element(env, writefdsArray, i, value);
        CHECK_STATUS;
      }
    }
    
    for(i = 0; i < args3l; i++){
      status = napi_get_element(env, args[3], i, &value);
      CHECK_STATUS;
      status = napi_get_value_int32(env, value, &fd);
      CHECK_STATUS;
      if (FD_ISSET(fd, &readfds)){
        napi_set_element(env, exceptfdsArray, i, value);
        CHECK_STATUS;
      }
    }
  }
  
  status = napi_set_element(env, result, 0, readfdsArray);
  CHECK_STATUS;
  status = napi_set_element(env, result, 1, writefdsArray);
  CHECK_STATUS;
  status = napi_set_element(env, result, 2, exceptfdsArray);
  CHECK_STATUS;
  return result;
}

napi_value Init(napi_env env, napi_value exports) {
  napi_status status;
  napi_value select;
  status = napi_create_function(env, "exports", NAPI_AUTO_LENGTH, Select, NULL, &select);
  CHECK_STATUS;
  return select;
}

NAPI_MODULE(NODE_GYP_MODULE_NAME, Init)