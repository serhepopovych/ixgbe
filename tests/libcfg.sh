#!/bin/bash -xe

[ -n "$__included_libcfg_sh" ] && return
declare -r __included_libcfg_sh=1

# Prepare environment, see how we've called and check arguments

# Usage: fatal [<fmt> [...]]
fatal()
{
	local -i rc=$?
	local fmt="${1:?missing 1st argument to \"$FUNCNAME\" (fmt)}"
	shift
	local fd

	[ $rc -eq 0 ] && fd=1 || fd=2
	printf -- "$fmt" "$@" >>"$fd"

	exit $rc
}
declare -fr fatal

# program invocation name short
[ -n "$prog_name" ] || declare prog_name="${0##*/}"
declare -r prog_name

# program directory
[ -n "$prog_dir" ] || declare prog_dir="${0%/*}"
declare -r prog_dir

# Requires root privileges to run
[ $UID -eq 0 ] ||
	fatal 'root (uid: 0) privileges required, but uid is %u\n' $UID

## Source configuration file

# Usage: test_fsr <file>
test_fsr()
{
	local file="${1:?missing 1st argument to \"$FUNCNAME\" (file)}"

	# Configuration file is:
	#
	#  1) regular file (i.e. not socket, fifo, device, directory)
	#  2) has size greather than zero
	#  3) readable by current user
	#
	test -f "$file" -a \
	     -s "$file" -a \
	     -r "$file"
}
declare -fr test_fsr

declare config
declare cfg="$1"

# First try config in same directory as script
config="$prog_dir/$cfg"
if ! test_fsr "$config"; then
	# Next try config specified on command line
	config="$cfg"
	test_fsr "$config" ||
		fatal 'missing, empty or unreadable config file\n'
fi

# Do not search for configs in "$PATH"
declare sourcepath_save="$(shopt -p sourcepath)"
shopt -u sourcepath

source "$config" ||
	fatal 'cannot source configuration file %s\n' "$config"

eval "$sourcepath_save"

# Check for configuration file data consistency
declare -ir NETNS_NR=${#NETNS_NAMES[@]}

[ $NETNS_NR -gt 0 ] ||
	fatal 'number of network namespaces in NETNS_NAMES must be > 0\n'

declare -i LOWERDEVS_NR=${#LOWERDEVS[@]}
[ $NETNS_NR -eq $LOWERDEVS_NR ] ||
	usage 'number of %s(%u) is not the same as number of NETNS(%u)\n' \
		'LOWERDEVS' $NETNS_NR $LOWERDEVS_NR

declare -i UPPERDEVS_NR=${#UPPERDEVS[@]}
[ $NETNS_NR -eq $UPPERDEVS_NR ] ||
	usage 'number of %s(%u) is not the same as number of NETNS(%u)\n' \
		'UPPERDEVS' $NETNS_NR $UPPERDEVS_NR

declare -i UPPERMACS_NR=${#UPPERMACS[@]}
[ $NETNS_NR -eq $UPPERMACS_NR ] ||
	usage 'number of %s(%u) is not the same as number of NETNS(%u)\n' \
		'UPPERMACS' $NETNS_NR $UPPERMACS_NR

declare -i UPPERIPAS_NR=${#UPPERIPAS[@]}
[ $NETNS_NR -eq $UPPERIPAS_NR ] ||
	usage 'number of %s(%u) is not the same as number of NETNS(%u)\n' \
		'UPPERIPAS' $NETNS_NR $UPPERIPAS_NR

for ((i = 0; i < NETNS_NR; i++)); do
	[ -n "${NETNS_NAMES[$i]}" ] ||
		usage '%s cannot be empty (index: %u)\n' \
			'netns name' $i
	[ -n "${LOWERDEVS[$i]}" ] ||
		usage '%s cannot be empty (index: %u)\n' \
			'upper device name' $i
	[ -n "${UPPERDEVS[$i]}" ] ||
		usage '%s cannot be empty (index: %u)\n' \
			'lower device name' $i
	# MACs can be empty: either random is used in case of MACVLAN or
	# inherited from parent network device in case of VLAN
	[ -n "${UPPERIPAS[$i]}" ] ||
		usage '%s cannot be empty (index: %u)\n' \
			'ip address' $i
done

[ -n "$MODULE" ] ||
	usage '%s must be specified\n' 'MODULE'

[ -n "$IPA_TEMPL" ] ||
	usage '%s must be specified\n' 'IPA_TEMPL'

# Cleanup environment
unset i cfg config sourcepath_save
unset LOWERDEVS_NR UPPERDEVS_NR UPPERMACS_NR UPPERIPAS_NR

:
