Things you should know before hand / step for the developing...

1. Make sure you are in the rocm-terminal container
2. Copy the tool.cpp and CMakeLists.txt file into the rocm-systems/projects/rocprofiler-sdk/source/lib/rocprofiler-sdk-tool/ directory
3. make sure LDMS is built onto the system in /opt/ovis
4. source the ldms_in_a_can.sh script before trying to build rocprofiler-sdk (this is important because some environment variables are set using the script)
5. build rocprofiler-sdk using
   ```
   cmake                                         \
      -B rocprofiler-sdk-build                \
      -DCMAKE_PREFIX_PATH=/opt/rocm           \
      projects/rocprofiler-sdk
   cmake --build rocprofiler-sdk-build --target all --parallel $(nproc)
   ```
6. run the newly built rocprofiler-sdk found in the build directory
   ```
   ./<path to rocprofiler sdk> --log-level info --kernel-trace -i <path to input file> -- <path to application>
   ```
7. check LDMS output in /local/amd_gpu_sampler/gpu_sampler_data/

This was how I particularly ran it on the `morgan` cluster...
```
ssh aamarli@morgan
ssh fat1
cd /projects/AMD_GPU_SAMPLER/workingFolder
podman run -it --device=/dev/kfd --device=/dev/dri --group-add video -v .:/workspace rocprofworking
cd /workspace/
source ldms_in_a_can.sh
cd testing
/workspace/rocm-systems/rocprofiler-sdk-build-dec12/bin/rocprofv3 -i /workspace/input.json --kernel-trace --log-level info -- /workspace/rocHPCG/build/release/bin/rochpcg
cat /local_data/amd_gpu_sampler_data/gpu_sampler_data/amd_gpu_sampler.1765826777 // this may vary depending on the name of the output file
```
