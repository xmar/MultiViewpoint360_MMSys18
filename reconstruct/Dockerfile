FROM xmar/ffmpeg_mp4box:mmsys18
ARG DOCKER_GROUP_ID

RUN buildDeps="python3-pip" &&\
    apt-get -yqq update && \
    apt-get install -yq --no-install-recommends python3 ${buildDeps}
ADD requirements.txt .
RUN pip3 install --upgrade pip&&\
    pip3 install -r requirements.txt &&\
    rm -f requirements.txt &&\
    apt-get purge -y ${buildDeps} && \
    apt-get autoremove -y && \
    apt-get clean -y
WORKDIR /home/reconstruct
RUN useradd -r -u 1000 -g users xmar && groupadd -g $DOCKER_GROUP_ID docker && usermod -aG docker xmar && mkdir /tmp/trans360 && chown -R xmar:users /tmp/trans360
USER xmar
ADD reconstruct_viewport_quality.py printContainerLogs.py /bin/
#ENTRYPOINT ["reconstruct_viewport_quality.py"]
