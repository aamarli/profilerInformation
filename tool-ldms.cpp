#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#ifdef HAVE_LDMS


/*
typedef struct rpConnector(
	int to;
        int ldms_lib;
        int posix_enable_ldms;
        int mpiio_enable_ldms;
        int stdio_enable_ldms;
        int hdf5_enable_ldms;
        const char *exepath;
        const char *exe_tmp;
        const char *schema;
        const char *filepath;
        const char* env_ldms_stream;
        const char* env_ldms_reinit;
        int server_rc;
        int64_t jobid;
        int64_t uid;
        char hname[HOST_NAME_MAX];
        int64_t hdf5_data[5];
        int64_t open_count;
        int64_t write_count;
        int conn_status;
        struct timespec ts;
        pthread_mutex_t ln_lock;
        ldms_t ldms_darsh;
        ldms_t ldms_g;
        sem_t recv_sem;
        sem_t conn_sem;
} rpConnector; */


static void event_cb(ldms_t x, ldms_xprt_event_t e, void *cb_arg)
{
        switch (e->type) {
        case LDMS_XPRT_EVENT_CONNECTED:
                sem_post(&conn_sem);
                conn_status = 0;
                break;
        case LDMS_XPRT_EVENT_REJECTED:
                ldms_xprt_put(x);
                conn_status = ECONNREFUSED;
                break;
        case LDMS_XPRT_EVENT_DISCONNECTED:
                ldms_xprt_put(x);
                conn_status = ENOTCONN;
                break;
        case LDMS_XPRT_EVENT_ERROR:
                conn_status = ECONNREFUSED;
                break;
        case LDMS_XPRT_EVENT_RECV:
                sem_post(&recv_sem);
                break;
        case LDMS_XPRT_EVENT_SEND_COMPLETE:
                break;
        default:
                printf("Received invalid event type %d\n", e->type);
        }
}

ldms_t setup_connection(const char *xprt, const char *host,
                        const char *port, const char *auth)
{
        char hostname[PATH_MAX];
        const char *timeout = "5";
        int rc;
        struct timespec ts;

        if (!host) {
                if (0 == gethostname(hostname, sizeof(hostname)))
                        host = hostname;
        }
        if (!timeout) {
                ts.tv_sec = time(NULL) + 5;
                ts.tv_nsec = 0;
        } else {
                int to = atoi(timeout);
                if (to <= 0)
                        to = 5;
                ts.tv_sec = time(NULL) + to;
                ts.tv_nsec = 0;
        }

        ldms_g = ldms_xprt_new_with_auth(xprt, auth, NULL);
        if (!ldms_g) {
                printf("Error %d creating the '%s' transport\n",
                       errno, xprt);
                return NULL;
        }

        sem_init(&recv_sem, 1, 0);
        sem_init(&conn_sem, 1, 0);

        rc = ldms_xprt_connect_by_name(ldms_g, host, port, event_cb, NULL);
        if (rc) {
                printf("Error %d connecting to %s:%s\n",
                       rc, host, port);
                return NULL;
        }
        sem_timedwait(&conn_sem, &ts);
        if (conn_status)
                return NULL;
        return ldms_g;
}

void ldms_connector_init() 
{
	const char* env_ldms_stream  = getenv("ROCP_LDMS_STREAM");
	const char* env_ldms_xprt    = getenv("ROCP_LDMS_XPRT");
	const char* env_ldms_host    = getenv("ROCP_LDMS_HOST");
	const char* env_ldms_port    = getenv("ROCP_LDMS_PORT");
	const char* env_ldms_auth    = getenv("ROCP_LDMS_AUTH");

	if (!env_ldms_xprt || !env_ldms_host || !env_ldms_port || !env_ldms_auth || env_ldms_stream) {
		printf("Either the transport, host, port or authentication is not given\n");
        return;
    }

	const char* slurm_job_id_str = getenv("SLURM_JOB_ID");
	const char* openmpi_job_str  = getenv("OMPI_COMM_WORLD_RANK");
	const char* slurm_rank_str   = getenv("SLURM_PROCID");

	const char* tool_verbose_str = getenv("ROCP_LDMS_VERBOSE");
	const char* slurm_node_list_char = getenv("SLURM_JOB_NODELIST");
	
	const char* rocp_input_metrics = getenv("ROCP_INPUT_METRICS");
	const char* rocp_timestamp = getenv("ROCP_TIMESTAMP_ON");

	gethostname(hostname_kp, HOST_NAME_MAX);
		    /* Check/set LDMS deamon connection AAMARLI - remove the struct stuff in the below */
    if (!env_ldms_xprt || *env_ldms_xprt == '\0'){
	rocp_core_fprintf(stderr, "LDMS library: rocpConnector - transport for LDMS streams deamon connection is not set. Setting to default value \"sock\".\n");
	env_ldms_xprt = "sock";}

    if (!env_ldms_host || *env_ldms_host == '\0'){
	rocp_core_fprintf(stderr, "LDMS library: rocpConnector - hostname for LDMS streams deamon connection is not set. Setting to default value \"localhost\".\n");
	env_ldms_host = "localhost";}

    if (!env_ldms_port || *env_ldms_port == '\0'){
	rocp_core_fprintf(stderr, "LDMS library: rocpConnector - port for LDMS streams deamon connection is not set. Setting to default value \"412\".\n");
	env_ldms_port = "412";}

    if (!env_ldms_auth || *env_ldms_auth == '\0'){
	rocp_core_fprintf(stderr, "LDMS library: rocpConnector - authentication for LDMS streams deamon connection is not set. Setting to default value \"munge\".\n");
	env_ldms_auth = "munge";}

    if (!dC.env_ldms_stream || *dC.env_ldms_stream == '\0'){
	rocp_core_fprintf(stderr, "LDMS library: darshanConnector - stream name for LDMS streams deamon connection is not set. Setting to default value \"darshanConnector\".\n");
	dC.env_ldms_stream = "darshanConnector";}
	
}

pthread_mutex_lock(&ln_lock);
void write_ldms_record(int mpi_rank, RegionProfile& profile)
{
    caliper_ldms_connector_initialize();

    std::map<std::string, double> region_times;
    double total_time = 0;

    int buffer_size = 4096;
    char* buffer = (char*) malloc (sizeof(char) * buffer_size);

    const char*   env_ldms_jobid_str            = getenv("SLURM_JOB_ID");
    const char*   env_ldms_procid               = getenv("SLURM_PROCID");
    const char*   env_ldms_slurm_nodelist       = getenv("SLURM_JOB_NODELIST");
    const char*   env_ldms_caliper_verbose_str  = getenv("CALIPER_LDMS_VERBOSE");

    int env_ldms_jobid = env_ldms_jobid_str == NULL ? 0 : atoi( env_ldms_jobid_str );
    int env_ldms_caliper_verbose = env_ldms_caliper_verbose_str == NULL ? 0 : atoi( env_ldms_caliper_verbose_str );

    std::tie(region_times, std::ignore, total_time) =
        profile.inclusive_region_times();

    double unix_ts = 1e-6 * std::chrono::duration_cast<std::chrono::microseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();

    for (const auto &p : region_times) {
        // ignore regions with < 5% of the epoch's total time
        if (p.second < 0.05 * total_time)
            continue;

    	// std::string path_msg = "" + p.first;
        const char* path = p.first.c_str();

        if (mpi_rank >= 0) {
            snprintf(buffer, buffer_size, "{ \"timestamp\": %f, \"jobid\" : %d, \"rank\" : %d, \"procid\" : %s, \"nodelist\" : %s, \"caliper-perf-data\", \"duration\": %f, \"path\": \"%s\"} \n", unix_ts, env_ldms_jobid, mpi_rank, env_ldms_procid, env_ldms_slurm_nodelist, p.second, path);
	    } else {
	        snprintf(buffer, buffer_size, "{ \"timestamp\": %f, \"jobid\" : %d, \"rank\": %d, \"procid\" : %s, \"nodelist\" : %s, \"caliper-perf-data\", \"duration\": %f, \"path\": \"%s\"} \n", unix_ts, env_ldms_jobid, 0, env_ldms_procid, env_ldms_slurm_nodelist, p.second, path);
	    }

	    if (env_ldms_caliper_verbose > 0)
		    puts(buffer);

    	int rc = ldmsd_stream_publish( ldms_cali, "caliper-perf-data", LDMSD_STREAM_JSON, buffer, strlen(buffer) + 1);

	    if (rc)
		     Log(0).stream() << "Error " << rc << " publishing data.\n";
    	else if (env_ldms_caliper_verbose > 0)
	    	 Log(2).stream() << "Caliper Message published successfully\n";
	}
}


























