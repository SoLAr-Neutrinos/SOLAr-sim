# =========================================================
# AlmaLinux 9 image with Git + CVMFS configuration
# ROOT and Geant4 come from /cvmfs (host-mounted)
# =========================================================
FROM cern/alma9-base:latest

# Basic system setup
RUN dnf update -y && \
    dnf install -y epel-release && \
    dnf install -y https://cvmrepo.s3.cern.ch/cvmrepo/yum/cvmfs-release-latest.noarch.rpm && \
    dnf install -y \
        git \
        wget \
        tar \
        which \
        python3 \
        python3-pip \
        nano \
        lsof \
        strace \
        fuse \
        cvmfs \
        cvmfs-config-default \
        && \
    dnf clean all

# Create CVMFS configuration
RUN echo "CVMFS_REPOSITORIES=sft.cern.ch" >> /etc/cvmfs/default.local && \
    echo "CVMFS_HTTP_PROXY=DIRECT" >> /etc/cvmfs/default.local

RUN 

# Entry shell
CMD ["/bin/bash"]

