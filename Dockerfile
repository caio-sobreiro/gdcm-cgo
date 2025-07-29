FROM debian:bullseye AS gdcm-builder

RUN apt-get update && \
    apt-get install -y --no-install-recommends \
    cmake \
    g++ \
    git \
    make \
    libexpat-dev \
    libjpeg-dev \
    libopenjp2-7-dev \
    libxml2-dev \
    liblzma-dev \
    libcharls-dev \
    zlib1g-dev \
    ca-certificates

RUN git clone https://github.com/malaterre/GDCM.git /gdcm && \
    mkdir /gdcm/build && cd /gdcm/build && \
    cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local && \
    make -j$(nproc) && make install


# --------- Stage 2: Build Go App ---------
FROM golang:1.24 AS go-builder

# Copy GDCM from previous stage
COPY --from=gdcm-builder /usr/local /usr/local

# Install runtime libraries for linking
RUN apt-get update && apt-get install -y \
    libexpat-dev \
    libjpeg-dev \
    libopenjp2-7-dev \
    libxml2-dev \
    liblzma-dev \
    libcharls-dev \
    zlib1g-dev

WORKDIR /app
COPY . .

# Build the Go binary
RUN go build -o dicom-j2k

# --------- Stage 3: Runtime Image ---------
FROM debian:bullseye AS runtime

# Install only required runtime libraries (no dev packages)
RUN apt-get update && apt-get install -y --no-install-recommends \
    libexpat1 \
    libjpeg62-turbo \
    libopenjp2-7 \
    libxml2 \
    liblzma5 \
    libcharls-dev \
    zlib1g \
    ca-certificates && \
    apt-get clean && rm -rf /var/lib/apt/lists/*

# Copy built binary
COPY --from=go-builder /app/dicom-j2k /usr/local/bin/dicom-j2k

# Create input/output directories (can be overridden with volumes)
WORKDIR /input
VOLUME ["/input", "/output"]

ENTRYPOINT ["/usr/local/bin/dicom-j2k"]
