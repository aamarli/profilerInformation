<h1> tool.cpp </h1>
We have three different tool.cpp files we can look at
*rocprof src/tool.cpp
*rocprof test/tool.cpp
*Simon's tool.cpp

<h2> How They Compare </h2>
Simon's tool.cpp file is very similar to rocprof's test/tool.cpp file
I think in the header notes on the file, Simon's is just AMD's 2018 version of the tool.cpp file, while the rocprof test/tool.cpp is the 2022 version.
So outside of the few optimizations, they functionally fit the same purpose.

As for the src/tool.cpp file found only in rocprof's implementation, there is practically no similarity when compared to either Simon's file or the file found in the test directory.
My interpretation is that this is overarching source for their profiling, while the test/tool.cpp is used specifically to run a test trial. I'm mainly confused as to why Simon uses the "test/tool" file from 2018 rather than the "src/tool" file
And how does this make a difference?


<h2> dlopen </h2>
In the util directories in the parent directory for all three of the files we are looking at, there is `hsa_rsrc_factory.cpp` which contains one `dlopen` call.

`dlopen` can be used to load and link shared libraries at runtime instead of load time. For example, if we only wanted to load a library if a particular conditional was passed, we would implement `dlopen`

```
HsaRsrcFactory::HsaRsrcFactory(bool initialize_hsa) : initialize_hsa_(initialize_hsa) {
  hsa_status_t status;

  cpu_pool_ = nullptr;
  kern_arg_pool_ = nullptr;

  InitHsaApiTable(nullptr);

  // Initialize the Hsa Runtime
  if (initialize_hsa_) {
    status = hsa_api_.hsa_init();
    CHECK_STATUS("Error in hsa_init", status);
  }

  // Discover the set of Gpu devices available on the platform
  status = hsa_api_.hsa_iterate_agents(GetHsaAgentsCallback, this);
  CHECK_STATUS("Error Calling hsa_iterate_agents", status);
  if (cpu_pool_ == nullptr) CHECK_STATUS("CPU memory pool is not found", HSA_STATUS_ERROR);
  if (kern_arg_pool_ == nullptr)
    CHECK_STATUS("Kern-arg memory pool is not found", HSA_STATUS_ERROR);

  // Get AqlProfile API table
  aqlprofile_api_ = {};
#ifdef ROCP_LD_AQLPROFILE
  status = LoadAqlProfileLib(&aqlprofile_api_);
#else
  status = hsa_api_.hsa_system_get_major_extension_table(HSA_EXTENSION_AMD_AQLPROFILE,
                                                         hsa_ven_amd_aqlprofile_VERSION_MAJOR,
                                                         sizeof(aqlprofile_api_), &aqlprofile_api_);
#endif
  CHECK_STATUS("aqlprofile API table load failed", status);

  // Get Loader API table
  loader_api_ = {};
  status = hsa_api_.hsa_system_get_major_extension_table(HSA_EXTENSION_AMD_LOADER, 1,
                                                         sizeof(loader_api_), &loader_api_);
  CHECK_STATUS("loader API table query failed", status);

  // Instantiate HSA timer
  timer_ = new HsaTimer(&hsa_api_);
  CHECK_STATUS("HSA timer allocation failed",
               (timer_ == nullptr) ? HSA_STATUS_ERROR : HSA_STATUS_SUCCESS);

  // Time correlation
  const uint32_t corr_iters = 1000;
  for (unsigned time_id = 0; time_id < HsaTimer::TIME_ID_NUMBER; time_id += 1) {
    CorrelateTime((HsaTimer::time_id_t)time_id, corr_iters);
  }

  // System timeout
  timeout_ =
      (timeout_ns_ == HsaTimer::TIMESTAMP_MAX) ? timeout_ns_ : timer_->ns_to_sysclock(timeout_ns_);

  // To dump code objects
  to_dump_code_obj_ = getenv("ROCP_DUMP_CODEOBJ");
}

hsa_status_t HsaRsrcFactory::LoadAqlProfileLib(aqlprofile_pfn_t* api) {
  void* handle = dlopen(kAqlProfileLib, RTLD_NOW);
  if (handle == nullptr) {
    fprintf(stderr, "Loading '%s' failed, %s\n", kAqlProfileLib, dlerror());
    return HSA_STATUS_ERROR;
  }
  dlerror(); /* Clear any existing error */

  api->hsa_ven_amd_aqlprofile_error_string =
      (decltype(::hsa_ven_amd_aqlprofile_error_string)*)dlsym(
          handle, "hsa_ven_amd_aqlprofile_error_string");
  api->hsa_ven_amd_aqlprofile_validate_event =
      (decltype(::hsa_ven_amd_aqlprofile_validate_event)*)dlsym(
          handle, "hsa_ven_amd_aqlprofile_validate_event");
  api->hsa_ven_amd_aqlprofile_start =
      (decltype(::hsa_ven_amd_aqlprofile_start)*)dlsym(handle, "hsa_ven_amd_aqlprofile_start");
  api->hsa_ven_amd_aqlprofile_stop =
      (decltype(::hsa_ven_amd_aqlprofile_stop)*)dlsym(handle, "hsa_ven_amd_aqlprofile_stop");
  api->hsa_ven_amd_aqlprofile_read =
      (decltype(::hsa_ven_amd_aqlprofile_read)*)dlsym(handle, "hsa_ven_amd_aqlprofile_read");
  api->hsa_ven_amd_aqlprofile_legacy_get_pm4 =
      (decltype(::hsa_ven_amd_aqlprofile_legacy_get_pm4)*)dlsym(
          handle, "hsa_ven_amd_aqlprofile_legacy_get_pm4");
  api->hsa_ven_amd_aqlprofile_get_info = (decltype(::hsa_ven_amd_aqlprofile_get_info)*)dlsym(
      handle, "hsa_ven_amd_aqlprofile_get_info");
  api->hsa_ven_amd_aqlprofile_iterate_data =
      (decltype(::hsa_ven_amd_aqlprofile_iterate_data)*)dlsym(
          handle, "hsa_ven_amd_aqlprofile_iterate_data");
  api->hsa_ven_amd_aqlprofile_att_marker = (decltype(::hsa_ven_amd_aqlprofile_att_marker)*)
                                          dlsym(handle, "hsa_ven_amd_aqlprofile_att_marker");
  return HSA_STATUS_SUCCESS;
}
```

The two parts of this code that I want to highlight are the #IFDEF statement found in the constructor, that if true, calls the `LoadAqlProfileLib` function which ends up calling the `dlopen`