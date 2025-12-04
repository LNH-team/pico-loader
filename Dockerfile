# -----------------------------------------------------------------------------
# Dockerfile for building pico-loader (with .NET 9.0)
# -----------------------------------------------------------------------------

FROM --platform=linux/amd64 skylyrac/blocksds:slim-v1.13.1

WORKDIR /workdir

RUN apt-get update && apt-get install -y \
    git \
    make \
    wget \
    ca-certificates \
    && rm -rf /var/lib/apt/lists/*

RUN wget https://packages.microsoft.com/config/ubuntu/22.04/packages-microsoft-prod.deb -O packages-microsoft-prod.deb \
    && dpkg -i packages-microsoft-prod.deb \
    && rm packages-microsoft-prod.deb \
    && apt-get update \
    && apt-get install -y dotnet-sdk-9.0 \
    && rm -rf /var/lib/apt/lists/*

ENV TARGET_PLATFORM=DSPICO

COPY . .

CMD ["sh", "-c", "make clean && make PICO_PLATFORM=${TARGET_PLATFORM}"]