FROM emscripten/emsdk:3.1.61 AS build
WORKDIR /src
COPY . .
RUN make web

FROM nginx:alpine
COPY --from=build /src/www /usr/share/nginx/html
