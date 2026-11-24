# APOCALYPSE — VERITY API

Gateway HTTPS entre o jogo Android e um provedor compatível com OpenAI Chat Completions.

## Render

- Runtime: Python
- Root directory: repositório (raiz)
- Build command: `pip install -r verity_api/requirements.txt`
- Start command: `gunicorn --chdir verity_api app:app --bind 0.0.0.0:$PORT`
- Health check path: `/health`

## Variáveis privadas necessárias

Configure na página Environment do serviço Render:

- `VERITY_UPSTREAM_URL`: URL completa do endpoint Chat Completions do provedor.
- `VERITY_API_KEY`: chave privada fornecida pelo provedor.
- `VERITY_MODEL`: identificador exato do modelo disponível na conta do provedor.
- `VERITY_GATEWAY_TOKEN`: token privado criado pelo proprietário e também informado no jogo.

Nunca coloque as chaves do provedor no código, APK, commits ou mensagens públicas.

## Rotas

- `GET /health`: estado do serviço e presença das configurações (não revela valores).
- `POST /v1/chat/completions`: exige `Authorization: Bearer <VERITY_GATEWAY_TOKEN>` e encaminha a conversa ao provedor configurado.

O gateway não inclui uma chave de IA nem fornece créditos de inferência. É necessário configurar um provedor e uma chave válida. O serviço responde 503 até isso ser feito.
