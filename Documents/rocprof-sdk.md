# ROCprofiler-SDK 0.4.0 Documentation

## Backgroud information on ROCPprofiler SDK
The rocprofiler SDK is a command line interface that allows users to get low-level accurate performance data on their GPUs

` rocprofv3 ` provides multiple tracing options using the following format

`rocprofv3 <tracing_option> -- <app_relative_path>`


### Tracing Options:
* __HIP (Heterogeneous-computing Interface for Portability) Trace__
    * All functions and activites at the HIP level will be traced 
    * The HIP trace is easier to map to the program 
    * describe this more
    * what limitations are
    * does this allow you to get low level hardware metrics
* __HSA (Hetergenous System Architecture) Trace__
    * According to AMD, users who are are trying to gain a deeper understanding of the application being run 
    * Should be used only when comfortable with HSA runtime
    * describe this more
    * what the limitations are
* __Marker Trace__
    * Allows us to "mark" what counters/metrics we want to keep track of
    * The `ROCKTX` Library provides some API calls for the Marker Trace
    * Important functions include:
        * `roctxMark` - inserts a marker allowing you to see when the line is executed
        * `roctxRangeStart` Defines the start of a range of code 
        * `rocktxRangePush` Starts a new range that is nested within a range
        * `rocktxRangePop` Stops the nested range that we are currently on
        * `rocktxRangeStop` Stops the range of code we are monitoring
    * In general, very useful for debugging specific pieces of code
* __Kernel Trace__
    * Traces the kernel dispatches 
* **Memory Copy Trace**
    * Traces the memory movement during the application runtime
* **Sys Trace** 
    * Combines all the prior mentioned traces
* **Scratch memory trace** 
    * Scratch memory is local memory reserved to the Scrate address space on the GPU
    * With this trace, we can follow when there is a memory allocation, free, or reclaim of scratch memory
* **Stats (flag)**
    * The stats flag or option allows us collect statisticcs for whichever trace type we select allowing us to focus on the function that takes the most time

### Kernel Profiling
Kernel profiling allows us to take a look at the kernel execution details allowing us to select kernels to profile and metrics to collect for every kernel execution

In order to run a kernel profile, we must define an .txt, .json, or .yaml **input file**

The **input file** we must include the metrics that we want to collect **proceeded** with `pmc:` 

There is a hardware limit to the number of metrics we can grab in one pass. If we want to collect more metrics than the hardware permits, we must take multiple executions which we can define in our input file by including multiple rows of `pmc` to define the metrics we collect at each run.

Input file example:
``` 
cat input.txt

pmc: GPUBusy SQ_WAVES
pmc: GRBM_GUI_ACTIVE
```
```
cat input.json

{
    "metrics": [ 
      {
        "pmc":["SQ_WAVES", "GRBM_COUNT", "GUI_ACTIVE"]
      },
      {
        "pmc":["FETCH_SIZE", "WRITE_SIZE"];
      }
    ]
}
```
```
cat input.yaml

metrics:
    - pmc:
        - SQ_WAVES
        - GRBM_COUNT
        - GUI_ACTIVE
    - pmc:
        - FETCH_SIZE 
        - WRITE_SIZE
```

The input file will be supplied by running
`rocprofv3 -i input.txt -- <path_to_application>`

An output file will be generated when running the profiler which produces a `CSV (default)` or you can specify the file type by including `--output-format` for JSON or PFTrace. The output file can be found in `./pmc_n/counter_collection.csv`. Every row in the file represents a kernel execution 

The `--kernel-names` option allows us to filter by kernel name/process ID when running `rocprof`

`rocprofv3 -i input.txt --kernel-names divide_kernel -- <path_to_application>`

### JSON output schema
`rocprofv3` also supports custom JSON output formats making analysis easier.
There is a very long description of the schema properties in the rocprof documentation



## API Refefrence

There are multiple ways to approach and invoke the profiler. In this section, we will cover the different services that `rocprofv3` provides users with

### Buffered Services
Callbacks are recieved via a buffer thread that holds a batch of records. Whenever the buffer is flushed implicitly or explicitly ,`rocprofiler_flush_buffer`, an array of buffer record(s) will be invoked via via a callback to the tool.

* In order to subscribe to the buffer tracing service we must create a buffer for the tracing records and then invoke `rocprofiler_configure_buffer_tracing_service`.

```
rocprofiler_status_t
rocprofiler_create_buffer(rocprofiler_context_id_t        context,
                          size_t                          size,
                          size_t                          watermark,
                          rocprofiler_buffer_policy_t     policy,
                          rocprofiler_buffer_tracing_cb_t callback,
                          void*                           callback_data,
                          rocprofiler_buffer_id_t*        buffer_id);
```
| Parameter | Description |
|:--------:|:-----------:|
|Size       | size of buffer in bytes|
|watermark  | specifes the number of bytes between "flushes"
|policy | specifies behavior when the record is larger than the available space in buffer|
|callback | defines what function should the SDK invoke when flushing the buffer|
|buffer_id | defines the output parameter for the function call|

If you want to configure the buffer tracing service you can use...
```
rocprofiler_status_t
rocprofiler_configure_buffer_tracing_service(rocprofiler_context_id_t          context_id,
                                             rocprofiler_buffer_tracing_kind_t kind,
                                             rocprofiler_tracing_operation_t*  operations,
                                             size_t                            operations_count,
                                             rocprofiler_buffer_id_t           buffer_id);
```
| Parameter | Description |
|:--------:|:-----------:|
|kind      | Specifies the domain that we are going to trace (ie. HIP API, HSA API, Kernal dispatches, etc.)|
|operations | An array that defines which operations within the domain we should trace. If all should be traced, use `nullptr`|
|operations_count | Specifies the number of operations we are tracing. Use `0` if we are tracing all|
|buffer_id | defines the output parameter for the function call|

** NOT COMPLETE **














