FROM node:current-buster
RUN apt-get update && export DEBIAN_FRONTEND=noninteractive \
    && apt-get -y install --no-install-recommends \
    ffmpeg \
    libavcodec-dev \
    libavformat-dev \
    libavresample-dev \
    mesa-common-dev \
    libglu1-mesa-dev \
    freeglut3-dev \
    libswscale-dev
WORKDIR /workspace
COPY package*json /workspace/
RUN npm install
COPY . /workspace
RUN npm run compile
CMD [ "node", "src/main.js" ]