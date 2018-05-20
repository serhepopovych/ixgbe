#!/bin/bash

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
	printf -- "$fmt" "$@" >&$fd
	printf -- 'Usage: %s <options to git-format-patch(1)>\n' \
		"$prog_name" >&$fd

	exit $rc
}
declare -fr usage

################################################################################

declare -a output_dir=('-o' '/tmp')

while getopts ':o:' c; do
	case "$c" in
		'o')
			output_dir=()
			;;
		*)
			# Only subset of options parsed. Not throwing
			# errors for unknown options or missing argument
			# for passthru options since we are just wrapper.
			;;
	esac
done

exec git format-patch \
	--no-to \
	--signoff \
	--no-thread \
	--no-cover-letter \
	"${output_dir[@]}" \
	"$@"
