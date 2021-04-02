FROM gcr.io/broad-getzlab-workflows/base_image:v0.0.4

WORKDIR build
COPY src .
RUN make

WORKDIR /app
ENV PATH=$PATH:/app
RUN mv /build/build/get_all_calls ./tokenizer
