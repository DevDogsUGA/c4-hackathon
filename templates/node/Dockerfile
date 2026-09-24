FROM node:20-slim

WORKDIR /app
COPY package.json *.js ./

ENV PORT=8000
EXPOSE 8000

CMD ["node", "server.js"]
