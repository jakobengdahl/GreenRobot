FROM arm32v7/node:latest
WORKDIR /app
COPY package.json /app
RUN npm install
COPY . /app
CMD node Robot.js
