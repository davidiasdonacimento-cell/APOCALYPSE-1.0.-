# Lobby 3D — VERITY

**Status:** personagem 3D procedural com rig articulado por juntas e animações de demonstração implementados no código. Compilação e validação Android ainda pendentes.

## Fluxo
1. INICIAR JOGO abre a cena 3D da Verity.
2. Apresentação curta com opção de pular.
3. O jogador pode conversar com Verity antes da partida.
4. Os controles do lobby permitem acenar e testar a caminhada.
5. COMEÇAR PARTIDA abre a cena nativa de sobrevivência.

## Rig e personagem
- Arquivo: `app/src/main/cpp/verity_3d.cpp`.
- Esqueleto procedural articulado: raiz/pelve, coluna/peito, pescoço/cabeça, ombros, cotovelos, mãos, quadris, joelhos e tornozelos.
- As posições das juntas são avaliadas por frame e alimentam os segmentos do corpo.
- Cabeça, tronco e membros são construídos como malha low-poly procedural.
- Cabelo claro, roupa azul, rosto, olhos e detalhes faciais simplificados.
- A malha é atualizada em buffer dinâmico para permitir poses animadas.

## Animações implementadas no código
- **Idle:** oscilação corporal sutil.
- **Aceno:** braço direito levantado com movimento repetido.
- **Caminhada demonstrativa:** braços e pernas alternam em ciclo.
- **Fala:** boca abre e fecha em ciclo enquanto o modo de conversa está ativo.
- **Piscar:** escala vertical dos olhos reduz brevemente em intervalos.
- **Expressão:** detalhes faciais simplificados e boca animada; ainda não há blendshapes/articulação facial avançada.

## Controles do lobby
- MENSAGEM PARA VERITY: abre a Activity de conversa e ativa o modo de fala.
- ACENAR / EXPRESSÃO: ativa a animação de aceno.
- TESTAR ANIMAÇÃO DE CAMINHADA: executa o ciclo de caminhada no lugar.
- COMEÇAR PARTIDA: inicia a cena de jogo somente por ação explícita.

## Limites que continuam pendentes
- O rig é procedural por juntas e segmentos; não é um esqueleto importado com skinning por pesos de vértices.
- A caminhada atual é demonstração no lugar; ainda não move a personagem pelo cenário.
- Não há captura de voz/sincronização labial com fonemas reais.
- Expressões são básicas; não há sistema completo de emoções ou blendshapes.
- O modelo ainda é low-poly sem textura PBR ou arquivo artístico externo.
- Movimento do jogador, proximidade física e menu contextual no lobby ainda não foram conectados.
- Compilação, teste visual, desempenho e ciclo de vida precisam ser validados em Android real.

## Próxima validação obrigatória
1. Compilar C++ e Java com a configuração Android do projeto.
2. Corrigir todos os erros de compilação/link JNI.
3. Executar no Redmi 12 e conferir enquadramento, juntas, animações e consumo.
4. Verificar retorno da conversa e início da partida.
5. Só então marcar a implementação como validada.
