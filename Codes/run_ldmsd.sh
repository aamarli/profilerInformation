#!/bin/bash
message () {
    local color
    local OPTIND
    local opt
    while getopts "crgymn" opt; do
        case $opt in
            c)  color=$(tput setaf 6) ;;
            r)  color=$(tput setaf 1) ;;
	    g)  color=$(tput setaf 2) ;;
	    y)  color=$(tput setaf 3) ;;
	    m)  color=$(tput setaf 5) ;;
            *)  color=$(tput sgr0)    ;;
        esac
    done
    shift $(($OPTIND -1))
    printf "${color}%-10s %-50s %-50s %-50s\n" "$1" "$2" "$3" "$4"
    tput sgr0
}

status_ldmsd () {
  message -g "Running LDMS Processes on $(hostname -s)"
  message -g "Sampler: $(ps aux |grep sampler_ldmsd| grep -v grep)"
  message -g "Aggregator: $(ps aux |grep aggregator_ldmsd |grep -v grep)"
  	
}


kill_ldmsd () {
  message -g "Killing the following PIDs"
  message -g "$(ps aux |grep sampler_ldmsd| grep -v grep)"
  message -g "$(ps aux |grep aggregator_ldmsd |grep -v grep)"
  ps aux |grep ldmsd | grep -v grep | awk '{print $2}' | xargs kill -9
}

die () {
  echo "ERROR: $@"
  exit -1
}

test_ldmsd_streams_publish () {
  local _testfile=$1
  local _token=$2
  ldmsd_stream_publish -x sock -h localhost -p ${SAMP_PORT} -t string -s amd_gpu_sampler -a munge -f $_testfile
  if grep -r "$TOKEN THIS IS A SIMPLE TEST TO EXERCISE THE USABILITY OF LDMSD STREAMS" $TEMPDIR &>/dev/null ; then
    message -g "TEST SUCCEEDED"
    grep -r "$TOKEN THIS IS A SIMPLE TEST TO EXERCISE THE USABILITY OF LDMSD STREAMS" $TEMPDIR
  else
    message -r "TEST FAILED"
  fi
}

usage () {
message -m "
	${0//*\/} [options] [command]
        -s | 	starts ldmsd services
        -X |    stops ldmsd services
        -S |	gets status of ldmsd services
        -h |    usage output
        -v |    verbose output
        -t |    test ldmsd_stream_publish
"
}
      
if [[ $# -lt 1 ]] ; then usage ; die "This script requires at least one argument" ; fi

while getopts "XsStvh" opt ; do
    case "${opt}" in
        s)  MODE="start"
            TEMPDIR=$(mktemp -p /local_data -d --suffix=.ldms XXXXX)
        ;;
	h)  usage && exit 0			;;
        X)  MODE="stop"				;;
        S)  MODE="status"			;;
        t)  MODE="test"				;;
	v)  DEBUG=true				;;
	*)  die "$* is unsupported"		;;
    esac
done

# hello :) 
AGG_PORT=10545
SAMP_PORT=10544
AUTH_TYPE="munge"
XPRT_TYPE="sock"
SAMPLE_INTERVAL=1000000
COMPONENT_ID=90002
TOKEN=$RANDOM
TEMPDIR=${TEMPDIR:=$(ps aux |grep ldmsd-sampler | awk -F'/' '$2 ~ /local_data/ {print "/"$2"/"$3"/"}')}

[[ -d $TEMPDIR ]] && chmod -Rf a+wrX $TEMPDIR


if [[ "$DEBUG"x == "truex" ]] ; then set -x ; fi

command -v ldmsd &>/dev/null || die "Cannot find ldmsd"

cat << SAMPLER > sampler.conf
### This is the configuration file for LDMS's sampler daemon
env SAMPLE_INTERVAL=$SAMPLE_INTERVAL
env COMPONENT_ID=$COMPONENT_ID
  
metric_sets_default_authz perm=0777
  
load name=meminfo
config name=meminfo producer=\${HOSTNAME} instance=\${HOSTNAME}/meminfo component_id=\${COMPONENT_ID} schema=meminfo perm=0777
start name=meminfo interval=\${SAMPLE_INTERVAL}
SAMPLER

cat << AGGREGATOR > aggregator.conf
### This is the configuration file for LDMS's aggregator daemon
auth_add name=$AUTH_TYPE plugin=$AUTH_TYPE
#listen port=$AGG_PORT xprt=$XPRT_TYPE auth=$AUTH_TYPE

# Loading the stream_csv_store plugin
load name=stream_csv_store
config name=stream_csv_store path=$TEMPDIR container=gpu_sampler_data stream=amd_gpu_sampler buffer=0

# Loading the store_csv plugin
load name=store_csv

# Store Group Add
strgp_add name=store_csv plugin=store_csv schema=meminfo container=meminfo

config name=store_csv path=$TEMPDIR

# Store Group Producer Add
strgp_prdcr_add name=store_csv regex=.*

# Store Group Start
strgp_start name=store_csv

# Producer Add
prdcr_add name=localhost type=active interval=$SAMPLE_INTERVAL xprt=$XPRT_TYPE host=localhost port=$SAMP_PORT auth=$AUTH_TYPE

# Producer Subscribe Definition
prdcr_subscribe regex=.* stream=amd_gpu_sampler

# Producer Start
prdcr_start name=localhost

# Updater Add
updtr_add name=update_all interval=$SAMPLE_INTERVAL auto_interval=true

# Updater Producer Add
updtr_prdcr_add name=update_all regex=.*

# Updater Start
updtr_start name=update_all

AGGREGATOR

[ -f sampler.conf ] || die "Cannot locate sampler.conf at $PWD"
# first, let's launch ldmsd samplers daemon as a regular user
case $MODE in

  start)
    ldmsd -x sock:$SAMP_PORT \
          -c sampler.conf \
          -l $TEMPDIR/sampler_ldmsd.log \
          -v DEBUG \
          -a ${AUTH_TYPE} \
          -r $TEMPDIR/ldmsd-sampler.pid \
          -m 2G
  
    sleep 10
    if ! [[ $(ps aux |grep sampler_ldmsd| grep -v grep) =~ sampler_ldmsd ]]
    then
      die "Sampler LDMSD didn't start"
    else
      message -g "STARTED: $(ps aux |grep sampler_ldmsd | grep -v grep)"
    fi
  
    [ -f aggregator.conf ] || die "Cannot locate aggregator.conf at $PWD"
    ldmsd -x sock:$AGG_PORT \
          -c aggregator.conf \
          -l $TEMPDIR/aggregator_ldmsd.log \
          -v DEBUG \
          -a ${AUTH_TYPE} \
          -r $TEMPDIR/ldmsd-aggregator.pid \
          -m 2G
  
    sleep 10
  
    if ! [[ $(ps aux |grep aggregator_ldmsd |grep -v grep) =~ aggregator_ldms ]]
    then
      die "Aggregator LDMSD didn't start" 
    else
      message -g "STARTED: $(ps aux |grep aggregator_ldmsd | grep -v grep)"
      
    fi
  ;;
  stop)
    kill_ldmsd
  ;;
  status)
    status_ldmsd
  ;;
  test)
cat << TESTFILE >testfile.txt

$TOKEN THIS IS A SIMPLE TEST TO EXERCISE THE USABILITY OF LDMSD STREAMS 

TESTFILE

    [ -f testfile.txt ] && test_ldmsd_streams_publish testfile.txt
  ;;

esac

