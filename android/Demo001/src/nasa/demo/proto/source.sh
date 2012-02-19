#!/bin/bash
shell_location=${0%/*}/
protoc --java_out=$shell_location/../../../ $shell_location/source.proto
