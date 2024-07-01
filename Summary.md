Metrics (Some of the descriptions on https://rocm.docs.amd.com/en/latest/conceptual/gpu-arch/mi300-mi200-performance-counters.html# are better)
ALUStalledByLDS   The percentage of GPUTime ALU units are stalled by the LDS input queue being full or the output queue being not ready. If there are LDS bank conflicts, reduce them. Otherwise, try reducing the number of LDS accesses if possible. Value range
FetchSize or FETCH_SIZE  The total kilobytes fetched from the video memory. This is measured with all extra fetches and any cache or memory effects taken into account.
FlatLDSInsts   The average number of FLAT instructions that read or write to LDS executed per work item (affected by flow control).
FlatVMemInsts   The average number of FLAT instructions that read from or write to the video memory executed per work item (affected by flow control). Includes FLAT instructions that read from or write to scratch.
GDSInsts   The average number of GDS read or GDS write instructions executed per work item (affected by flow control).
GPUBusy   The percentage of time GPU was busy.
L2CacheHit   The percentage of fetch, write, atomic, and other instructions that hit the data in L2 cache. Value range
LDSBankConflict   The percentage of GPUTime LDS is stalled by bank conflicts. Value range
LDSInsts   The average number of LDS read or LDS write instructions executed per work item (affected by flow control).  Excludes FLAT instructions that read from or write to LDS.
MeanOccupancyPerActiveCU   Mean occupancy per active compute unit.
MeanOccupancyPerCU   Mean occupancy per compute unit.
MemUnitBusy   The percentage of GPUTime the memory unit is active. The result includes the stall time (MemUnitStalled). This is measured with all extra fetches and writes and any cache or memory effects taken into account. Value range
MemUnitStalled   The percentage of GPUTime the memory unit is stalled. Try reducing the number or size of fetches and writes if possible. Value range
MemWrites32B   The total number of effective 32B write transactions to the memory
SALUBusy   The percentage of GPUTime scalar ALU instructions are processed. Value range
SALUInsts   The average number of scalar ALU instructions executed per work-item (affected by flow control).
SFetchInsts   The average number of scalar fetch instructions from the video memory executed per work-item (affected by flow control).
------------------------------------------------------------------------------------------------------------------------------------------
Refer to "Texture Addressing Unit" section for more information on results https://rocm.docs.amd.com/en/docs-5.7.1/understand/gpu_arch/mi200_performance_counters.html
TA_ADDR_STALLED_BY_TC_CYCLES_sum   Number of cycles addr path stalled by TC. Perf_Windowing not supported for this counter. Sum over TA instances.
TA_ADDR_STALLED_BY_TD_CYCLES_sum   Number of cycles addr path stalled by TD. Perf_Windowing not supported for this counter. Sum over TA instances.
TA_BUFFER_ATOMIC_WAVEFRONTS_sum   Number of buffer atomic wavefronts processed by TA. Sum over TA instances.
TA_BUFFER_COALESCED_READ_CYCLES_sum   Number of buffer coalesced read cycles issued to TC. Sum over TA instances.
TA_BUFFER_COALESCED_WRITE_CYCLES_sum   Number of buffer coalesced write cycles issued to TC. Sum over TA instances.
TA_BUFFER_READ_WAVEFRONTS_sum   Number of buffer read wavefronts processed by TA. Sum over TA instances.
TA_BUFFER_TOTAL_CYCLES_sum   Number of buffer cycles issued to TC. Sum over TA instances.
TA_BUFFER_WAVEFRONTS_sum   Number of buffer wavefronts processed by TA. Sum over TA instances.
TA_BUFFER_WRITE_WAVEFRONTS_sum   Number of buffer write wavefronts processed by TA. Sum over TA instances.
TA_BUSY_avr   TA block is busy. Average over TA instances.
TA_BUSY_max   TA block is busy. Max over TA instances.
TA_BUSY_min   TA block is busy. Min over TA instances.
TA_DATA_STALLED_BY_TC_CYCLES_sum   Number of cycles data path stalled by TC. Perf_Windowing not supported for this counter. Sum over TA instances.
TA_FLAT_ATOMIC_WAVEFRONTS_sum   Number of flat opcode atomics processed by the TA. Sum over TA instances.
TA_FLAT_READ_WAVEFRONTS_sum   Number of flat opcode reads processed by the TA. Sum over TA instances.
TA_FLAT_WAVEFRONTS_sum   Number of flat opcode wavfronts processed by the TA. Sum over TA instances.
TA_FLAT_WRITE_WAVEFRONTS_sum   Number of flat opcode writes processed by the TA. Sum over TA instances.
TA_TA_BUSY_sum   TA block is busy. Perf_Windowing not supported for this counter. Sum over TA instances.
TA_TOTAL_WAVEFRONTS_sum   Total number of wavefronts processed by TA. Sum over TA instances.
------------------------------------------------------------------------------------------------------------------------------------------
Refer to "Texture Cache Arbiter" section for more information on results https://rocm.docs.amd.com/en/docs-5.7.1/understand/gpu_arch/mi200_performance_counters.html
TCA_BUSY_sum   Number of cycles we have a request pending. Sum over all TCA instances.
TCA_CYCLE_sum   Number of cycles. Sum over all TCA instances
Refer to  L2 Cache Access  section for more information on results https://rocm.docs.amd.com/en/docs-5.7.1/understand/gpu_arch/mi200_performance_counters.html
TCC_ALL_TC_OP_INV_EVICT_sum   Number of evictions due to all TC_OP invalidate requests. Sum over TCC instances.
TCC_ALL_TC_OP_WB_WRITEBACK_sum   Number of writebacks due to all TC_OP writeback requests. Sum over TCC instances.
TCC_ATOMIC_sum   Number of atomic requests of all types. Sum over TCC instances.
TCC_BUSY_avr   TCC_BUSY avr over all memory channels.
TCC_BUSY_sum   Number of cycles we have a request pending. Not windowable. Sum over TCC instances.
TCC_CC_REQ_sum   The number of coherently cached requests. This is measured at the tag block. Sum over TCC instances.
TCC_CYCLE_sum   Number of cycles. Not windowable. Sum over TCC instances.
TCC_EA_ATOMIC_LEVEL_sum   The sum of the number of EA atomics in flight. This is primarily meant for measure average EA atomic latency. Average atomic latency = TCC_PERF_SEL_EA_WRREQ_ATOMIC_LEVEL/TCC_PERF_SEL_EA_WRREQ_ATOMIC. Sum over TCC instances.
TCC_EA_ATOMIC_sum   Number of transactions going over the TC_EA_wrreq interface that are actually atomic requests. Sum over TCC instances.
TCC_EA_RDREQ_32B_sum   Number of 32-byte TCC/EA read requests Sum over TCC instances.
TCC_EA_RDREQ_DRAM_CREDIT_STALL_sum   Number of cycles there was a stall because the read request interface was out of DRAM credits. Stalls occur regardless of whether a read needed to be performed or not. Sum over TCC instances.
TCC_EA_RDREQ_DRAM_sum   Number of TCC/EA read requests (either 32-byte or 64-byte) destined for DRAM (MC). Sum over TCC instances.
TCC_EA_RDREQ_GMI_CREDIT_STALL_sum   Number of cycles there was a stall because the read request interface was out of GMI credits. Stalls occur regardless of whether a read needed to be performed or not. Sum over TCC instances.
TCC_EA_RDREQ_IO_CREDIT_STALL_sum   Number of cycles there was a stall because the read request interface was out of IO credits. Stalls occur regardless of whether a read needed to be performed or not. Sum over TCC instances.
TCC_EA_RDREQ_LEVEL_sum   The sum of the number of TCC/EA read requests in flight. This is primarily meant for measure average EA read latency. Average read latency = TCC_PERF_SEL_EA_RDREQ_LEVEL/TCC_PERF_SEL_EA_RDREQ. Sum over TCC instances.
TCC_EA_RDREQ_sum   Number of TCC/EA read requests (either 32-byte or 64-byte) Sum over TCC instances.
TCC_EA_RD_UNCACHED_32B_sum   Number of 32-byte TCC/EA read due to uncached traffic. A 64-byte request will be counted as 2 Sum over TCC instances.
TCC_EA_WRREQ_64B_sum   Number of 64-byte transactions going (64-byte write or CMPSWAP) over the TC_EA_wrreq interface. Sum over TCC instances.
TCC_EA_WRREQ_DRAM_CREDIT_STALL_sum   Number of cycles a EA write request was stalled because the interface was out of DRAM credits. Sum over TCC instances.
TCC_EA_WRREQ_DRAM_sum   Number of TCC/EA write requests (either 32-byte of 64-byte) destined for DRAM (MC). Sum over TCC instances.
TCC_EA_WRREQ_GMI_CREDIT_STALL_sum   Number of cycles a EA write request was stalled because the interface was out of GMI credits. Sum over TCC instances.
TCC_EA_WRREQ_IO_CREDIT_STALL_sum   Number of cycles a EA write request was stalled because the interface was out of IO credits. Sum over TCC instances.
TCC_EA_WRREQ_LEVEL_sum   The sum of the number of EA write requests in flight. This is primarily meant for measure average EA write latency. Average write latency = TCC_PERF_SEL_EA_WRREQ_LEVEL/TCC_PERF_SEL_EA_WRREQ. Sum over TCC instances.
TCC_EA_WRREQ_STALL_sum   Number of cycles a write request was stalled. Sum over TCC instances.
TCC_EA_WRREQ_sum   Number of transactions (either 32-byte or 64-byte) going over the TC_EA_wrreq interface. Atomics may travel over the same interface and are generally classified as write requests. This does not include probe commands. Sum over TCC instances.
TCC_EA_WR_UNCACHED_32B_sum   Number of 32-byte write/atomic going over the TC_EA_wrreq interface due to uncached traffic. Note that CC mtypes can produce uncached requests, and those are included in this. A 64-byte request will be counted as 2. Sum over TCC instances.
TCC_HIT_sum   Number of cache hits. Sum over TCC instances.
TCC_MISS_sum   Number of cache misses. UC reads count as misses. Sum over TCC instances.
TCC_NC_REQ_sum   The number of noncoherently cached requests. This is measured at the tag block. Sum over TCC instances.
TCC_NORMAL_EVICT_sum   Number of evictions due to requests that are not invalidate or probe requests. Sum over TCC instances.
TCC_NORMAL_WRITEBACK_sum   Number of writebacks due to requests that are not writeback requests. Sum over TCC instances.
TCC_PROBE_ALL_sum   Number of external probe requests with with EA_TCC_preq_all== 1. Not windowable. Sum over TCC instances.
TCC_PROBE_sum   Number of probe requests. Not windowable. Sum over TCC instances.
TCC_READ_sum   Number of read requests. Compressed reads are included in this, but metadata reads are not included. Sum over TCC instances.
TCC_REQ_sum   Number of requests of all types. This is measured at the tag block. This may be more than the number of requests arriving at the TCC, but it is a good indication of the total amount of work that needs to be performed. Sum over TCC instances.
TCC_RW_REQ_sum   The number of RW requests. This is measured at the tag block. Sum over TCC instances.
TCC_STREAMING_REQ_sum   Number of streaming requests. This is measured at the tag block. Sum over TCC instances.
TCC_TAG_STALL_PERCENT   Percentage of time the TCC tag lookup pipeline is stalled.
TCC_TAG_STALL_sum   Total number of cycles the normal request pipeline in the tag is stalled for any reason.
TCC_TOO_MANY_EA_WRREQS_STALL_sum   Number of cycles the TCC could not send a EA write request because it already reached its maximum number of pending EA write requests. Sum over TCC instances.
TCC_UC_REQ_sum   The number of uncached requests. This is measured at the tag block. Sum over TCC instances.
TCC_WRITEBACK_sum   Number of lines written back to main memory. This includes writebacks of dirty lines and uncached write/atomic requests. Sum over TCC instances.
TCC_WRITE_sum   Number of write requests. Sum over TCC instances.
TCC_WRREQ_STALL_max   Number of cycles a write request was stalled. Max over TCC instances.
------------------------------------------------------------------------------------------------------------------------------------------
Refer to "Vector L1D Cache" Section for more information https://rocm.docs.amd.com/en/docs-5.7.1/understand/gpu_arch/mi200_performance_counters.html
TCP_ATOMIC_TAGCONFLICT_STALL_CYCLES_sum   Tagram conflict stall on an atomic. Sum over TCP instances.
TCP_GATE_EN1_sum   TCP interface clocks are turned on. Not Windowed. Sum over TCP instances.
TCP_GATE_EN2_sum   TCP core clocks are turned on. Not Windowed. Sum over TCP instances.
TCP_PENDING_STALL_CYCLES_sum   Stall due to data pending from L2. Sum over TCP instances.
TCP_READ_TAGCONFLICT_STALL_CYCLES_sum   Tagram conflict stall on a read. Sum over TCP instances.
TCP_TA_TCP_STATE_READ_sum   Number of state reads Sum over TCP instances.
TCP_TCC_ATOMIC_WITHOUT_RET_REQ_sum   Total atomic without return requests from TCP to all TCCs Sum over TCP instances.
TCP_TCC_ATOMIC_WITH_RET_REQ_sum   Total atomic with return requests from TCP to all TCCs Sum over TCP instances.
TCP_TCC_CC_ATOMIC_REQ_sum   Total atomic requests with CC mtype from this TCP to all TCCs Sum over TCP instances.
TCP_TCC_CC_READ_REQ_sum   Total write requests with CC mtype from this TCP to all TCCs Sum over TCP instances.
TCP_TCC_CC_WRITE_REQ_sum   Total write requests with CC mtype from this TCP to all TCCs Sum over TCP instances.
TCP_TCC_NC_ATOMIC_REQ_sum   Total atomic requests with NC mtype from this TCP to all TCCs Sum over TCP instances.
TCP_TCC_NC_READ_REQ_sum   Total read requests with NC mtype from this TCP to all TCCs Sum over TCP instances.
TCP_TCC_NC_WRITE_REQ_sum   Total write requests with NC mtype from this TCP to all TCCs Sum over TCP instances.
TCP_TCC_READ_REQ_LATENCY_sum   Total TCP->TCC request latency for reads and atomics with return. Not Windowed. Sum over TCP instances.
TCP_TCC_READ_REQ_sum   Total read requests from TCP to all TCCs Sum over TCP instances.
TCP_TCC_RW_ATOMIC_REQ_sum   Total atomic requests with RW mtype from this TCP to all TCCs. Sum over TCP instances.
TCP_TCC_RW_READ_REQ_sum   Total write requests with RW mtype from this TCP to all TCCs. Sum over TCP instances.
TCP_TCC_RW_WRITE_REQ_sum   Total write requests with RW mtype from this TCP to all TCCs. Sum over TCP instances.
TCP_TCC_UC_ATOMIC_REQ_sum   Total atomic requests with UC mtype from this TCP to all TCCs Sum over TCP instances.
TCP_TCC_UC_READ_REQ_sum   Total read requests with UC mtype from this TCP to all TCCs Sum over TCP instances.
TCP_TCC_UC_WRITE_REQ_sum   Total write requests with UC mtype from this TCP to all TCCs Sum over TCP instances.
TCP_TCC_WRITE_REQ_LATENCY_sum   Total TCP->TCC request latency for writes and atomics without return. Not Windowed. Sum over TCP instances.
TCP_TCC_WRITE_REQ_sum   Total write requests from TCP to all TCCs Sum over TCP instances.
TCP_TCP_LATENCY_sum   Total TCP wave latency (from first clock of wave entering to first clock of wave leaving), divide by TA_TCP_STATE_READ to avg wave latency Sum over TCP instances.
TCP_TCP_TA_DATA_STALL_CYCLES_max   Maximum number of TCP stalls TA data interface.
TCP_TCP_TA_DATA_STALL_CYCLES_sum   Total number of TCP stalls TA data interface.
TCP_TCR_TCP_STALL_CYCLES_PERCENT   Percentage of time TCP is stalled by TCR.
TCP_TCR_TCP_STALL_CYCLES_sum   TCR stalls TCP_TCR_req interface. Sum over TCP instances.
TCP_TD_TCP_STALL_CYCLES_sum   TD stalls TCP. Sum over TCP instances.
TCP_TOTAL_ACCESSES_sum   Total number of pixels/buffers from TA. Equals TCP_PERF_SEL_TOTAL_READ+TCP_PERF_SEL_TOTAL_NONREAD. Sum over TCP instances.
TCP_TOTAL_ATOMIC_WITHOUT_RET_sum   Total number of atomic without return pixels/buffers from TA Sum over TCP instances.
TCP_TOTAL_ATOMIC_WITH_RET_sum   Total number of atomic with return pixels/buffers from TA. Sum over TCP instances.
TCP_TOTAL_CACHE_ACCESSES_sum   Count of total cache line (tag) accesses (includes hits and misses). Sum over TCP instances.
TCP_TOTAL_READ_sum   Total number of read pixels/buffers from TA. Equals TCP_PERF_SEL_TOTAL_HIT_LRU_READ + TCP_PERF_SEL_TOTAL_MISS_LRU_READ + TCP_PERF_SEL_TOTAL_MISS_EVICT_READ. Sum over TCP instances.
TCP_TOTAL_WRITEBACK_INVALIDATES_sum   Total number of cache invalidates. Equals TCP_PERF_SEL_TOTAL_WBINVL1+ TCP_PERF_SEL_TOTAL_WBINVL1_VOL+ TCP_PERF_SEL_CP_TCP_INVALIDATE+ TCP_PERF_SEL_SQ_TCP_INVALIDATE_VOL. Not Windowed. Sum over TCP instances.
TCP_TOTAL_WRITE_sum   Total number of local write pixels/buffers from TA. Equals TCP_PERF_SEL_TOTAL_MISS_LRU_WRITE+ TCP_PERF_SEL_TOTAL_MISS_EVICT_WRITE. Sum over TCP instances.
TCP_UTCL1_PERMISSION_MISS_sum   Total utcl1 permission misses Sum over TCP instances.
TCP_UTCL1_REQUEST_sum   Total CLIENT_UTCL1 NORMAL requests Sum over TCP instances.
TCP_UTCL1_TRANSLATION_HIT_sum   Total utcl1 translation hits Sum over TCP instances.
TCP_UTCL1_TRANSLATION_MISS_sum   Total utcl1 translation misses Sum over TCP instances.
TCP_VOLATILE_sum   Total number of L1 volatile pixels/buffers from TA. Sum over TCP instances.
TCP_WRITE_TAGCONFLICT_STALL_CYCLES_sum   Tagram conflict stall on a write. Sum over TCP instances.
------------------------------------------------------------------------------------------------------------------------------------------
Refer to "Texture Data Unit" section for more information on results https://rocm.docs.amd.com/en/docs-5.7.1/understand/gpu_arch/mi200_performance_counters.html
TD_ATOMIC_WAVEFRONT_sum   Count the wavefronts with opcode = atomic. Sum over TD instances.
TD_COALESCABLE_WAVEFRONT_sum   Count wavefronts that TA finds coalescable. Sum over TD instances.
TD_LOAD_WAVEFRONT_sum   Count the wavefronts with opcode = load, include atomics and store. Sum over TD instances.
TD_SPI_STALL_sum   TD is stalled SPI vinit, sum of TCP instances
TD_STORE_WAVEFRONT_sum   Count the wavefronts with opcode = store. Sum over TD instances.
TD_TC_STALL_sum   TD is stalled waiting for TC data. Sum over TD instances.
TD_TD_BUSY_sum   TD is processing or waiting for data. Perf_Windowing not supported for this counter. Sum over TD instances.
------------------------------------------------------------------------------------------------------------------------------------------
VALUBusy   The percentage of GPUTime vector ALU instructions are processed. Value range
VALUInsts   The average number of vector ALU instructions executed per work-item (affected by flow control).
VALUUtilization   The percentage of active vector ALU threads in a wave. A lower number can mean either more thread divergence in a wave or that the work-group size is not a multiple of 64. Value range
VFetchInsts   The average number of vector fetch instructions from the video memory executed per work-item (affected by flow control). Excludes FLAT instructions that fetch from video memory.
VWriteInsts   The average number of vector write instructions to the video memory executed per work-item (affected by flow control). Excludes FLAT instructions that write to video memory.
WRITE_REQ_32B   The total number of 32-byte effective memory writes.
WRITE_SIZE   The total kilobytes written to the video memory. This is measured with all extra fetches and any cache or memory effects taken into account.
Wavefronts   Total wavefronts.
WriteSize   The total kilobytes written to the video memory. This is measured with all extra fetches and any cache or memory effects taken into account.
WriteUnitStalled   The percentage of GPUTime the Write unit is stalled. Value range


Acronyms used to describe the metrics
LDS (Local data share): allows efficient communication and data sharing between threads within a compute unit
GDS (Global Data Share): Instructions for exporting can be directed towards the Global Data Share. The GDS functions as a globally accessible, explicitly addressed memory space, similar to LDS, and it facilitates synchronization across all wavefronts as well as with fixed-function hardware components. Additionally, the GDS is equipped with ALUs that are specifically used for conducting global atomic operations. Each dual compute unit's export unit has the capability to transmit up to four vector General-Purpose Registers (vGPRs) to the primitive units, Render Backends (RBs), or directly to the GDS.
L2 Cache (in the MI250x): There are two L2 caches on the MI250x, one on each half of the board. These individually act like regular L2 caches that helps store data between the 2 L1 caches on each half of the board.
Wavefront (AMD) Warp (NVDA): GPUs use a single instruction multi thread execution model where threads executing the same instruction are grouped into batches that are fixed in size. These are known as wavefronts in AMD GPUs and warps  in NVIDIA GPUs.
Occupancy: Ratio of assigned wavefronts to maximum available slots
Memory Bank: local memory is divided into banks where each one can only address a single dataset at a time.
Ctx (context): Context refers to the all the required information (data, variables, conditions, etc) to perform certain tasks like encoding and different GPU processes.  
TA (Texture Addresser)   It s important to determine the proper U, V and X,Y coordinates when applications need to utilize textured polygons in order to avoid doubled seams or pixels. Texture addressing insures that the proper coordinates are used.
TC (Texture Cache): Read only cache that stores image data that is used for texture mapping
TD (Texture Data): Self explanatory, just Texture data.
TCA (Texture Cache Arbiter): The arbiter determines whether the cache has enough resources to process the request
TCC (Texture Cache per Channel): This is also just known as the L2 Cache
UTCL1 (Unified Translation Cache Level 1): Not really a term in GPU architecture, but from the terms used I assume this is a L1 cache that is abele to cache data/instructions as well as address translations. A mix of a translation lookaside buffer and a unified cache.
NC (Noncoherently Cached): My understanding of this is that Noncoherent cache is the cache that isn t shared between caches. So L1 cache, and all the special caches like the TC and TCA, in the MI250x GPU architecture would be considered noncoherent cache.
EA (Efficiency Arbiter): This seems redundant as arbiters in general are used to improve efficiency by allocating access to shared resources. 
RW (Coherently Cached with Write): Not too sure, but seems like it could be a write-through cache where the data is written to the main memory location as well as the cache at the same time.
TCP (Texture Cache Pipe): AKA the L1 vector Cache. Maybe a vector cache is a cache optimized for storing vector data structures and quick access for vector processing.
CC (Coherently Cached): When information is cached so that all caches are able to have a consistent view of the memory to all processors. Allows for efficient data sharing among the cores/processors.
UC (Uncached): Not cached
SPI (Shader processor input): Shader processors are specialized units in the GPU designed to handle tasks that required shading specifically for rendering graphics. The inputs can vary depending on the stage within the rendering pipeline but they include vertex data, textures/samplers, uniforms/constants, interpolants, and buffers.


-------------------------------------------------------------Input file------------------------------------------------------------------
pmc : MemUnitBusy Wavefronts GPUBusy L2CacheHit
# Filter by dispatches range, GPU index and kernel names
# supported range formats: "3:9", "3:", "3"
range: 0:1
gpu: 0 1
#kernel: simple Pass1 simpleConvolutionPass2

pmc refers to which metrics we are tracking
range specifies the range of the kernels being dispatched 
gpu refers to which GPUs we are going to use
kernel specifies the kernel(s) we want to specifically monitor





















Using rocprof for application tracing 
Options
Description
-d <data directory>
To specify the directory where the profiler stores traces and the profiling data. The profiler stores the profiling data in a temporary directory [/tmp] by default, which is removed automatically after a specific period. Specifying a directory explicitly allows you to prevent loss of data.
--hip-trace
To trace API execution stats, HIP API calls, and copy operation calls. Read more 
--hsa-trace
To trace API execution stats, HSA API calls, kernel execution stats, and copy operation calls. Read more 
--sys-trace
To trace API execution stats, HIP and HSA API calls, and copy operation calls. Read more 
--roctx-trace
To enable roctx application code annotation trace. Allows you to trace a particular block of code when Markers and Ranges are specified in the application code. Read more 
--stats
To trace API execution stats and kernel execution stats
--basenames <on|off>
To enable/disable truncation of the kernel full function names in the trace, till the base ones. Default value: [off]. Read more 
--flush-rate
To enable trace flush rate . Supported time formats: <number(m|s|ms|us)>. Read more 
--roctx-rename
To rename long kernel names. Kernel renaming is recommended only in special cases where kernel names are auto generated in an incomprehensible format.
--trace-start <on|off>
To enable/disable tracing for a HIP API or code block. Default value: [on]. Read more 
--trace-period <delay:length:rate>
To enable tracing with an initial delay, periodic sample length and rate. Supported time formats: <number(m|s|ms|us)> 

Ex. 
rocprof --stats --basenames on --timestamp on -d ${SLURM_JOBID} -i inputs.txt -o first_attempt.csv /home/aamarli/rocHPCG/build/release/bin/rochpcg 280 280 280 15
