/******************************************************************************
Copyright (c) 2018 Advanced Micro Devices, Inc. All rights reserved.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*******************************************************************************/

///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// Test tool used as ROC profiler library demo                               //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#include <assert.h>
#include <cxxabi.h>
#include <dirent.h>
#include <hsa.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/syscall.h>   /* For SYS_xxx definitions */
#include <sys/types.h>
#include <unistd.h>

#include <atomic>
#include <chrono>
#include <iostream>
#include <list>
#include <map>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

#include <rocprofiler.h>
#include "util/hsa_rsrc_factory.h"

#include <ldms/ldms.h>
#include <ldms/ldmsd_stream.h>
#include <ldms/ldms_xprt.h>


#define PUBLIC_API __attribute__((visibility("default")))
#define CONSTRUCTOR_API __attribute__((constructor))
#define DESTRUCTOR_API __attribute__((destructor))
#define KERNEL_NAME_LEN_MAX 128

#define ONLOAD_TRACE(str) \
  if (getenv("ROCP_ONLOAD_TRACE")) do { \
    std::cout << "PID(" << GetPid() << "): PROF_TOOL_LIB::" << __FUNCTION__ << " " << str << std::endl << std::flush; \
  } while(0);
#define ONLOAD_TRACE_BEG() ONLOAD_TRACE("begin")
#define ONLOAD_TRACE_END() ONLOAD_TRACE("end")

// Disoatch callback data type
struct callbacks_data_t {
  rocprofiler_feature_t* features;
  unsigned feature_count;
  std::vector<uint32_t>* set;
  unsigned group_index;
  FILE* file_handle;
  int filter_on;
  std::vector<uint32_t>* gpu_index;
  std::vector<std::string>* kernel_string;
  std::vector<uint32_t>* range;
};

// kernel properties structure
struct kernel_properties_t {
  uint32_t grid_size;
  uint32_t workgroup_size;
  uint32_t lds_size;
  uint32_t scratch_size;
  uint32_t vgpr_count;
  uint32_t sgpr_count;
  uint32_t fbarrier_count;
  hsa_signal_t signal;
  uint64_t object;
};

// Context stored entry type
struct context_entry_t {
  bool valid;
  bool active;
  uint32_t index;
  hsa_agent_t agent;
  rocprofiler_group_t group;
  rocprofiler_feature_t* features;
  unsigned feature_count;
  rocprofiler_callback_data_t data;
  kernel_properties_t kernel_properties;
  HsaRsrcFactory::symbols_map_it_t kernel_name_it;
  FILE* file_handle;
};

// verbose mode
static uint32_t verbose = 0;
// Enable tracing
static const bool trace_on = false;
// Tool is unloaded
volatile bool is_loaded = false;
// Dispatch callbacks and context handlers synchronization
pthread_mutex_t mutex = PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;
// Dispatch callback data
callbacks_data_t* callbacks_data = NULL;
// Stored contexts array
typedef std::map<uint32_t, context_entry_t> context_array_t;
context_array_t* context_array = NULL;
// Contexts collected count
volatile uint32_t context_count = 0;
volatile uint32_t context_collected = 0;
// Profiling results output dir
const char* result_prefix = NULL;
// Global results file handle
FILE* result_file_handle = NULL;
// True if a result file is opened
bool result_file_opened = false;
// GPU index filter
std::vector<uint32_t>* gpu_index_vec = NULL;
//  Kernel name filter
std::vector<std::string>* kernel_string_vec = NULL;
// Otstanding dispatches parameters
static uint32_t CTX_OUTSTANDING_WAIT = 1;
static uint32_t CTX_OUTSTANDING_MAX = 0;
// to truncate kernel names
uint32_t to_truncate_names = 0;
// local trace buffer
bool is_trace_local = true;
static inline uint32_t GetPid() { return syscall(__NR_getpid); }
static inline uint32_t GetTid() { return syscall(__NR_gettid); }

uint32_t my_pid = GetPid(); //grabs process ID 

// Error handler
void fatal(const std::string msg) {
  fflush(stdout);
  fprintf(stderr, "%s\n\n", msg.c_str()); //prints out the error encountered
  fflush(stderr);
  abort();
}

// Check returned HSA API status
void check_status(hsa_status_t status) {
  if (status != HSA_STATUS_SUCCESS) {
    const char* error_string = NULL;
    rocprofiler_error_string(&error_string);
    fprintf(stderr, "ERROR: %s\n", error_string);
    abort();
  }
}

//////////////////////////////////////////////////////////////////////////////////////
// Dispatch opt code /////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////
// Context callback arg
struct callbacks_arg_t {
  rocprofiler_pool_t** pools;
};

// Handler callback arg
struct handler_arg_t {
  rocprofiler_feature_t* features;
  unsigned feature_count;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////
// Print profiling results output break if terminal output is enabled
void results_output_break() {
  const bool is_terminal_output = (result_file_opened == false);
  if (is_terminal_output) printf("\nROCprofiler results:\n");
}

// Filtering kernel name
std::string filter_kernel_name(const std::string name) {
  auto rit = name.rbegin(); //grab the last character of the kernel
  auto rend = name.rend(); //grab the first character of the kernel
  uint32_t counter = 0;
  char open_token = 0;
  char close_token = 0;
  while (rit != rend) { //we loop until the first and last character are the same
    if (counter == 0) {
      switch (*rit) {
        case ')':
          counter = 1;
          open_token = ')';
          close_token = '(';
          break;
        case '>':
          counter = 1;
          open_token = '>';
          close_token = '<';
          break;
        case ']':
          counter = 1;
          open_token = ']';
          close_token = '[';
          break;
        case ' ':
          ++rit;
          continue;
      }
      if (counter == 0) break;
    } else {
      if (*rit == open_token) counter++;
      if (*rit == close_token) counter--;
    }
    ++rit;
  }
  auto rbeg = rit;
  while ((rit != rend) && (*rit != ' ') && (*rit != ':')) rit++;
  const uint32_t pos = rend - rit;
  const uint32_t length = rit - rbeg;
  return name.substr(pos, length);
}

// Increment profiling context counter value
uint32_t next_context_count() {
  if (pthread_mutex_lock(&mutex) != 0) {
    perror("pthread_mutex_lock");
    abort();
  }
  ++context_count;
  if (pthread_mutex_unlock(&mutex) != 0) {
    perror("pthread_mutex_unlock");
    abort();
  }
  return context_count;
}

// Allocate entry to store profiling context
context_entry_t* alloc_context_entry() {
  if (CTX_OUTSTANDING_MAX != 0) {
    while((context_count - context_collected) > CTX_OUTSTANDING_MAX) usleep(1000);
  }

  if (pthread_mutex_lock(&mutex) != 0) {
    perror("pthread_mutex_lock");
    abort();
  }

  const uint32_t index = next_context_count() - 1;
  auto ret = context_array->insert({index, context_entry_t{}});
  if (ret.second == false) {
    fprintf(stderr, "context_array corruption, index repeated %u\n", index);
    abort();
  }

  if (pthread_mutex_unlock(&mutex) != 0) {
    perror("pthread_mutex_unlock");
    abort();
  }

  context_entry_t* entry = &(ret.first->second);
  entry->index = index;
  return entry;
}

// Allocate entry to store profiling context
void dealloc_context_entry(context_entry_t* entry) {
  if (pthread_mutex_lock(&mutex) != 0) {
    perror("pthread_mutex_lock");
    abort();
  }

  assert(context_array != NULL);
  context_array->erase(entry->index);

  if (pthread_mutex_unlock(&mutex) != 0) {
    perror("pthread_mutex_unlock");
    abort();
  }
}

// Output profiling results for input features
std::string output_results(const context_entry_t* entry, const char* label) {
  std::stringstream str;
  // FILE* file = entry->file_handle;
  const rocprofiler_feature_t* features = entry->features;
  const unsigned feature_count = entry->feature_count;

  for (unsigned i = 0; i < feature_count; ++i) {
    const rocprofiler_feature_t* p = &features[i];
    // fprintf(file, "  %s ", p->name);
    switch (p->data.kind) {
      // Output metrics results
      case ROCPROFILER_DATA_KIND_INT64:
        // fprintf(file, "(%lu)\n", p->data.result_int64);
        str << ", " << p->name << ":" << p->data.result_int64;
        break;
      case ROCPROFILER_DATA_KIND_DOUBLE:
        // fprintf(file, "(%.10lf)\n", p->data.result_double);
        str << ", " << p->name << ":" << p->data.result_double;
        break;
      default:
        fprintf(stderr, "RPL-tool: undefined data kind(%u)\n", p->data.kind);
        abort();
    }
  }

  return str.str();
}

// Output group intermediate profiling results, created internally for complex metrics
void output_group(const context_entry_t* entry, const char* label) {
  const rocprofiler_group_t* group = &(entry->group);
  context_entry_t group_entry = *entry;
  for (unsigned i = 0; i < group->feature_count; ++i) {
    if (group->features[i]->data.kind == ROCPROFILER_DATA_KIND_INT64 ||
        group->features[i]->data.kind == ROCPROFILER_DATA_KIND_DOUBLE) {
      group_entry.features = group->features[i];
      group_entry.feature_count = 1;
      output_results(&group_entry, label);
    }
  }
}

// Dump stored context entry
bool dump_context_entry(context_entry_t* entry, bool to_clean = true) {
  hsa_status_t status = HSA_STATUS_ERROR;

  volatile std::atomic<bool>* valid = reinterpret_cast<std::atomic<bool>*>(&entry->valid);
  while (valid->load() == false) sched_yield();

  const rocprofiler_dispatch_record_t* record = entry->data.record;
  if (record) {
    if (record->complete == 0) {
      return false;
    }
  }

  ++context_collected;

  const uint32_t index = entry->index;
  if (index != UINT32_MAX) {
    FILE* file_handle = entry->file_handle;
    int bufferSize = 4096;
    char* buffer = (char*) malloc (sizeof(char) * bufferSize);

    char xprt[] = "sock";
    char auth[] = "munge";
    char stream[] = "amd_gpu_sampler";
    ldmsd_stream_type_t typ = LDMSD_STREAM_STRING;
    char port[] = "10544";
    char host[] = "localhost";

    ldms_t ldms = NULL;
    int rc;
    const std::string nik_name = (to_truncate_names == 0) ? entry->data.kernel_name : filter_kernel_name(entry->data.kernel_name);
    const AgentInfo* agent_info = HsaRsrcFactory::Instance().GetAgentInfo(entry->agent);
    hsa_status_t status;
    rocprofiler_time_id_t time_id = ROCPROFILER_TIME_ID_CLOCK_REALTIME;
    uint64_t currTime;
    //rocprofiler_timestamp_t timestamp;
    //rocprofiler_get_timestamp(&timestamp);
    rocprofiler_group_t& group = entry->group;
    if (group.context != NULL) {
      if (entry->feature_count > 0) {
        status = rocprofiler_group_get_data(&group);
        check_status(status);
      	if (verbose == 1) output_group(entry, "group0-data");

        status = rocprofiler_get_metrics(group.context);
        check_status(status);
      }
      std::ostringstream oss;
      oss << index << "__" << filter_kernel_name(entry->data.kernel_name);

      status = rocprofiler_get_time(time_id, index, &currTime, NULL);
      int cx;

      ldms = ldms_xprt_new_with_auth(xprt, NULL, auth, NULL);
      rc = ldms_xprt_connect_by_name(ldms, host, port, NULL, NULL);
      



      cx = snprintf(buffer, bufferSize, "{ \"timestamp\":%u, \"dispatch\":%u, \"gpu-id\":%u, \"queue-id\":%u, \"queue-index\":%lu, \"pid\":%u, \"tid\":%u, \"grd\":%u, \"wgr\":%u, \"lds\":%u, \"scr\":%u, \"vgpr\":%u, \"sgpr\":%u, \"fbar\":%u, \"sig\":\"0x%lx\", \"obj\":\"0x%lx\", \"kernel-name\":\"%s\"%s", 
        currTime,
        index,
        agent_info->dev_index,
        entry->data.queue_id,
        entry->data.queue_index,
        my_pid,
        entry->data.thread_id,
        entry->kernel_properties.grid_size,
        entry->kernel_properties.workgroup_size,
        (entry->kernel_properties.lds_size + (AgentInfo::lds_block_size - 1)) & ~(AgentInfo::lds_block_size - 1),
        entry->kernel_properties.scratch_size,
        (entry->kernel_properties.vgpr_count + 1) * agent_info->vgpr_block_size,
        (entry->kernel_properties.sgpr_count + agent_info->sgpr_block_dflt) * agent_info->sgpr_block_size,
        entry->kernel_properties.fbarrier_count,
        entry->kernel_properties.signal.handle,
        entry->kernel_properties.object,
        nik_name.c_str(),
        output_results(entry, oss.str().substr(0, KERNEL_NAME_LEN_MAX).c_str()).c_str());
      if (record) snprintf(buffer+cx, bufferSize-cx, ", \"dispatch_time\":%lu, \"start\":%lu, \"end\":%lu, \"dispatch_end\":%lu }\n",
        record->dispatch,
        record->begin,
        record->end,
        record->complete);
      else snprintf(buffer+cx, bufferSize-cx, " }\n");
      /*fflush(file_handle); <<< we need to understand this better before we use it */
    
   }
    ldmsd_stream_publish(ldms, stream, typ, buffer, strlen(buffer)+1);
    printf("%s", buffer);
    if (record && to_clean) {
      delete record;
      entry->data.record = NULL;
    }
    
    if (to_clean) free(const_cast<char*>(entry->data.kernel_name));
    if (to_clean) free(buffer);
    // Finishing cleanup
    // Deleting profiling context will delete all allocated resources
    if (to_clean) rocprofiler_close(group.context);
  }

  return true;
}

// Wait for and dump all stored contexts for a given queue if not NULL
void dump_context_array(hsa_queue_t* queue) {
  bool done = false;
  while (done == false) {
    done = true;
    if (pthread_mutex_lock(&mutex) != 0) {
      perror("pthread_mutex_lock");
      abort();
    }

    if (context_array) {
      auto it = context_array->begin();
      auto end = context_array->end();
      while (it != end) {
        auto cur = it++;
        context_entry_t* entry = &(cur->second);
        volatile std::atomic<bool>* valid = reinterpret_cast<std::atomic<bool>*>(&entry->valid);
        while (valid->load() == false) sched_yield();
        if ((queue == NULL) || (entry->data.queue == queue)) {
          if (entry->active == true) {
            if (dump_context_entry(&(cur->second)) == false) done = false;
            else entry->active = false;
          }
        }
      }
    }

    if (pthread_mutex_unlock(&mutex) != 0) {
      perror("pthread_mutex_unlock");
      abort();
    }
    if (done == false) sched_yield();
  }
}

// Profiling completion handler
// Dump and delete the context entry
bool context_handler(rocprofiler_group_t group, void* arg) {
  context_entry_t* entry = reinterpret_cast<context_entry_t*>(arg);

  if (pthread_mutex_lock(&mutex) != 0) {
    perror("pthread_mutex_lock");
    abort();
  }

  bool ret = true;
  if (entry->active == true) {
    ret = dump_context_entry(entry);
    if (ret == false) {
      fprintf(stderr, "tool error: context is not complete\n");
      abort();
    }
  }
  if (ret) dealloc_context_entry(entry);

  if (trace_on) {
    fprintf(stdout, "tool::handler: context_array %d tid %u\n", (int)(context_array->size()), GetTid());
    fflush(stdout);
  }

  if (pthread_mutex_unlock(&mutex) != 0) {
    perror("pthread_mutex_unlock");
    abort();
  }

  return false;
}

// Profiling completion handler
// Dump context entry
bool context_pool_handler(const rocprofiler_pool_entry_t* entry, void* arg) {
  // Context entry
  context_entry_t* ctx_entry = reinterpret_cast<context_entry_t*>(entry->payload);
  handler_arg_t* handler_arg = reinterpret_cast<handler_arg_t*>(arg);
  ctx_entry->features = handler_arg->features;
  ctx_entry->feature_count = handler_arg->feature_count;
  ctx_entry->data.kernel_name = ctx_entry->kernel_name_it->second.name;
  ctx_entry->file_handle = result_file_handle;

  if (pthread_mutex_lock(&mutex) != 0) {
    perror("pthread_mutex_lock");
    abort();
  }

  dump_context_entry(ctx_entry, false);

  if (pthread_mutex_unlock(&mutex) != 0) {
    perror("pthread_mutex_unlock");
    abort();
  }

  HsaRsrcFactory::ReleaseKernelNameRef(ctx_entry->kernel_name_it);

  return false;
}

bool check_filter(const rocprofiler_callback_data_t* callback_data, const callbacks_data_t* tool_data) {
  bool found = true;

  std::vector<uint32_t>* gpu_index = tool_data->gpu_index;
  if (found && gpu_index) {
    found = false;
    for (uint32_t i : *gpu_index) {
      if (i == callback_data->agent_index) {
        found = true;
      }
    }
  }
  std::vector<std::string>* kernel_string  = tool_data->kernel_string;
  if (found && kernel_string) {
    found = false;
    for (const std::string& s : *kernel_string) {
      if (std::string(callback_data->kernel_name).find(s) != std::string::npos) {
        found = true;
      }
    }
  }

  return found;
}

static const amd_kernel_code_t* GetKernelCode(uint64_t kernel_object) {
  const amd_kernel_code_t* kernel_code = NULL;
  hsa_status_t status =
      HsaRsrcFactory::Instance().LoaderApi()->hsa_ven_amd_loader_query_host_address(
          reinterpret_cast<const void*>(kernel_object),
          reinterpret_cast<const void**>(&kernel_code));
  if (HSA_STATUS_SUCCESS != status) {
    kernel_code = reinterpret_cast<amd_kernel_code_t*>(kernel_object);
  }
  return kernel_code;
}

// Setting kernel properties
void set_kernel_properties(const rocprofiler_callback_data_t* callback_data,
                           context_entry_t* entry)
{
  const hsa_kernel_dispatch_packet_t* packet = callback_data->packet;
  kernel_properties_t* kernel_properties_ptr = &(entry->kernel_properties);
  const amd_kernel_code_t* kernel_code = callback_data->kernel_code;

  entry->data = *callback_data;

  if (kernel_code == NULL) {
    const uint64_t kernel_object = callback_data->packet->kernel_object;
    kernel_code = GetKernelCode(kernel_object);
    entry->kernel_name_it = HsaRsrcFactory::AcquireKernelNameRef(kernel_object);
  } else {
    entry->data.kernel_name = strdup(callback_data->kernel_name);
  }

  uint64_t grid_size = packet->grid_size_x * packet->grid_size_y * packet->grid_size_z;
  if (grid_size > UINT32_MAX) abort();
  kernel_properties_ptr->grid_size = (uint32_t)grid_size;
  uint64_t workgroup_size = packet->workgroup_size_x * packet->workgroup_size_y * packet->workgroup_size_z;
  if (workgroup_size > UINT32_MAX) abort();
  kernel_properties_ptr->workgroup_size = (uint32_t)workgroup_size;
  kernel_properties_ptr->lds_size = packet->group_segment_size;
  kernel_properties_ptr->scratch_size = packet->private_segment_size;
  kernel_properties_ptr->vgpr_count = AMD_HSA_BITS_GET(kernel_code->compute_pgm_rsrc1, AMD_COMPUTE_PGM_RSRC_ONE_GRANULATED_WORKITEM_VGPR_COUNT);
  kernel_properties_ptr->sgpr_count = AMD_HSA_BITS_GET(kernel_code->compute_pgm_rsrc1, AMD_COMPUTE_PGM_RSRC_ONE_GRANULATED_WAVEFRONT_SGPR_COUNT);
  kernel_properties_ptr->fbarrier_count = kernel_code->workgroup_fbarrier_count;
  kernel_properties_ptr->signal = callback_data->completion_signal;
  kernel_properties_ptr->object = callback_data->packet->kernel_object;
}

// Kernel disoatch callback
hsa_status_t dispatch_callback(const rocprofiler_callback_data_t* callback_data, void* user_data,
                               rocprofiler_group_t* group) {
  // Passed tool data
  callbacks_data_t* tool_data = reinterpret_cast<callbacks_data_t*>(user_data);
  // HSA status
  hsa_status_t status = HSA_STATUS_ERROR;

  // Checking dispatch condition
  if (tool_data->filter_on == 1) {
    if (check_filter(callback_data, tool_data) == false) {
      next_context_count();
      return HSA_STATUS_SUCCESS;
    }
  }
  // Profiling context
  // Context entry
  context_entry_t* entry = alloc_context_entry();
  // Setting kernel properties
  set_kernel_properties(callback_data, entry);

  // context properties
  rocprofiler_properties_t properties{};
  properties.handler = (result_prefix != NULL) ? context_handler : NULL;
  properties.handler_arg = (void*)entry;

  rocprofiler_feature_t* features = tool_data->features;
  unsigned feature_count = tool_data->feature_count;

  if (tool_data->set != NULL) {
    uint32_t set_offset = 0;
    uint32_t next_offset = 0;
    const auto entry_index = entry->index;
    if (entry_index < (tool_data->set->size() - 1)) {
      set_offset = (*(tool_data->set))[entry_index];
      next_offset = (*(tool_data->set))[entry_index + 1];
    } else {
      set_offset = tool_data->set->back();
      next_offset = feature_count;
    }
    features += set_offset;
    feature_count = next_offset - set_offset;
  }

  // Open profiling context
  rocprofiler_t* context = NULL;
  status = rocprofiler_open(callback_data->agent, features, feature_count,
                            &context, 0 /*ROCPROFILER_MODE_SINGLEGROUP*/, &properties);
  check_status(status);

  // Check that we have only one profiling group
  uint32_t group_count = 0;
  status = rocprofiler_group_count(context, &group_count);
  check_status(status);
  assert(group_count == 1);
  // Get group[0]
  const uint32_t group_index = 0;
  status = rocprofiler_get_group(context, group_index, group);
  check_status(status);

  // Fill profiling context entry
  entry->agent = callback_data->agent;
  entry->group = *group;
  entry->features = features;
  entry->feature_count = feature_count;
  entry->file_handle = tool_data->file_handle;
  entry->active = true;
  reinterpret_cast<std::atomic<bool>*>(&entry->valid)->store(true);

  if (trace_on) {
    fprintf(stdout, "tool::dispatch: context_array %d tid %u\n", (int)(context_array->size()), GetTid());
    fflush(stdout);
  }

  return status;
}

// Kernel disoatch callback
hsa_status_t dispatch_callback_opt(const rocprofiler_callback_data_t* callback_data, void* user_data,
                               rocprofiler_group_t* group) {
  hsa_status_t status = HSA_STATUS_ERROR;
  hsa_agent_t agent = callback_data->agent;
  const unsigned gpu_id = HsaRsrcFactory::Instance().GetAgentInfo(agent)->dev_index;
  callbacks_arg_t* callbacks_arg = reinterpret_cast<callbacks_arg_t*>(user_data);
  rocprofiler_pool_t* pool = callbacks_arg->pools[gpu_id];
  rocprofiler_pool_entry_t pool_entry{};
  status = rocprofiler_pool_fetch(pool, &pool_entry);
  check_status(status);
  // Profiling context entry
  rocprofiler_t* context = pool_entry.context;
  context_entry_t* entry = reinterpret_cast<context_entry_t*>(pool_entry.payload);
  // Setting kernel properties
  set_kernel_properties(callback_data, entry);
  // Get group[0]
  status = rocprofiler_get_group(context, 0, group);
  check_status(status);

  // Fill profiling context entry
  entry->index = UINT32_MAX;
  entry->agent = agent;
  entry->group = *group;

  reinterpret_cast<std::atomic<bool>*>(&entry->valid)->store(true);
  return status;
}

hsa_status_t destroy_callback(hsa_queue_t* queue, void*) {
  results_output_break();
  dump_context_array(queue);
  return HSA_STATUS_SUCCESS;
}

static inline void check_env_var(const char* var_name, uint32_t& val) {
  const char* str = getenv(var_name);
  if (str != NULL ) val = atol(str);
}
static inline void check_env_var(const char* var_name, uint64_t& val) {
  const char* str = getenv(var_name);
  if (str != NULL ) val = atoll(str);
}

// HSA intercepting routines

// HSA unified callback function
hsa_status_t hsa_unified_callback(
  rocprofiler_hsa_cb_id_t id,
  const rocprofiler_hsa_callback_data_t* data,
  void* arg)
{
  printf("hsa_unified_callback(%d, %p, %p):\n", (int)id, data, arg);
  if (data == NULL) abort();

  switch (id) {
    case ROCPROFILER_HSA_CB_ID_ALLOCATE:
      printf("  alloc ptr = %p\n", data->allocate.ptr);
      printf("  alloc size = %zu\n", data->allocate.size);
      printf("  segment type = 0x%x\n", data->allocate.segment);
      printf("  global flag = 0x%x\n", data->allocate.global_flag);
      printf("  is_code = %x\n", data->allocate.is_code);
      break;
    case ROCPROFILER_HSA_CB_ID_DEVICE:
      printf("  device type = 0x%x\n", data->device.type);
      printf("  device id = %u\n", data->device.id);
      printf("  device agent = 0x%lx\n", data->device.agent.handle);
      printf("  assigned ptr = %p\n", data->device.ptr);
      break;
    case ROCPROFILER_HSA_CB_ID_MEMCOPY:
      printf("  memcopy dst = %p\n", data->memcopy.dst);
      printf("  memcopy src = %p\n", data->memcopy.src);
      printf("  memcopy size = %zu\n", data->memcopy.size);
      break;
    case ROCPROFILER_HSA_CB_ID_SUBMIT:
      printf("  packet %p\n", data->submit.packet);
      if (data->submit.kernel_name != NULL) {
        printf("  submit kernel \"%s\"\n", data->submit.kernel_name);
        printf("  device type = %u\n", data->submit.device_type);
        printf("  device id = %u\n", data->submit.device_id);
      }
      break;
    default:
      printf("Unknown callback id(%u)\n", id);
      abort();
  }

  fflush(stdout);
  return HSA_STATUS_SUCCESS;
}

// HSA callbacks structure
rocprofiler_hsa_callbacks_t hsa_callbacks {
  hsa_unified_callback,
  hsa_unified_callback,
  hsa_unified_callback,
  hsa_unified_callback,
  NULL,
  NULL
};

// HSA kernel symbol callback
hsa_status_t hsa_ksymbol_cb(rocprofiler_hsa_cb_id_t id,
                    const rocprofiler_hsa_callback_data_t* data,
                    void* arg)
{
  HsaRsrcFactory::SetKernelNameRef(data->ksymbol.object, data->ksymbol.name, data->ksymbol.unload);
  return HSA_STATUS_SUCCESS;
}

// Tool constructor
extern "C" PUBLIC_API void OnLoadToolProp(rocprofiler_settings_t* settings)
{
  ONLOAD_TRACE_BEG();

  if (pthread_mutex_lock(&mutex) != 0) {
    perror("pthread_mutex_lock");
    abort();
  }
  if (is_loaded) return;
  is_loaded = true;
  if (pthread_mutex_unlock(&mutex) != 0) {
    perror("pthread_mutex_unlock");
    abort();
  }

  // Enable timestamping
  check_env_var("ROCP_TIMESTAMP_ON", settings->timestamp_on);
  is_trace_local = settings->trace_local;

  // Set output file
  result_prefix = getenv("ROCP_OUTPUT_DIR");
  if (result_prefix != NULL) {
    DIR* dir = opendir(result_prefix);
    if (dir == NULL) {
      std::ostringstream errmsg;
      errmsg << "ROCProfiler: Cannot open output directory '" << result_prefix << "'";
      perror(errmsg.str().c_str());
      abort();
    }
    std::ostringstream oss;
    oss << result_prefix << "/" << GetPid() << "_results.txt";
    result_file_handle = fopen(oss.str().c_str(), "w");
    if (result_file_handle == NULL) {
      std::ostringstream errmsg;
      errmsg << "ROCProfiler: fopen error, file '" << oss.str().c_str() << "'";
      perror(errmsg.str().c_str());
      abort();
    }
  } else result_file_handle = stdout;

  result_file_opened = (result_prefix != NULL) && (result_file_handle != NULL);

  const char *dil = " ";

  // Getting metrics
  std::vector<std::string> metrics_vec;
  char *input_metrics = getenv("ROCP_INPUT_METRICS"); //find the roc profiler input metrics environment variable
  char *metric = strtok(input_metrics, dil); //splits the metric into tokens wherever there is a space
  while( metric != NULL ) 
  {
    metrics_vec.push_back(metric); //appends the metric 
    metric = strtok(NULL, dil); // keeps on going through
  }

  // // Getting GPU indexes
  gpu_index_vec = new std::vector<uint32_t>; //similar process a previously 
  char *filter_gpus = getenv("ROCP_FILTER_GPUS");
  char *gpu = strtok(filter_gpus, dil);
  while( metric != NULL ) 
  {
    gpu_index_vec->push_back(atoi(gpu)); //converts information from string to integer and pushes to back
    gpu = strtok(NULL, dil);
  }

  // // Getting kernel names
  kernel_string_vec = new std::vector<std::string>;
  char *filter_kernels = getenv("ROCP_FILTER_KERNELS");  //so it seems important to grab the environment variable names
  char *kernel = strtok(filter_kernels, dil);
  while( kernel != NULL ) 
  {
    kernel_string_vec->push_back(kernel);
    kernel = strtok(NULL, dil);
  }

  const bool filter_disabled = (gpu_index_vec->empty() && kernel_string_vec->empty());

  const unsigned feature_count = metrics_vec.size();
  rocprofiler_feature_t* features = new rocprofiler_feature_t[feature_count];
  memset(features, 0, feature_count * sizeof(rocprofiler_feature_t)); //replaces the first couple of chars with NULL?

  printf("  %d metrics\n", (int)metrics_vec.size());
  for (unsigned i = 0; i < metrics_vec.size(); ++i) {
    const std::string& name = metrics_vec[i];
    printf("%s%s", (i == 0) ? "    " : ", ", name.c_str());
    features[i] = {};
    features[i].kind = ROCPROFILER_FEATURE_KIND_METRIC;
    features[i].name = strdup(name.c_str());
  }
  if (metrics_vec.size()) printf("\n");

  const uint32_t features_found = metrics_vec.size();

  // Context array aloocation
  context_array = new context_array_t;

  bool opt_mode_cond = ((features_found != 0) &&
                              (filter_disabled == true));
  if (settings->opt_mode == 0) opt_mode_cond = false;
  if (!opt_mode_cond) settings->opt_mode = 0;
  if (opt_mode_cond) {
    // Handler arg
    handler_arg_t* handler_arg = new handler_arg_t{};
    handler_arg->features = features;
    handler_arg->feature_count = feature_count;

    // Context properties
    rocprofiler_pool_properties_t properties{};
    properties.num_entries = (CTX_OUTSTANDING_MAX != 0) ? CTX_OUTSTANDING_MAX : 1000;
    properties.payload_bytes = sizeof(context_entry_t);
    properties.handler = context_pool_handler;
    properties.handler_arg = handler_arg;

    // Available GPU agents
    const unsigned gpu_count = HsaRsrcFactory::Instance().GetCountOfGpuAgents();
    callbacks_arg_t* callbacks_arg = new callbacks_arg_t{};
    callbacks_arg->pools = new rocprofiler_pool_t* [gpu_count];
    for (unsigned gpu_id = 0; gpu_id < gpu_count; gpu_id++) {
      // Getting GPU device info
      const AgentInfo* agent_info = NULL;
      if (HsaRsrcFactory::Instance().GetGpuAgentInfo(gpu_id, &agent_info) == false) {
        fprintf(stderr, "GetGpuAgentInfo failed\n");
        abort();
      }

      // Open profiling pool
      rocprofiler_pool_t* pool = NULL;
      hsa_status_t status = rocprofiler_pool_open(agent_info->dev_id, features, features_found,
                                                  &pool, 0, &properties);
      check_status(status);
      callbacks_arg->pools[gpu_id] = pool;
    }

    //Adding dispatch observer
    rocprofiler_queue_callbacks_t callbacks_ptrs{0};
    callbacks_ptrs.dispatch = dispatch_callback_opt;
    callbacks_ptrs.destroy = destroy_callback;

    rocprofiler_set_queue_callbacks(callbacks_ptrs, callbacks_arg);

    rocprofiler_hsa_callbacks_t cs{};
    cs.ksymbol = hsa_ksymbol_cb;
    rocprofiler_set_hsa_callbacks(cs, NULL);
    settings->code_obj_tracking = 0;
    settings->hsa_intercepting = 0;
  } else {
    // Adding dispatch observer
    rocprofiler_queue_callbacks_t callbacks_ptrs{0};
    callbacks_ptrs.dispatch = dispatch_callback;
    callbacks_ptrs.destroy = destroy_callback;

    callbacks_data = new callbacks_data_t{};
    callbacks_data->features = features;
    callbacks_data->feature_count = features_found;
    callbacks_data->set = NULL;
    callbacks_data->group_index = 0;
    callbacks_data->file_handle = result_file_handle;
    callbacks_data->gpu_index = (gpu_index_vec->empty()) ? NULL : gpu_index_vec;
    callbacks_data->kernel_string = (kernel_string_vec->empty()) ? NULL : kernel_string_vec;
    callbacks_data->range = NULL;
    callbacks_data->filter_on = (callbacks_data->gpu_index != NULL) ||
                                (callbacks_data->kernel_string != NULL) ||
                                (callbacks_data->range != NULL)
                                ? 1 : 0;

    rocprofiler_set_queue_callbacks(callbacks_ptrs, callbacks_data);
  }

  ONLOAD_TRACE_END();
}

// Tool destructor
void rocprofiler_unload(bool is_destr) {
  ONLOAD_TRACE("begin loaded(" << is_loaded << ") destr(" << is_destr << ")");

  if (pthread_mutex_lock(&mutex) != 0) {
    perror("pthread_mutex_lock");
    abort();
  }
  if (!is_loaded) return;
  is_loaded = false;
  if (pthread_mutex_unlock(&mutex) != 0) {
    perror("pthread_mutex_unlock");
    abort();
  }

  // Unregister dispatch callback
  rocprofiler_remove_queue_callbacks();

  // Dump stored profiling output data
  fflush(stdout);
  if (result_file_opened) {
    printf("\nROCPRofiler:"); fflush(stdout);
    if (CTX_OUTSTANDING_WAIT == 1) dump_context_array(NULL);
    fclose(result_file_handle);
    printf(" %u contexts collected, output directory %s AARON1 WAS HERE<<<<<\n", context_collected, result_prefix);
  } else {
    if (context_collected != context_count) {
      results_output_break();
      if (CTX_OUTSTANDING_WAIT == 1) dump_context_array(NULL);
    }
    printf("\nROCPRofiler: %u contexts collected AARON2 WAS HERE<<<<<\n", context_collected);
  }
  fflush(stdout);

  // Cleanup
  if (callbacks_data != NULL) {
    delete[] callbacks_data->features;
    delete callbacks_data;
    callbacks_data = NULL;
  }
  delete gpu_index_vec;
  gpu_index_vec = NULL;
  delete kernel_string_vec;
  kernel_string_vec = NULL;
  delete context_array;
  context_array = NULL;

  ONLOAD_TRACE_END();
}

extern "C" PUBLIC_API void OnUnloadTool() {
  ONLOAD_TRACE("begin loaded(" << is_loaded << ")");
  if (is_loaded == true) rocprofiler_unload(false);
  ONLOAD_TRACE_END();
}

extern "C" DESTRUCTOR_API void destructor() {
  ONLOAD_TRACE("begin loaded(" << is_loaded << ")");
  if (is_loaded == true) rocprofiler_unload(true);
  ONLOAD_TRACE_END();
}
