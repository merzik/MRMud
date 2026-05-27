FROM ubuntu:24.04 AS build

ARG DEBIAN_FRONTEND=noninteractive

RUN apt-get update \
    && apt-get install -y --no-install-recommends \
        build-essential \
        libcrypt-dev \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /game
COPY . /game

RUN find /game -name .gitkeep -type f -delete \
    && mkdir -p /game/bin \
    && make -C /game/src clean \
    && make -C /game/src \
    && cp /game/bin/md /game/bin/next_md \
    && mkdir -p /usr/local/share/mrmud-seed/player/c \
    && cp /game/player/c/Chaos /usr/local/share/mrmud-seed/player/c/Chaos \
    && cp -a /game/areas /usr/local/share/mrmud-seed/areas \
    && cp -a /game/clans /usr/local/share/mrmud-seed/clans

FROM ubuntu:24.04

ARG DEBIAN_FRONTEND=noninteractive

RUN apt-get update \
    && apt-get install -y --no-install-recommends \
        bash \
        gdb \
        gosu \
        libcrypt1 \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /game
COPY --from=build /game /game
COPY --from=build /usr/local/share/mrmud-seed /usr/local/share/mrmud-seed
COPY docker-entrypoint.sh /usr/local/bin/mrmud-entrypoint

RUN chmod +x /usr/local/bin/mrmud-entrypoint

ENV MRMUD_PORT=4321

EXPOSE 4321
VOLUME ["/game/log", "/game/player", "/game/areas", "/game/clans"]

ENTRYPOINT ["mrmud-entrypoint"]
CMD ["startup"]
