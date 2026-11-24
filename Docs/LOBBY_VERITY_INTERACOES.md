# Lobby 3D — VERITY

**Status:** cena 3D procedural mínima presente no código; ainda não equivale a um lobby completo. O primeiro APK foi compilado, mas a experiência visual e o funcionamento no aparelho não foram validados.

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


## Correção de orientação e avaliação do primeiro APK
- O primeiro APK de CI tinha aproximadamente 695 KB e continha apenas código Android e bibliotecas nativas; não inclui modelos 3D artísticos, texturas ou cenário de lobby detalhado.
- Foi corrigido o manifesto para abrir MainActivity, Verity3DActivity, VerityLobbyActivity, VeritySceneActivity e NativeGameActivity em orientação landscape.
- Isso corrige a orientação solicitada, mas não prova que o lobby 3D esteja funcional ou visualmente completo.
- Próximas tarefas obrigatórias: construir cenário 3D de lobby com piso, iluminação, pontos de câmera e elementos de ambiente; ajustar enquadramento da personagem em landscape; integrar o botão de conversa sem perder o estado do lobby; testar fluxo lobby -> conversa -> retorno -> partida; validar no Redmi 12.
- Não marcar como concluído até haver APK recompilado e teste real no dispositivo.


## Integração da VERITY na partida — 24/11/2026
- A malha procedural animada da VERITY agora possui uma interface C++ compartilhada e é inicializada no contexto OpenGL da partida.
- O motor chama a renderização da companheira ao lado do jogador, sem depender da Activity separada do lobby.
- A animação de caminhada é aplicada durante a renderização no mundo.
- A presença visual foi integrada no código, mas ainda exige validação em aparelho Android real; não declarar como visualmente confirmada antes do teste.
- O modelo continua procedural/low-poly. Um modelo artístico rigado e texturizado continua pendente.
