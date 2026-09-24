FROM node:22-slim

WORKDIR /app
COPY package.json package-lock.json ./
RUN npm ci

COPY tsconfig.json *.ts ./

ENV PORT=8000
EXPOSE 8000

CMD ["node", "--import", "tsx", "server.ts"]
