# Use a lighter version of Node as a parent image
# FROM mhart/alpine-node:10
FROM node:10
# Set the working directory to /api
WORKDIR /server
# copy package.json into the container at /api
COPY package*.json /server/
# install nodemon
RUN npm install -g nodemon
# install dependencies
RUN npm install
# Copy the current directory contents into the container at /api
COPY . /server/
# Make port 80 available to the world outside this container
# EXPOSE 3001
# Run the app when the container launches
CMD [ "nodemon", "src/server.js" ]