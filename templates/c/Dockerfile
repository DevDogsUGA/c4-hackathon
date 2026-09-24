FROM gcc:13-bookworm AS build

WORKDIR /app
COPY Makefile *.c *.h ./
COPY vendor ./vendor
RUN make

FROM debian:bookworm-slim

WORKDIR /app
COPY --from=build /app/bot ./bot

ENV PORT=8000
EXPOSE 8000

CMD ["./bot"]
