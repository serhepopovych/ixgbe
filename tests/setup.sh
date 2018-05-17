#!/bin/bash -e

# Usage: init <name> <lowerdev> <upperdev> <ip> [<uppermac>] [<svid>] [<cvid>] [<mtu>]
init()
{
	local name="${1:?missing 1st argument to \"$FUNCNAME\" (name)}"
	local lowerdev="${2:?missing 2d argument to \"$FUNCNAME\" (lowerdev)}"
	local upperdev="${3:?missing 3rd argument to \"$FUNCNAME\" (upperdev)}"
	local upperipa="${4:?missing 4th argument to \"$FUNCNAME\" (upperipa)}"
	local uppermac="$5"
	local -i svid=${6:-32}
	local -i cvid=${7:-1}
	local -i mtu=${8:-1500}

	# Create network namespace and bring loopback UP
	ip netns add "$name"
	ip netns exec "$name" ip link set dev lo up

	# Configure host real device MTU, bring it UP
	ip link set dev "$lowerdev" mtu $((mtu + 4)) up

	# Turn on Double VLAN via ethtool on host real device
	ethtool --set-priv-flags "$lowerdev" vlan-stag-rx on

	# Create and configure S-VLAN
	local if_svid="$upperdev.${svid}"
	if [ "$upperdev" = "$lowerdev" ]; then
		# VLAN
		ip link add dev "$if_svid" link "$lowerdev" netns "$name" \
			type vlan id $svid
	else
		# MACVLAN
		ip link add dev "$upperdev" link "$lowerdev" netns "$name" \
			${uppermac:+address "$uppermac"} \
			type macvlan mode bridge
		ip netns exec "$name" ip link set dev "$upperdev" up
		ip netns exec "$name" ip link add dev "$if_svid" link "$upperdev" \
			type vlan id $svid
	fi
	ip netns exec "$name" ip link set dev "$if_svid" up

	# Create and configure C-VLAN
	local if_cvid="$if_svid.${cvid}"
	ip netns exec "$name" ip link add dev "$if_cvid" link "$if_svid" mtu $mtu \
		type vlan id $cvid
	ip netns exec "$name" ip link set dev "$if_cvid" up

	# Configure IPv4 addresses on S-VLAN and C-VLAN
	local ip_svid
	ip_svid="$(printf '%03x\n' $svid)"
	ip_svid="${ip_svid:1}"
	ip_svid=$((0x$ip_svid))

	local ip_cvid
	ip_cvid="$(printf '%03x\n' $cvid)"
	ip_cvid="${ip_cvid:1}"
	ip_cvid=$((0x$ip_cvid))

	ip_cvid="$(printf "$IPA_TEMPL\n" $ip_svid $ip_cvid $upperipa)"
	ip netns exec "$name" ip -4 addr add "$ip_cvid" dev "$if_cvid"

	ip_svid="$(printf "$IPA_TEMPL\n" $ip_svid 0 $upperipa)"
	ip netns exec "$name" ip -4 addr add "$ip_svid" dev "$if_svid"
}
declare -fr init

# Usage: fini
fini()
{
	rmmod "$MODULE"

	local -i i
	for ((i = 0; i < NETNS_NR; i++)); do
		ip netns del "${NETNS_NAMES[$i]}" &>/dev/null
	done
}
declare -fr fini

################################################################################

declare -r prog_name="${0##*/}"
declare -r prog_dir="${0%/*}"

# Usage: usage [<fmt> [...]]
usage()
{
	local -i rc=$?
	local fmt="$1"
	shift
	local -i fd

	[ $rc -eq 0 ] && fd=1 || fd=2
	printf -- "$fmt" "$@" >>"$fd"
	printf -- 'Usage: %s { init| fini } <config>\n' "$prog_name" >>"$fd"

	exit $rc
}
declare -fr usage

## See how we've called

declare -r action="$1"

case "$action" in
	'init'|'fini')
		;;
	*)
		usage 'unknown action %s\n' "$action"
		;;
esac

# Check number of arguments
[ $# -eq 2 ] ||
	usage '%s requires config file as second argument\n' "$action"

## Source config and environment initialization code

source "$prog_dir/libcfg.sh" "$2"

## Try to cleanup first

fini &>/dev/null ||:

[ "$action" != 'fini' ] || exit 0

## Install exit handler to cleanup on failure

__exit_handler()
{
	local -i rc=$?
	[ $rc -eq 0 ] || fini
	return $rc
}

trap '__exit_handler' EXIT

## Load module

declare m="$HOME/tests/$MODULE/$MODULE.ko"
if [ -f "$m" ]; then
	# Load out-of-tree module dependencies manually
	modprobe ptp ||:
	modprobe dca ||:
	modprobe i2c-core &>/dev/null ||:
	modprobe i2c-algo-bit &>/dev/null ||:

	# Load module
	insmod "$m"
else
	# Dependencies will be loaded automatically
	modprobe "$MODULE"
fi

## Create devices in network namespaces

for ((i = 0; i < NETNS_NR; i++)); do
	name="${NETNS_NAMES[$i]}"
	lowerdev="${LOWERDEVS[$i]}"
	upperdev="${UPPERDEVS[$i]}"
	upperipa="${UPPERIPAS[$i]}"
	uppermac="${UPPERMACS[$i]}"

	init "$name" "$lowerdev" "$upperdev" "$upperipa" "$uppermac"
done

exit 0
