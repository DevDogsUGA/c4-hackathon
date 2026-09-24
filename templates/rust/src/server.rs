//! HTTP server for your Connect Four bot (Rust; tiny_http + serde for JSON).
//!
//! You shouldn't need to edit this file. Write your bot in bot.rs: this server
//! handles HTTP, CORS headers, and JSON parsing, then calls `choose_move` from
//! bot.rs once per turn.
//!
//! Run it:
//!     cargo run --release
//! Then test it:
//!     curl -X POST http://localhost:8000/move \
//!       -H "Content-Type: application/json" \
//!       -d '{"you":1,"board":[[0,0,0,0,0,0,0,0],[0,0,0,0,0,0,0,0],[0,0,0,0,0,0,0,0],[0,0,0,0,0,0,0,0],[0,0,0,0,0,0,0,0],[0,0,0,0,0,0,0,0],[0,0,0,0,0,0,0,0],[0,0,0,0,0,0,0,0]],"moves":[],"game":{"match_id":"local","game_number":1,"clock_remaining_ms":10000}}'

mod bot;

use std::panic::{self, AssertUnwindSafe};
use std::sync::Arc;
use std::thread;

use serde::Deserialize;
use serde_json::{json, Value};
use tiny_http::{Header, Method, Response, Server};

// Columns that aren't full yet. Kept separate from bot.rs so the server's
// safety check still works however you change your bot.
fn legal_columns(board: &[Vec<u8>]) -> Vec<usize> {
    board
        .iter()
        .enumerate()
        .filter(|(_, column)| matches!(column.last(), Some(0)))
        .map(|(col, _)| col)
        .collect()
}

#[derive(Deserialize)]
struct MoveRequest {
    board: Vec<Vec<u8>>,
    you: u8,
}

fn cors_headers() -> Vec<Header> {
    vec![
        Header::from_bytes(&b"Access-Control-Allow-Origin"[..], &b"*"[..]).unwrap(),
        Header::from_bytes(
            &b"Access-Control-Allow-Methods"[..],
            &b"GET, POST, OPTIONS"[..],
        )
        .unwrap(),
        Header::from_bytes(&b"Access-Control-Allow-Headers"[..], &b"Content-Type"[..]).unwrap(),
        // Chrome Private Network Access: lets a hosted https:// page call
        // http://localhost during local testing.
        Header::from_bytes(&b"Access-Control-Allow-Private-Network"[..], &b"true"[..]).unwrap(),
        Header::from_bytes(&b"Content-Type"[..], &b"application/json"[..]).unwrap(),
    ]
}

fn send(request: tiny_http::Request, status: u16, body: Option<Value>) {
    let payload = body.map(|v| v.to_string()).unwrap_or_default();
    let response = cors_headers().into_iter().fold(
        Response::from_string(payload).with_status_code(status),
        |r, h| r.with_header(h),
    );
    let _ = request.respond(response);
}

fn handle_move(request: tiny_http::Request, raw: &str) {
    let result: Result<Value, String> = (|| {
        let parsed: MoveRequest =
            serde_json::from_str(raw).map_err(|e| format!("invalid JSON body: {e}"))?;

        let board = parsed.board;
        let you = parsed.you;

        let outcome = panic::catch_unwind(AssertUnwindSafe(|| bot::choose_move(&board, you)));

        let column = match outcome {
            Ok(column) => column,
            Err(payload) => {
                let message = payload
                    .downcast_ref::<&str>()
                    .map(|s| s.to_string())
                    .or_else(|| payload.downcast_ref::<String>().cloned())
                    .unwrap_or_else(|| "bot panicked".to_string());
                return Err(format!("choose_move panicked: {message}"));
            }
        };

        if !legal_columns(&board).contains(&column) {
            return Err(format!("choose_move returned an illegal column: {column}"));
        }

        Ok(json!({ "column": column }))
    })();

    match result {
        Ok(body) => send(request, 200, Some(body)),
        Err(message) => send(request, 500, Some(json!({ "error": message }))),
    }
}

fn handle(mut request: tiny_http::Request) {
    match (request.method(), request.url()) {
        (Method::Options, _) => send(request, 204, None),
        (Method::Get, "/health") => send(request, 200, Some(json!({ "status": "ok" }))),
        (Method::Post, "/move") => {
            let mut raw = String::new();
            if let Err(e) = request.as_reader().read_to_string(&mut raw) {
                send(
                    request,
                    500,
                    Some(json!({ "error": format!("failed to read body: {e}") })),
                );
                return;
            }
            handle_move(request, &raw);
        }
        _ => send(request, 404, None),
    }
}

fn main() {
    // Keep panic output out of the bot's own logs, matching the other
    // templates' quiet request handling; the client still gets a 500.
    panic::set_hook(Box::new(|_| {}));

    // Exit promptly on `docker stop` / Ctrl-C (as PID 1 in a container,
    // signals without a handler are ignored and Docker waits 10s).
    ctrlc::set_handler(|| std::process::exit(0)).expect("failed to set signal handler");

    let port = std::env::var("PORT")
        .ok()
        .and_then(|p| p.parse::<u16>().ok())
        .unwrap_or(8000);

    let server = Arc::new(Server::http(("0.0.0.0", port)).unwrap_or_else(|e| {
        eprintln!("failed to bind to port {port}: {e}");
        std::process::exit(1);
    }));

    println!("Bot listening on http://0.0.0.0:{port}");

    let workers: Vec<_> = (0..4)
        .map(|_| {
            let server = Arc::clone(&server);
            thread::spawn(move || {
                for request in server.incoming_requests() {
                    handle(request);
                }
            })
        })
        .collect();

    for worker in workers {
        let _ = worker.join();
    }
}
