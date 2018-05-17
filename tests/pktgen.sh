#!/bin/bash -xe

function pgset()
{
	local result

	echo "$1" >"$PGDEV"

	result="$(cat "$PGDEV" |fgrep 'Result: OK:')"
	if [ "$result" = "" ]; then
		cat "$PGDEV" |fgrep 'Result:'
	fi
}
declare -fr pgset

function pg()
{
	echo inject >"$PGDEV"
	cat "$PGDEV"
}
declare -fr pg

################################################################################

declare -r prog_name="${0##*/}"

# Usage: usage [<fmt> [...]]
usage()
{
	local -i rc=$?
	local fmt="$1"
	[ -z "$fmt" ] || shift
	local fd

	[ $rc -eq 0 ] && fd=1 || fd=2
	printf -- "$fmt" "$@" >>"$fd"
	printf -- 'Usage: %s <lowerdev> [<dst_mac>]\n' "$prog_name" >>"$fd"

	exit $rc
}
declare -fr usage

# Requires root privileges to run
[ $UID -eq 0 ] ||
	usage 'root (uid: 0) privileges required, but uid is %u\n' $UID

# Check number of arguments
[ $# -ge 1 -a $# -le 2 ] ||
	usage
[ -n "$1" ] ||
	usage

declare -r dst_dev="${1}"
declare -r dst_mac="${2:-ff:ff:ff:ff:ff:ff}"

## Load module
modprobe pktgen ||:

## Configure thread

PGDEV='/proc/net/pktgen/kpktgend_0'

echo 'Remove all devices'
pgset 'rem_device_all'
echo "Add $dst_dev device"
pgset "add_device $dst_dev"

## Configure output netdev

PGDEV="/proc/net/pktgen/$dst_dev"

echo 'count'
pgset 'count 2000000000'

echo 'pkt size'
pgset 'min_pkt_size 62'
pgset 'max_pkt_size 126'

echo 'udp dport'
pgset 'udp_dst_min 32768'
pgset 'udp_dst_max 49152'
#pgset 'flag UDPDST_RND'

echo 'udp sport'
pgset 'udp_src_min 1024'
pgset 'udp_src_max 32767'
#pgset 'flag UDPSRC_RND'

echo 'ip daddr'
pgset 'dst_min 10.0.32.2'
pgset 'dst_max 10.0.32.254'
#pgset 'flag IPDST_RND'

echo 'ip saddr'
pgset 'src_min 192.168.0.2'
pgset 'src_max 192.168.255.254'
#pgset 'flag IPSRC_RND'

echo 'vlan'
pgset 'vlan_id 4094'
pgset 'flag VID_RND'

echo 'svlan'
pgset 'svlan_id 32'

echo 'mac dst'
pgset "dst_mac $dst_mac"

echo 'mac src'
pgset 'src_mac 02:aa:bb:cc:ff:ff'
pgset 'src_mac_count 255'

echo 'tx queue mapping'
pgset 'queue_map_min 0'
pgset 'queue_map_max 3'
pgset 'flag QUEUE_MAP_RND'

## Start

PGDEV='/proc/net/pktgen/pgctrl'

echo 'Start'
pgset 'start'
echo 'Finish'
