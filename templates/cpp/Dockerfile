FROM gcc:13-bookworm AS build

WORKDIR /app
COPY Makefile *.cpp *.hpp ./
COPY vendor ./vendor

# Static libstdc++: the gcc image's libstdc++ is newer than the runtime image's.
RUN make LDFLAGS="-pthread -static-libstdc++ -static-libgcc"

FROM debian:bookworm-slim

WORKDIR /app
COPY --from=build /app/bot ./bot

ENV PORT=8000
EXPOSE 8000

CMD ["./bot"]
