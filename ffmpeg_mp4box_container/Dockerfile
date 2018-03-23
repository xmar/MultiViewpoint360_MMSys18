FROM jrottenberg/ffmpeg

ENTRYPOINT  []

ENV     MP4BOX_VERSION=v0.7.1

RUN      buildDeps="build-essential \
                    fakeroot \
                    dpkg-dev \
                    devscripts \
                    ccache \
                    debhelper \
                    pkg-config \
                    git \
                    gcc \
                    make \
                    libpthread-stubs0-dev \
                    g++" && \
        apt-get -yqq update && \
        apt-get install -yq --no-install-recommends ${buildDeps} && \
        DIR=$(mktemp -d) && cd ${DIR} && \
## MP4Box https://gpac.wp.mines-telecom.fr/mp4box/
        git clone https://github.com/gpac/gpac.git . && \
        git checkout tags/${MP4BOX_VERSION} && \
        ./configure --prefix="${SRC}" --static-mp4box --use-zlib=no && \
        make && \
        make install && \
        ldconfig && \
        rm -rf ${DIR} && \
        cd && \
        apt-get purge -y ${buildDeps} && \
        apt-get autoremove -y && \
        apt-get clean -y && \
        rm -rf /var/lib/apt/lists \
