# ROCprofiler-SDK 0.4.0 Documentation

### Backgroud information on ROCPprofiler SDK
The rocprofiler SDK is a command line interface that allows users to get low-level accurate performance data on their GPUs

` rocprofv3 ` provides multiple tracing options using the following format

`rocprofv3 <tracing_option> -- <app_relative_path>`


Tracing Options:
* __HIP (Heterogeneous-computing Interface for Portability) Trace__
    * All functions and activites at the HIP level will be traced 
    * The HIP trace is easier to map to the program 
* __HSA (Hetergenous System Architecture) Trace__
    * According to AMD, users who are are trying to gain a deeper understanding of the application being run 
    * Should be used only when comfortable with HSA runtime
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
* Kernel Trace 
    * Traces the kernel dispatches 
* Memory Copy Trace 
    * Traces the memory movement during the application runtime
* Sys Trace 
    * Combines all the prior mentioned traces
* Scratch memory trace 

