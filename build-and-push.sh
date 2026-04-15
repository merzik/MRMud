#!/bin/bash

# Build and push script for the Mortal Realms Docker image.
# Usage: ./build-and-push.sh [tag]
#   If tag is not provided, defaults to 'latest'.

set -e

IMAGE_NAME="mrmud"
REGISTRY="registry.merzik.net"
FULL_IMAGE_NAME="${REGISTRY}/${IMAGE_NAME}"
CURRENT_CONTEXT="$(docker context show)"
SANITIZED_CONTEXT="$(echo "${CURRENT_CONTEXT}" | tr -c '[:alnum:]' '-')"

TAG="${1:-latest}"
FULL_TAG="${FULL_IMAGE_NAME}:${TAG}"

echo "=========================================="
echo "Building and Pushing Mortal Realms"
echo "=========================================="
echo "Image: ${FULL_TAG}"
echo "Docker context: ${CURRENT_CONTEXT}"
echo ""

echo "Step 1: Setting up buildx..."
docker buildx version > /dev/null 2>&1 || (echo "docker buildx not available!" && exit 1)

# Create a context-specific builder so we do not reuse one pinned to another Docker endpoint.
BUILDER_NAME="mrmud-builder-${SANITIZED_CONTEXT}"
if ! docker buildx inspect "${BUILDER_NAME}" > /dev/null 2>&1; then
    echo "Creating buildx builder instance for context '${CURRENT_CONTEXT}'..."
    docker buildx create \
        --name "${BUILDER_NAME}" \
        --driver docker-container \
        --use \
        "${CURRENT_CONTEXT}"
fi

docker buildx use "${BUILDER_NAME}" > /dev/null 2>&1
docker buildx inspect "${BUILDER_NAME}" --bootstrap > /dev/null

echo "Buildx ready"
echo ""

echo "Step 2: Building and pushing multi-architecture image (linux/amd64, linux/arm64)..."
docker buildx build \
    --builder "${BUILDER_NAME}" \
    --platform linux/amd64,linux/arm64 \
    --tag "${FULL_TAG}" \
    --push \
    .

echo "Multi-architecture image built and pushed successfully"
echo ""
echo "=========================================="
echo "Success! Multi-arch image available at:"
echo "   ${FULL_TAG}"
echo "   Platforms: linux/amd64, linux/arm64"
echo "=========================================="
