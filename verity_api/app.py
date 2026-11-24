import hmac
import os

import requests
from flask import Flask, jsonify, request

app = Flask(__name__)
app.config["MAX_CONTENT_LENGTH"] = 256 * 1024

UPSTREAM_URL = os.getenv("VERITY_UPSTREAM_URL", "").strip()
UPSTREAM_KEY = os.getenv("VERITY_API_KEY", "").strip()
UPSTREAM_MODEL = os.getenv("VERITY_MODEL", "").strip()
GATEWAY_TOKEN = os.getenv("VERITY_GATEWAY_TOKEN", "").strip()


def authorized():
    if not GATEWAY_TOKEN:
        return False
    supplied = request.headers.get("Authorization", "")
    if not supplied.startswith("Bearer "):
        return False
    return hmac.compare_digest(supplied[7:].strip(), GATEWAY_TOKEN)


@app.get("/health")
def health():
    return jsonify({
        "service": "APOCALYPSE-VERITY-API",
        "status": "online",
        "model_configured": bool(UPSTREAM_URL and UPSTREAM_MODEL),
        "provider_key_configured": bool(UPSTREAM_KEY),
        "gateway_auth_configured": bool(GATEWAY_TOKEN),
    })


@app.post("/v1/chat/completions")
def chat_completions():
    if not authorized():
        return jsonify({"error": {"message": "Acesso não autorizado ao gateway."}}), 401
    if not UPSTREAM_URL or not UPSTREAM_MODEL or not UPSTREAM_KEY:
        return jsonify({
            "error": {
                "message": "A API da VERITY ainda não foi configurada no Render."
            }
        }), 503

    payload = request.get_json(silent=True)
    if not isinstance(payload, dict):
        return jsonify({"error": {"message": "Envie um JSON válido."}}), 400
    messages = payload.get("messages")
    if not isinstance(messages, list) or not messages:
        return jsonify({"error": {"message": "O campo messages é obrigatório."}}), 400

    safe_payload = {
        "model": UPSTREAM_MODEL,
        "messages": messages,
        "temperature": payload.get("temperature", 0.7),
        "stream": False,
    }

    headers = {
        "Authorization": f"Bearer {UPSTREAM_KEY}",
        "Content-Type": "application/json",
        "Accept": "application/json",
    }

    try:
        upstream = requests.post(
            UPSTREAM_URL,
            json=safe_payload,
            headers=headers,
            timeout=(10, 75),
        )
    except requests.RequestException:
        app.logger.exception("Falha ao conectar ao provedor configurado.")
        return jsonify({"error": {"message": "Não foi possível conectar ao provedor de IA."}}), 502

    try:
        response_body = upstream.json()
    except ValueError:
        response_body = {"error": {"message": "O provedor retornou uma resposta inválida."}}

    if upstream.status_code >= 400:
        app.logger.warning("Provedor de IA respondeu HTTP %s.", upstream.status_code)
        return jsonify({
            "error": {
                "message": "O provedor de IA recusou a solicitação.",
                "provider_status": upstream.status_code,
            }
        }), 502

    return jsonify(response_body), 200


if __name__ == "__main__":
    app.run(host="0.0.0.0", port=int(os.getenv("PORT", "10000")))
