#!/usr/bin/bash
#shellcheck shell=bash

if ! ps -p $PPID -o cmd= | grep -qE '^make '; then
    echo >&2 "ERROR: This script should be called from Makefile"
    exit 1
fi

REQUIRES_CMD=(
    "rpmbuild"
    "rpmdev-setuptree"
    "pushd"
    "popd"
    "rpmspec"
)

for cmd in "${REQUIRES_CMD[@]}"; do
    if ! command -v "${cmd}" &>/dev/null; then
        echo "ERROR: ${cmd} not found"
        exit 1
    fi
done

SPEC_FILE="./isosm3sum.spec"
if [[ ! -f "${SPEC_FILE}" ]]; then
    echo "ERROR: ${SPEC_FILE} not found"
    exit 1
fi

SOURCE0="$(rpmspec -P "${SPEC_FILE}" -D 'dist %{nil}' -q --srpm --qf="%{name}-%{version}.tar.bz2")"
if [[ ! -f "${SOURCE0:?}" ]]; then
    echo "ERROR: ${SOURCE0} not found"
    exit 1
fi

RPMBUILD_TOPDIR="$(realpath .)"
rpmdev-setuptree -D "_topdir ${RPMBUILD_TOPDIR}"
if [[ ! -d "${RPMBUILD_TOPDIR:?}" ]]; then
    echo "ERROR: ${RPMBUILD_TOPDIR} not found"
    exit 1
fi

SRPM_NAME="${RPMBUILD_TOPDIR}/SRPMS/$(rpmspec -P "${SPEC_FILE}" -D 'dist %{nil}' \
    -q --srpm --qf="%{name}-%{version}-%{release}.src.rpm")"
rpmbuild -bs --target noarch "${SPEC_FILE}" --nodeps -D "dist %{nil}" \
    -D "_topdir ${RPMBUILD_TOPDIR}" \
    -D "_sourcedir ${RPMBUILD_TOPDIR}" &>/dev/null || {
    echo "rpmbuild failed."
    exit 1
}

if [[ ! -f "${SRPM_NAME:?}" ]]; then
    echo "ERROR: ${SRPM_NAME} not found"
    exit 1
fi

echo "Source RPM: ${SRPM_NAME}"
