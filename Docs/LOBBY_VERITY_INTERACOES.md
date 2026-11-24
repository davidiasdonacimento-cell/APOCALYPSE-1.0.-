# Lobby 3D — VERITY

**Status:** primeira cena 3D procedural integrada ao projeto Android/OpenGL ES; compilação e validação no aparelho ainda pendentes.

## Fluxo confirmado
1. O jogador toca em INICIAR JOGO no menu principal.
2. Abre o lobby 3D de Verity, com apresentação curta e opção de pular.
3. Verity aparece como personagem tridimensional renderizada pelo OpenGL ES 3.0.
4. O jogador pode abrir a conversa da Verity e voltar ao lobby sem iniciar a partida.
5. A partida só começa ao tocar em COMEÇAR PARTIDA.

## Implementação 3D atual
- `Verity3DActivity.java`: Activity Android com GLSurfaceView, cena de apresentação, botão de conversa e botão de início explícito da partida.
- `verity_3d.cpp`: módulo C++ separado para renderização OpenGL ES 3.0.
- Personagem low-poly procedural com cabeça, rosto simplificado, cabelo claro, tronco, jaqueta azul, braços, pernas e botas.
- Câmera em perspectiva, profundidade e leve movimento idle.
- O módulo é compilado pela biblioteca nativa `apocalypse_engine`.
- `MainActivity` direciona INICIAR JOGO para a nova cena 3D.

## Limites desta primeira versão
- O modelo é construído por geometria procedural; ainda não é um modelo artístico externo com textura PBR.
- Ainda não há esqueleto/rig, animação de caminhada, sincronização labial ou expressões faciais.
- A movimentação do jogador pelo lobby e a detecção de proximidade real ainda não foram conectadas.
- O diálogo usa a Activity de conversa já existente; o retorno deve preservar o lobby.
- Não afirmar que está funcional no aparelho até compilar e testar.

## Próximas etapas
1. Compilar a biblioteca C++ e corrigir erros reais.
2. Abrir a cena no Android e verificar câmera, proporções e desempenho.
3. Melhorar o modelo e adicionar materiais/texturas.
4. Adicionar rig e animações idle, olhar, aceno e caminhada.
5. Implementar movimento do jogador, proximidade e menu contextual.
6. Validar retorno da conversa e início da partida no dispositivo.
