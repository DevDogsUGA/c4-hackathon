// HTTP server for your Connect Four bot (C#, ASP.NET Core minimal API; no
// NuGet packages needed).
//
// You shouldn't need to edit this file. Write your bot in Bot.cs: this
// server handles HTTP, CORS headers, and JSON parsing, then calls
// `Bot.ChooseMove` once per turn.
//
// Run it:
//   dotnet run
// Then test it:
//   curl -X POST http://localhost:8000/move \
//     -H "Content-Type: application/json" \
//     -d '{"you":1,"board":[[0,0,0,0,0,0,0,0],[0,0,0,0,0,0,0,0],[0,0,0,0,0,0,0,0],[0,0,0,0,0,0,0,0],[0,0,0,0,0,0,0,0],[0,0,0,0,0,0,0,0],[0,0,0,0,0,0,0,0],[0,0,0,0,0,0,0,0]],"moves":[],"game":{"match_id":"local","game_number":1,"clock_remaining_ms":5000}}'

using System.Text.Json;
using C4Bot;

// Columns that aren't full yet. Kept separate from Bot.cs so the server's
// safety check still works however you change your bot.
static List<int> LegalColumns(int[][] board)
{
    var moves = new List<int>();
    for (var col = 0; col < board.Length; col++)
    {
        if (board[col][^1] == 0)
        {
            moves.Add(col);
        }
    }
    return moves;
}

var port = Environment.GetEnvironmentVariable("PORT") ?? "8000";

var builder = WebApplication.CreateBuilder(args);
builder.Logging.ClearProviders();
builder.Logging.AddFilter(null, LogLevel.Warning);
builder.WebHost.UseUrls($"http://0.0.0.0:{port}");

var app = builder.Build();

app.Run(async context =>
{
    var request = context.Request;
    var response = context.Response;

    // CORS headers on every response.
    response.Headers["Access-Control-Allow-Origin"] = "*";
    response.Headers["Access-Control-Allow-Methods"] = "GET, POST, OPTIONS";
    response.Headers["Access-Control-Allow-Headers"] = "Content-Type";
    // Chrome Private Network Access: lets a hosted https:// page call
    // http://localhost during local testing.
    response.Headers["Access-Control-Allow-Private-Network"] = "true";
    response.ContentType = "application/json";

    async Task Send(int status, object? bodyObj = null)
    {
        response.StatusCode = status;
        if (bodyObj is not null)
        {
            await response.WriteAsync(JsonSerializer.Serialize(bodyObj));
        }
    }

    if (HttpMethods.IsOptions(request.Method))
    {
        await Send(204);
        return;
    }

    if (HttpMethods.IsGet(request.Method) && request.Path == "/health")
    {
        await Send(200, new { status = "ok" });
        return;
    }

    if (HttpMethods.IsPost(request.Method) && request.Path == "/move")
    {
        try
        {
            using var doc = await JsonDocument.ParseAsync(request.Body);
            var root = doc.RootElement;

            var board = root.GetProperty("board")
                .EnumerateArray()
                .Select(col => col.EnumerateArray().Select(cell => cell.GetInt32()).ToArray())
                .ToArray();
            var you = root.GetProperty("you").GetInt32();

            var moves = root.TryGetProperty("moves", out var movesEl)
                ? movesEl.EnumerateArray().Select(m => m.GetInt32()).ToList()
                : new List<int>();
            var matchId = "";
            var gameNumber = 0;
            long clockRemainingMs = 0;
            if (root.TryGetProperty("game", out var gameEl))
            {
                if (gameEl.TryGetProperty("match_id", out var matchIdEl))
                    matchId = matchIdEl.GetString() ?? "";
                if (gameEl.TryGetProperty("game_number", out var gameNumberEl))
                    gameNumber = gameNumberEl.GetInt32();
                if (gameEl.TryGetProperty("clock_remaining_ms", out var clockEl))
                    clockRemainingMs = clockEl.GetInt64();
            }
            var info = new MoveInfo(moves, matchId, gameNumber, clockRemainingMs);

            var column = Bot.ChooseMove(board, you, info);

            if (!LegalColumns(board).Contains(column))
            {
                throw new InvalidOperationException($"ChooseMove returned an illegal column: {column}");
            }

            await Send(200, new { column });
        }
        catch (Exception exc)
        {
            await Send(500, new { error = exc.Message });
        }
        return;
    }

    await Send(404);
});

Console.WriteLine($"Bot listening on http://0.0.0.0:{port}");
app.Run();
