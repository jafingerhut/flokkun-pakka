#! /bin/bash

linux_version_warning() {
    1>&2 echo "Found ID ${ID} and VERSION_ID ${VERSION_ID} in /etc/os-release"
    1>&2 echo "This script only supports these:"
    1>&2 echo "    ID ubuntu, VERSION_ID in 20.04 22.04"
    1>&2 echo ""
    1>&2 echo "Proceed installing manually at your own risk of"
    1>&2 echo "significant time spent figuring out how to make it all"
    1>&2 echo "work, or consider getting VirtualBox and creating a"
    1>&2 echo "virtual machine with one of the tested versions."
}

if [ ! -r /etc/os-release ]
then
    1>&2 echo "No file /etc/os-release.  Cannot determine what OS this is."
    linux_version_warning
    exit 1
fi
source /etc/os-release

supported_distribution=0
if [ "${ID}" = "ubuntu" ]
then
    case "${VERSION_ID}" in
	20.04)
	    supported_distribution=1
	    ;;
	22.04)
	    supported_distribution=1
	    ;;
	24.04)
	    supported_distribution=1
	    ;;
    esac
fi

if [ ${supported_distribution} -eq 1 ]
then
    echo "Found supported ID ${ID} and VERSION_ID ${VERSION_ID} in /etc/os-release"
else
    linux_version_warning
    exit 1
fi

set -ex

set +x
echo "------------------------------------------------------------"
echo "Time and disk space used before installation begins:"
set -x
date
df -BM .

# Common packages to install on all tested Ubuntu versions
sudo apt-get --yes install \
    make \
    texlive-xetex \
    dvipng \
    texlive-fonts-extra \
    texlive-science

set +x
echo "------------------------------------------------------------"
echo "Time and disk space used just before 'apt clean':"
set -x
date
df -BM .

# After install of the packages above, this command often seems to
# help reduce the disk space used by a gigabyte or so.
sudo apt clean

set +x
echo "------------------------------------------------------------"
echo "Time and disk space used when installation was complete:"
set -x
date
df -BM .
