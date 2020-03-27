#!/bin/bash

docker build -t broadinstitute/tokenizer:v1 .

docker tag broadinstitute/tokenizer:v1 gcr.io/broad-getzlab-workflows/tokenizer:v1
docker tag broadinstitute/tokenizer:v1 gcr.io/broad-getzlab-workflows/tokenizer:latest

docker push gcr.io/broad-getzlab-workflows/tokenizer:v1
docker push gcr.io/broad-getzlab-workflows/tokenizer:latest
