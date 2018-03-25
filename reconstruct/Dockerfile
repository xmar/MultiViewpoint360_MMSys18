FROM xmar/recontruct_tiled_mpv360_mmsys18:base
ARG DOCKER_GROUP_ID

RUN useradd -r -u 1000 -g users xmar && groupadd -g $DOCKER_GROUP_ID docker && usermod -aG docker xmar && mkdir /tmp/trans360 && chown -R xmar:users /tmp/trans360
USER xmar
ADD reconstruct_viewport_quality.py printContainerLogs.py /bin/
#ENTRYPOINT ["reconstruct_viewport_quality.py"]
