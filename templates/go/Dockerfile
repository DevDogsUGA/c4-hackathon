FROM golang:1.23-alpine AS build

WORKDIR /app
COPY go.mod *.go ./
RUN CGO_ENABLED=0 go build -o bot .

FROM gcr.io/distroless/static-debian12

WORKDIR /app
COPY --from=build /app/bot ./bot

ENV PORT=8000
EXPOSE 8000

CMD ["./bot"]
