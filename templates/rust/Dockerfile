FROM rust:1-slim AS build

WORKDIR /app

# Cache the dependency build in its own layer so editing bot.rs doesn't
# re-download and re-compile every crate.
COPY Cargo.toml Cargo.lock ./
RUN mkdir src && echo "fn main() {}" > src/server.rs
RUN cargo build --release
RUN rm -rf src

COPY src ./src
RUN touch src/server.rs src/bot.rs && cargo build --release

FROM debian:bookworm-slim

WORKDIR /app
COPY --from=build /app/target/release/bot ./bot

ENV PORT=8000
EXPOSE 8000

CMD ["./bot"]
