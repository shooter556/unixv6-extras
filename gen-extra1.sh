#!/usr/bin/env bash
set -euo pipefail

DISTRO="${1:-}"
if [[ -z $DISTRO ]]; then
    echo
    echo "./$(basename $0) <distro>"
    echo
    echo "Please specify distro."
    echo "Available options are: v6, v7, 1bsd, 2bsd, ..."
    echo
    exit 1
fi

DIRNAME="${HOME}/vm/simhsys/unix/${DISTRO}"
if [[ ! -d ${DIRNAME} ]]; then
    mkdir -p "${DIRNAME}"
fi

# Link the folders into one place, so that their names on the tape
# have no vanilla/ prefix.
ROOT="${PWD}"
STAGE="$(mktemp -d)"
trap 'rm -rf "${STAGE}"' EXIT
for d in aredit fsck halt man setclock sh sysconf time trees tty; do
    ln -s "${ROOT}/vanilla/${d}" "${STAGE}/${d}"
done
ln -s "${ROOT}/em" "${STAGE}/em"

cd "${STAGE}"
"${ROOT}/vanilla/uxtp/uxtp.sh" -r -v -s -o 0:0 ${DIRNAME}/extra1.tap \
aredit em fsck halt man setclock sh sysconf time trees tty
