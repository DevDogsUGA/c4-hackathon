FROM mcr.microsoft.com/dotnet/sdk:8.0 AS build

WORKDIR /src
COPY Bot.csproj ./
RUN dotnet restore Bot.csproj

COPY *.cs ./
RUN dotnet publish Bot.csproj -c Release -o /app

FROM mcr.microsoft.com/dotnet/aspnet:8.0

WORKDIR /app
COPY --from=build /app ./

ENV PORT=8000
EXPOSE 8000

CMD ["dotnet", "Bot.dll"]
